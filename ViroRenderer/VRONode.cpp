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
#include "VROAction.h"
#include "VROLog.h"
#include "VROHitTestResult.h"
#include "VROAllocationTracker.h"

#pragma mark - Initialization

VRONode::VRONode(const VRORenderContext &context) :
    _scale({1.0, 1.0, 1.0}) {
    
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::VRONode(const VRONode &node) :
    _geometry(node._geometry),
    _light(node._light),
    _scale(node._scale),
    _position(node._position),
    _rotation(node._rotation) {
        
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::~VRONode() {
    ALLOCATION_TRACKER_SUB(Nodes, 1);
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
    processActions();
    
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

#pragma mark - Actions

void VRONode::processActions() {
    std::vector<std::shared_ptr<VROAction>>::iterator it;
    for (it = _actions.begin(); it != _actions.end(); ++it) {
        std::shared_ptr<VROAction> &action = *it;

        action->execute(this);

        if (action->getType() == VROActionType::PerFrame ||
            action->getType() == VROActionType::Timed) {
            
            // Remove the action when it's complete
            if (!action->shouldRepeat()) {
                _actions.erase(it);
                --it;
            }
        }
        else {            
            // Remove the action; it will be re-added (if needed) after the animation
            _actions.erase(it);
            --it;
        }
    }
}

void VRONode::runAction(std::shared_ptr<VROAction> action) {
    _actions.push_back(action);
}

void VRONode::removeAction(std::shared_ptr<VROAction> action) {
    _actions.erase(std::remove_if(_actions.begin(), _actions.end(),
                                  [action](std::shared_ptr<VROAction> candidate) {
                                      return candidate == action;
                                  }), _actions.end());
}

void VRONode::removeAllActions() {
    _actions.clear();
}

#pragma mark - Hit Testing

VROBoundingBox VRONode::getBoundingBox() {
    return _geometry->getBoundingBox().transform(getTransform());
}

std::vector<VROHitTestResult> VRONode::hitTest(VROVector3f ray) {
    std::vector<VROHitTestResult> results;
    
    VROMatrix4f identity;
    hitTest(ray, identity, results);
    
    return results;
}

void VRONode::hitTest(VROVector3f ray, VROMatrix4f parentTransform,
                      std::vector<VROHitTestResult> &results) {
    
    // TODO Use camera location for origin
    VROVector3f origin;
    
    VROMatrix4f transform = parentTransform.multiply(getTransform());
    
    if (_geometry) {
        VROBoundingBox bounds = _geometry->getBoundingBox().transform(transform);
        
        VROVector3f intPt;
        if (bounds.intersectsRay(ray, origin, &intPt)) {
            results.push_back({std::static_pointer_cast<VRONode>(shared_from_this()), intPt});
        }
    }
    
    for (std::shared_ptr<VRONode> &subnode : _subnodes) {
        subnode->hitTest(ray, transform, results);
    }
}

