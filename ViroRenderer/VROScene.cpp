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

void VROScene::render(const VRORenderContext &renderContext) {
    VROMatrix4f identity;

    VRORenderParameters renderParams;
    renderParams.rotations.push(identity);
    renderParams.transforms.push(identity);
    
    if (_background) {
        //TODO Make the skybox track the camera position
        _background->render(renderContext, renderParams);
    }
    
    for (std::shared_ptr<VRONode> &node : _nodes) {
        node->render(renderContext, renderParams);
    }
}

void VROScene::addNode(std::shared_ptr<VRONode> node) {
    _nodes.push_back(node);
}

void VROScene::setBackground(std::shared_ptr<VROTexture> textureCube) {
    _background = VROSkybox::createSkybox(textureCube);
}

