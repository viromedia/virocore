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
#include <stack>

void VROScene::render(const VRORenderContext &renderContext) {
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)renderContext;
    
    std::stack<matrix_float4x4> mvStack;
    mvStack.push(metal.getViewMatrix());
    
    for (std::shared_ptr<VROLayer> &layer : _layers) {
        layer->render(renderContext, mvStack);
    }
}

void VROScene::addLayer(std::shared_ptr<VROLayer> layer) {
    _layers.push_back(layer);
}

