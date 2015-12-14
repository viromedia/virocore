//
//  VRONode.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRONode.h"
#include "VROGeometry.h"
#include "VROLight.h"

VRONode::VRONode(const VRORenderContext &context) :
    _scale({1.0, 1.0, 1.0}) {
    
}

VRONode::~VRONode() {
    
}

void VRONode::render(const VRORenderContext  &context,
                     std::stack<VROMatrix4f> &rotations,
                     std::stack<VROMatrix4f> &xforms,
                     std::vector<std::shared_ptr<VROLight>> &lights) {
    
    VROMatrix4f rotation = rotations.top().multiply(_rotation.getMatrix());
    VROMatrix4f transform = xforms.top().multiply(getTransform());
    
    if (_light) {
        _light->setTransformedPosition(transform.multiply(_light->getPosition()));
        lights.push_back(_light);
    }

    if (_geometry) {
        _geometry->render(context, rotation, transform, lights);
    }
    
    rotations.push(rotation);
    xforms.push(transform);
    
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->render(context, rotations, xforms, lights);
    }
    
    xforms.pop();
    rotations.pop();
    
    if (_light) {
        lights.pop_back();
    }
}

VROMatrix4f VRONode::getTransform() const {
    VROMatrix4f transform = _rotation.getMatrix();
    transform.scale(_scale.x, _scale.y, _scale.z);
    transform.translate(_position.x, _position.y, _position.z);
    
    return transform;
}

