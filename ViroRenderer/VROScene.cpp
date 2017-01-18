//
//  VROScene.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/19/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROScene.h"
#include "VRORenderContext.h"
#include "VRONode.h"
#include "VROGeometry.h"
#include "VROSkybox.h"
#include "VROLight.h"
#include "VROHitTestResult.h"
#include "VROSphere.h"
#include "VROMaterial.h"
#include "VROLog.h"
#include "VROAudioPlayer.h"
#include <stack>
#include <algorithm>

static const float kSphereBackgroundRadius = 1;
static const float kSphereBackgroundNumSegments = 20;

VROScene::VROScene() {
    ALLOCATION_TRACKER_ADD(Scenes, 1);
}

VROScene::~VROScene() {
    ALLOCATION_TRACKER_SUB(Scenes, 1);
}

void VROScene::renderBackground(const VRORenderContext &renderContext,
                                VRODriver &driver) {
    if (!_background) {
        return;
    }
    
    std::shared_ptr<VROMaterial> &material = _background->getMaterials()[0];
    material->bindShader(driver);

    VROMatrix4f transform;
    transform = _backgroundRotation.getMatrix().multiply(transform);

    _background->render(0, material, transform, {}, 1.0, renderContext, driver);
}

void VROScene::render(const VRORenderContext &context,
                      VRODriver &driver) {
    
    uint32_t boundShaderId = UINT32_MAX;
    std::vector<std::shared_ptr<VROLight>> boundLights;
    
    for (VROSortKey &key : _keys) {
        VRONode *node = (VRONode *)key.node;
        int elementIndex = key.elementIndex;
        
        const std::shared_ptr<VROGeometry> &geometry = node->getGeometry();
        if (geometry) {
            std::shared_ptr<VROMaterial> material = geometry->getMaterialForElement(elementIndex);
            if (!key.incoming) {
                material = material->getOutgoing();
            }
            
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
            
            // Only render the material if there are lights, or if the material uses
            // constant lighting. Non-constant materials do not render unless we have
            // at least one light.
            if (!boundLights.empty() || material->getLightingModel() == VROLightingModel::Constant) {
                node->render(elementIndex, material, context, driver);
            }
        }
    }
}

void VROScene::updateSortKeys(const VRORenderContext &context, VRODriver &driver) {
    VROMatrix4f identity;

    VRORenderParameters renderParams;
    renderParams.transforms.push(identity);
    renderParams.opacities.push(1.0);
    
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->updateSortKeys(renderParams, context, driver);
    }
    
    _keys.clear();
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->getSortKeys(&_keys);
    }
    
    std::sort(_keys.begin(), _keys.end());
}

void VROScene::addNode(std::shared_ptr<VRONode> node) {
    _nodes.push_back(node);
}

void VROScene::setBackgroundCube(std::shared_ptr<VROTexture> textureCube) {
    _background = VROSkybox::createSkybox(textureCube);
    _background->setName("Background");
}

void VROScene::setBackgroundCube(VROVector4f color) {
    _background = VROSkybox::createSkybox(color);
    _background->setName("Background");
}

void VROScene::setBackgroundSphere(std::shared_ptr<VROTexture> textureSphere) {
    _background = VROSphere::createSphere(kSphereBackgroundRadius,
                                          kSphereBackgroundNumSegments,
                                          kSphereBackgroundNumSegments,
                                          false);
    _background->setCameraEnclosure(true);
    _background->setName("Background");

    std::shared_ptr<VROMaterial> material = _background->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Constant);
    material->getDiffuse().setTexture(textureSphere);
    material->setWritesToDepthBuffer(false);
    material->setReadsFromDepthBuffer(false);
}

void VROScene::setBackgroundRotation(VROQuaternion rotation) {
    _backgroundRotation = rotation;
}
