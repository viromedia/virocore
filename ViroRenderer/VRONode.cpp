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

VRONode::VRONode() : VROThreadRestricted(VROThreadName::Renderer),
    _scale({1.0, 1.0, 1.0}),
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

VRONode::VRONode(const VRONode &node) : VROThreadRestricted(VROThreadName::Renderer),
    _geometry(node._geometry),
    _lights(node._lights),
    _sounds(node._sounds),
    _scale(node._scale),
    _position(node._position),
    _rotation(node._rotation),
    _euler(node._euler),
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
                     std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold) {
        _geometry->render(elementIndex, material,
                          _computedTransform, _computedInverseTransposeTransform, _computedOpacity,
                          context, driver);
    }
}

void VRONode::resetDebugSortIndex() {
    sDebugSortIndex = 0;
}

void VRONode::updateSortKeys(uint32_t depth,
                             VRORenderParameters &params,
                             const VRORenderContext &context,
                             std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    processActions();
    
    std::stack<VROMatrix4f> &transforms = params.transforms;
    std::stack<float> &opacities = params.opacities;
    std::vector<std::shared_ptr<VROLight>> &lights = params.lights;
    std::stack<int> &hierarchyDepths = params.hierarchyDepths;
    std::stack<float> &distancesFromCamera = params.distancesFromCamera;
    
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
    int parentHierarchyDepth = hierarchyDepths.top();
    float parentDistanceFromCamera = distancesFromCamera.top();
    
    bool isParentHierarchical = (parentHierarchyDepth >= 0);
    bool isHierarchical = _hierarchicalRendering || isParentHierarchical;
    bool isTopOfHierarchy = _hierarchicalRendering && !isParentHierarchical;
    
    int hierarchyId = 0;
    
    // Distance to camera tracks the min distance between this node's bounding box to
    // the camera, for sort order
    float distanceFromCamera = 0;
    
    // The furthest distance from camera tracks the max distance between this node's
    // bounding box to the camera, for FCP computation
    float furthestDistanceFromCamera = 0;
    
    if (isHierarchical) {
        hierarchyDepth = parentHierarchyDepth + 1;
        hierarchyDepths.push(hierarchyDepth);
        
        if (isTopOfHierarchy) {
            hierarchyId = ++params.hierarchyId;
        }
        else {
            hierarchyId = params.hierarchyId;

            // All children of a hierarchy share the same distance from the camera.
            // This ensures the sort remains stable.
            distanceFromCamera = parentDistanceFromCamera;
        }
    }
    else {
        hierarchyDepths.push(-1);
    }
    
    /*
     Compute the sort key for this node's geometry elements.
     */
    if (_geometry) {
        int lightsHash = VROLight::hashLights(lights);
        if (!isHierarchical || isTopOfHierarchy) {
            distanceFromCamera = _computedBoundingBox.getDistanceToPoint(context.getCamera().getPosition());
            furthestDistanceFromCamera = _computedBoundingBox.getFurthestDistanceToPoint(context.getCamera().getPosition());
        }
        _geometry->updateSortKeys(this, hierarchyId, hierarchyDepth, lightsHash, _computedOpacity,
                                  distanceFromCamera, context.getZFar(), driver);
        
        if (kDebugSortOrder) {
            pinfo("   [%d] Pushed node with position [%f, %f, %f], rendering order %d, hierarchy depth %d (actual depth %d), distance to camera %f, hierarchy ID %d, lights %d",
                  sDebugSortIndex, _computedPosition.x, _computedPosition.y, _computedPosition.z, _renderingOrder, hierarchyDepth, depth, distanceFromCamera, hierarchyId, lightsHash);
            _geometry->setName(VROStringUtil::toString(sDebugSortIndex));
        }
    }
    else if (kDebugSortOrder) {
        pinfo("   [%d] Ignored empty node with position [%f, %f, %f] hierarchy depth %d, distance to camera %f, actual depth %d, hierarchy ID %d",
              sDebugSortIndex, _computedPosition.x, _computedPosition.y, _computedPosition.z,
              hierarchyDepth, 0.0, depth, hierarchyId);
    }

    distancesFromCamera.push(distanceFromCamera);
    params.furthestDistanceFromCamera = std::max(params.furthestDistanceFromCamera, furthestDistanceFromCamera);
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
    hierarchyDepths.pop();
    distancesFromCamera.pop();
}

void VRONode::getSortKeys(std::vector<VROSortKey> *outKeys) {
    passert_thread();
    
    if (_geometry) {
        _geometry->getSortKeys(outKeys);
    }
    for (std::shared_ptr<VRONode> childNode : _subnodes) {
        childNode->getSortKeys(outKeys);
    }
}

void VRONode::computeTransform(const VRORenderContext &context, VROMatrix4f parentTransforms) {
    passert_thread();
    
    VROMatrix4f transform;
    transform.scale(_scale.x, _scale.y, _scale.z);
    transform = _rotation.getMatrix().multiply(transform);
    transform.translate(_position.x, _position.y, _position.z);

    _computedTransform = parentTransforms.multiply(transform);
    _computedPosition = { _computedTransform[12], _computedTransform[13], _computedTransform[14] };
    if (_geometry) {
        _computedBoundingBox = _geometry->getBoundingBox().transform(_computedTransform);
    }
    
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
    passert_thread();
    animate(std::make_shared<VROAnimationQuaternion>([](VROAnimatable *const animatable, VROQuaternion r) {
                                                         ((VRONode *)animatable)->_rotation = r;
                                                         ((VRONode *)animatable)->_euler = r.toEuler();
                                                     }, _rotation, rotation));
}

void VRONode::setRotationEuler(VROVector3f euler) {
    passert_thread();
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f r) {
                                                        ((VRONode *)animatable)->_euler = VROMathNormalizeAngles2PI(r);
                                                        ((VRONode *)animatable)->_rotation = { r.x, r.y, r.z };
                                                     }, _euler, euler));
}

void VRONode::setPosition(VROVector3f position) {
    passert_thread();
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f p) {
                                                       ((VRONode *)animatable)->_position = p;
                                                   }, _position, position));
}

void VRONode::setScale(VROVector3f scale) {
    passert_thread();
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f s) {
                                                       ((VRONode *)animatable)->_scale = s;
                                                   }, _scale, scale));
}

void VRONode::setPositionX(float x) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        ((VRONode *)animatable)->_position.x = p;
    }, _position.x, x));
}

void VRONode::setPositionY(float y) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        ((VRONode *)animatable)->_position.y = p;
    }, _position.y, y));
}

void VRONode::setPositionZ(float z) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        ((VRONode *)animatable)->_position.z = p;
    }, _position.z, z));
}

void VRONode::setScaleX(float x) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.x = s;
    }, _scale.x, x));
}

void VRONode::setScaleY(float y) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.y = s;
    }, _scale.y, y));
}

void VRONode::setScaleZ(float z) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.z = s;
    }, _scale.z, z));
}

void VRONode::setRotationEulerX(float radians) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.x = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.x, radians));
}

void VRONode::setRotationEulerY(float radians) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.y = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.y, radians));
}

void VRONode::setRotationEulerZ(float radians) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.z = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.z, radians));
}

void VRONode::setOpacity(float opacity) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_opacity = s;
    }, _opacity, opacity));
}

void VRONode::setHidden(bool hidden) {
    passert_thread();
    _hidden = hidden;
    
    float opacity = hidden ? 0.0 : 1.0;
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_opacityFromHiddenFlag = s;
    }, _opacityFromHiddenFlag, opacity));
}

void VRONode::setHighAccuracyGaze(bool enabled) {
    passert_thread();
    _highAccuracyGaze = enabled;
}

#pragma mark - Actions

void VRONode::processActions() {
    passert_thread();
    
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
    passert_thread();
    _actions.push_back(action);
}

void VRONode::removeAction(std::shared_ptr<VROAction> action) {
    passert_thread();
    _actions.erase(std::remove_if(_actions.begin(), _actions.end(),
                                  [action](std::shared_ptr<VROAction> candidate) {
                                      return candidate == action;
                                  }), _actions.end());
}

void VRONode::removeAllActions() {
    passert_thread();
    _actions.clear();
}

#pragma mark - Hit Testing

VROBoundingBox VRONode::getBoundingBox() {
    passert_thread();
    return _computedBoundingBox;
}

std::vector<VROHitTestResult> VRONode::hitTest(const VROCamera &camera, VROVector3f origin, VROVector3f ray,
                                               bool boundsOnly) {
    passert_thread();
    std::vector<VROHitTestResult> results;

    VROMatrix4f identity;
    hitTest(camera, origin, ray, boundsOnly, results);

    return results;
}

void VRONode::hitTest(const VROCamera &camera, VROVector3f origin, VROVector3f ray, bool boundsOnly,
                      std::vector<VROHitTestResult> &results) {
    passert_thread();
    if (!_selectable) {
        return;
    }
    
    VROMatrix4f transform = _computedTransform;
    boundsOnly = boundsOnly && !getHighAccuracyGaze();
    
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold) {
        VROVector3f intPt;
        if (_computedBoundingBox.intersectsRay(ray, origin, &intPt)) {
            if (boundsOnly || hitTestGeometry(origin, ray, transform)) {
                results.push_back( {std::static_pointer_cast<VRONode>(shared_from_this()),
                                    intPt,
                                    origin.distance(intPt), false,
                                    camera });
            }
        }
    }
    
    for (std::shared_ptr<VRONode> &subnode : _subnodes) {
        subnode->hitTest(camera, origin, ray, boundsOnly, results);
    }
}

bool VRONode::hitTestGeometry(VROVector3f origin, VROVector3f ray, VROMatrix4f transform) {
    passert_thread();
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
    passert_thread();
    _constraints.push_back(constraint);
}

void VRONode::removeConstraint(std::shared_ptr<VROConstraint> constraint) {
    passert_thread();
    _constraints.erase(std::remove_if(_constraints.begin(), _constraints.end(),
                                  [constraint](std::shared_ptr<VROConstraint> candidate) {
                                      return candidate == constraint;
                                  }), _constraints.end());
}

void VRONode::removeAllConstraints() {
    passert_thread();
    _constraints.clear();
}


