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
#include <stack>

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
    
    VROMatrix4f identity;
    
    VRORenderParameters renderParams;
    renderParams.rotations.push(identity);
    renderParams.transforms.push(identity);
    
    //TODO Make the skybox track the camera position
    _background->render(context, renderParams);
}

void VROScene::render(const VRORenderContext &context) {
    VROMatrix4f identity;

    VRORenderParameters renderParams;
    renderParams.rotations.push(identity);
    renderParams.transforms.push(identity);
    
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->render(context, renderParams);
    }
}

void VROScene::addNode(std::shared_ptr<VRONode> node) {
    _nodes.push_back(node);
}

void VROScene::setBackground(std::shared_ptr<VROTexture> textureCube) {
    _background = VROSkybox::createSkybox(textureCube);
}

