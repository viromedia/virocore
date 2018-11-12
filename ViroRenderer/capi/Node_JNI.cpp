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
#include "ViroUtils_JNI.h"
#include "FixedParticleEmitter_JNI.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Node_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Node_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VRONode), nativeCreateNode)(VRO_NO_ARGS) {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    return VRO_REF_NEW(VRONode, node);
}

VRO_METHOD(VRO_INT, nativeGetUniqueIdentifier)(VRO_ARGS
                                               VRO_REF(VRONode) node_j) {
    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    passert (node != nullptr);
    return node->getUniqueID();
}

VRO_METHOD(void, nativeDestroyNode)(VRO_ARGS
                                    VRO_REF(VRONode) native_node_ref) {

    VRO_REF_DELETE(VRONode, native_node_ref);
}

VRO_METHOD(void, nativeAddChildNode)(VRO_ARGS
                                     VRO_REF(VRONode) native_node_ref,
                                     VRO_REF(VRONode) child_node_ref) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    std::shared_ptr<VRONode> childNode = VRO_REF_GET(VRONode, child_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, childNode] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->addChildNode(childNode);
        }
    });
}

VRO_METHOD(void, nativeRemoveAllChildNodes)(VRO_ARGS
                                            VRO_REF(VRONode) native_node_ref) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllChildren();
        }
    });
}

VRO_METHOD(void, nativeRemoveFromParent)(VRO_ARGS
                                         VRO_REF(VRONode) native_node_ref) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeFromParentNode();
        }
    });
}

VRO_METHOD(void, nativeSetTag)(VRO_ARGS
                               VRO_REF(VRONode) native_node_ref,
                               VRO_STRING tag) {
    std::string strBodyType = VRO_STRING_STL(tag);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, strBodyType] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setTag(strBodyType);
        }
    });
}

VRO_METHOD(void, nativeSetGeometry)(VRO_ARGS
                                    VRO_REF(VRONode) node_j,
                                    VRO_REF(VROGeometry) geo_j) {
    std::weak_ptr<VROGeometry> geo_w = VRO_REF_GET(VROGeometry, geo_j);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([geo_w, node_w] {
        std::shared_ptr<VROGeometry> geo = geo_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (geo && node) {
            node->setGeometry(geo);
        }
    });
}

VRO_METHOD(void, nativeClearGeometry)(VRO_ARGS
                                      VRO_REF(VRONode) node_j) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setGeometry(nullptr);
        }
    });
}

VRO_METHOD(void, nativeSetFixedParticleEmitter)(VRO_ARGS
                                                VRO_REF(VRONode) node_j,
                                                VRO_REF(VROFixedParticleEmitter) particle_j) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    std::weak_ptr<VROFixedParticleEmitter> particle_w = VRO_REF_GET(VROFixedParticleEmitter, particle_j);
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
                                           VRO_REF(VRONode) node_j,
                                           VRO_REF(VROParticleEmitter) particle_j) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    std::weak_ptr<VROParticleEmitter> particle_w = VRO_REF_GET(VROParticleEmitter, particle_j);
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
                                              VRO_REF(VRONode) node_j) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (!node) {
            return;
        }
        if (node->getParticleEmitter()) {
            node->removeParticleEmitter();
        }
    });
}

VRO_METHOD(void, nativeAddLight)(VRO_ARGS
                                 VRO_REF(VRONode) node_j,
                                 VRO_REF(VROLight) light_j) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, light_j);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([light_w, node_w] {
        std::shared_ptr<VROLight> light = light_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (light && node) {
            node->addLight(light);
        }
    });
}

VRO_METHOD(void, nativeRemoveLight)(VRO_ARGS
                                    VRO_REF(VRONode) node_j,
                                    VRO_REF(VROLight) light_j) {
    std::weak_ptr<VROLight> light_w = VRO_REF_GET(VROLight, light_j);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([light_w, node_w] {
        std::shared_ptr<VROLight> light = light_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (light && node) {
            node->removeLight(light);
        }
    });
}

VRO_METHOD(void, nativeRemoveAllLights)(VRO_ARGS
                                        VRO_REF(VRONode) node_j) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllLights();
        }
    });
}

VRO_METHOD(void, nativeAddSound)(VRO_ARGS
                                 VRO_REF(VRONode) node_j,
                                 VRO_REF(VROSoundGVR) sound_j) {
    std::weak_ptr<VROSoundGVR> sound_w = VRO_REF_GET(VROSoundGVR, sound_j);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([sound_w, node_w] {
        std::shared_ptr<VROSoundGVR> sound = sound_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (sound && node) {
            node->addSound(sound);
        }
    });
}

VRO_METHOD(void, nativeRemoveSound)(VRO_ARGS
                                    VRO_REF(VRONode) node_j,
                                    VRO_REF(VROSoundGVR) sound_j) {
    std::weak_ptr<VROSoundGVR> sound_w = VRO_REF_GET(VROSoundGVR, sound_j);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([sound_w, node_w] {
        std::shared_ptr<VROSoundGVR> sound = sound_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (sound && node) {
            node->removeSound(sound);
        }
    });
}

VRO_METHOD(void, nativeRemoveAllSounds)(VRO_ARGS
                                        VRO_REF(VRONode) node_j) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllSounds();
        }
    });
}

VRO_METHOD(void, nativeSetCamera)(VRO_ARGS
                                  VRO_REF(VRONode) node_j,
                                  VRO_REF(VRONodeCamera) camera_j) {
    std::weak_ptr<VRONodeCamera> camera_w = VRO_REF_GET(VRONodeCamera, camera_j);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([camera_w, node_w] {
        std::shared_ptr<VRONodeCamera> camera = camera_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();

        if (camera && node) {
            node->setCamera(camera);
        }
    });
}

VRO_METHOD(void, nativeClearCamera)(VRO_ARGS
                                    VRO_REF(VRONode) node_j) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setCamera(nullptr);
        }
    });
}

VRO_METHOD(void, nativeSetPosition)(VRO_ARGS
                                    VRO_REF(VRONode) native_node_ref,
                                    VRO_FLOAT positionX,
                                    VRO_FLOAT positionY,
                                    VRO_FLOAT positionZ) {
    VRO_REF_GET(VRONode, native_node_ref)->setPositionAtomic({ positionX, positionY, positionZ });
}

VRO_METHOD(void, nativeSetRotationEuler)(VRO_ARGS
                                         VRO_REF(VRONode) native_node_ref,
                                         VRO_FLOAT rotationRadiansX,
                                         VRO_FLOAT rotationRadiansY,
                                         VRO_FLOAT rotationRadiansZ) {
    VRO_REF_GET(VRONode, native_node_ref)->setRotationAtomic({ rotationRadiansX, rotationRadiansY, rotationRadiansZ });
}

VRO_METHOD(void, nativeSetRotationQuaternion)(VRO_ARGS
                                              VRO_REF(VRONode) native_node_ref,
                                              VRO_FLOAT quatX,
                                              VRO_FLOAT quatY,
                                              VRO_FLOAT quatZ,
                                              VRO_FLOAT quatW) {
    VROQuaternion quat(quatX, quatY, quatZ, quatW);
    VRO_REF_GET(VRONode, native_node_ref)->setRotationAtomic(quat);
}

VRO_METHOD(void, nativeSetScale)(VRO_ARGS
                                 VRO_REF(VRONode) native_node_ref,
                                 VRO_FLOAT scaleX,
                                 VRO_FLOAT scaleY,
                                 VRO_FLOAT scaleZ) {
    VRO_REF_GET(VRONode, native_node_ref)->setScaleAtomic({ scaleX, scaleY, scaleZ });
}

VRO_METHOD(void, nativeSetRotationPivot)(VRO_ARGS
                                         VRO_REF(VRONode) native_node_ref,
                                         VRO_FLOAT pivotX,
                                         VRO_FLOAT pivotY,
                                         VRO_FLOAT pivotZ) {
    VROMatrix4f pivotMatrix;
    pivotMatrix.translate(pivotX, pivotY, pivotZ);
    VRO_REF_GET(VRONode, native_node_ref)->setRotationPivotAtomic(pivotMatrix);
}

VRO_METHOD(void, nativeSetScalePivot)(VRO_ARGS
                                      VRO_REF(VRONode) native_node_ref,
                                      VRO_FLOAT pivotX,
                                      VRO_FLOAT pivotY,
                                      VRO_FLOAT pivotZ) {
    VROMatrix4f pivotMatrix;
    pivotMatrix.translate(pivotX, pivotY, pivotZ);
    VRO_REF_GET(VRONode, native_node_ref)->setScalePivotAtomic(pivotMatrix);
}

VRO_METHOD(void, nativeUpdateWorldTransforms)(VRO_ARGS
                                              VRO_REF(VRONode) node_j,
                                              VRO_REF(VRONode) parent_j) {
    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    if (VRO_REF_NULL(parent_j)) {
        node->computeTransformsAtomic(VROMatrix4f::identity(), VROMatrix4f::identity());
    } else {
        std::shared_ptr<VRONode> parent = VRO_REF_GET(VRONode, parent_j);
        node->computeTransformsAtomic(parent->getLastWorldTransform(), parent->getLastWorldRotation());
    }
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetPosition)(VRO_ARGS
                                               VRO_REF(VRONode) node_j) {

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalPosition());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetScale)(VRO_ARGS
                                            VRO_REF(VRONode) node_j) {

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalScale());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetRotationEuler)(VRO_ARGS
                                                    VRO_REF(VRONode) node_j) {

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalRotation().toEuler());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetRotationQuaternion)(VRO_ARGS
                                                         VRO_REF(VRONode) node_j) {

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    VROQuaternion quaternion = node->getLastLocalRotation();

    VRO_FLOAT_ARRAY array_j = VRO_NEW_FLOAT_ARRAY(4);
    float array_c[4];
    array_c[0] = quaternion.X; array_c[1] = quaternion.Y; array_c[2] = quaternion.Z; array_c[3] = quaternion.W;
    VRO_FLOAT_ARRAY_SET(array_j, 0, 4, array_c);

    return array_j;
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetBoundingBox)(VRO_ARGS
                                                  VRO_REF(VRONode) node_j) {
    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    return ARUtilsCreateFloatArrayFromBoundingBox(node->getLastUmbrellaBoundingBox());
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeConvertLocalPositionToWorldSpace)(VRO_ARGS
                                                                    VRO_REF(VRONode) node_j,
                                                                    float x, float y, float z) {

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    VROVector3f localPosition(x, y, z);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastWorldTransform().multiply(localPosition));
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeConvertWorldPositionToLocalSpace)(VRO_ARGS
                                                                    VRO_REF(VRONode) node_j,
                                                                    float x, float y, float z) {

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    VROVector3f worldPosition(x, y, z);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastWorldTransform().invert().multiply(worldPosition));
}

VRO_METHOD(void, nativeSetOpacity)(VRO_ARGS
                                   VRO_REF(VRONode) native_node_ref,
                                   VRO_FLOAT opacity) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, opacity] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setOpacity(opacity);
        }
    });
}

VRO_METHOD(void, nativeSetLightReceivingBitMask)(VRO_ARGS
                                                 VRO_REF(VRONode) native_node_ref,
                                                 VRO_INT bitMask,
                                                 VRO_BOOL recursive) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, bitMask, recursive] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setLightReceivingBitMask(bitMask, recursive);
        }
    });
}

VRO_METHOD(void, nativeSetShadowCastingBitMask)(VRO_ARGS
                                                VRO_REF(VRONode) native_node_ref,
                                                VRO_INT bitMask,
                                                VRO_BOOL recursive) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, bitMask, recursive] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setShadowCastingBitMask(bitMask, recursive);
        }
    });
}

VRO_METHOD(void, nativeSetHighAccuracyEvents)(VRO_ARGS
                                            VRO_REF(VRONode) native_node_ref,
                                            VRO_BOOL enabled) {
    VRO_REF_GET(VRONode, native_node_ref)->setHighAccuracyEvents(enabled);
}

VRO_METHOD(void, nativeSetVisible)(VRO_ARGS
                                   VRO_REF(VRONode) native_node_ref,
                                   VRO_BOOL visible) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, visible] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setHidden(!visible);
        }
    });
}

VRO_METHOD(void, nativeSetRenderingOrder)(VRO_ARGS
                                          VRO_REF(VRONode) native_node_ref,
                                          VRO_INT renderingOrder) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, renderingOrder] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setRenderingOrder(renderingOrder);
        }
    });
}

VRO_METHOD(void, nativeSetDragType)(VRO_ARGS
                                    VRO_REF(VRONode) native_node_ref,
                                    VRO_STRING dragType) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);

    // default type to FixedDistance if we don't recognize the given string
    VRODragType type = VRODragType::FixedDistance;
    std::string dragTypeStr = VRO_STRING_STL(dragType);
    if (VROStringUtil::strcmpinsensitive(dragTypeStr, "FixedDistance")) {
        type = VRODragType::FixedDistance;
    } else if (VROStringUtil::strcmpinsensitive(dragTypeStr, "FixedDistanceOrigin")) {
        type = VRODragType::FixedDistanceOrigin;
    } else if (VROStringUtil::strcmpinsensitive(dragTypeStr, "FixedToWorld")) {
        type = VRODragType::FixedToWorld;
    } else if (VROStringUtil::strcmpinsensitive(dragTypeStr, "FixedToPlane")) {
        type = VRODragType::FixedToPlane;
    }

    VROPlatformDispatchAsyncRenderer([node_w, type] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setDragType(type);
        }
    });
}

VRO_METHOD(void, nativeSetDragPlanePoint)(VRO_ARGS
                                          VRO_REF(VRONode) native_node_ref,
                                          VRO_FLOAT_ARRAY planePoint) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);

    VROVector3f planePointVec;
    VRO_FLOAT *planePointArr = VRO_FLOAT_ARRAY_GET_ELEMENTS(planePoint);
    planePointVec.x = planePointArr[0];
    planePointVec.y = planePointArr[1];
    planePointVec.z = planePointArr[2];
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(planePoint, planePointArr);

    VROPlatformDispatchAsyncRenderer([node_w, planePointVec] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setDragPlanePoint(planePointVec);
        }
    });
}

VRO_METHOD(void, nativeSetDragPlaneNormal)(VRO_ARGS
                                           VRO_REF(VRONode) native_node_ref,
                                           VRO_FLOAT_ARRAY planeNormal) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);

    VROVector3f planeNormalVec;
    VRO_FLOAT *planeNormalArr = VRO_FLOAT_ARRAY_GET_ELEMENTS(planeNormal);
    planeNormalVec.x = planeNormalArr[0];
    planeNormalVec.y = planeNormalArr[1];
    planeNormalVec.z = planeNormalArr[2];
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(planeNormal, planeNormalArr);

    VROPlatformDispatchAsyncRenderer([node_w, planeNormalVec] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setDragPlaneNormal(planeNormalVec);
        }
    });
}

VRO_METHOD(void, nativeSetDragMaxDistance)(VRO_ARGS
                                           VRO_REF(VRONode) native_node_ref,
                                           VRO_FLOAT maxDistance) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);

    VROPlatformDispatchAsyncRenderer([node_w, maxDistance] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setDragMaxDistance(maxDistance);
        }
    });
}

VRO_METHOD(void, nativeSetIgnoreEventHandling)(VRO_ARGS
                                               VRO_REF(VRONode) native_node_ref,
                                               VRO_BOOL ignoreEventHandling) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, ignoreEventHandling] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setIgnoreEventHandling(ignoreEventHandling);
        }
    });
}

VRO_METHOD(void, nativeSetHierarchicalRendering)(VRO_ARGS
                                                 VRO_REF(VRONode) native_node_ref,
                                                 VRO_BOOL hierarchicalRendering) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, hierarchicalRendering] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setHierarchicalRendering(hierarchicalRendering);
        }
    });
}

VRO_METHOD(void, nativeSetTransformBehaviors)(VRO_ARGS
                                              VRO_REF(VRONode) nativeNodeRef,
                                              VRO_STRING_ARRAY stringArrayRef) {
    VRO_METHOD_PREAMBLE;
    std::vector<std::shared_ptr<VROBillboardConstraint>> tempConstraints;
    int length = VRO_ARRAY_LENGTH(stringArrayRef);

    for (int i = 0; i < length; i++) {
        VRO_STRING string = VRO_STRING_ARRAY_GET(stringArrayRef, i);
        std::string transformBehavior = VRO_STRING_STL(string);

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
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, nativeNodeRef);
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

VRO_METHOD(VRO_STRING_ARRAY, nativeGetAnimationKeys)(VRO_ARGS
                                                     VRO_REF(VRONode) nativeRef) {

    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, nativeRef);
    std::set<std::string> keys = node->getAnimationKeys(true);
    VRO_STRING_ARRAY array = VRO_NEW_STRING_ARRAY(keys.size());

    int i = 0;
    for (const std::string &key : keys) {
        VRO_STRING_ARRAY_SET(array, i, key);
        ++i;
    }
    return array;
}

VRO_METHOD(void, nativeSetEventDelegate)(VRO_ARGS
                                         VRO_REF(VRONode) nativeRef,
                                         VRO_REF(EventDelegate_JNI) delegateRef) {

    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, nativeRef);
    std::shared_ptr<EventDelegate_JNI> delegate = VRO_REF_GET(EventDelegate_JNI, delegateRef);
    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setEventDelegate(delegate);
        }
    });
}

VRO_METHOD(VRO_REF(TransformDelegate_JNI), nativeSetTransformDelegate)(VRO_ARGS
                                                                       VRO_REF(VRONode) nativeRef,
                                                                       VRO_DOUBLE distanceFilter) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, nativeRef);
    std::shared_ptr<TransformDelegate_JNI> delegate = std::make_shared<TransformDelegate_JNI>(obj, distanceFilter);
    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->setTransformDelegate(delegate);
    });
    return VRO_REF_NEW(TransformDelegate_JNI, delegate);
}

VRO_METHOD(void, nativeRemoveTransformDelegate)(VRO_ARGS
                                                VRO_REF(VRONode) nativeRef,
                                                VRO_REF(TransformDelegate_JNI) delegateRef) {
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->setTransformDelegate(nullptr);
    });

    VRO_REF_DELETE(TransformDelegate_JNI, delegateRef);
}


VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetWorldTransform)(VRO_ARGS
                                                     VRO_REF(VRONode) node_j) {
    std::shared_ptr<VRONode> node = VRO_REF_GET(VRONode, node_j);
    return ARUtilsCreateFloatArrayFromMatrix(node->getLastWorldTransform());
}

VRO_METHOD(void, nativeSetName(VRO_ARGS
                               VRO_REF(VRONode) node_j,
                               VRO_STRING name_j)) {
    VRO_METHOD_PREAMBLE;
    std::string name = VRO_STRING_STL(name_j);
    std::weak_ptr<VRONode> node_w = VRO_REF_GET(VRONode, node_j);

    VROPlatformDispatchAsyncRenderer([node_w, name] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (!node) {
            return;
        }
        node->setName(name);
    });
}

}  // extern "C"
