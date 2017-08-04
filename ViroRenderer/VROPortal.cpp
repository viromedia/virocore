//
//  VROPortal.cpp
//  ViroKit
//
//  Created by Raj Advani on 7/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPortal.h"
#include "VROLog.h"
#include "VROGeometry.h"
#include "VROMaterial.h"
#include "VROSkybox.h"
#include "VROSphere.h"
#include "VROBoundingBox.h"
#include "VROShaderModifier.h"

// Parameters for sphere backgrounds
static const float kSphereBackgroundRadius = 1;
static const float kSphereBackgroundNumSegments = 60;

VROPortal::VROPortal() :
    VRONode(),
    _passable(false),
    _twoSided(false) {
    _isPortal = true;
}

VROPortal::~VROPortal() {
    
}

#pragma mark - Scene Preparation

void VROPortal::traversePortals(int frame, int recursionLevel,
                                std::shared_ptr<VROPortal> portalToRender,
                                tree<std::shared_ptr<VROPortal>> *outPortals) {
    passert (isPortal());
    passert (_lastVisitedRenderingFrame < frame);
    
    _lastVisitedRenderingFrame = frame;
    _recursionLevel = recursionLevel;
    
    // Assign the geometry that will be used to render this portal
    // We use our own geometry when rendering this portal as an entrance
    // (e.g. down the tree) and the parent's geometry when rendering as
    // an exit (e.g. up the tree).
    _portalToRender = portalToRender;
    
    outPortals->value = std::dynamic_pointer_cast<VROPortal>(shared_from_this());
    
    // Search down the scene graph
    std::vector<std::shared_ptr<VROPortal>> childPortals;
    getChildPortals(&childPortals);
    for (std::shared_ptr<VROPortal> &childPortal : childPortals) {
        if (childPortal->_lastVisitedRenderingFrame < frame) {
            outPortals->children.push_back({});
            tree<std::shared_ptr<VROPortal>> *node = &outPortals->children.back();
            
            childPortal->traversePortals(frame, recursionLevel + 1, childPortal, node);
        }
    }
    
    // Search up the scene graph
    const std::shared_ptr<VROPortal> parentPortal = getParentPortal();
    if (parentPortal) {
         if (parentPortal->_lastVisitedRenderingFrame < frame) {
             outPortals->children.push_back({});
             tree<std::shared_ptr<VROPortal>> *node = &outPortals->children.back();
             parentPortal->traversePortals(frame, recursionLevel + 1,
                                           std::dynamic_pointer_cast<VROPortal>(shared_from_this()), node);
         }
    }
}

void VROPortal::sortNodesBySortKeys() {
    _keys.clear();
    getSortKeysForVisibleNodes(&_keys);
    
    std::sort(_keys.begin(), _keys.end());
}

#pragma mark - Rendering

void VROPortal::renderPortalSilhouette(std::shared_ptr<VROMaterial> &material,
                              const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (_portalToRender && _portalToRender->getGeometry()) {
        _portalToRender->getGeometry()->renderSilhouette(_portalToRender->getComputedTransform(), material, context, driver);
    }
}

void VROPortal::renderPortal(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (_portalToRender && _portalToRender->getGeometry()) {
        for (int i = 0; i < _portalToRender->getGeometry()->getGeometryElements().size(); i++) {
            std::shared_ptr<VROMaterial> &material = _portalToRender->getGeometry()->getMaterialForElement(i);
            material->bindShader(driver);
            material->bindProperties(driver);
            material->bindLights(getComputedLightsHash(), getComputedLights(), context, driver);

            _portalToRender->render(i, material, context, driver);
        }
    }
}

void VROPortal::renderBackground(const VRORenderContext &renderContext,
                                 std::shared_ptr<VRODriver> &driver) {
    if (_background) {
        const std::shared_ptr<VROMaterial> &material = _background->getMaterials()[0];
        material->bindShader(driver);
        material->bindProperties(driver);
        
        VROMatrix4f transform;
        transform = _backgroundTransform.multiply(transform);
        
        _background->render(0, material, transform, {}, 1.0, renderContext, driver);
    }
}

void VROPortal::renderContents(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    uint32_t boundShaderId = UINT32_MAX;
    uint32_t boundMaterialId = UINT32_MAX;
    std::vector<std::shared_ptr<VROLight>> boundLights;
    
    if (kDebugSortOrder) {
        pinfo("Rendering");
    }
    
    for (VROSortKey &key : _keys) {
        VRONode *node = (VRONode *)key.node;
        int elementIndex = key.elementIndex;
        
        const std::shared_ptr<VROGeometry> &geometry = node->getGeometry();
        if (geometry) {
            std::shared_ptr<VROMaterial> material = geometry->getMaterialForElement(elementIndex);
            if (!key.incoming) {
                material = material->getOutgoing();
            }
            
            // Bind the new shader if it changed
            if (key.shader != boundShaderId) {
                material->bindShader(driver);
                boundShaderId = key.shader;
                
                // If the shader changes, we have to rebind the lights so they attach
                // to the new shader
                material->bindLights(key.lights, node->getComputedLights(), context, driver);
                boundLights = node->getComputedLights();
            }
            else {
                // Otherwise we only rebind lights if the lights themselves have changed
                if (boundLights != node->getComputedLights()) {
                    material->bindLights(key.lights, node->getComputedLights(), context, driver);
                    boundLights = node->getComputedLights();
                }
            }
            
            // Bind material properties if they changed
            if (key.material != boundMaterialId) {
                material->bindProperties(driver);
                boundMaterialId = key.material;
            }
            
            // Only render the material if there are lights, or if the material uses
            // constant lighting. Non-constant materials do not render unless we have
            // at least one light.
            if (!boundLights.empty() || material->getLightingModel() == VROLightingModel::Constant) {
                if (kDebugSortOrder) {
                    if (node->getGeometry() && elementIndex == 0) {
                        pinfo("   Rendering node [%s], element %d", node->getGeometry()->getName().c_str(), elementIndex);
                    }
                }
                node->render(elementIndex, material, context, driver);
            }
        }
    }
}

#pragma mark - Backgrounds

void VROPortal::setBackgroundCube(std::shared_ptr<VROTexture> textureCube) {
    passert_thread();
    _background = VROSkybox::createSkybox(textureCube);
    _background->setName("Background");
    
    installBackgroundModifier();
}

void VROPortal::setBackgroundCube(VROVector4f color) {
    passert_thread();
    _background = VROSkybox::createSkybox(color);
    _background->setName("Background");
    
    installBackgroundModifier();
}

void VROPortal::setBackgroundSphere(std::shared_ptr<VROTexture> textureSphere) {
    passert_thread();
    _background = VROSphere::createSphere(kSphereBackgroundRadius,
                                          kSphereBackgroundNumSegments,
                                          kSphereBackgroundNumSegments,
                                          false);
    _background->setCameraEnclosure(true);
    _background->setName("Background");
    
    std::shared_ptr<VROMaterial> material = _background->getMaterials().front();
    material->setLightingModel(VROLightingModel::Constant);
    material->getDiffuse().setTexture(textureSphere);
    material->setWritesToDepthBuffer(false);
    
    installBackgroundModifier();
}

void VROPortal::setBackground(std::shared_ptr<VROGeometry> background) {
    passert_thread();
    _background = background;
    
    installBackgroundModifier();
}

void VROPortal::setBackgroundTransform(VROMatrix4f transform) {
    _backgroundTransform = transform;
}

void VROPortal::setBackgroundRotation(VROQuaternion rotation) {
    passert_thread();
    _backgroundTransform = rotation.getMatrix();
}

void VROPortal::installBackgroundModifier() {
    std::vector<std::string> modifierCode =  { "_vertex.position = _vertex.position.xyww;"};
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Vertex,
                                                                                      modifierCode);
    _background->getMaterials().front()->addShaderModifier(modifier);
}

#pragma mark - Intersection

bool VROPortal::intersectsLineSegment(VROLineSegment segment) const {
    if (_portalToRender && _portalToRender->getGeometry()) {
        // VIRO-1400 TODO: make the intersection test work
        return false;
    }
    else {
        return false;
    }
}
