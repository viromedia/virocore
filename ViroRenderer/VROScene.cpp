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
#include "VRORenderContextMetal.h"
#include "VRONode.h"
#include "VROGeometry.h"
#include "VROSkybox.h"
#include "VROLight.h"
#include "VROHitTestResult.h"
#include "VROSphere.h"
#include "VROHoverController.h"
#include <stack>

static const float kSphereBackgroundRadius = 1;
static const float kSphereBackgroundNumSegments = 20;

VROScene::VROScene() {
    ALLOCATION_TRACKER_ADD(Scenes, 1);
}

VROScene::~VROScene() {
    ALLOCATION_TRACKER_SUB(Scenes, 1);
}

void VROScene::renderBackground(const VRORenderContext &context) {
    if (!_background) {
        return;
    }
    
    VROMatrix4f translation;
    translation.translate(context.getCamera().getPosition());
    
    VRORenderParameters renderParams;
    renderParams.transforms.push(translation);
    renderParams.opacities.push(1.0);
    
    _background->render(context, renderParams);
}

void VROScene::render(const VRORenderContext &context) {
    VROMatrix4f identity;

    VRORenderParameters renderParams;
    renderParams.transforms.push(identity);
    renderParams.opacities.push(1.0);
    
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->render(context, renderParams);
    }
}

void VROScene::addNode(std::shared_ptr<VRONode> node) {
    _nodes.push_back(node);
}

void VROScene::setBackgroundCube(std::shared_ptr<VROTexture> textureCube) {
    _background = VROSkybox::createSkybox(textureCube);
}

void VROScene::setBackgroundSphere(std::shared_ptr<VROTexture> textureSphere) {
    _background = VROSphere::createSphere(kSphereBackgroundRadius,
                                          kSphereBackgroundNumSegments,
                                          kSphereBackgroundNumSegments,
                                          false);
    _background->setStereoRenderingEnabled(false);
    
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
    
    return results;
}
