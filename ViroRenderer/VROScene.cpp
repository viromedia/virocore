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
#include <stack>

void VROScene::render(const VRORenderContext &renderContext) {
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)renderContext;
    
    std::stack<VROMatrix4f> mvStack;
    mvStack.push(metal.getViewMatrix());
    
    for (std::shared_ptr<VROLayer> &node : _nodes) {
        node->render(renderContext, mvStack);
    }
}

void VROScene::addNode(std::shared_ptr<VROLayer> node) {
    _nodes.push_back(node);
}

