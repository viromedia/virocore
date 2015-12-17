//
//  VROLayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROLayer.h"
#include "VROMath.h"
#include "VROSurface.h"
#include "VROTexture.h"
#include "VROMaterial.h"

#pragma mark - Initialization

VROLayer::VROLayer(const VRORenderContext &context) :
    VRONode(context) {
    setBackgroundColor({ 1.0, 1.0, 1.0, 1.0 });
    
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(1.0, 1.0);
    surface->getMaterials()[0]->setLightingModel(VROLightingModel::Blinn);
        
    setGeometry(surface);
}

VROLayer::~VROLayer() {

}

#pragma mark - Layer Properties

void VROLayer::setContents(UIImage *image) {
    const std::shared_ptr<VROGeometry> &geometry = getGeometry();
    const std::vector<std::shared_ptr<VROMaterial>> &materials = geometry->getMaterials_const();
    
    materials.front()->getDiffuse().setContents(std::make_shared<VROTexture>(image));
}

void VROLayer::setBackgroundColor(vector_float4 backgroundColor) {
    _backgroundColor = backgroundColor;
}

vector_float4 VROLayer::getBackgroundColor() const {
    return _backgroundColor;
}

#pragma mark - Spatial Position

void VROLayer::setFrame(VRORect frame) {
    _frame = frame;
    onFrameUpdate();
}

void VROLayer::setBounds(VRORect bounds) {
    _frame.size = bounds.size;
    onFrameUpdate();
}

void VROLayer::setPosition(VROPoint point) {
    _frame.origin.x = point.x - _frame.size.width  / 2.0f;
    _frame.origin.y = point.y - _frame.size.height / 2.0f;
    _frame.origin.z = point.z;
    
    onFrameUpdate();
}

void VROLayer::onFrameUpdate() {
    VROPoint pt = getPosition();
    
    VRONode::setPosition({ pt.x, pt.y, pt.z });
    VRONode::setScale( { _frame.size.width, _frame.size.height, 1.0 });
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

