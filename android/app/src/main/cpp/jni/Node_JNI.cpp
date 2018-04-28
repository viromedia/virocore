//
//  Node_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <iostream>
#include <memory>
#include <VROBillboardConstraint.h>
#include "VROGeometry.h"
#include "VRONode.h"
#include "PersistentRef.h"
#include "Node_JNI.h"
#include "Material_JNI.h"
#include "Geometry_JNI.h"
#include "VROStringUtil.h"
#include "Camera_JNI.h"
#include "SpatialSound_JNI.h"
#include "Light_JNI.h"
#include "EventDelegate_JNI.h"
#include "PhysicsDelegate_JNI.h"
#include "TransformDelegate_JNI.h"
#include "ParticleEmitter_JNI.h"
#include "ARUtils_JNI.h"
#include "FixedParticleEmitter_JNI.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Node_##method_name
#elif VRO_PLATFORM_WASM
#define VRO_METHOD(return_type, method_name) \
    return_type method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateNode)(VRO_NO_ARGS) {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    return Node::jptr(node);
}

VRO_METHOD(VRO_INT, nativeGetUniqueIdentifier)(VRO_ARGS
                                               VRO_REF node_j) {
    std::shared_ptr<VRONode> node = Node::native(node_j);
    return node->getUniqueID();
}

VRO_METHOD(void, nativeDestroyNode)(VRO_ARGS
                                    VRO_REF native_node_ref) {

    delete reinterpret_cast<PersistentRef<VRONode> *>(native_node_ref);
}

VRO_METHOD(void, nativeAddChildNode)(VRO_ARGS
                                     VRO_REF native_node_ref,
                                     VRO_REF child_node_ref) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    std::shared_ptr<VRONode> childNode = Node::native(child_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, childNode] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->addChildNode(childNode);
        }
    });
}

VRO_METHOD(void, nativeRemoveAllChildNodes)(VRO_ARGS
                                            VRO_REF native_node_ref) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->removeAllChildren();
    });
}

VRO_METHOD(void, nativeRemoveFromParent)(VRO_ARGS
                                         VRO_REF native_node_ref) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeFromParentNode();
        }
    });
}

VRO_METHOD(void, nativeSetTag)(VRO_ARGS
                               VRO_REF native_node_ref,
                               VRO_STRING tag) {
    std::string strBodyType = VROPlatformGetString(tag, env);
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, strBodyType] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setTag(strBodyType);
        }
    });
}

VRO_METHOD(void, nativeSetGeometry)(VRO_ARGS
                                    VRO_REF node_j,
                                    VRO_REF geo_j) {
    std::weak_ptr<VROGeometry> geo_w = Geometry::native(geo_j);
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([geo_w, node_w] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (geo && node) {
            node->setGeometry(geo);
        }
    });
}

VRO_METHOD(void, nativeClearGeometry)(VRO_ARGS
                                      VRO_REF node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setGeometry(nullptr);
        }
    });
}

VRO_METHOD(void, nativeSetFixedParticleEmitter)(VRO_ARGS
                                                VRO_REF node_j,
                                                VRO_REF particle_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    std::weak_ptr<VROFixedParticleEmitter> particle_w = FixedParticleEmitter::native(particle_j);
    VROPlatformDispatchAsyncRenderer([node_w, particle_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        std::shared_ptr<VROFixedParticleEmitter> particle = particle_w.lock();
        if (!node || !particle) {
            return;
        }

        if (node->getParticleEmitter()) {
            node->removeParticleEmitter();
        }

        node->setParticleEmitter(particle);
    });
}

VRO_METHOD(void, nativeSetParticleEmitter)(VRO_ARGS
                                           VRO_REF node_j,
                                           VRO_REF particle_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    std::weak_ptr<VROParticleEmitter> particle_w = ParticleEmitter::native(particle_j);
    VROPlatformDispatchAsyncRenderer([node_w, particle_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        std::shared_ptr<VROParticleEmitter> particle = particle_w.lock();
        if (!node || !particle) {
            return;
        }

        if (node->getParticleEmitter()) {
            node->removeParticleEmitter();
        }

        node->setParticleEmitter(particle);
    });
}

VRO_METHOD(void, nativeRemoveParticleEmitter)(VRO_ARGS
                                              VRO_REF node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeParticleEmitter();
        }
    });
}

VRO_METHOD(void, nativeAddLight)(VRO_ARGS
                                 VRO_REF node_j,
                                 VRO_REF light_j) {
    std::weak_ptr<VROLight> light_w = Light::native(light_j);
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([light_w, node_w] {
        std::shared_ptr<VROLight> light = light_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (light && node) {
            node->addLight(light);
        }
    });
}

VRO_METHOD(void, nativeRemoveLight)(VRO_ARGS
                                    VRO_REF node_j,
                                    VRO_REF light_j) {
    std::weak_ptr<VROLight> light_w = Light::native(light_j);
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([light_w, node_w] {
        std::shared_ptr<VROLight> light = light_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (light && node) {
            node->removeLight(light);
        }
    });
}

VRO_METHOD(void, nativeRemoveAllLights)(VRO_ARGS
                                        VRO_REF node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllLights();
        }
    });
}

VRO_METHOD(void, nativeAddSound)(VRO_ARGS
                                 VRO_REF node_j,
                                 VRO_REF sound_j) {
    std::weak_ptr<VROSoundGVR> sound_w = SpatialSound::native(sound_j);
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([sound_w, node_w] {
        std::shared_ptr<VROSoundGVR> sound = sound_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (sound && node) {
            node->addSound(sound);
        }
    });
}

VRO_METHOD(void, nativeRemoveSound)(VRO_ARGS
                                    VRO_REF node_j,
                                    VRO_REF sound_j) {
    std::weak_ptr<VROSoundGVR> sound_w = SpatialSound::native(sound_j);
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([sound_w, node_w] {
        std::shared_ptr<VROSoundGVR> sound = sound_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (sound && node) {
            node->removeSound(sound);
        }
    });
}

VRO_METHOD(void, nativeRemoveAllSounds)(VRO_ARGS
                                        VRO_REF node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllSounds();
        }
    });
}

VRO_METHOD(void, nativeSetCamera)(VRO_ARGS
                                  VRO_REF node_j,
                                  VRO_REF camera_j) {
    std::weak_ptr<VRONodeCamera> camera_w = Camera::native(camera_j);
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([camera_w, node_w] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (camera && node) {
            node->setCamera(camera);
        }
    });
}

VRO_METHOD(void, nativeClearCamera)(VRO_ARGS
                                    VRO_REF node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setCamera(nullptr);
        }
    });
}

VRO_METHOD(void, nativeSetPosition)(VRO_ARGS
                                    VRO_REF native_node_ref,
                                    VRO_FLOAT positionX,
                                    VRO_FLOAT positionY,
                                    VRO_FLOAT positionZ) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, positionX, positionY, positionZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setPosition({positionX, positionY, positionZ});
        }
    });
}

VRO_METHOD(void, nativeSetRotationEuler)(VRO_ARGS
                                         VRO_REF native_node_ref,
                                         VRO_FLOAT rotationRadiansX,
                                         VRO_FLOAT rotationRadiansY,
                                         VRO_FLOAT rotationRadiansZ) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, rotationRadiansX, rotationRadiansY, rotationRadiansZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setRotation({rotationRadiansX,
                               rotationRadiansY,
                               rotationRadiansZ});
        }
    });
}

VRO_METHOD(void, nativeSetRotationQuaternion)(VRO_ARGS
                                              VRO_REF native_node_ref,
                                              VRO_FLOAT quatX,
                                              VRO_FLOAT quatY,
                                              VRO_FLOAT quatZ,
                                              VRO_FLOAT quatW) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, quatX, quatY, quatZ, quatW] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            VROQuaternion quat(quatX, quatY, quatZ, quatW);
            node->setRotation(quat);
        }
    });
}

VRO_METHOD(void, nativeSetScale)(VRO_ARGS
                                 VRO_REF native_node_ref,
                                 VRO_FLOAT scaleX,
                                 VRO_FLOAT scaleY,
                                 VRO_FLOAT scaleZ) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, scaleX, scaleY, scaleZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setScale({scaleX, scaleY, scaleZ});
        }
    });
}

VRO_METHOD(void, nativeSetRotationPivot)(VRO_ARGS
                                         VRO_REF native_node_ref,
                                         VRO_FLOAT pivotX,
                                         VRO_FLOAT pivotY,
                                         VRO_FLOAT pivotZ) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, pivotX, pivotY, pivotZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            VROMatrix4f pivotMatrix;
            pivotMatrix.translate(pivotX, pivotY, pivotZ);
            node->setRotationPivot(pivotMatrix);
        }
    });
}

VRO_METHOD(void, nativeSetScalePivot)(VRO_ARGS
                                      VRO_REF native_node_ref,
                                      VRO_FLOAT pivotX,
                                      VRO_FLOAT pivotY,
                                      VRO_FLOAT pivotZ) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, pivotX, pivotY, pivotZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            VROMatrix4f pivotMatrix;
            pivotMatrix.translate(pivotX, pivotY, pivotZ);
            node->setScalePivot(pivotMatrix);
        }
    });
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetPosition)(VRO_ARGS
                                               VRO_REF node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalPosition());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetScale)(VRO_ARGS
                                            VRO_REF node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalScale());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetRotationEuler)(VRO_ARGS
                                                    VRO_REF node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalRotation().toEuler());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetRotationQuaternion)(VRO_ARGS
                                                         VRO_REF node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROQuaternion quaternion = node->getLastLocalRotation();

    VRO_FLOAT_ARRAY array_j = VRO_NEW_FLOAT_ARRAY(4);
    float array_c[4];
    array_c[0] = quaternion.X; array_c[1] = quaternion.Y; array_c[2] = quaternion.Z; array_c[3] = quaternion.W;
    VRO_FLOAT_ARRAY_SET(array_j, 0, 4, array_c);

    return array_j;
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetBoundingBox)(VRO_ARGS VRO_REF node_j) {
    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromBoundingBox(node->getLastUmbrellaBoundingBox());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeConvertLocalPositionToWorldSpace)(VRO_ARGS
                                                                    VRO_REF node_j, float x, float y, float z) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROVector3f localPosition(x, y, z);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastWorldTransform().invert().multiply(localPosition));
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeConvertWorldPositionToLocalSpace)(VRO_ARGS
                                                                    VRO_REF node_j, float x, float y, float z) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROVector3f worldPosition(x, y, z);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastWorldTransform().multiply(worldPosition));
}

VRO_METHOD(void, nativeSetOpacity)(VRO_ARGS
                                   VRO_REF native_node_ref,
                                   VRO_FLOAT opacity) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, opacity] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setOpacity(opacity);
        }
    });
}

VRO_METHOD(void, nativeSetLightReceivingBitMask)(VRO_ARGS
                                                 VRO_REF native_node_ref,
                                                 VRO_INT bitMask,
                                                 VRO_BOOL recursive) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, bitMask, recursive] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setLightReceivingBitMask(bitMask, recursive);
        }
    });
}

VRO_METHOD(void, nativeSetShadowCastingBitMask)(VRO_ARGS
                                                VRO_REF native_node_ref,
                                                VRO_INT bitMask,
                                                VRO_BOOL recursive) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, bitMask, recursive] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setShadowCastingBitMask(bitMask, recursive);
        }
    });
}

VRO_METHOD(void, nativeSetHighAccuracyGaze)(VRO_ARGS
                                            VRO_REF native_node_ref,
                                            VRO_BOOL enabled) {
    Node::native(native_node_ref)->setHighAccuracyGaze(enabled);
}

VRO_METHOD(void, nativeSetVisible)(VRO_ARGS
                                   VRO_REF native_node_ref,
                                   VRO_BOOL visible) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, visible] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setHidden(!visible);
        }
    });
}

VRO_METHOD(void, nativeSetDragType)(VRO_ARGS
                                    VRO_REF native_node_ref,
                                    VRO_STRING dragType) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);

    // default type to FixedDistance if we don't recognize the given string
    VRODragType type = VRODragType::FixedDistance;
    std::string dragTypeStr = VROPlatformGetString(dragType, env);

    if (VROStringUtil::strcmpinsensitive(dragTypeStr, "FixedDistance")) {
        type = VRODragType::FixedDistance;
    } else if (VROStringUtil::strcmpinsensitive(dragTypeStr, "FixedToWorld")) {
        type = VRODragType ::FixedToWorld;
    }

    VROPlatformDispatchAsyncRenderer([node_w, type] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setDragType(type);
        }
    });
}

VRO_METHOD(void, nativeSetIgnoreEventHandling)(VRO_ARGS
                                               VRO_REF native_node_ref,
                                               VRO_BOOL ignoreEventHandling) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, ignoreEventHandling] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setIgnoreEventHandling(ignoreEventHandling);
        }
    });
}

VRO_METHOD(void, nativeSetHierarchicalRendering)(VRO_ARGS
                                                 VRO_REF native_node_ref,
                                                 VRO_BOOL hierarchicalRendering) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, hierarchicalRendering] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setHierarchicalRendering(hierarchicalRendering);
        }
    });
}

VRO_METHOD(void, nativeSetTransformBehaviors)(VRO_ARGS
                                              VRO_REF nativeNodeRef,
                                              VRO_ARRAY stringArrayRef) {
    std::vector<std::shared_ptr<VROBillboardConstraint>> tempConstraints;
    int length = VRO_ARRAY_LENGTH(stringArrayRef);

    for (int i = 0; i < length; i++) {
        VRO_STRING string = VRO_STRING_ARRAY_GET(stringArrayRef, i);
        std::string transformBehavior = VROPlatformGetString(string, env);

        // Parse out the constraints and save a copy into tempConstraints
        if (VROStringUtil::strcmpinsensitive(transformBehavior, "billboard")) {
            tempConstraints.push_back(
                    std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        } else if (VROStringUtil::strcmpinsensitive(transformBehavior, "billboardX")) {
            tempConstraints.push_back(
                    std::make_shared<VROBillboardConstraint>(VROBillboardAxis::X));
        } else if (VROStringUtil::strcmpinsensitive(transformBehavior, "billboardY")) {
            tempConstraints.push_back(
                    std::make_shared<VROBillboardConstraint>(VROBillboardAxis::Y));
        }
    }

    // Post set the constraints into the node
    std::weak_ptr<VRONode> node_w = Node::native(nativeNodeRef);
    VROPlatformDispatchAsyncRenderer([node_w, tempConstraints] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllConstraints();
            for (std::shared_ptr<VROBillboardConstraint> constraint : tempConstraints) {
                node->addConstraint(constraint);
            }
        }
    });
}

VRO_METHOD(VRO_ARRAY, nativeGetAnimationKeys)(VRO_ARGS
                                              VRO_REF nativeRef) {

    std::shared_ptr<VRONode> node = Node::native(nativeRef);
    std::set<std::string> keys = node->getAnimationKeys(true);
    VRO_ARRAY array = VRO_NEW_STRING_ARRAY(keys.size());

    int i = 0;
    for (const std::string &key : keys) {
        VRO_STRING_ARRAY_SET(array, i, key);
        ++i;
    }
    return array;
}

VRO_METHOD(void, nativeSetEventDelegate)(VRO_ARGS
                                         VRO_REF nativeRef,
                                         VRO_REF delegateRef) {

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    std::shared_ptr<EventDelegate_JNI> delegate = EventDelegate::native(delegateRef);
    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setEventDelegate(delegate);
        }
    });
}

VRO_METHOD(VRO_REF, nativeSetTransformDelegate)(VRO_ARGS
                                                VRO_REF nativeRef,
                                                VRO_DOUBLE distanceFilter) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    std::shared_ptr<TransformDelegate_JNI> delegate
            = std::make_shared<TransformDelegate_JNI>(obj , distanceFilter);
    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->setTransformDelegate(delegate);
    });
    return TransformDelegate_JNI::jptr(delegate);
}

VRO_METHOD(void, nativeRemoveTransformDelegate)(VRO_ARGS
                                                VRO_REF nativeRef,
                                                VRO_REF delegateRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->setTransformDelegate(nullptr);
    });

    delete reinterpret_cast<PersistentRef<TransformDelegate_JNI> *>(delegateRef);
}


VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetWorldTransform)(VRO_ARGS
                                                     VRO_REF node_j) {
    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromMatrix(node->getLastWorldTransform());
}

VRO_METHOD(void, nativeSetName(VRO_ARGS
                               VRO_REF node_j, VRO_STRING name_j)) {
    std::string name = VROPlatformGetString(name_j, env);
    std::weak_ptr<VRONode> node_w = Node::native(node_j);

    VROPlatformDispatchAsyncRenderer([node_w, name] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (!node) {
            return;
        }
        node->setName(name);
    });
}

}  // extern "C"
