//
//  VROScene.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/19/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROScene.h"
#include "VROLayer.h"
#include "VRORenderContext.h"
#include "VRONode.h"
#include "VROGeometry.h"
#include "VROSkybox.h"
#include "VROLight.h"
#include "VROHitTestResult.h"
#include "VROSphere.h"
#include "VROHoverController.h"
#include "VROLog.h"
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
    
    // Background will not render without any light
    std::vector<std::shared_ptr<VROLight>> &lights = _nodes[0]->getLights();
    if (lights.empty()) {
        return;
    }
    
    std::shared_ptr<VROLight> &light = lights.front();
    
    std::shared_ptr<VROMaterial> &material = _background->getMaterials()[0];
    material->bindShader(driver);
    material->bindLights({ light }, renderContext, driver);
    
    _background->render(0, material, {}, 1.0, renderContext, driver);
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
            if (key.outgoing) {
                material = material->getOutgoing();
            }
            
            if (key.shader != boundShaderId) {
                material->bindShader(driver);
                boundShaderId = key.shader;
            }
            
            if (boundLights != node->getComputedLights()) {
                material->bindLights(node->getComputedLights(), context, driver);
                boundLights = node->getComputedLights();
            }
            
            node->render(elementIndex, material, context, driver);
        }
    }
}

void VROScene::updateSortKeys(const VRORenderContext &context) {
    VROMatrix4f identity;

    VRORenderParameters renderParams;
    renderParams.transforms.push(identity);
    renderParams.opacities.push(1.0);
    
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->updateSortKeys(renderParams, context);
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
}

void VROScene::setBackgroundCube(VROVector4f color) {
    _background = VROSkybox::createSkybox(color);
}

void VROScene::setBackgroundSphere(std::shared_ptr<VROTexture> textureSphere) {
    _background = VROSphere::createSphere(kSphereBackgroundRadius,
                                          kSphereBackgroundNumSegments,
                                          kSphereBackgroundNumSegments,
                                          false);
    _background->setCameraEnclosure(true);
    
    std::shared_ptr<VROMaterial> material = _background->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Constant);
    material->getDiffuse().setContents(textureSphere);
    material->setWritesToDepthBuffer(false);
    material->setReadsFromDepthBuffer(false);
}

std::vector<VROHitTestResult> VROScene::hitTest(VROVector3f ray, const VRORenderContext &context,
                                                bool boundsOnly) {
    std::vector<VROHitTestResult> results;
    
    for (std::shared_ptr<VRONode> &node : _nodes) {
        std::vector<VROHitTestResult> nodeResults = node->hitTest(ray, context, boundsOnly);
        results.insert(results.end(), nodeResults.begin(), nodeResults.end());
    }
    
    std::sort(results.begin(), results.end(), [](VROHitTestResult a, VROHitTestResult b) {
        return a.getDistance() < b.getDistance();
    });

    return results;
}
