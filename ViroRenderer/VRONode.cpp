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
#include "VROAnimationFloat.h"
#include "VROAnimationQuaternion.h"
#include "VROAction.h"
#include "VROLog.h"
#include "VROHitTestResult.h"
#include "VROAllocationTracker.h"
#include "VROGeometrySource.h"
#include "VROGeometryElement.h"
#include "VROByteBuffer.h"
#include "VROConstraint.h"

#pragma mark - Initialization

VRONode::VRONode() :
    _scale({1.0, 1.0, 1.0}),
    _pivot({0.5f, 0.5f, 0.5f}),
    _opacity(1.0),
    _selectable(true) {
    
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::VRONode(const VRONode &node) :
    _geometry(node._geometry),
    _light(node._light),
    _scale(node._scale),
    _position(node._position),
    _rotation(node._rotation),
    _pivot(node._pivot) {
        
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

void VRONode::render(const VRORenderContext &renderContext,
                     const VRODriver &driver,
                     VRORenderParameters &params) {
    processActions();
    
    pushTransforms(renderContext, params);
    renderNode(renderContext, driver, params);
    
    /*
     Node the node tree is only present in the model node, not in the
     presentation node, so we find children using the model node's hierarchy.
     */
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->render(renderContext, driver, params);
    }
    
    popTransforms(params);
}

void VRONode::updateSortKeys(std::vector<std::shared_ptr<VROLight>> &lights) {
    if (_light) {
        lights.push_back(_light);
    }
    
    if (_geometry) {
        int lightsHash = hashLights(lights);
        _geometry->updateSortKeys(lightsHash);
    }
    
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->updateSortKeys(lights);
    }
    
    if (_light) {
        lights.pop_back();
    }
}

uint32_t VRONode::hashLights(std::vector<std::shared_ptr<VROLight>> &lights) {
    uint32_t h = 0;
    for (std::shared_ptr<VROLight> &light : lights) {
        h = 31 * h + light->getLightId();
    }
    return h;
}

void VRONode::pushTransforms(const VRORenderContext &context, VRORenderParameters &params) {
    std::stack<VROMatrix4f> &transforms = params.transforms;
    std::stack<float> &opacities = params.opacities;
    std::vector<std::shared_ptr<VROLight>> &lights = params.lights;
    
    VROMatrix4f transform = transforms.top().multiply(getTransform(context));
    transforms.push(transform);
    
    opacities.push(opacities.top() * _opacity);
    
    if (_light) {
        _light->setTransformedPosition(transform.multiply(_light->getPosition()));
        lights.push_back(_light);
    }
}

void VRONode::renderNode(const VRORenderContext &renderContext,
                         const VRODriver &driver,
                         VRORenderParameters &params) {
    
    if (_geometry) {
        _geometry->render(renderContext, driver, params);
    }
}

void VRONode::popTransforms(VRORenderParameters &params) {
    params.transforms.pop();
    params.opacities.pop();
    
    if (_light) {
        params.lights.pop_back();
    }
}

VROMatrix4f VRONode::getTransform(const VRORenderContext &context) const {
    VROMatrix4f pivotMtx, unpivotMtx;
    
    if (_geometry) {
        VROBoundingBox bounds = _geometry->getBoundingBox();
        VROVector3f extents = bounds.getExtents();
        
        VROVector3f pivotCoordinate(bounds.getMinX() * (1 - _pivot.x) + bounds.getMaxX() * _pivot.x,
                                    bounds.getMinY() * (1 - _pivot.y) + bounds.getMaxY() * _pivot.y,
                                    bounds.getMinZ() * (1 - _pivot.z) + bounds.getMaxZ() * _pivot.z);
        
        pivotMtx.translate(-pivotCoordinate.x, -pivotCoordinate.y, -pivotCoordinate.z);
        unpivotMtx.translate(pivotCoordinate.x, pivotCoordinate.y, pivotCoordinate.z);
    }
    
    VROMatrix4f transform;
    transform.scale(_scale.x, _scale.y, _scale.z);
    transform = _rotation.getMatrix().multiply(transform);
    transform.translate(_position.x, _position.y, _position.z);
    transform = unpivotMtx.multiply(transform).multiply(pivotMtx);
    
    for (const std::shared_ptr<VROConstraint> &constraint : _constraints) {
        transform = constraint->getTransform(*this, transform, context);
    }
    
    return transform;
}

VROVector3f VRONode::getTransformedPosition() const {
    std::shared_ptr<VRONode> supernode = _supernode.lock();
    if (supernode) {
        return _position + supernode->getTransformedPosition();
    }
    else {
        return _position;
    }
}

#pragma mark - Setters

void VRONode::setRotation(VROQuaternion rotation) {
    animate(std::make_shared<VROAnimationQuaternion>([](VROAnimatable *const animatable, VROQuaternion r) {
                                                         ((VRONode *)animatable)->_rotation = r;
                                                     }, _rotation, rotation));
}

void VRONode::setPosition(VROVector3f position) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f p) {
                                                       ((VRONode *)animatable)->_position = p;
                                                   }, _position, position));
}

void VRONode::setScale(VROVector3f scale) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f s) {
                                                       ((VRONode *)animatable)->_scale = s;
                                                   }, _scale, scale));
}

void VRONode::setPivot(VROVector3f pivot) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f s) {
                                                        ((VRONode *)animatable)->_pivot = s;
                                                   }, _pivot, pivot));
}

void VRONode::setOpacity(float opacity) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_opacity = s;
    }, _opacity, opacity));
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

VROBoundingBox VRONode::getBoundingBox(const VRORenderContext &context) {
    return _geometry->getBoundingBox().transform(getTransform(context));
}

std::vector<VROHitTestResult> VRONode::hitTest(VROVector3f ray, const VRORenderContext &context,
                                               bool boundsOnly) {
    std::vector<VROHitTestResult> results;
    
    VROMatrix4f identity;
    hitTest(ray, identity, boundsOnly, context, results);
    
    return results;
}

void VRONode::hitTest(VROVector3f ray, VROMatrix4f parentTransform, bool boundsOnly,
                      const VRORenderContext &context,
                      std::vector<VROHitTestResult> &results) {
    
    if (!_selectable) {
        return;
    }
    
    VROVector3f origin = context.getCamera().getPosition();
    VROMatrix4f transform = parentTransform.multiply(getTransform(context));
    
    if (_geometry) {
        VROBoundingBox bounds = _geometry->getBoundingBox().transform(transform);
        
        VROVector3f intPt;
        if (bounds.intersectsRay(ray, origin, &intPt)) {
            if (boundsOnly || hitTestGeometry(ray, origin, transform)) {
                results.push_back({std::static_pointer_cast<VRONode>(shared_from_this()), intPt});
            }
        }
    }
    
    for (std::shared_ptr<VRONode> &subnode : _subnodes) {
        subnode->hitTest(ray, transform, boundsOnly, context, results);
    }
}

bool VRONode::hitTestGeometry(VROVector3f ray, VROVector3f origin, VROMatrix4f transform) {
    std::shared_ptr<VROGeometrySource> vertexSource = _geometry->getGeometrySourcesForSemantic(VROGeometrySourceSemantic::Vertex).front();
    
    bool hit = false;
    for (std::shared_ptr<VROGeometryElement> element : _geometry->getGeometryElements()) {
         element->processTriangles([&hit, ray, origin, transform](int index, VROTriangle triangle) {
             VROTriangle transformed = triangle.transformByMatrix(transform);
             
             VROVector3f intPt;
             if (transformed.intersectsRay(ray, origin, &intPt)) {
                 hit = true;
                 //TODO Offer a way to break out of here, as optimization
             }
         }, vertexSource);
    }
    
    return hit;
}

#pragma mark - Constraints

void VRONode::addConstraint(std::shared_ptr<VROConstraint> constraint) {
    _constraints.push_back(constraint);
}

void VRONode::removeConstraint(std::shared_ptr<VROConstraint> constraint) {
    _constraints.erase(std::remove_if(_constraints.begin(), _constraints.end(),
                                  [constraint](std::shared_ptr<VROConstraint> candidate) {
                                      return candidate == constraint;
                                  }), _constraints.end());
}

void VRONode::removeAllConstraints() {
    _constraints.clear();
}


