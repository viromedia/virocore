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
#include "VROAnimation.h"
#include "VROTransaction.h"
#include "VROAnimationVector3f.h"
#include "VROAnimationQuaternion.h"

#pragma mark - Initialization

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

#pragma mark - Rendering

void VRONode::render(const VRORenderContext &context, VRORenderParameters &params) {
    /*
     Render the presentation node if one is present. The presentation node
     reflects the current state of animations.
     */
    VRONode *nodeToRender = _presentationNode ? _presentationNode.get() : this;
    
    nodeToRender->pushTransforms(params);
    nodeToRender->renderNode(context, params);
    
    /*
     Node the node tree is only present in the model node, not in the
     presentation node, so we find children using the model node's hierarchy.
     */
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->render(context, params);
    }
    
    nodeToRender->popTransforms(params);
}

void VRONode::pushTransforms(VRORenderParameters &params) {
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
}

void VRONode::renderNode(const VRORenderContext &context, VRORenderParameters &params) {
    if (_geometry) {
        _geometry->render(context, params);
    }
}

void VRONode::popTransforms(VRORenderParameters &params) {
    params.transforms.pop();
    params.rotations.pop();
    
    if (_light) {
        params.lights.pop_back();
    }
}

VROMatrix4f VRONode::getTransform() const {
    VROMatrix4f transform = _rotation.getMatrix();
    transform.scale(_scale.x, _scale.y, _scale.z);
    transform.translate(_position.x, _position.y, _position.z);
    
    return transform;
}

#pragma mark - Setters

void VRONode::setRotation(VROQuaternion rotation) {
    animate(std::make_shared<VROAnimationQuaternion>([this](VROQuaternion r) {
                                                         _rotation = r;
                                                     }, _rotation, rotation));
}

void VRONode::setPosition(VROVector3f position) {
    animate(std::make_shared<VROAnimationVector3f>([this](VROVector3f p) {
                                                       _position = p;
                                                   }, _position, position));
}

void VRONode::setScale(VROVector3f scale) {
    animate(std::make_shared<VROAnimationVector3f>([this](VROVector3f s) {
                                                       _scale = s;
                                                   }, _scale, scale));
}
