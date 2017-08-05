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
#include "VROPortal.h"
#include "VROPhysicsBody.h"
#include "VROAnimationChain.h"
#include "VROExecutableAnimation.h"
#include "VROExecutableNodeAnimation.h"
#include "VROTransformDelegate.h"

// Opacity below which a node is considered hidden
static const float kHiddenOpacityThreshold = 0.02;

// Set to false to disable visibility testing
static const bool kEnableVisibilityFrustumTest = true;

// Set to true to debut the sort order
bool kDebugSortOrder = false;
static int sDebugSortIndex = 0;
const std::string kDefaultNodeTag = "undefined";

#pragma mark - Initialization

VRONode::VRONode() : VROThreadRestricted(VROThreadName::Renderer),
    _type(VRONodeType::Normal),
    _visible(false),
    _lastVisitedRenderingFrame(-1),
    _scale({1.0, 1.0, 1.0}),
    _euler({0, 0, 0}),
    _renderingOrder(0),
    _hidden(false),
    _opacityFromHiddenFlag(1.0),
    _opacity(1.0),
    _computedOpacity(1.0),
    _selectable(true),
    _highAccuracyGaze(false),
    _hierarchicalRendering(false),
    _portalInsideOut(false) {
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::VRONode(const VRONode &node) : VROThreadRestricted(VROThreadName::Renderer),
    _type(node._type),
    _visible(false),
    _lastVisitedRenderingFrame(-1),
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
    _hierarchicalRendering(node._hierarchicalRendering),
    _portalInsideOut(false) {
        
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::~VRONode() {
    ALLOCATION_TRACKER_SUB(Nodes, 1);
}

std::shared_ptr<VRONode> VRONode::clone() {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>(*this);
    for (std::shared_ptr<VRONode> &subnode : _subnodes) {
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

void VRONode::collectLights(std::vector<std::shared_ptr<VROLight>> *outLights) {
    for (std::shared_ptr<VROLight> &light : _lights) {
        light->setTransformedPosition(_computedTransform.multiply(light->getPosition()));
        outLights->push_back(light);
    }
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->collectLights(outLights);
    }
}

void VRONode::updateSortKeys(uint32_t depth,
                             VRORenderParameters &params,
                             const VRORenderContext &context,
                             std::shared_ptr<VRODriver> &driver) {
    passert_thread();
    processActions();
    
    /*
     If a node is not visible, that means none of its children are visible
     either (we use the umbrella bounding box for visibility tests), so we do
     not have to recurse down.
     */
    if (!_visible) {
        return;
    }
    
    std::stack<float> &opacities = params.opacities;
    std::vector<std::shared_ptr<VROLight>> &lights = params.lights;
    std::stack<int> &hierarchyDepths = params.hierarchyDepths;
    std::stack<float> &distancesFromCamera = params.distancesFromCamera;
    
    /*
     Compute specific parameters for this node.
     */
    _computedInverseTransposeTransform = _computedTransform.invert().transpose();
    _computedOpacity = opacities.top() * _opacity * _opacityFromHiddenFlag;
    opacities.push(_computedOpacity);
    
    _computedLights.clear();
    for (std::shared_ptr<VROLight> &light : lights) {
        if (_computedBoundingBox.getDistanceToPoint(light->getTransformedPosition()) < light->getAttenuationEndDistance()) {
            _computedLights.push_back(light);
        }
    }
    _computedLightsHash = VROLight::hashLights(_computedLights);

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
        if (!isHierarchical || isTopOfHierarchy) {
            distanceFromCamera = _computedPosition.distance(context.getCamera().getPosition());
            
            // TODO Using the bounding box may be preferred but currently leads to more
            //      artifacts
            // distanceFromCamera = _computedBoundingBox.getDistanceToPoint(context.getCamera().getPosition());
            
            furthestDistanceFromCamera = _computedBoundingBox.getFurthestDistanceToPoint(context.getCamera().getPosition());
        }
        _geometry->updateSortKeys(this, hierarchyId, hierarchyDepth, _computedLightsHash, _computedOpacity,
                                  distanceFromCamera, context.getZFar(), driver);
        
        if (kDebugSortOrder) {
            pinfo("   [%d] Pushed node with position [%f, %f, %f], rendering order %d, hierarchy depth %d (actual depth %d), distance to camera %f, hierarchy ID %d, lights %d",
                  sDebugSortIndex, _computedPosition.x, _computedPosition.y, _computedPosition.z, _renderingOrder, hierarchyDepth, depth, distanceFromCamera, hierarchyId, _computedLightsHash);
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
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->updateSortKeys(depth + 1, params, context, driver);
    }
    
    opacities.pop();
    hierarchyDepths.pop();
    distancesFromCamera.pop();
}

void VRONode::getSortKeysForVisibleNodes(std::vector<VROSortKey> *outKeys) {
    passert_thread();
    
    // Add the geometry of this node, if available
    if (_visible && _geometry && getType() == VRONodeType::Normal) {
        _geometry->getSortKeys(outKeys);
    }
    
    // Search down the scene graph. If a child is a portal, stop the search.
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        if (childNode->getType() == VRONodeType::Normal) {
            childNode->getSortKeysForVisibleNodes(outKeys);
        }
    }
}

void VRONode::computeTransforms(VROMatrix4f parentTransform, VROMatrix4f parentRotation) {
    passert_thread();
    
    /*
     Compute the transform for this node.
     */
    doComputeTransform(parentTransform);

    /*
     Compute the rotation for this node.
     */
    _computedRotation = parentRotation.multiply(_rotation.getMatrix());

    /*
     Move down the tree.
     */
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->computeTransforms(_computedTransform, _computedRotation);
    }
}

void VRONode::doComputeTransform(VROMatrix4f parentTransform) {
    /*
     Compute the world transform for this node. The full formula is:
     _computedTransform = parentTransform * T * Rpiv * R * Rpiv -1 * Spiv * S * Spiv-1
     */
    _computedTransform.toIdentity();
    
    /*
     Scale.
     */
    if (_scalePivot) {
        VROMatrix4f scale;
        scale.scale(_scale.x, _scale.y, _scale.z);
        _computedTransform = *_scalePivot * scale * *_scalePivotInverse;
    }
    else {
        _computedTransform.scale(_scale.x, _scale.y, _scale.z);
    }
    
    /*
     Rotation.
     */
    if (_rotationPivot) {
        _computedTransform = *_rotationPivotInverse * _computedTransform;
    }
    _computedTransform = _rotation.getMatrix() * _computedTransform;
    if (_rotationPivot) {
        _computedTransform = *_rotationPivot * _computedTransform;
    }
    
    /*
     Translation.
     */
    VROMatrix4f translate;
    translate.translate(_position.x, _position.y, _position.z);
    _computedTransform = translate * _computedTransform;

    _computedTransform = parentTransform * _computedTransform;
    _computedPosition = { _computedTransform[12], _computedTransform[13], _computedTransform[14] };
    if (_geometry) {
        _computedBoundingBox = _geometry->getBoundingBox().transform(_computedTransform);
    }
}

void VRONode::applyConstraints(const VRORenderContext &context, VROMatrix4f parentTransform,
                               bool parentUpdated) {
    
    bool updated = false;
    
    /*
     If a parent's _computedTransform was updated by constraints, we have to recompute
     the transform for this node as well.
     */
    if (parentUpdated) {
        doComputeTransform(parentTransform);
        updated = true;
    }
    
    /*
     Compute constraints for this node. Do not update _computedRotation as it isn't
     necessary after the afterConstraints() phase.
     */
    for (const std::shared_ptr<VROConstraint> &constraint : _constraints) {
        VROMatrix4f billboardRotation = constraint->getTransform(*this, context, _computedTransform);
        
        // To apply the billboard rotation, translate the object to the origin, apply
        // the rotation, then translate back to its previously computed position
        _computedTransform.translate(_computedPosition.scale(-1));
        _computedTransform = billboardRotation.multiply(_computedTransform);
        _computedTransform.translate(_computedPosition);
        
        updated = true;
    }
    
    /*
     Move down the tree.
     */
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->applyConstraints(context, _computedTransform, updated);
    }
}

void VRONode::setWorldTransform(VROVector3f finalPosition, VROQuaternion finalRotation){
    // Create a final compute transform representing the desired, final world position and rotation.
    VROVector3f worldScale = getComputedTransform().extractScale();
    VROMatrix4f finalComputedTransform;
    finalComputedTransform.toIdentity();
    finalComputedTransform.scale(worldScale.x, worldScale.y, worldScale.z);
    finalComputedTransform = finalRotation.makeInverse().getMatrix() * finalComputedTransform;
    finalComputedTransform.translate(finalPosition);

    // Calculate local transformations needed to achieve the desired final compute transform
    // by applying: FinalCompute_Inv_Trans * Parent_Trans = Local_Trans
    VROMatrix4f parentTransform = getParentNode()->getComputedTransform();
    VROMatrix4f currentTransformInverted = finalComputedTransform.invert() * parentTransform;
    VROMatrix4f currentTransform = currentTransformInverted.invert();
    _scale = currentTransform.extractScale();
    _position = currentTransform.extractTranslation();
    _rotation = currentTransform.extractRotation(_scale);

    // Trigger a computeTransform pass to update the node's bounding boxes and as well as its
    // child's node transforms recursively.
    computeTransforms(getParentNode()->getComputedTransform(), getParentNode()->getComputedRotation());
}

#pragma mark - Visibility

void VRONode::updateVisibility(const VRORenderContext &context) {
    const VROFrustum &frustum = context.getCamera().getFrustum();
    
    _umbrellaBoundingBox = VROBoundingBox();
    computeUmbrellaBounds(&_umbrellaBoundingBox);
    
    VROFrustumResult result = frustum.intersectAllOpt(_umbrellaBoundingBox, &_umbrellaBoxMetadata);
    if (result == VROFrustumResult::Inside || !kEnableVisibilityFrustumTest) {
        setVisibilityRecursive(true);
    }
    else if (result == VROFrustumResult::Intersects) {
        _visible = true;
        for (std::shared_ptr<VRONode> &childNode : _subnodes) {
            childNode->updateVisibility(context);
        }
    }
    else {
        setVisibilityRecursive(false);
    }
}

void VRONode::setVisibilityRecursive(bool visible) {
    _visible = visible;
    
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->setVisibilityRecursive(visible);
    }
}

void VRONode::computeUmbrellaBounds(VROBoundingBox *bounds) const {
    bounds->unionDestructive(_computedBoundingBox);
    for (const std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->computeUmbrellaBounds(bounds);
    }
}

int VRONode::countVisibleNodes() const {
    int count = _visible ? 1 : 0;
    for (const std::shared_ptr<VRONode> &childNode : _subnodes) {
        count += childNode->countVisibleNodes();
    }
    return count;
}

VROVector3f VRONode::getComputedPosition() const {
    return _computedPosition;
}

VROMatrix4f VRONode::getComputedRotation() const {
    return _computedRotation;
}

VROMatrix4f VRONode::getComputedTransform() const {
    return _computedTransform;
}

#pragma mark - Portals

const std::shared_ptr<VROPortal> VRONode::getParentPortal() const {
    const std::shared_ptr<VRONode> parent = _supernode.lock();
    if (!parent) {
        return nullptr;
    }
    
    if (parent->getType() == VRONodeType::Portal) {
        return std::dynamic_pointer_cast<VROPortal>(parent);
    }
    else {
        return parent->getParentPortal();
    }
}

void VRONode::getChildPortals(std::vector<std::shared_ptr<VROPortal>> *outPortals) const {
    for (const std::shared_ptr<VRONode> &childNode : _subnodes) {
        if (childNode->getType() == VRONodeType::Portal) {
            outPortals->push_back(std::dynamic_pointer_cast<VROPortal>(childNode));
        }
        else {
            childNode->getChildPortals(outPortals);
        }
    }
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
                                                        VRONode *node = ((VRONode *)animatable);
                                                        node->_position = p;
                                                        node->notifyTransformUpdate(false);
                                                   }, _position, position));
}

void VRONode::setScale(VROVector3f scale) {
    passert_thread();
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f s) {
                                                       ((VRONode *)animatable)->_scale = s;
                                                   }, _scale, scale));
}

void VRONode::setTransformDelegate(std::shared_ptr<VROTransformDelegate> delegate) {
    _transformDelegate = delegate;

    // Refresh the delegate with the latest position data as it is attached.
    notifyTransformUpdate(true);
}

void VRONode::notifyTransformUpdate(bool forced) {
    std::shared_ptr<VROTransformDelegate> delegate = _transformDelegate.lock();
    if (delegate != nullptr){
        delegate->processPositionUpdate(_position, forced);
    }
}

void VRONode::setPositionX(float x) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        VRONode *node = ((VRONode *)animatable);
        node->_position.x = p;
        node->notifyTransformUpdate(false);
    }, _position.x, x));
}

void VRONode::setPositionY(float y) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        VRONode *node = ((VRONode *)animatable);
        node->_position.y = p;
        node->notifyTransformUpdate(false);
    }, _position.y, y));
}

void VRONode::setPositionZ(float z) {
    passert_thread();
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        VRONode *node = ((VRONode *)animatable);
        node->_position.z = p;
        node->notifyTransformUpdate(false);
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

void VRONode::setRotationPivot(VROMatrix4f pivot) {
    passert_thread();
    _rotationPivot = pivot;
    _rotationPivotInverse = pivot.invert();
}

void VRONode::setScalePivot(VROMatrix4f pivot) {
    passert_thread();
    _scalePivot = pivot;
    _scalePivotInverse = pivot.invert();
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

#pragma mark - Actions and Animations

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

void VRONode::addAnimation(std::string key, std::shared_ptr<VROExecutableAnimation> animation) {
    passert_thread();
    std::shared_ptr<VRONode> shared = std::dynamic_pointer_cast<VRONode>(shared_from_this());
    _animations[key].push_back(std::make_shared<VROExecutableNodeAnimation>(shared, animation));
}

void VRONode::removeAnimation(std::string key) {
    passert_thread();
    auto kv = _animations.find(key);
    if (kv == _animations.end()) {
        return;
    }
    
    for (std::shared_ptr<VROExecutableAnimation> &animation : kv->second) {
        animation->terminate();
    }
    kv->second.clear();
}

std::shared_ptr<VROExecutableAnimation> VRONode::getAnimation(std::string key, bool recursive) {
    std::vector<std::shared_ptr<VROExecutableAnimation>> animations;
    getAnimations(animations, key, recursive);
    
    return std::make_shared<VROAnimationChain>(animations, VROAnimationChainExecution::Parallel);
}

void VRONode::getAnimations(std::vector<std::shared_ptr<VROExecutableAnimation>> &animations,
                            std::string key, bool recursive) {
    auto kv = _animations.find(key);
    if (kv != _animations.end()) {
        animations.insert(animations.end(), kv->second.begin(), kv->second.end());
    }
    
    if (recursive) {
        for (std::shared_ptr<VRONode> &subnode : _subnodes) {
            subnode->getAnimations(animations, key, recursive);
        }
    }
}

std::set<std::string> VRONode::getAnimationKeys(bool recursive) {
    std::set<std::string> animations;
    getAnimationKeys(animations, recursive);
    
    return animations;
}

void VRONode::getAnimationKeys(std::set<std::string> &keys, bool recursive) {
    for (auto kv : _animations) {
        if (!kv.second.empty()) {
            keys.insert(kv.first);
        }
    }
    if (recursive) {
        for (std::shared_ptr<VRONode> &subnode : _subnodes) {
            subnode->getAnimationKeys(keys, recursive);
        }
    }
}

void VRONode::removeAllAnimations() {
    passert_thread();
    for (auto kv : _animations) {
        for (std::shared_ptr<VROExecutableAnimation> &animation : kv.second) {
            animation->terminate();
        }
        kv.second.clear();
    }
    _animations.clear();
}

void VRONode::onAnimationFinished(){
    notifyTransformUpdate(true);

    std::shared_ptr<VROPhysicsBody> body = getPhysicsBody();
    if (body){
        body->refreshBody();
    }
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

#pragma mark - Physics

std::shared_ptr<VROPhysicsBody> VRONode::initPhysicsBody(VROPhysicsBody::VROPhysicsBodyType type,
                                                         float mass,
                                                         std::shared_ptr<VROPhysicsShape> shape) {
    std::shared_ptr<VRONode> node = std::static_pointer_cast<VRONode>(shared_from_this());
    _physicsBody = std::make_shared<VROPhysicsBody>(node, type, mass, shape);
    return _physicsBody;
}

std::shared_ptr<VROPhysicsBody> VRONode::getPhysicsBody() const {
    return _physicsBody;
}

void VRONode::clearPhysicsBody(){
    _physicsBody = nullptr;
}
