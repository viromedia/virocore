//
//  VROLayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROLayer.h"
#include "VROPresentationLayer.h"
#include "VROMath.h"

#pragma mark - Initialization

VROLayer::VROLayer() {
    _presentationLayer = std::make_shared<VROPresentationLayer>(this);
}

VROLayer::VROLayer(VROLayer *layer) {
    
}

VROLayer::~VROLayer() {

}

#pragma mark - Rendering

void VROLayer::hydrate(const VRORenderContext &context) {
    _presentationLayer->hydrate(context);
}

void VROLayer::render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack) {
    _presentationLayer->render(context, mvStack);
}

#pragma mark - Layer Properties

void VROLayer::setContents(const void *data, size_t dataLength, int width, int height) {
    _presentationLayer->setContents(data, dataLength, width, height);
}

void VROLayer::setBackgroundColor(vector_float4 backgroundColor) {
    _backgroundColor = backgroundColor;
    
    if (_presentationLayer) {
        _presentationLayer->setBackgroundColor(backgroundColor);
    }
}

vector_float4 VROLayer::getBackgroundColor() const {
    return _backgroundColor;
}

#pragma mark - Spatial Position

void VROLayer::setFrame(VRORect frame) {
    _frame = frame;
    
    if (_presentationLayer) {
        _presentationLayer->setFrame(frame);
    }
}

void VROLayer::setBounds(VRORect bounds) {
    _frame.size = bounds.size;
    
    if (_presentationLayer) {
        _presentationLayer->setBounds(bounds);
    }
}

void VROLayer::setPosition(VROPoint point) {
    _frame.origin.x = point.x - _frame.size.width  / 2.0f;
    _frame.origin.y = point.y - _frame.size.height / 2.0f;
    _frame.origin.z = point.z;
    
    if (_presentationLayer) {
        _presentationLayer->setPosition(point);
    }
}

VRORect VROLayer::getFrame() const {
    return _frame;
}

VRORect VROLayer::getBounds() const {
    return {{0, 0}, {_frame.size.width, _frame.size.height}};
}

VROPoint VROLayer::getPosition() const {
    return {_frame.origin.x + _frame.size.width  / 2.0f,
            _frame.origin.y + _frame.size.height / 2.0f,
            _frame.origin.z };
}

#pragma mark - Layer Tree

void VROLayer::addSublayer(std::shared_ptr<VROLayer> layer) {
    _sublayers.push_back(layer);
    layer->_superlayer = shared_from_this();
    
    if (_presentationLayer) {
        _presentationLayer->addSublayer(layer->_presentationLayer);
    }
}

void VROLayer::removeFromSuperlayer() {
    std::vector<std::shared_ptr<VROLayer>> parentSublayers = _superlayer->_sublayers;
    parentSublayers.erase(
                          std::remove_if(parentSublayers.begin(), parentSublayers.end(),
                                         [this](std::shared_ptr<VROLayer> layer) {
                                             return layer.get() == this;
                                         }), parentSublayers.end());
    _superlayer.reset();
    
    if (_presentationLayer) {
        _presentationLayer->removeFromSuperlayer();
    }
}

