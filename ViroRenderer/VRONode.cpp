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
#include "VROStringUtil.h"

// Opacity below which a node is considered hidden
static const float kHiddenOpacityThreshold = 0.02;

// Set to true to debut the sort order
bool kDebugSortOrder = false;
static int sDebugSortIndex = 0;

#pragma mark - Initialization

VRONode::VRONode() :
    _scale({1.0, 1.0, 1.0}),
    _pivot({0.5f, 0.5f, 0.5f}),
    _euler({0, 0, 0}),
    _renderingOrder(0),
    _hidden(false),
    _opacityFromHiddenFlag(1.0),
    _opacity(1.0),
    _computedOpacity(1.0),
    _selectable(true),
    _highAccuracyGaze(false),
    _hierarchicalRendering(false) {
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::VRONode(const VRONode &node) :
    _geometry(node._geometry),
    _lights(node._lights),
    _sounds(node._sounds),
    _scale(node._scale),
    _position(node._position),
    _rotation(node._rotation),
    _euler(node._euler),
    _pivot(node._pivot),
    _renderingOrder(node._renderingOrder),
    _hidden(node._hidden),
    _opacityFromHiddenFlag(node._opacityFromHiddenFlag),
    _opacity(node._opacity),
    _selectable(node._selectable),
    _highAccuracyGaze(node._highAccuracyGaze),
    _hierarchicalRendering(node._hierarchicalRendering) {
        
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

void VRONode::render(int elementIndex,
                     std::shared_ptr<VROMaterial> &material,
                     const VRORenderContext &context,
                     VRODriver &driver) {
    
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold) {
        _geometry->render(elementIndex, material,
                          _computedTransform, _computedInverseTransposeTransform, _computedOpacity,
                          context, driver);
    }
}

void VRONode::resetDebugSortIndex() {
    sDebugSortIndex = 0;
}

void VRONode::updateSortKeys(uint32_t depth, VRORenderParameters &params,
                             const VRORenderContext &context, VRODriver &driver) {
    processActions();
    
    std::stack<VROMatrix4f> &transforms = params.transforms;
    std::stack<float> &opacities = params.opacities;
    std::vector<std::shared_ptr<VROLight>> &lights = params.lights;
    std::stack<int> &hierarchical = params.hierarchical;
    
    /*
     Compute the specific parameters for this node.
     */
    computeTransform(context, transforms.top());
    transforms.push(_computedTransform);
    
    _computedOpacity = opacities.top() * _opacity * _opacityFromHiddenFlag;
    opacities.push(_computedOpacity);
    
    for (std::shared_ptr<VROLight> &light : _lights) {
        light->setTransformedPosition(_computedTransform.multiply(light->getPosition()));
        lights.push_back(light);
    }
    _computedLights.clear();
    _computedLights.insert(_computedLights.begin(), lights.begin(), lights.end());

    for (std::shared_ptr<VROSound> &sound : _sounds) {
        sound->setTransformedPosition(_computedTransform.multiply(sound->getPosition()));
    }

    /*
     This node uses hierarchical rendering if its flag is set, or if its parent
     used hierarchical rendering.
     */
    int hierarchyDepth = 0;
    int parentHierarchyDepth = hierarchical.top();
    
    bool isParentHierarchical = (parentHierarchyDepth >= 0);
    bool isHierarchical = _hierarchicalRendering || isParentHierarchical;
    bool isTopOfHierarchy = _hierarchicalRendering && !isParentHierarchical;
    
    int hierarchyId = 0;
    
    if (isHierarchical) {
        hierarchyDepth = parentHierarchyDepth + 1;
        hierarchical.push(hierarchyDepth);
        
        if (isTopOfHierarchy) {
            hierarchyId = ++params.hierarchyId;
        }
        else {
            hierarchyId = params.hierarchyId;
        }
    }
    else {
        hierarchical.push(-1);
    }
    
    /*
     Compute the sort key for this node's geometry elements.
     */
    if (_geometry) {
        int lightsHash = VROLight::hashLights(lights);
        float distanceFromCamera = getTransformedPosition().distance(context.getCamera().getPosition());
        _geometry->updateSortKeys(this, hierarchyId, hierarchyDepth, lightsHash, _computedOpacity,
                                  distanceFromCamera, context.getZFar(), driver);
        
        if (kDebugSortOrder) {
            pinfo("   [%d] Pushed node with position [%f, %f, %f] hierarchy depth %d, distance to camera %f, actual depth %d, hierarchy ID %d",
                  sDebugSortIndex, _computedPosition.x, _computedPosition.y, _computedPosition.z,
                  hierarchyDepth, distanceFromCamera, depth, hierarchyId);
            _geometry->setName(VROStringUtil::toString(sDebugSortIndex));
        }
    }
    else if (kDebugSortOrder) {
        pinfo("   [%d] Ignored empty node with position [%f, %f, %f] hierarchy depth %d, distance to camera %f, actual depth %d, hierarchy ID %d",
              sDebugSortIndex, _computedPosition.x, _computedPosition.y, _computedPosition.z,
              hierarchyDepth, 0.0, depth, hierarchyId);
    }
    sDebugSortIndex++;
    
    /*
     Move down the tree.
     */
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->updateSortKeys(depth + 1, params, context, driver);
    }
    
    transforms.pop();
    opacities.pop();
    for (int i = 0; i < _lights.size(); i++) {
        lights.pop_back();
    }
    hierarchical.pop();
}

void VRONode::getSortKeys(std::vector<VROSortKey> *outKeys) {
    if (_geometry) {
        _geometry->getSortKeys(outKeys);
    }
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->getSortKeys(outKeys);
    }
}

void VRONode::computeTransform(const VRORenderContext &context, VROMatrix4f parentTransforms) {
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

    _computedTransform = parentTransforms.multiply(transform);
    _computedPosition = { _computedTransform[12], _computedTransform[13], _computedTransform[14] };
    
    for (const std::shared_ptr<VROConstraint> &constraint : _constraints) {
        VROMatrix4f billboardRotation = constraint->getTransform(*this, context, _computedTransform);
        
        // To apply the billboard rotation, translate the object to the origin, apply
        // the rotation, then translate back to its previously computed position
        _computedTransform.translate(_computedPosition.scale(-1));
        _computedTransform = billboardRotation.multiply(_computedTransform);
        _computedTransform.translate(_computedPosition);
    }
    
    _computedInverseTransposeTransform = _computedTransform.invert().transpose();
}

VROVector3f VRONode::getTransformedPosition() const {
    return _computedPosition;
}

#pragma mark - Setters

void VRONode::setRotation(VROQuaternion rotation) {
    animate(std::make_shared<VROAnimationQuaternion>([](VROAnimatable *const animatable, VROQuaternion r) {
                                                         ((VRONode *)animatable)->_rotation = r;
                                                         ((VRONode *)animatable)->_euler = r.toEuler();
                                                     }, _rotation, rotation));
}

void VRONode::setRotationEuler(VROVector3f euler) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f r) {
                                                        ((VRONode *)animatable)->_euler = VROMathNormalizeAngles2PI(r);
                                                        ((VRONode *)animatable)->_rotation = { r.x, r.y, r.z };
                                                     }, _euler, euler));
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

void VRONode::setPositionX(float x) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        ((VRONode *)animatable)->_position.x = p;
    }, _position.x, x));
}

void VRONode::setPositionY(float y) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        ((VRONode *)animatable)->_position.y = p;
    }, _position.y, y));
}

void VRONode::setPositionZ(float z) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        ((VRONode *)animatable)->_position.z = p;
    }, _position.z, z));
}

void VRONode::setScaleX(float x) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.x = s;
    }, _scale.x, x));
}

void VRONode::setScaleY(float y) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.y = s;
    }, _scale.y, y));
}

void VRONode::setScaleZ(float z) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.z = s;
    }, _scale.z, z));
}

void VRONode::setRotationEulerX(float radians) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.x = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.x, radians));
}

void VRONode::setRotationEulerY(float radians) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.y = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.y, radians));
}

void VRONode::setRotationEulerZ(float radians) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.z = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.z, radians));
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

void VRONode::setHidden(bool hidden) {
    _hidden = hidden;
    
    float opacity = hidden ? 0.0 : 1.0;
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_opacityFromHiddenFlag = s;
    }, _opacityFromHiddenFlag, opacity));
}

void VRONode::setHighAccuracyGaze(bool enabled) {
    _highAccuracyGaze = enabled;
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
    return _geometry->getBoundingBox().transform(_computedTransform);
}

std::vector<VROHitTestResult> VRONode::hitTest(VROVector3f ray, VROVector3f origin,
                                               bool boundsOnly) {
    std::vector<VROHitTestResult> results;

    VROMatrix4f identity;
    hitTest(ray, identity, boundsOnly, origin, results);

    return results;
}

void VRONode::hitTest(VROVector3f ray, VROMatrix4f parentTransform, bool boundsOnly,
                      VROVector3f origin, std::vector<VROHitTestResult> &results) {
    
    if (!_selectable) {
        return;
    }
    
    VROMatrix4f transform = _computedTransform;
    boundsOnly = boundsOnly && !getHighAccuracyGaze();
    
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold) {
        VROBoundingBox bounds = _geometry->getBoundingBox().transform(transform);
        
        VROVector3f intPt;
        if (bounds.intersectsRay(ray, origin, &intPt)) {
            if (boundsOnly || hitTestGeometry(ray, origin, transform)) {
                results.push_back({std::static_pointer_cast<VRONode>(shared_from_this()), intPt, origin.distance(intPt), false});
            }
        }
    }
    
    for (std::shared_ptr<VRONode> &subnode : _subnodes) {
        subnode->hitTest(ray, transform, boundsOnly, origin, results);
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


