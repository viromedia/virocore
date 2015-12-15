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

VRONode::VRONode(const VRONode &node) :
    _geometry(node._geometry),
    _light(node._light),
    _scale(node._scale),
    _position(node._position),
    _rotation(node._rotation) {
        
}

VRONode::~VRONode() {
    
}

std::shared_ptr<VRONode> VRONode::clone() {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>(*this);
    for (std::shared_ptr<VRONode> subnode : _subnodes) {
        node->addChildNode(subnode->clone());
    }
    
    return node;
}

void VRONode::render(const VRORenderContext &context,
                     VRORenderParameters &params) {
    
    std::stack<VROMatrix4f> &rotations = params.rotations;
    std::stack<VROMatrix4f> &transforms = params.transforms;
    std::vector<std::shared_ptr<VROLight>> &lights = params.lights;
    
    rotations.push(rotations.top().multiply(_rotation.getMatrix()));
    
    VROMatrix4f transform = transforms.top().multiply(getTransform());
    transforms.push(transform);
    
    if (_light) {
        _light->setTransformedPosition(transform.multiply(_light->getPosition()));
        lights.push_back(_light);
    }

    if (_geometry) {
        _geometry->render(context, params);
    }
    
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->render(context, params);
    }
    
    transforms.pop();
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

