//
//  VRONode.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VRONode.h"
#include "VROIKRig.h"
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
#include "VROMaterial.h"
#include "VROPhysicsBody.h"
#include "VROParticleEmitter.h"
#include "VROScene.h"
#include "VROAnimationChain.h"
#include "VROExecutableAnimation.h"
#include "VROExecutableNodeAnimation.h"
#include "VROSkeletalAnimation.h"
#include "VROLayeredSkeletalAnimation.h"
#include "VROSkeletalAnimationLayer.h"
#include "VROTransformDelegate.h"
#include "VROInstancedUBO.h"
#include "VROPlatformUtil.h"
#include "VROMorpher.h"

// Opacity below which a node is considered hidden
static const float kHiddenOpacityThreshold = 0.02;

// Set to false to disable visibility testing
static const bool kEnableVisibilityFrustumTest = true;

// Set to true to debut the sort order
bool kDebugSortOrder = false;
int  kDebugSortOrderFrameFrequency = 60;
static int sDebugSortIndex = 0;
const std::string kDefaultNodeTag = "undefined";

// Note: if you change the initial value below, make that sNullNodeID
// in EventDelegate_JNI.cpp still represents a value that this will
// never vend.
std::atomic<int> sUniqueIDGenerator(0);

#pragma mark - Initialization

VRONode::VRONode() : VROThreadRestricted(VROThreadName::Renderer),
    _uniqueID(sUniqueIDGenerator++),
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
    _highAccuracyEvents(false),
    _hierarchicalRendering(false),
    _lightReceivingBitMask(1),
    _shadowCastingBitMask(1),
    _ignoreEventHandling(false),
    _dragType(VRODragType::FixedDistance),
    _dragPlanePoint({ 0, 0, 0 }),
    _dragPlaneNormal({ 0, 0 ,0 }),
    _dragMaxDistance(10),
    _lastLocalTransform(VROMatrix4f::identity()),
    _lastWorldTransform(VROMatrix4f::identity()),
    _lastWorldPosition({ 0, 0, 0 }),
    _lastWorldRotation(VROMatrix4f::identity()),
    _lastPosition({ 0, 0, 0 }),
    _lastScale({ 1, 1, 1 }),
    _lastRotation(VROMatrix4f::identity()),
    _lastHasScalePivot(false),
    _lastHasRotationPivot(false),
    _holdRendering(false) {
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::VRONode(const VRONode &node) : VROThreadRestricted(VROThreadName::Renderer),
    _uniqueID(sUniqueIDGenerator++),
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
    _highAccuracyEvents(node._highAccuracyEvents),
    _hierarchicalRendering(node._hierarchicalRendering),
    _lightReceivingBitMask(node._lightReceivingBitMask),
    _shadowCastingBitMask(node._shadowCastingBitMask),
    _ignoreEventHandling(node._ignoreEventHandling),
    _dragType(node._dragType),
    _dragPlanePoint(node._dragPlanePoint),
    _dragPlaneNormal(node._dragPlaneNormal),
    _dragMaxDistance(node._dragMaxDistance),
#if VRO_PLATFORM_IOS || VRO_PLATFORM_MACOS || VRO_PLATFORM_ANDROID
    // Atomics need to be explicitly loaded if in initializer lists
    // (normally the assignment operator will automatically load the
    // atomic)
    _lastLocalTransform(node._lastLocalTransform.load()),
    _lastWorldTransform(node._lastWorldTransform.load()),
    _lastWorldPosition(node._lastWorldPosition.load()),
    _lastWorldRotation(node._lastWorldRotation.load()),
    _lastPosition(node._lastPosition.load()),
    _lastScale(node._lastScale.load()),
    _lastRotation(node._lastRotation.load()),
    _lastScalePivot(node._lastScalePivot.load()),
    _lastScalePivotInverse(node._lastScalePivotInverse.load()),
    _lastRotationPivot(node._lastRotationPivot.load()),
    _lastRotationPivotInverse(node._lastRotationPivotInverse.load()),
    _lastHasScalePivot(node._lastHasScalePivot.load()),
    _lastHasRotationPivot(node._lastHasRotationPivot.load()),
#else
    _lastLocalTransform(node._lastLocalTransform),
    _lastWorldTransform(node._lastWorldTransform),
    _lastWorldPosition(node._lastWorldPosition),
    _lastWorldRotation(node._lastWorldRotation),
    _lastPosition(node._lastPosition),
    _lastScale(node._lastScale),
    _lastRotation(node._lastRotation),
    _lastScalePivot(node._lastScalePivot),
    _lastScalePivotInverse(node._lastScalePivotInverse),
    _lastRotationPivot(node._lastRotationPivot),
    _lastRotationPivotInverse(node._lastRotationPivotInverse),
    _lastHasScalePivot(node._lastHasScalePivot),
    _lastHasRotationPivot(node._lastHasRotationPivot),
#endif
    _holdRendering(node._holdRendering) {
        
    ALLOCATION_TRACKER_ADD(Nodes, 1);
}

VRONode::~VRONode() {
    ALLOCATION_TRACKER_SUB(Nodes, 1);
}

void VRONode::deleteGL() {
    if (_geometry) {
        _geometry->deleteGL();
    }
    for (std::shared_ptr<VRONode> &subnode : _subnodes) {
        subnode->deleteGL();
    }
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
    passert_thread(__func__);
    if (_holdRendering) {
        return;
    }
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold) {
        _geometry->render(elementIndex, material,
                          _worldTransform, _worldInverseTransposeTransform, _computedOpacity,
                          context, driver);
    }
}

void VRONode::render(const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (_holdRendering) {
        return;
    }
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold) {
        for (int i = 0; i < _geometry->getGeometryElements().size(); i++) {
            std::shared_ptr<VROMaterial> &material = _geometry->getMaterialForElement(i);
            if (!material->bindShader(_computedLightsHash, _computedLights, context, driver)) {
                continue;
            }
            material->bindProperties(driver);

            // We render the material if at least one of the following is true:
            //
            // 1. There are lights in the scene that haven't been culled (if there are no lights, then
            //    nothing will be visible! Or,
            // 2. The material is Constant. Constant materials do not need light to be visible. Or,
            // 3. The material is PBR, and we have an active lighting environment. Lighting environments
            //    provide ambient light for PBR materials
            if (!_computedLights.empty() ||
                 material->getLightingModel() == VROLightingModel::Constant ||
                (material->getLightingModel() == VROLightingModel::PhysicallyBased && context.getIrradianceMap() != nullptr)) {


                render(i, material, context, driver);
            }
        }
    }
    
    for (std::shared_ptr<VRONode> &child : _subnodes) {
        child->render(context, driver);
    }
}

void VRONode::renderSilhouettes(std::shared_ptr<VROMaterial> &material,
                                VROSilhouetteMode mode, std::function<bool(const VRONode&)> filter,
                                const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (_holdRendering) {
        return;
    }
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold) {
        if (!filter || filter(*this)) {
            if (mode == VROSilhouetteMode::Flat) {
                _geometry->renderSilhouette(_worldTransform, material, context, driver);
            }
            else {
                for (int i = 0; i < _geometry->getGeometryElements().size(); i++) {
                    std::shared_ptr<VROTexture> texture = _geometry->getMaterialForElement(i)->getDiffuse().getTexture();
                    if (material->getDiffuse().swapTexture(texture)) {
                        if (!material->bindShader(0, {}, context, driver)) {
                            continue;
                        }
                        material->bindProperties(driver);
                    }
                    _geometry->renderSilhouetteTextured(i, _worldTransform, material, context, driver);
                }
            }
        }
    }
    
    for (std::shared_ptr<VRONode> &child : _subnodes) {
        child->renderSilhouettes(material, mode, filter, context, driver);
    }
}

void VRONode::recomputeUmbrellaBoundingBox() {
    VROMatrix4f parentTransform;
    VROMatrix4f parentRotation;
    
    std::shared_ptr<VRONode> parent = getParentNode();
    if (parent) {
        parentTransform = parent->getWorldTransform();
        parentRotation = parent->getWorldRotation();
    }
    
    // Trigger a computeTransform pass to update the node's bounding boxes and as well as its
    // child's node transforms recursively.
    computeTransforms(parentTransform, parentRotation);
}

#pragma mark - Sorting and Transforms

void VRONode::resetDebugSortIndex() {
    sDebugSortIndex = 0;
}

void VRONode::collectLights(std::vector<std::shared_ptr<VROLight>> *outLights) {
    for (std::shared_ptr<VROLight> &light : _lights) {
        light->setTransformedPosition(_worldTransform.multiply(light->getPosition()));
        light->setTransformedDirection(_worldRotation.multiply(light->getDirection()));
        outLights->push_back(light);
    }
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->collectLights(outLights);
    }
}

void VRONode::updateSortKeys(uint32_t depth,
                             VRORenderParameters &params,
                             std::shared_ptr<VRORenderMetadata> &metadata,
                             const VRORenderContext &context,
                             std::shared_ptr<VRODriver> &driver) {
    passert_thread(__func__);
    processActions();

    /*
     If a node is not visible, that means none of its children are visible
     either (we use the umbrella bounding box for visibility tests), so we do
     not have to recurse down.
     */
    if (!_visible) {
        return;
    }
    
    /*
     If rendering is held for this node, do not process any of its children either.
     */
    if (_holdRendering) {
        return;
    }

    std::stack<float> &opacities = params.opacities;
    std::vector<std::shared_ptr<VROLight>> &lights = params.lights;
    std::stack<int> &hierarchyDepths = params.hierarchyDepths;
    std::stack<float> &distancesFromCamera = params.distancesFromCamera;

    /*
     Compute specific parameters for this node.
     */
    _worldInverseTransposeTransform = _worldTransform.invert().transpose();
    _computedOpacity = opacities.top() * _opacity * _opacityFromHiddenFlag;
    opacities.push(_computedOpacity);
    
    _computedLights.clear();
    for (std::shared_ptr<VROLight> &light : lights) {
        if ((light->getInfluenceBitMask() & _lightReceivingBitMask) != 0) {

            // Ambient and Directional lights do not attenuate so do not cull them here
            if (light->getType() == VROLightType::Ambient ||
                light->getType() == VROLightType::Directional ||
                getBoundingBox().getDistanceToPoint(light->getTransformedPosition()) < light->getAttenuationEndDistance()) {
                _computedLights.push_back(light);
            }
        }
    }
    _computedLightsHash = VROLight::hashLights(_computedLights);

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
            distanceFromCamera = _worldBoundingBox.getCenter().distance(context.getCamera().getPosition());

            // TODO Using the bounding box may be preferred but currently leads to more
            //      artifacts
            // distanceFromCamera = _worldBoundingBox.getDistanceToPoint(context.getCamera().getPosition());

            furthestDistanceFromCamera = getBoundingBox().getFurthestDistanceToPoint(context.getCamera().getPosition());

            // Sanity checks to ensure we are not placing objects at inf. If so, clamp it at
            // the last known furthestDistanceFromCamera.
            if (isinf(furthestDistanceFromCamera)) {
                furthestDistanceFromCamera = params.furthestDistanceFromCamera;
            }

            if (isinf(distanceFromCamera)) {
                distanceFromCamera = params.furthestDistanceFromCamera;
            }
        }
        _geometry->updateSortKeys(this, hierarchyId, hierarchyDepth, _computedLightsHash, _computedLights, _computedOpacity,
                                  distanceFromCamera, context.getZFar(), metadata, context, driver);
        
        if (kDebugSortOrder && context.getFrame() % kDebugSortOrderFrameFrequency == 0) {
            pinfo("   [%d] Pushed node with position [%f, %f, %f], rendering order %d, hierarchy depth %d (actual depth %d), distance to camera %f, hierarchy ID %d, lights %d",
                  sDebugSortIndex, _worldPosition.x, _worldPosition.y, _worldPosition.z, _renderingOrder, hierarchyDepth, depth, distanceFromCamera, hierarchyId, _computedLightsHash);
            _geometry->setName(VROStringUtil::toString(sDebugSortIndex));
        }
    }
    else if (kDebugSortOrder && context.getFrame() % kDebugSortOrderFrameFrequency == 0) {
        pinfo("   [%d] Ignored empty node with position [%f, %f, %f] hierarchy depth %d, distance to camera %f, actual depth %d, hierarchy ID %d",
              sDebugSortIndex, _worldPosition.x, _worldPosition.y, _worldPosition.z,
              hierarchyDepth, 0.0, depth, hierarchyId);
    }

    distancesFromCamera.push(distanceFromCamera);
    params.furthestDistanceFromCamera = std::max(params.furthestDistanceFromCamera, furthestDistanceFromCamera);
    sDebugSortIndex++;
    
    /*
     Move down the tree.
     */
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->updateSortKeys(depth + 1, params, metadata, context, driver);
    }
    
    opacities.pop();
    hierarchyDepths.pop();
    distancesFromCamera.pop();
}

void VRONode::getSortKeysForVisibleNodes(std::vector<VROSortKey> *outKeys) {
    passert_thread(__func__);
    
    // Add the geometry of this node, if available
    if (_visible && _geometry && getType() == VRONodeType::Normal) {
        _geometry->getSortKeys(outKeys);
    }
    
    // Search down the scene graph. If a child is a portal or portal frame,
    // stop the search.
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        if (childNode->getType() == VRONodeType::Normal) {
            childNode->getSortKeysForVisibleNodes(outKeys);
        }
    }
}

void VRONode::computeTransforms(VROMatrix4f parentTransform, VROMatrix4f parentRotation) {
    passert_thread(__func__);
    
    // Compute the transform for this node
    doComputeTransform(parentTransform);

    //Compute the rotation for this node
    _worldRotation = parentRotation.multiply(_rotation.getMatrix());

    // Apply the world transform for spatial sounds, if any
    for (std::shared_ptr<VROSound> &sound : _sounds) {
        sound->setTransformedPosition(_worldTransform.multiply(sound->getPosition()));
    }
    
    // Compute the umbrella bounding box for this node
    computeUmbrellaBounds();

    // Recurse down the tree
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->computeTransforms(_worldTransform, _worldRotation);
    }
}

void VRONode::doComputeTransform(VROMatrix4f parentTransform) {
    /*
     Compute the world transform for this node. The full formula is:
     _worldTransform = parentTransform * T * Rpiv * R * Rpiv -1 * Spiv * S * Spiv-1
     */
    _localTransform.toIdentity();
    
    /*
     Scale.
     */
    if (_scalePivot) {
        VROMatrix4f scale;
        scale.scale(_scale.x, _scale.y, _scale.z);
        _localTransform = *_scalePivot * scale * *_scalePivotInverse;
    } else {
        _localTransform.scale(_scale.x, _scale.y, _scale.z);
    }
    
    /*
     Rotation.
     */
    if (_rotationPivot) {
        _localTransform = *_rotationPivotInverse * _localTransform;
    }
    _localTransform = _rotation.getMatrix() * _localTransform;
    if (_rotationPivot) {
        _localTransform = *_rotationPivot * _localTransform;
    }
    
    /*
     Translation.
     */
    VROMatrix4f translate;
    translate.translate(_position.x, _position.y, _position.z);
    _localTransform = translate * _localTransform;

    _worldTransform = parentTransform * _localTransform;
    _worldPosition = { _worldTransform[12], _worldTransform[13], _worldTransform[14] };
    
    if (_geometry) {
        if (_geometry->getInstancedUBO() != nullptr) {
            _worldBoundingBox = _geometry->getInstancedUBO()->getInstancedBoundingBox();
            
            // TODO The local bounding box for particles is not set correctly. Try
            //      simply subtracting the world position.
            _geometryBoundingBox = _worldBoundingBox;
            _localBoundingBox = _worldBoundingBox;
        } else {
            // The local bounding box is the geometry's bounding box multiplied by
            // local tranforms only. The world bounding box is the geometry's bounding box
            // multiplied by the full world transform.
            _geometryBoundingBox = _geometry->getBoundingBox();
            _localBoundingBox    = _geometryBoundingBox.transform(_localTransform);
            _worldBoundingBox    = _geometryBoundingBox.transform(_worldTransform);
        }
    } else {
        // If there is no geometry, then the bounding box should be updated to be a 0 size box at the node's position.
        _geometryBoundingBox.set(_position.x, _position.x, _position.y,
                                 _position.y, _position.z, _position.z);
        _localBoundingBox.set(_position.x, _position.x, _position.y,
                              _position.y, _position.z, _position.z);
        _worldBoundingBox.set(_worldPosition.x, _worldPosition.x, _worldPosition.y,
                              _worldPosition.y, _worldPosition.z, _worldPosition.z);
    }
}

void VRONode::applyConstraints(const VRORenderContext &context, VROMatrix4f parentTransform,
                               bool parentUpdated) {
    
    bool updated = false;
    
    /*
     If a parent's _worldTransform was updated by constraints, we have to recompute
     the transform for this node as well.
     */
    if (parentUpdated) {
        doComputeTransform(parentTransform);
        updated = true;
    }
    
    /*
     Compute constraints for this node.
     */
    for (const std::shared_ptr<VROConstraint> &constraint : _constraints) {
        if (constraint->getConstraintType() == VROConstraintType::Bone) {
            _worldTransform = constraint->getTransform(context, _worldTransform);
        } else {
            VROMatrix4f billboardRotation = constraint->getTransform(context, _worldTransform);

            // To apply the billboard rotation, translate the object to the origin, apply
            // the rotation, then translate back to its previously computed world position.
            // Do not update _worldRotation as it isn't necessary after the afterConstraints() phase
            _worldTransform.translate(_worldPosition.scale(-1));
            _worldTransform = billboardRotation.multiply(_worldTransform);
            _worldTransform.translate(_worldPosition);
        }
        
        updated = true;
    }

    /*
     Move down the tree.
     */
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->applyConstraints(context, _worldTransform, updated);
    }
}

void VRONode::computeIKRig() {
    std::shared_ptr<VROIKRig> rig = getIKRig();
    if (rig != nullptr) {
        rig->processRig();
        return;
    }

    for (auto node : _subnodes) {
        node->computeIKRig();
    }
}

void VRONode::setWorldTransform(VROVector3f finalPosition, VROQuaternion finalRotation, bool animated) {
    // Create a final compute transform representing the desired, final world position and rotation.
    VROVector3f worldScale = getWorldTransform().extractScale();
    VROMatrix4f finalWorldTransform;
    finalWorldTransform.toIdentity();
    finalWorldTransform.scale(worldScale.x, worldScale.y, worldScale.z);
    finalWorldTransform = finalRotation.getMatrix() * finalWorldTransform;
    finalWorldTransform.translate(finalPosition);

    // Calculate local transformations needed to achieve the desired final compute transform
    // by applying: Parent_Trans_INV * FinalCompute = Local_Trans
    VROMatrix4f parentTransform = getParentNode()->getWorldTransform();
    VROMatrix4f currentTransform = parentTransform.invert() * finalWorldTransform;

    if (!animated) {
        _scale = currentTransform.extractScale();
        _position = currentTransform.extractTranslation();
        _rotation = currentTransform.extractRotation(_scale);
    } else {
        // we want this "setWorldTransform" to animate to the new scale/position/rotation. This is
        // slightly problematic because the computeTransforms is recursive, but this is only used
        // for AR's FixedToWorld dragging right now.
        setScale(currentTransform.extractScale());
        setPosition(currentTransform.extractTranslation());
        setRotation(currentTransform.extractRotation(currentTransform.extractScale()));
    }

    // Finally calculate and set the world transform for this node.
    doComputeTransform(parentTransform);
    _worldRotation = parentTransform.multiply(_rotation.getMatrix());
}

#pragma mark - Visibility

void VRONode::updateVisibility(const VRORenderContext &context) {
    const VROFrustum &frustum = context.getCamera().getFrustum();
    VROFrustumResult result = VROFrustumResult::Outside;
    
    // First check for an edge case: if the bounds of the object _enclose_ the
    // camera. This is common for mdoels or effects that surround the user, and
    // our usual frustum test fails to handle this correctly.
    if (_worldUmbrellaBoundingBox.containsPoint(context.getCamera().getPosition())) {
        result = VROFrustumResult::Intersects;
    }
    // Otherwise do the normal frustum test.
    else {
        result = frustum.intersectAllOpt(_worldUmbrellaBoundingBox, &_umbrellaBoxMetadata);
    }
    
    // Process the results of the frustum test, iterating down the tree if there
    // was an intersection, or else wholesale including or excluding all child nodes
    // in the other two cases.
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

void VRONode::computeUmbrellaBounds() {
    // The world umbrella bounding box is the bounding box of this Node in world coordinates,
    // union-ed with the bounding boxes of all children, and their children, etc.
    //
    // The local unbrella bounding box is similar, except its in the coordinate system of
    // the node. This means it is the bounding box of this Node's geometry (with no transforms
    // applied), union-ed with the bounding box of each child's geometry multiplied by
    // each child's transform, union-ed with the bounding box of each grandchild's geometry
    // multiplied by the child and grandchild transforms, etc. Importantly, the transform of
    // _this_ node is not applied.
    bool isSet = false;
    if (_geometry) {
        // Use _geometryBoundingBox instead of _localBoundingBox because we do not apply this Node's transform
        _localUmbrellaBoundingBox = _geometryBoundingBox;
        _worldUmbrellaBoundingBox = _worldBoundingBox;
        isSet = true;
    }
    for (const std::shared_ptr<VRONode> &childNode : _subnodes) {
        if (childNode->computeUmbrellaBounds(&_localUmbrellaBoundingBox, &_worldUmbrellaBoundingBox, VROMatrix4f::identity(), isSet)) {
            isSet = true;
        }
    }
    
    // If the bounds were empty (e.g. no geometry all the way down), then set the
    // bounds to the position.
    if (!isSet) {
        _worldUmbrellaBoundingBox.set(_worldPosition.x, _worldPosition.x, _worldPosition.y,
                                      _worldPosition.y, _worldPosition.z, _worldPosition.z);
        _localUmbrellaBoundingBox.set(0, 0, 0, 0, 0, 0);
    }
}

bool VRONode::computeUmbrellaBounds(VROBoundingBox *localBounds, VROBoundingBox *worldBounds, VROMatrix4f transform, bool isSet) const {
    if (_geometry) {
        if (!isSet) {
            *localBounds = _localBoundingBox.transform(transform);
            *worldBounds = _worldBoundingBox;
            isSet = true;
        } else {
            localBounds->unionDestructive(_localBoundingBox.transform(transform));
            worldBounds->unionDestructive(_worldBoundingBox);
        }
    }
    
    transform = transform * _localTransform;
    
    for (const std::shared_ptr<VRONode> &childNode : _subnodes) {
        if (childNode->computeUmbrellaBounds(localBounds, worldBounds, transform, isSet)) {
            isSet = true;
        }
    }
    return isSet;
}

int VRONode::countVisibleNodes() const {
    int count = _visible ? 1 : 0;
    for (const std::shared_ptr<VRONode> &childNode : _subnodes) {
        count += childNode->countVisibleNodes();
    }
    return count;
}

VROVector3f VRONode::getWorldPosition() const {
    return _worldPosition;
}

VROMatrix4f VRONode::getWorldRotation() const {
    return _worldRotation;
}

VROMatrix4f VRONode::getWorldTransform() const {
    return _worldTransform;
}

VROMatrix4f VRONode::getLastWorldTransform() const {
    return _lastWorldTransform;
}

VROVector3f VRONode::getLastWorldPosition() const {
    return _lastWorldPosition;
}

VROMatrix4f VRONode::getLastWorldRotation() const {
    return _lastWorldRotation;
}

VROVector3f VRONode::getLastLocalPosition() const {
    return _lastPosition;
}

VROQuaternion VRONode::getLastLocalRotation() const {
    return _lastRotation;
}

VROVector3f VRONode::getLastLocalScale() const {
    return _lastScale;
}

VROMatrix4f VRONode::getLastScalePivot() const {
    if (_lastHasRotationPivot) {
        return _lastRotationPivot;
    } else {
        return VROMatrix4f::identity();
    }
}

VROMatrix4f VRONode::getLastRotationPivot() const {
    if (_lastHasRotationPivot) {
        return _lastRotationPivot;
    } else {
        return VROMatrix4f::identity();
    }
}

VROBoundingBox VRONode::getLastWorldUmbrellaBoundingBox() const {
    return _lastWorldUmbrellaBoundingBox;
}

VROBoundingBox VRONode::getLastLocalUmbrellaBoundingBox() const {
    return _lastLocalUmbrellaBoundingBox;
}

VROBoundingBox VRONode::getLastLocalBoundingBox() const {
    return _lastLocalBoundingBox;
}

#pragma mark - Scene Graph

void VRONode::addChildNode(std::shared_ptr<VRONode> node) {
    passert_thread(__func__);
    passert (node);
    
    _subnodes.push_back(node);
    node->_supernode = std::static_pointer_cast<VRONode>(shared_from_this());
    
    /*
     If this node is attached to a VROScene, cascade and assign that scene to
     all children.
     */
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (scene) {
        node->setScene(scene, true);
    }
}

void VRONode::removeFromParentNode() {
    passert_thread(__func__);
    
    std::shared_ptr<VRONode> supernode = _supernode.lock();
    if (supernode) {
        std::vector<std::shared_ptr<VRONode>> &parentSubnodes = supernode->_subnodes;
        parentSubnodes.erase(
                             std::remove_if(parentSubnodes.begin(), parentSubnodes.end(),
                                            [this](std::shared_ptr<VRONode> node) {
                                                return node.get() == this;
                                            }), parentSubnodes.end());
        _supernode.reset();
    }
    
    /*
     Detach this node and all its children from the scene.
     */
    setScene(nullptr, true);
}

std::vector<std::shared_ptr<VRONode>> VRONode::getChildNodes() const {
    return _subnodes;
}

void VRONode::getSkinner(std::vector<std::shared_ptr<VROSkinner>> &skinnersOut, bool recurse) {
    if (_geometry != nullptr && _geometry->getSkinner() != nullptr) {
        skinnersOut.push_back(_geometry->getSkinner());
    }

    if (!recurse) {
        return;
    }

    for (auto child : _subnodes) {
        child->getSkinner(skinnersOut, recurse);
    }
}

void VRONode::setScene(std::shared_ptr<VROScene> scene, bool recursive) {
    /*
     When we detach from a scene, remove any physics bodies from that scene's
     physics world.
     */
    std::shared_ptr<VROScene> currentScene = _scene.lock();
    if (currentScene) {
        if (currentScene->hasPhysicsWorld() && _physicsBody) {
            currentScene->getPhysicsWorld()->removePhysicsBody(_physicsBody);
        }
    }
    
    _scene = scene;
    
    /*
     When we attach to a new scene, add the physics body to the scene's physics
     world.
     */
    if (scene && _physicsBody) {
        scene->getPhysicsWorld()->addPhysicsBody(_physicsBody);
    }
     
    if (recursive) {
        for (std::shared_ptr<VRONode> &node : _subnodes) {
            node->setScene(scene, true);
        }
    }
}

void VRONode::removeAllChildren() {
    std::vector<std::shared_ptr<VRONode>> children = _subnodes;
    for (std::shared_ptr<VRONode> &node : children) {
        node->removeFromParentNode();
    }
}

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

#pragma mark - Rendering Thread Setters

void VRONode::setRotation(VROQuaternion rotation) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationQuaternion>([](VROAnimatable *const animatable, VROQuaternion r) {
                                                         ((VRONode *)animatable)->_rotation = r;
                                                         ((VRONode *)animatable)->_euler = r.toEuler();
                                                     }, _rotation, rotation));
}

void VRONode::setRotationEuler(VROVector3f euler) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f r) {
                                                        ((VRONode *)animatable)->_euler = VROMathNormalizeAngles2PI(r);
                                                        ((VRONode *)animatable)->_rotation = { r.x, r.y, r.z };
                                                     }, _euler, euler));
    
    VROQuaternion rotation = { euler.x, euler.y, euler.z };
}

void VRONode::setPosition(VROVector3f position) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f p) {
                                                        VRONode *node = ((VRONode *)animatable);
                                                        node->_position = p;
                                                        node->notifyTransformUpdate(false);
                                                   }, _position, position));
}

void VRONode::setScale(VROVector3f scale) {
    passert_thread(__func__);
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
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        VRONode *node = ((VRONode *)animatable);
        node->_position.x = p;
        node->notifyTransformUpdate(false);
    }, _position.x, x));
}

void VRONode::setPositionY(float y) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        VRONode *node = ((VRONode *)animatable);
        node->_position.y = p;
        node->notifyTransformUpdate(false);
    }, _position.y, y));
}

void VRONode::setPositionZ(float z) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float p) {
        VRONode *node = ((VRONode *)animatable);
        node->_position.z = p;
        node->notifyTransformUpdate(false);
    }, _position.z, z));
}

void VRONode::setScaleX(float x) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.x = s;
    }, _scale.x, x));
}

void VRONode::setScaleY(float y) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.y = s;
    }, _scale.y, y));
}

void VRONode::setScaleZ(float z) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_scale.z = s;
    }, _scale.z, z));
}

void VRONode::setRotationEulerX(float radians) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.x = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.x, radians));
}

void VRONode::setRotationEulerY(float radians) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.y = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.y, radians));
}

void VRONode::setRotationEulerZ(float radians) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float r) {
        VROVector3f &euler = ((VRONode *) animatable)->_euler;
        euler.z = VROMathNormalizeAngle2PI(r);
        ((VRONode *)animatable)->_rotation = { euler.x, euler.y, euler.z };
    }, _euler.z, radians));
}

void VRONode::setRotationPivot(VROMatrix4f pivot) {
    passert_thread(__func__);
    _rotationPivot = pivot;
    _rotationPivotInverse = pivot.invert();
}

void VRONode::setScalePivot(VROMatrix4f pivot) {
    passert_thread(__func__);
    _scalePivot = pivot;
    _scalePivotInverse = pivot.invert();
}

void VRONode::setOpacity(float opacity) {
    passert_thread(__func__);
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_opacity = s;
    }, _opacity, opacity));
}

void VRONode::setHidden(bool hidden) {
    passert_thread(__func__);
    _hidden = hidden;
    
    float opacity = hidden ? 0.0 : 1.0;
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float s) {
        ((VRONode *)animatable)->_opacityFromHiddenFlag = s;
    }, _opacityFromHiddenFlag, opacity));
}

void VRONode::setHighAccuracyEvents(bool enabled) {
    passert_thread(__func__);
    _highAccuracyEvents = enabled;
}

#pragma mark - Application Thread Setters

void VRONode::setPositionAtomic(VROVector3f position) {
    _lastPosition = position;

    std::weak_ptr<VRONode> node_w = std::dynamic_pointer_cast<VRONode>(shared_from_this());
    VROPlatformDispatchAsyncRenderer([node_w, position] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setPosition(position);
        }
    });
}

void VRONode::setRotationAtomic(VROQuaternion rotation) {
    _lastRotation = rotation;

    std::weak_ptr<VRONode> node_w = std::dynamic_pointer_cast<VRONode>(shared_from_this());
    VROPlatformDispatchAsyncRenderer([node_w, rotation] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setRotation(rotation);
        }
    });
}

void VRONode::setScaleAtomic(VROVector3f scale) {
    _lastScale = scale;

    std::weak_ptr<VRONode> node_w = std::dynamic_pointer_cast<VRONode>(shared_from_this());
    VROPlatformDispatchAsyncRenderer([node_w, scale] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setScale(scale);
        }
    });
}

void VRONode::setRotationPivotAtomic(VROMatrix4f pivot) {
    _lastHasRotationPivot = true;
    _lastRotationPivot = pivot;
    _lastRotationPivotInverse = pivot.invert();

    std::weak_ptr<VRONode> node_w = std::dynamic_pointer_cast<VRONode>(shared_from_this());
    VROPlatformDispatchAsyncRenderer([node_w, pivot] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setRotationPivot(pivot);
        }
    });
}

void VRONode::setScalePivotAtomic(VROMatrix4f pivot) {
    _lastHasScalePivot = true;
    _lastScalePivot = pivot;
    _lastScalePivotInverse = pivot.invert();

    std::weak_ptr<VRONode> node_w = std::dynamic_pointer_cast<VRONode>(shared_from_this());
    VROPlatformDispatchAsyncRenderer([node_w, pivot] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setScalePivot(pivot);
        }
    });
}

void VRONode::computeTransformsAtomic(VROMatrix4f parentTransform, VROMatrix4f parentRotation) {
    VROQuaternion rotation = _lastRotation;
    VROVector3f position = _lastPosition;
    
    // The local transform is aggregated into this matrix
    VROMatrix4f localTransform;
    
    // First compute scale
    VROVector3f scale = _lastScale;
    if (_lastHasScalePivot) {
        VROMatrix4f scaleMatrix;
        scaleMatrix.scale(scale.x, scale.y, scale.z);
        localTransform = _lastScalePivot * scaleMatrix * _lastScalePivotInverse;
    }
    else {
        localTransform.scale(scale.x, scale.y, scale.z);
    }
    
    // Rotation is after scale
    bool hasRotationPivot = _lastHasRotationPivot;
    if (hasRotationPivot) {
        localTransform = _lastRotationPivotInverse * localTransform;
    }
    localTransform = rotation.getMatrix() * localTransform;
    if (hasRotationPivot) {
        localTransform = _lastRotationPivot * localTransform;
    }
    
    // Translation is after scale and rotation
    VROMatrix4f translate;
    translate.translate(position.x, position.y, position.z);
    localTransform = translate * localTransform;
    _lastLocalTransform = localTransform;
    
    // Finally multiply by the parent transform to get the final world transform
    VROMatrix4f worldTransform = parentTransform * localTransform;

    // Store the final values in the atomics
    VROVector3f worldPosition = { worldTransform[12], worldTransform[13], worldTransform[14] };
    _lastWorldPosition = worldPosition;
    _lastWorldRotation = parentRotation.multiply(rotation.getMatrix());
    _lastWorldTransform = worldTransform;
    
    // We have to set the _lastWorldBoundingBox using the latest transform.
    // Because _lastLocalBoundingBox is flawed for particle systems, we check
    // if we have an instanced UBO first.
    if (_geometry && _geometry->getInstancedUBO() != nullptr) {
        _lastWorldBoundingBox = _geometry->getInstancedUBO()->getInstancedBoundingBox();
        _lastLocalBoundingBox = _lastWorldBoundingBox.load(); // TODO This is flawed
    } else {
        VROBoundingBox lastGeometryBounds = _lastGeometryBoundingBox;
        _lastLocalBoundingBox = lastGeometryBounds.transform(localTransform);
        _lastWorldBoundingBox = lastGeometryBounds.transform(worldTransform);
    }
}

void VRONode::startComputeAtomicUmbrellaBounds() {
    _lastUmbrellaBoundsSet = false;
    
    VROBoundingBox box = _lastGeometryBoundingBox;
    if (box.getSpanX() * box.getSpanY() * box.getSpanZ() > kEpsilon) {
        // Use _geometryBoundingBox instead of _localBoundingBox because we do not apply this Node's transform
        _lastLocalUmbrellaBoundingBox = _lastGeometryBoundingBox.load();
        _lastWorldUmbrellaBoundingBox = _lastWorldBoundingBox.load();
        _lastUmbrellaBoundsSet = true;
    }
}

VROMatrix4f VRONode::computeAtomicUmbrellaBounds(std::shared_ptr<VRONode> parentNodeBeingUpdated, VROMatrix4f transform) {
    VROBoundingBox box = _lastGeometryBoundingBox;
    if (box.getSpanX() * box.getSpanY() * box.getSpanZ() > kEpsilon) {
        if (!parentNodeBeingUpdated->_lastUmbrellaBoundsSet) {
            parentNodeBeingUpdated->_lastLocalUmbrellaBoundingBox.store(_lastLocalBoundingBox.load().transform(transform));
            parentNodeBeingUpdated->_lastWorldUmbrellaBoundingBox.store(_lastWorldBoundingBox);
            parentNodeBeingUpdated->_lastUmbrellaBoundsSet = true;
        } else {
            VROBoundingBox currentLocal = parentNodeBeingUpdated->_lastLocalUmbrellaBoundingBox;
            currentLocal.unionDestructive(_lastLocalBoundingBox.load().transform(transform));
            parentNodeBeingUpdated->_lastLocalUmbrellaBoundingBox.store(currentLocal);
            
            VROBoundingBox currentWorld = parentNodeBeingUpdated->_lastWorldUmbrellaBoundingBox;
            currentWorld.unionDestructive(_lastWorldBoundingBox.load());
            parentNodeBeingUpdated->_lastWorldUmbrellaBoundingBox.store(currentWorld);
        }
    }
    
    return transform * _lastLocalTransform;
}

void VRONode::endComputeAtomicUmbrellaBounds() {
    VROBoundingBox box = _lastWorldUmbrellaBoundingBox;
    if (box.getSpanX() * box.getSpanY() * box.getSpanZ() < kEpsilon) {
        VROVector3f lastWorldPosition = _lastWorldPosition;
        
        // If there is no geometry, then the bounding box should be updated to be a 0 size box at the node's position.
        _lastWorldUmbrellaBoundingBox = { lastWorldPosition.x, lastWorldPosition.x,
                                          lastWorldPosition.y, lastWorldPosition.y,
                                          lastWorldPosition.z, lastWorldPosition.z };
        _lastLocalUmbrellaBoundingBox =  { 0, 0, 0, 0, 0, 0 };
    }
}

void VRONode::setLastGeometryBoundingBox(VROBoundingBox bounds) {
    _lastGeometryBoundingBox.store(bounds);
}

#pragma mark - Sync Rendering Thread <> Application Thread

void VRONode::syncAppThreadProperties() {
    std::weak_ptr<VRONode> node_w = std::dynamic_pointer_cast<VRONode>(shared_from_this());

    /*
     The application thread properties are only updated on the application thread
     because there may be operations in-flight on that thread (e.g. coordinate space
     computations) and we don't want to intersect and modify these values off-thread,
     as that can create inconsistencies.

     Since this function is invoked on the rendering thread, first copy all the properties
     to local variables, then copy again via the dispatch function over to the application
     thread.
     
     Because we insist on this, we can consider in the future making these variables
     NOT atomic.
     */
    VROMatrix4f localTransform = _localTransform;
    VROMatrix4f worldTransform = _worldTransform;
    VROVector3f worldPosition = _worldPosition;
    VROMatrix4f worldRotation = _worldRotation;
    VROVector3f position = _position;
    VROQuaternion rotation = _rotation;
    VROVector3f scale = _scale;
    VROBoundingBox worldBoundingBox = _worldBoundingBox;
    VROBoundingBox worldUmbrellaBoundingBox = _worldUmbrellaBoundingBox;
    VROBoundingBox localBoundingBox = _localBoundingBox;
    VROBoundingBox localUmbrellaBoundingBox = _localUmbrellaBoundingBox;
    VROBoundingBox geometryBoundingBox = _geometryBoundingBox;
    
    std::string name = _name;
    
    VROPlatformDispatchAsyncApplication([name, localTransform, worldTransform, worldPosition, worldRotation, position,
                                         rotation, scale, worldBoundingBox, worldUmbrellaBoundingBox,
                                         localBoundingBox, localUmbrellaBoundingBox, geometryBoundingBox, node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->_lastLocalTransform = localTransform;
            node->_lastWorldTransform = worldTransform;
            node->_lastWorldPosition = worldPosition;
            node->_lastWorldRotation = worldRotation;
            node->_lastPosition = position;
            node->_lastRotation = rotation;
            node->_lastScale = scale;
            node->_lastWorldBoundingBox = worldBoundingBox;
            node->_lastWorldUmbrellaBoundingBox = worldUmbrellaBoundingBox;
            node->_lastLocalBoundingBox = localBoundingBox;
            node->_lastLocalUmbrellaBoundingBox = localUmbrellaBoundingBox;
            node->_lastGeometryBoundingBox = geometryBoundingBox;
        }
    });
    for (std::shared_ptr<VRONode> &childNode : _subnodes) {
        childNode->syncAppThreadProperties();
    }
}

#pragma mark - Actions and Animations

void VRONode::processActions() {
    passert_thread(__func__);
    
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
    passert_thread(__func__);
    _actions.push_back(action);
}

void VRONode::removeAction(std::shared_ptr<VROAction> action) {
    passert_thread(__func__);
    _actions.erase(std::remove_if(_actions.begin(), _actions.end(),
                                  [action](std::shared_ptr<VROAction> candidate) {
                                      return candidate == action;
                                  }), _actions.end());
}

void VRONode::removeAllActions() {
    passert_thread(__func__);
    _actions.clear();
}

void VRONode::addAnimation(std::string key, std::shared_ptr<VROExecutableAnimation> animation) {
    passert_thread(__func__);
    std::shared_ptr<VRONode> shared = std::dynamic_pointer_cast<VRONode>(shared_from_this());
    _animations[key].push_back(std::make_shared<VROExecutableNodeAnimation>(shared, animation));
}

void VRONode::removeAnimation(std::string key) {
    passert_thread(__func__);
    auto kv = _animations.find(key);
    if (kv == _animations.end()) {
        return;
    }
    
    for (std::shared_ptr<VROExecutableAnimation> &animation : kv->second) {
        animation->terminate(false);
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

std::shared_ptr<VROExecutableAnimation> VRONode::getLayeredAnimation(std::vector<std::shared_ptr<VROSkeletalAnimationLayer>> layers,
                                                                     bool recursive) {
    for (std::shared_ptr<VROSkeletalAnimationLayer> &layer : layers) {
        layer->animation = getAnimation(layer->name, recursive);
    }
    return VROLayeredSkeletalAnimation::createLayeredAnimation(layers);
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
    passert_thread(__func__);
    for (auto kv : _animations) {
        for (std::shared_ptr<VROExecutableAnimation> &animation : kv.second) {
            animation->terminate(true);
        }
        kv.second.clear();
    }
    _animations.clear();
}

void VRONode::onAnimationFinished() {
    notifyTransformUpdate(true);

    std::shared_ptr<VROPhysicsBody> body = getPhysicsBody();
    if (body){
        body->refreshBody();
    }
}

std::set<std::shared_ptr<VROMorpher>> VRONode::getMorphers(bool recursive) {
    std::set<std::shared_ptr<VROMorpher>> result;
    if (_geometry != nullptr) {
        for (auto morph : _geometry->getMorphers()) {
            result.insert(morph.second);
        }
    }

    if (recursive && getChildNodes().size() > 0) {
        for (auto childNode : _subnodes) {
            std::set<std::shared_ptr<VROMorpher>> subResults = childNode->getMorphers(recursive);
            result.insert(subResults.begin(), subResults.end());
        }
    }

    return result;
}

#pragma mark - Hit Testing

VROBoundingBox VRONode::getBoundingBox() const {
    return _worldBoundingBox;
}

VROBoundingBox VRONode::getUmbrellaBoundingBox() const {
    return _worldUmbrellaBoundingBox;
}

std::vector<VROHitTestResult> VRONode::hitTest(const VROCamera &camera, VROVector3f origin, VROVector3f ray,
                                               bool boundsOnly) {
    passert_thread(__func__);
    std::vector<VROHitTestResult> results;

    VROMatrix4f identity;
    hitTest(camera, origin, ray, boundsOnly, results);
    return results;
}

void VRONode::hitTest(const VROCamera &camera, VROVector3f origin, VROVector3f ray, bool boundsOnly,
                      std::vector<VROHitTestResult> &results) {
    passert_thread(__func__);
    if (!_selectable) {
        return;
    }
    
    VROMatrix4f transform = _worldTransform;
    boundsOnly = boundsOnly && !getHighAccuracyEvents();
    if (_geometry && _computedOpacity > kHiddenOpacityThreshold && _visible) {
        VROVector3f intPt;
        if (getBoundingBox().intersectsRay(ray, origin, &intPt)) {
            if (boundsOnly || hitTestGeometry(origin, ray, transform, &intPt)) {
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

bool VRONode::hitTestGeometry(VROVector3f origin, VROVector3f ray,
                              VROMatrix4f transform, VROVector3f *intPt) {
    passert_thread(__func__);
    std::shared_ptr<VROGeometrySource> vertexSource = _geometry->getGeometrySourcesForSemantic(VROGeometrySourceSemantic::Vertex).front();
    
    bool hit = false;
    float currentDistance = FLT_MAX;
    std::string tag = getTag();
    for (std::shared_ptr<VROGeometryElement> element : _geometry->getGeometryElements()) {
         element->processTriangles([&hit, ray, origin, transform, &intPt, &currentDistance, tag](int index, VROTriangle triangle) {
             VROTriangle transformed = triangle.transformByMatrix(transform);
             VROVector3f intPtGeom;
             if (transformed.intersectsRay(ray, origin, &intPtGeom)) {
                 float distance = intPtGeom.distance(origin);
                 if (distance < currentDistance){
                     currentDistance = distance;
                     *intPt = intPtGeom;
                     hit = true;
                 }

                 //TODO Offer a way to break out of here, as optimization
             }
         }, vertexSource);
    }
    
    return hit;
}

#pragma mark - Constraints

void VRONode::addConstraint(std::shared_ptr<VROConstraint> constraint) {
    passert_thread(__func__);
    _constraints.push_back(constraint);
}

void VRONode::removeConstraint(std::shared_ptr<VROConstraint> constraint) {
    passert_thread(__func__);
    _constraints.erase(std::remove_if(_constraints.begin(), _constraints.end(),
                                  [constraint](std::shared_ptr<VROConstraint> candidate) {
                                      return candidate == constraint;
                                  }), _constraints.end());
}

void VRONode::removeAllConstraints() {
    passert_thread(__func__);
    _constraints.clear();
}

#pragma mark - Physics

std::shared_ptr<VROPhysicsBody> VRONode::initPhysicsBody(VROPhysicsBody::VROPhysicsBodyType type, float mass,
                                                         std::shared_ptr<VROPhysicsShape> shape) {
    std::shared_ptr<VRONode> node = std::static_pointer_cast<VRONode>(shared_from_this());
    _physicsBody = std::make_shared<VROPhysicsBody>(node, type, mass, shape);
    
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (scene) {
        scene->getPhysicsWorld()->addPhysicsBody(_physicsBody);
    }
    return _physicsBody;
}

std::shared_ptr<VROPhysicsBody> VRONode::getPhysicsBody() const {
    return _physicsBody;
}

void VRONode::clearPhysicsBody() {
    if (_physicsBody) {
        std::shared_ptr<VROScene> scene = _scene.lock();
        if (scene && scene->hasPhysicsWorld()) {
            scene->getPhysicsWorld()->removePhysicsBody(_physicsBody);
        }
    }
    _physicsBody = nullptr;
}

#pragma mark - Particle Emitters

void VRONode::updateParticles(const VRORenderContext &context) {
    if (_particleEmitter) {
        // Check if the particle emitter's surface has changed
        if (_geometry != _particleEmitter->getParticleSurface()) {
            _geometry = _particleEmitter->getParticleSurface();
        }
        
        // Update the emitter
        _particleEmitter->update(context, _worldTransform);
    }
    
    // Recurse to children
    for (std::shared_ptr<VRONode> &child : _subnodes) {
        child->updateParticles(context);
    }
}

void VRONode::setParticleEmitter(std::shared_ptr<VROParticleEmitter> emitter) {
    passert_thread(__func__);
    _particleEmitter = emitter;
    _geometry = emitter->getParticleSurface();
    setIgnoreEventHandling(true);
}

void VRONode::removeParticleEmitter() {
    passert_thread(__func__);
    _particleEmitter.reset();
    _geometry.reset();
    setIgnoreEventHandling(false);
}

std::shared_ptr<VROParticleEmitter> VRONode::getParticleEmitter() const {
    return _particleEmitter;
}

#pragma mark - Task Queues

void VRONode::addTaskQueue(std::shared_ptr<VROTaskQueue> queue) {
    _taskQueues.push_back(queue);
}

void VRONode::removeTaskQueue(std::shared_ptr<VROTaskQueue> queue) {
    auto it = std::find(_taskQueues.begin(), _taskQueues.end(), queue);
    if (it != _taskQueues.end()) {
        _taskQueues.erase(it);
    }
}
