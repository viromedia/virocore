//
//  VROPresentationLayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROPresentationLayer.h"
#include "VROLayerSubstrateMetal.h"

VROPresentationLayer::VROPresentationLayer(const VROLayer *layer) :
    VROLayer(this),
    _substrate(nullptr) {
    
}

VROPresentationLayer::~VROPresentationLayer() {
    delete (_substrate);
}

void VROPresentationLayer::setContents(const void *data, size_t dataLength, int width, int height) {
    _substrate->setContents(data, dataLength, width, height);
}

void VROPresentationLayer::hydrate(const VRORenderContext &context) {
    // TODO assert not hydrating twice!
    
    _substrate = new VROLayerSubstrateMetal(shared_from_this());
    _substrate->hydrate(context);
}

void VROPresentationLayer::render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack) {
    _substrate->render(context, mvStack);
    
    /*
     Now render the children. The children are all transformed to the parent's origin (its top
     left corner).
     */
    mvStack.push(matrix_multiply(mvStack.top(), _substrate->getChildTransform()));
    
    for (std::shared_ptr<VROLayer> childLayer : _sublayers) {
        childLayer->render(context, mvStack);
    }
    mvStack.pop();
}
