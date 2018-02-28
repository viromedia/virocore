//
//  Node_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <iostream>
#include <jni.h>
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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Node_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateNode)(JNIEnv *env,
                                    jclass clazz) {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    return Node::jptr(node);
}

JNI_METHOD(jint , nativeGetUniqueIdentifier)(JNIEnv *env,
                                             jobject obj,
                                             jlong node_j) {
    std::shared_ptr<VRONode> node = Node::native(node_j);
    return node->getUniqueID();
}

JNI_METHOD(void, nativeDestroyNode)(JNIEnv *env,
                                    jclass clazz,
                                    jlong native_node_ref) {

    delete reinterpret_cast<PersistentRef<VRONode> *>(native_node_ref);
}

JNI_METHOD(void, nativeAddChildNode)(JNIEnv *env,
                                     jobject obj,
                                     jlong native_node_ref,
                                     jlong child_node_ref) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    std::shared_ptr<VRONode> childNode = Node::native(child_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, childNode] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->addChildNode(childNode);
        }
    });
}

JNI_METHOD(void, nativeRemoveAllChildNodes)(JNIEnv *env,
                                            jobject obj,
                                            jlong native_node_ref) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->removeAllChildren();
    });
}

JNI_METHOD(void, nativeRemoveFromParent)(JNIEnv *env,
                                         jobject obj,
                                         jlong native_node_ref) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeFromParentNode();
        }
    });
}

JNI_METHOD(void, nativeSetTag)(JNIEnv *env,
                                    jobject obj,
                                    jlong native_node_ref,
                                    jstring tag) {
    std::string strBodyType = VROPlatformGetString(tag, env);
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, strBodyType] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setTag(strBodyType);
        }
    });
}

JNI_METHOD(void, nativeSetGeometry)(JNIEnv *env,
                                    jobject obj,
                                    jlong node_j,
                                    jlong geo_j) {
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

JNI_METHOD(void, nativeClearGeometry)(JNIEnv *env,
                                      jobject obj,
                                      jlong node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setGeometry(nullptr);
        }
    });
}

JNI_METHOD(void, nativeSetFixedParticleEmitter)(JNIEnv *env,
                                           jclass clazz,
                                           jlong node_j,
                                           jlong particle_j) {
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

JNI_METHOD(void, nativeSetParticleEmitter)(JNIEnv *env,
                                           jclass clazz,
                                           jlong node_j,
                                           jlong particle_j) {
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

JNI_METHOD(void, nativeRemoveFixedParticleEmitter)(JNIEnv *env,
                                                   jclass clazz,
                                                   jlong node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeParticleEmitter();
        }
    });
}

JNI_METHOD(void, nativeRemoveParticleEmitter)(JNIEnv *env,
                                              jclass clazz,
                                              jlong node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeParticleEmitter();
        }
    });
}

JNI_METHOD(void, nativeAddLight)(JNIEnv *env,
                                 jobject obj,
                                 jlong node_j,
                                 jlong light_j) {
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

JNI_METHOD(void, nativeRemoveLight)(JNIEnv *env,
                                    jobject obj,
                                    jlong node_j,
                                    jlong light_j) {
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

JNI_METHOD(void, nativeRemoveAllLights)(JNIEnv *env,
                                        jobject obj,
                                        jlong node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllLights();
        }
    });
}

JNI_METHOD(void, nativeAddSound)(JNIEnv *env,
                                 jobject obj,
                                 jlong node_j,
                                 jlong sound_j) {
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

JNI_METHOD(void, nativeRemoveSound)(JNIEnv *env,
                                    jobject obj,
                                    jlong node_j,
                                    jlong sound_j) {
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

JNI_METHOD(void, nativeRemoveAllSounds)(JNIEnv *env,
                                        jobject obj,
                                        jlong node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeAllSounds();
        }
    });
}

JNI_METHOD(void, nativeSetCamera)(JNIEnv *env,
                                  jobject obj,
                                  jlong node_j,
                                  jlong camera_j) {
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

JNI_METHOD(void, nativeClearCamera)(JNIEnv *env,
                                    jobject obj,
                                    jlong node_j) {
    std::weak_ptr<VRONode> node_w = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setCamera(nullptr);
        }
    });
}

JNI_METHOD(void, nativeSetPosition)(JNIEnv *env,
                                    jobject obj,
                                    jlong native_node_ref,
                                    jfloat positionX,
                                    jfloat positionY,
                                    jfloat positionZ) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, positionX, positionY, positionZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setPosition({positionX, positionY, positionZ});
        }
    });
}

JNI_METHOD(void, nativeSetRotationEuler)(JNIEnv *env,
                                         jobject obj,
                                         jlong native_node_ref,
                                         jfloat rotationRadiansX,
                                         jfloat rotationRadiansY,
                                         jfloat rotationRadiansZ) {
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

JNI_METHOD(void, nativeSetRotationQuaternion)(JNIEnv *env,
                                              jobject obj,
                                              jlong native_node_ref,
                                              jfloat quatX,
                                              jfloat quatY,
                                              jfloat quatZ,
                                              jfloat quatW) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, quatX, quatY, quatZ, quatW] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            VROQuaternion quat(quatX, quatY, quatZ, quatW);
            node->setRotation(quat);
        }
    });
}

JNI_METHOD(void, nativeSetScale)(JNIEnv *env,
                                 jobject obj,
                                 jlong native_node_ref,
                                 jfloat scaleX,
                                 jfloat scaleY,
                                 jfloat scaleZ) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, scaleX, scaleY, scaleZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setScale({scaleX, scaleY, scaleZ});
        }
    });
}

JNI_METHOD(void, nativeSetRotationPivot)(JNIEnv *env,
                                         jobject obj,
                                         jlong native_node_ref,
                                         jfloat pivotX,
                                         jfloat pivotY,
                                         jfloat pivotZ) {

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

JNI_METHOD(void, nativeSetScalePivot)(JNIEnv *env,
                                      jobject obj,
                                      jlong native_node_ref,
                                      jfloat pivotX,
                                      jfloat pivotY,
                                      jfloat pivotZ) {

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

JNI_METHOD(jfloatArray, nativeGetPosition)(JNIEnv *env,
                                           jobject obj,
                                           jlong node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalPosition());
}

JNI_METHOD(jfloatArray, nativeGetScale)(JNIEnv *env,
                                        jobject obj,
                                        jlong node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalScale());
}

JNI_METHOD(jfloatArray, nativeGetRotationEuler)(JNIEnv *env,
                                                jobject obj,
                                                jlong node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastLocalRotation().toEuler());
}

JNI_METHOD(jfloatArray, nativeGetRotationQuaternion)(JNIEnv *env,
                                                     jobject obj,
                                                     jlong node_j) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROQuaternion quaternion = node->getLastLocalRotation();

    jfloatArray array_j = env->NewFloatArray(4);
    jfloat array_c[4];
    array_c[0] = quaternion.X; array_c[1] = quaternion.Y; array_c[2] = quaternion.Z; array_c[3] = quaternion.W;
    env->SetFloatArrayRegion(array_j, 0, 4, array_c);
    return array_j;
}

JNI_METHOD(jfloatArray, nativeGetBoundingBox)(JNIEnv *env, jobject obj, jlong node_j) {
    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromBoundingBox(node->getLastUmbrellaBoundingBox());
}

JNI_METHOD(jfloatArray, nativeConvertLocalPositionToWorldSpace)(JNIEnv *env,
                                                                 jobject obj,
                                                                 jlong node_j, float x, float y, float z) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROVector3f localPosition(x, y, z);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastWorldTransform().invert().multiply(localPosition));
}

JNI_METHOD(jfloatArray, nativeConvertWorldPositionToLocalSpace)(JNIEnv *env,
                                                                jobject obj,
                                                                jlong node_j, float x, float y, float z) {

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROVector3f worldPosition(x, y, z);
    return ARUtilsCreateFloatArrayFromVector3f(node->getLastWorldTransform().multiply(worldPosition));
}

JNI_METHOD(void, nativeSetOpacity)(JNIEnv *env,
                                   jobject obj,
                                   jlong native_node_ref,
                                   jfloat opacity) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, opacity] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setOpacity(opacity);
        }
    });
}

JNI_METHOD(void, nativeSetLightReceivingBitMask)(JNIEnv *env,
                                                  jobject obj,
                                                  jlong native_node_ref,
                                                  jint bitMask,
                                                  jboolean recursive) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, bitMask, recursive] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setLightReceivingBitMask(bitMask, recursive);
        }
    });
}

JNI_METHOD(void, nativeSetShadowCastingBitMask)(JNIEnv *env,
                                                 jobject obj,
                                                 jlong native_node_ref,
                                                 jint bitMask,
                                                 jboolean recursive) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, bitMask, recursive] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setShadowCastingBitMask(bitMask, recursive);
        }
    });
}

JNI_METHOD(void, nativeSetHighAccuracyGaze)(JNIEnv *env,
                                            jobject obj,
                                            jlong native_node_ref,
                                            jboolean enabled) {
    Node::native(native_node_ref)->setHighAccuracyGaze(enabled);
}

JNI_METHOD(void, nativeSetVisible)(JNIEnv *env,
                                   jobject obj,
                                   jlong native_node_ref,
                                   jboolean visible) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, visible] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setHidden(!visible);
        }
    });
}

JNI_METHOD(void, nativeSetDragType)(JNIEnv *env,
                                    jobject obj,
                                    jlong native_node_ref,
                                    jstring dragType) {

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

JNI_METHOD(void, nativeSetIgnoreEventHandling)(JNIEnv *env,
                                   jobject obj,
                                   jlong native_node_ref,
                                   jboolean ignoreEventHandling) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, ignoreEventHandling] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setIgnoreEventHandling(ignoreEventHandling);
        }
    });
}

JNI_METHOD(void, nativeSetHierarchicalRendering)(JNIEnv *env,
                                                 jobject obj,
                                                 jlong native_node_ref,
                                                 jboolean hierarchicalRendering) {

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, hierarchicalRendering] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setHierarchicalRendering(hierarchicalRendering);
        }
    });
}

JNI_METHOD(void, nativeSetTransformBehaviors)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeNodeRef,
                                              jobjectArray stringArrayRef) {
    std::vector<std::shared_ptr<VROBillboardConstraint>> tempConstraints;
    int length = env->GetArrayLength(stringArrayRef);

    for (int i = 0; i < length; i++) {
        jstring string = (jstring) (env->GetObjectArrayElement(stringArrayRef, i));
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
    env->DeleteLocalRef(stringArrayRef);

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

JNI_METHOD(jobjectArray, nativeGetAnimationKeys)(JNIEnv *env,
                                                 jobject obj,
                                                 jlong nativeRef) {

    std::shared_ptr<VRONode> node = Node::native(nativeRef);
    std::set<std::string> keys = node->getAnimationKeys(true);
    jobjectArray array = (jobjectArray) env->NewObjectArray(keys.size(),
                                                            env->FindClass("java/lang/String"),
                                                            env->NewStringUTF(""));

    int i = 0;
    for (const std::string &key : keys) {
        jstring jkey = env->NewStringUTF(key.c_str());
        env->SetObjectArrayElement(array, i, jkey);
        ++i;

        env->DeleteLocalRef(jkey);
    }
    return array;
}

JNI_METHOD(void, nativeSetEventDelegate)(JNIEnv *env,
                                         jobject obj,
                                         jlong nativeRef,
                                         jlong delegateRef) {

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    std::shared_ptr<EventDelegate_JNI> delegate = EventDelegate::native(delegateRef);
    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setEventDelegate(delegate);
        }
    });
}

JNI_METHOD(jlong, nativeSetTransformDelegate)(JNIEnv *env,
                                           jobject obj,
                                           jlong nativeRef,
                                           jdouble distanceFilter) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    std::shared_ptr<TransformDelegate_JNI> delegate
            = std::make_shared<TransformDelegate_JNI>(obj , distanceFilter);
    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->setTransformDelegate(delegate);
    });
    return TransformDelegate_JNI::jptr(delegate);
}

JNI_METHOD(void, nativeRemoveTransformDelegate)(JNIEnv *env,
                                           jobject obj,
                                           jlong nativeRef,
                                           jlong delegateRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        node->setTransformDelegate(nullptr);
    });

    delete reinterpret_cast<PersistentRef<TransformDelegate_JNI> *>(delegateRef);
}


JNI_METHOD(jfloatArray, nativeGetWorldTransform)(JNIEnv *env,
                                        jobject obj,
                                        jlong node_j) {
    std::shared_ptr<VRONode> node = Node::native(node_j);
    return ARUtilsCreateFloatArrayFromMatrix(node->getLastWorldTransform());
}

}  // extern "C"
