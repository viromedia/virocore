//
//  VROPortal.cpp
//  ViroKit
//
//  Created by Raj Advani on 7/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROPortal.h"
#include "VROLog.h"
#include "VROGeometry.h"
#include "VROMaterial.h"
#include "VROSkybox.h"
#include "VROSphere.h"
#include "VROBoundingBox.h"
#include "VROPortalFrame.h"
#include "VROShaderModifier.h"

// Parameters for sphere backgrounds
static const float kSphereBackgroundRadius = 1;
static const float kSphereBackgroundNumSegments = 60;

VROPortal::VROPortal() :
    VRONode(),
    _passable(false) {
    _type = VRONodeType::Portal;
}

VROPortal::~VROPortal() {
    
}

void VROPortal::deleteGL() {
    if (_background) {
        _background->deleteGL();
    }
    VRONode::deleteGL();
}

#pragma mark - Scene Preparation

void VROPortal::traversePortals(int frame, int recursionLevel,
                                std::shared_ptr<VROPortalFrame> activeFrame,
                                tree<std::shared_ptr<VROPortal>> *outPortals) {
    passert (_type == VRONodeType::Portal);
    passert (_lastVisitedRenderingFrame < frame);
    
    _lastVisitedRenderingFrame = frame;
    _recursionLevel = recursionLevel;
    _activePortalFrame = activeFrame;
    
    outPortals->value = std::dynamic_pointer_cast<VROPortal>(shared_from_this());
    
    // Search down the scene graph
    std::vector<std::shared_ptr<VROPortal>> childPortals;
    getChildPortals(&childPortals);
    for (std::shared_ptr<VROPortal> &childPortal : childPortals) {
        if (childPortal->_lastVisitedRenderingFrame < frame) {
            outPortals->children.push_back({});
            tree<std::shared_ptr<VROPortal>> *node = &outPortals->children.back();
            
            // When moving down the tree, we assign the child's own frame
            // as its frame to render.
            childPortal->traversePortals(frame, recursionLevel + 1,
                                         childPortal->getPortalEntrance(), node);
        }
    }
    
    // Search up the scene graph
    const std::shared_ptr<VROPortal> parentPortal = getParentPortal();
    if (parentPortal) {
         if (parentPortal->_lastVisitedRenderingFrame < frame) {
             outPortals->children.push_back({});
             tree<std::shared_ptr<VROPortal>> *node = &outPortals->children.back();
             
             // When moving up the tree, we assign *this* portal's entrance as the
             // portal above's frame to render. That way, the parent will render
             // this portal's entrance, which will make it appear like an exit from
             // this portal into the parent.
             parentPortal->traversePortals(frame, recursionLevel + 1, _portalEntrance, node);
         }
    }
}

void VROPortal::sortNodesBySortKeys() {
    _keys.clear();
    getSortKeysForVisibleNodes(&_keys);
    
    std::sort(_keys.begin(), _keys.end());
}

#pragma mark - Rendering Contents

void VROPortal::renderBackground(const VRORenderContext &context,
                                 std::shared_ptr<VRODriver> &driver) {
    if (_background) {
        const std::shared_ptr<VROMaterial> &material = _background->getMaterials()[0];
        if (material->bindShader(0, {}, context, driver)) {
            material->bindProperties(driver);
            
            VROMatrix4f transform;
            transform = _backgroundTransform.multiply(transform);
            _background->render(0, material, transform, {}, 1.0, context, driver);
        }
    }
}

void VROPortal::renderContents(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    uint32_t boundMaterialId = UINT32_MAX;
    uint32_t boundHierarchyId = kMaxHierarchyId; // kMaxHierarchyId == Not a hierarchy
    VROSortKey *boundHierarchyParent = nullptr;
    std::vector<std::shared_ptr<VROLight>> boundLights;
    
    if (kDebugSortOrder && context.getFrame() % kDebugSortOrderFrameFrequency == 0) {
        pinfo("Rendering");
    }
    
    // Note that since portals and portal frames are not returned in _keys,
    // they will not be rendered here
    for (VROSortKey &key : _keys) {
        VRONode *node = (VRONode *)key.node;
        int elementIndex = key.elementIndex;
        
        const std::shared_ptr<VROGeometry> &geometry = node->getGeometry();
        if (!geometry) {
            continue;
        }
        
        std::shared_ptr<VROMaterial> material = geometry->getMaterialForElement(elementIndex);
        if (!key.incoming) {
            material = material->getOutgoing();
        }
        
        // Rebind if materials or lights changed. We always have to rebind material
        // properties even if only the lights changed, because new lights imply
        // a potential change of shader -- and we have to upload our material's uniforms
        // to any new shader.
        if (key.material != boundMaterialId || boundLights != node->getComputedLights()) {

            // If we're rendering a hierarchical object -- meaning, an object that's part of a close-knit
            // 2D unit like a flex-view -- then the entire hierarchy of these 2D objects will appear
            // consecutively in the sort order. In order to ensure there is no Z-fighting between the
            // objects in the hierarchy (as they share the same 2D plane), we do not render the any object
            // in the hierarchy to the depth buffer. Then, when we're done rendering the entire hierarchy
            // we go back and re-render the parent of the hierarchy to the depth buffer, so that the
            // hierarchy as a whole plays well in the depth buffer with other 3D objects.

            // When the active hierarchy changes (to a new hierarchy, or to none)
            if (key.hierarchyId != boundHierarchyId) {
                // Finish the last hierarchy by writing the parent to the depth buffer
                if (boundHierarchyId < kMaxHierarchyId) {
                    passert (boundHierarchyParent != nullptr);
                    writeHierarchyParentToDepthBuffer(*boundHierarchyParent, context, driver);
                    boundHierarchyId = kMaxHierarchyId;
                }

                // Set the parent of the new hierarchy
                if (key.hierarchyId < kMaxHierarchyId) {
                    boundHierarchyParent = &key;
                } else {
                    boundHierarchyParent = nullptr;
                }
            }

            // TODO Perhaps we can check if the shader changed, and if so bind
            //      properties? We could also meld these two methods into one, simplifying
            //      the API?
            if (!material->bindShader(key.lights, node->getComputedLights(), context, driver)) {
                pinfo("Failed to bind shader: will not render associated geometry");
                continue;
            }
            material->bindProperties(driver);

            // When rendering a hierarchy, ensure nothing is written to the depth buffer
            if (key.hierarchyId < kMaxHierarchyId) {
                driver->setDepthWritingEnabled(false);
                boundHierarchyId = key.hierarchyId;
            }

            boundMaterialId = key.material;
            boundLights = node->getComputedLights();
        }
        
        // We render the material if at least one of the following is true:
        //
        // 1. There are lights in the scene that haven't been culled (if there are no lights, then
        //    nothing will be visible! Or,
        // 2. The material is Constant. Constant materials do not need light to be visible. Or,
        // 3. The material is PBR, and we have an active lighting environment. Lighting environments
        //    provide ambient light for PBR materials
        if (!boundLights.empty() ||
            material->getLightingModel() == VROLightingModel::Constant ||
            (material->getLightingModel() == VROLightingModel::PhysicallyBased && context.getIrradianceMap() != nullptr)) {

            if (kDebugSortOrder && context.getFrame() % kDebugSortOrderFrameFrequency == 0) {
                if (node->getGeometry() && elementIndex == 0) {
                    pinfo("   Rendering node [%s], element %d [transparent %d, distance from far plane %f, hierarchy [%d-%d]",
                          node->getName().c_str(), elementIndex, key.transparent, key.distanceFromCamera, key.hierarchyId, key.hierarchyDepth);
                }
            }

            node->render(elementIndex, material, context, driver);
        }
    }
}

void VROPortal::writeHierarchyParentToDepthBuffer(VROSortKey &hierarchyParent,
                                                  const VRORenderContext &context,
                                                  std::shared_ptr<VRODriver> &driver) {
    VRONode *hParentNode = (VRONode *)hierarchyParent.node;
    if (!hParentNode->getGeometry()) {
        return;
    }

    std::shared_ptr<VROMaterial> hParentMaterial = hParentNode->getGeometry()->getMaterialForElement(hierarchyParent.elementIndex);
    if (!hParentMaterial->bindShader(hierarchyParent.lights, hParentNode->getComputedLights(), context, driver)) {
        pinfo("Failed to bind shader: will not render associated geometry");
        return;
    }
    hParentMaterial->bindProperties(driver);

    driver->setDepthWritingEnabled(true);
    driver->setDepthReadingEnabled(true);
    driver->setRenderTargetColorWritingMask(VROColorMaskNone);
    hParentNode->render(hierarchyParent.elementIndex, hParentMaterial, context, driver);
    driver->setRenderTargetColorWritingMask(VROColorMaskAll);
}

#pragma mark - Portal Entrance

void VROPortal::setPortalEntrance(std::shared_ptr<VROPortalFrame> entrance) {
    if (_portalEntrance) {
        _portalEntrance->removeFromParentNode();
    }
    _portalEntrance = entrance;
    addChildNode(_portalEntrance);
}

void VROPortal::renderPortalSilhouette(std::shared_ptr<VROMaterial> &material,
                                       VROSilhouetteMode mode, std::function<bool(const VRONode &)> filter,
                                       const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (_activePortalFrame) {
        _activePortalFrame->renderSilhouettes(material, mode, filter, context, driver);
    }
}

void VROPortal::renderPortal(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (_activePortalFrame) {
        deactivateCulling(_activePortalFrame);
        _activePortalFrame->render(context, driver);
    }
}

void VROPortal::deactivateCulling(std::shared_ptr<VRONode> node) {
    if (node->getGeometry()) {
        for (const std::shared_ptr<VROMaterial> &material : node->getGeometry()->getMaterials()) {
            material->setCullMode(VROCullMode::None);
        }
    }
    for (std::shared_ptr<VRONode> &child : node->getChildNodes()) {
        deactivateCulling(child);
    }
}

#pragma mark - Lighting Environment

void VROPortal::setLightingEnvironment(std::shared_ptr<VROTexture> texture) {
    _lightingEnvironment = texture;
}

std::shared_ptr<VROTexture> VROPortal::getLightingEnvironment() const {
    return _lightingEnvironment;
}

#pragma mark - Backgrounds

static thread_local std::shared_ptr<VROShaderModifier> sBackgroundShaderModifier;

void VROPortal::installBackgroundShaderModifier() {
    /*
     Modifier that pushes backgrounds to the back of the depth buffer.
     */
    if (!sBackgroundShaderModifier) {
        std::vector<std::string> modifierCode =  { "_vertex.position = _vertex.position.xyww;"};
        sBackgroundShaderModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Vertex,
                                                                        modifierCode);
        sBackgroundShaderModifier->setName("background");
    }
    _background->getMaterials().front()->addShaderModifier(sBackgroundShaderModifier);
}

void VROPortal::setBackgroundCube(std::shared_ptr<VROTexture> textureCube) {
    passert_thread(__func__);
    _background = VROSkybox::createSkybox(textureCube);
    _background->setName("Background");
    
    installBackgroundShaderModifier();
}

void VROPortal::setBackgroundCube(VROVector4f color) {
    passert_thread(__func__);
    _background = VROSkybox::createSkybox(color);
    _background->setName("Background");
    
    installBackgroundShaderModifier();
}

void VROPortal::setBackgroundSphere(std::shared_ptr<VROTexture> textureSphere) {
    passert_thread(__func__);
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
    material->setNeedsToneMapping(false);
    
    installBackgroundShaderModifier();
}

void VROPortal::setBackground(std::shared_ptr<VROGeometry> background) {
    passert_thread(__func__);
    _background = background;
    
    installBackgroundShaderModifier();
}

void VROPortal::setBackgroundTransform(VROMatrix4f transform) {
    _backgroundTransform = transform;
}

void VROPortal::setBackgroundRotation(VROQuaternion rotation) {
    passert_thread(__func__);
    _backgroundTransform = rotation.getMatrix();
}

void VROPortal::removeBackground() {
    passert_thread(__func__);
    _background->getMaterials().front()->removeShaderModifier(sBackgroundShaderModifier);
    _background.reset();
}

#pragma mark - Intersection

bool VROPortal::intersectsLineSegment(VROLineSegment segment) const {
    if (!_activePortalFrame) {
        return false;
    }
    return _activePortalFrame->intersectsLineSegment(segment);
}
