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
#include "VROStringUtil.h"
#include "EventDelegate_JNI.h"
#include "PhysicsDelegate_JNI.h"
#include "TransformDelegate_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_NodeJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateNode)(JNIEnv *env,
                                    jclass clazz) {
    std::shared_ptr<VRONode> node = std::make_shared<VRONode>();
    return Node::jptr(node);
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
    const char *cStrTag = env->GetStringUTFChars(tag, NULL);
    std::string strBodyType(cStrTag);
    env->ReleaseStringUTFChars(tag, cStrTag);

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, strBodyType] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setTag(strBodyType);
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

JNI_METHOD(void, nativeSetRotation)(JNIEnv *env,
                                    jobject obj,
                                    jlong native_node_ref,
                                    jfloat rotationDegreesX,
                                    jfloat rotationDegreesY,
                                    jfloat rotationDegreesZ) {
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w, rotationDegreesX, rotationDegreesY, rotationDegreesZ] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->setRotation({toRadians(rotationDegreesX),
                               toRadians(rotationDegreesY),
                               toRadians(rotationDegreesZ)});
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

JNI_METHOD(void, nativeSetMaterials)(JNIEnv *env,
                                     jobject obj,
                                     jlong nativeNodeRef,
                                     jlongArray longArrayRef) {
    jlong *longArray = env->GetLongArrayElements(longArrayRef, 0);
    jsize len = env->GetArrayLength(longArrayRef);

    std::vector<std::shared_ptr<VROMaterial>> tempMaterials;
    for (int i = 0; i < len; i++) {
        // Always copy materials from the material manager, as they may be
        // modified by animations, etc. and we don't want these changes to
        // propagate to the reference material held by the material manager
        tempMaterials.push_back(std::make_shared<VROMaterial>(Material::native(longArray[i])));
    }

    std::weak_ptr<VRONode> node_w = Node::native(nativeNodeRef);
    VROPlatformDispatchAsyncRenderer([node_w, tempMaterials] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            std::shared_ptr<VROGeometry> geometryPtr = node->getGeometry();
            if (geometryPtr != nullptr) {
                geometryPtr->setMaterials(tempMaterials);
            }
        }
    });

    env->ReleaseLongArrayElements(longArrayRef, longArray, 0);
}

JNI_METHOD(void, nativeSetTransformBehaviors)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeNodeRef,
                                              jobjectArray stringArrayRef) {
    std::vector<std::shared_ptr<VROBillboardConstraint>> tempConstraints;
    int length = env->GetArrayLength(stringArrayRef);

    for (int i = 0; i < length; i++) {
        jstring string = (jstring) (env->GetObjectArrayElement(stringArrayRef, i));
        const char *rawString = env->GetStringUTFChars(string, NULL);
        std::string transformBehavior(rawString);

        // Parse out the constratins and save a copy into tempConstraints
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
        env->ReleaseStringUTFChars(string, rawString);
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

JNI_METHOD(jobject, nativeSetEventDelegate)(JNIEnv *env,
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

JNI_METHOD(void, nativeInitPhysicsBody)(JNIEnv *env,
                                         jobject obj,
                                         jlong nativeRef,
                                         jstring bodyTypeStr,
                                         jfloat mass,
                                         jstring shapeTypeStr,
                                         jfloatArray shapeParams) {
    // Get Physics Body type
    const char *cStrBodyType = env->GetStringUTFChars(bodyTypeStr, NULL);
    std::string strBodyType(cStrBodyType);
    VROPhysicsBody::VROPhysicsBodyType bodyType = VROPhysicsBody::getBodyTypeForString(strBodyType);
    env->ReleaseStringUTFChars(bodyTypeStr, cStrBodyType);

    // Build a VROPhysicsShape if possible
    std::shared_ptr<VROPhysicsShape> propPhysicsShape = nullptr;
    if (shapeTypeStr != NULL) {
        const char *cStrShapeType = env->GetStringUTFChars(shapeTypeStr, NULL);
        std::string strShapeType(cStrShapeType);
        VROPhysicsShape::VROShapeType shapeType = VROPhysicsShape::getTypeForString(strShapeType);
        env->ReleaseStringUTFChars(shapeTypeStr, cStrShapeType);

        int paramsLength = env->GetArrayLength(shapeParams);
        jfloat *pointArray = env->GetFloatArrayElements(shapeParams, 0);
        std::vector<float> params;
        for (int i = 0; i < paramsLength; i ++) {
            params.push_back(pointArray[i]);
        }
        env->ReleaseFloatArrayElements(shapeParams, pointArray, 0);
        propPhysicsShape = propPhysicsShape = std::make_shared<VROPhysicsShape>(shapeType, params);
    }

    // Create a VROPhysicsBody within VRONode.
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, bodyType, mass, propPhysicsShape] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->initPhysicsBody(bodyType, mass, propPhysicsShape);
        }
    });
}

JNI_METHOD(void, nativeClearPhysicsBody)(JNIEnv *env,
                                 jobject obj,
                                 jlong nativeRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->clearPhysicsBody();
        }
    });
}

JNI_METHOD(void, nativeSetPhysicsShape)(JNIEnv *env,
                                   jobject obj,
                                   jlong nativeRef,
                                   jstring shapeTypeStr,
                                   jfloatArray shapeParams) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);

    // Build a VROPhysicsShape if possible
    std::shared_ptr<VROPhysicsShape> propPhysicsShape = nullptr;
    if (shapeTypeStr != NULL){
        // Get the shape type
        const char *cStrShapeType = env->GetStringUTFChars(shapeTypeStr, NULL);
        std::string strShapeType(cStrShapeType);
        VROPhysicsShape::VROShapeType shapeType = VROPhysicsShape::getTypeForString(strShapeType);
        env->ReleaseStringUTFChars(shapeTypeStr, cStrShapeType);

        // Get the shape params
        int paramsLength = env->GetArrayLength(shapeParams);
        jfloat *pointArray = env->GetFloatArrayElements(shapeParams, 0);
        std::vector<float> params;
        for (int i = 0; i < paramsLength; i ++) {
            params.push_back(pointArray[i]);
        }
        env->ReleaseFloatArrayElements(shapeParams, pointArray, 0);

        // Construct a VROPhysicsShape
        propPhysicsShape = propPhysicsShape = std::make_shared<VROPhysicsShape>(shapeType, params);
    }

    // Set the built VROPhysicsShape on the node's physics body
    VROPlatformDispatchAsyncRenderer([node_w, propPhysicsShape] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setPhysicsShape(propPhysicsShape);
        }
    });
}


JNI_METHOD(void, nativeSetPhysicsMass)(JNIEnv *env,
                                   jobject obj,
                                   jlong nativeRef,
                                   jfloat mass) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, mass] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setMass(mass);
        }
    });
}


JNI_METHOD(void, nativeSetPhysicsInertia)(JNIEnv *env,
                                 jobject obj,
                                 jlong nativeRef,
                                 jfloatArray inertiaArray) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    jfloat *inertia = env->GetFloatArrayElements(inertiaArray, 0);
    VROVector3f vectorInertia = VROVector3f(inertia[0], inertia[1], inertia[2]);
    env->ReleaseFloatArrayElements(inertiaArray, inertia, 0);

    VROPlatformDispatchAsyncRenderer([node_w, vectorInertia] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setInertia(vectorInertia);
        }
    });
}

JNI_METHOD(void, nativeSetPhysicsFriction)(JNIEnv *env,
                                 jobject obj,
                                 jlong nativeRef,
                                 jfloat friction) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, friction] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setFriction(friction);
        }
    });
}

JNI_METHOD(void, nativeSetPhysicsRestitution)(JNIEnv *env,
                                        jobject obj,
                                        jlong nativeRef,
                                        jfloat restitution) {

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, restitution] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setRestitution(restitution);
        }
    });
}

JNI_METHOD(void, nativeSetPhysicsEnabled)(JNIEnv *env,
                                        jobject obj,
                                        jlong nativeRef,
                                        jboolean enabled) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, enabled] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setIsSimulated(enabled);
        }
    });
}

JNI_METHOD(void, nativeSetPhsyicsUseGravity)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeRef,
                                              jboolean useGravity) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, useGravity] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setUseGravity(useGravity);
        }
    });
}

JNI_METHOD(void, nativeClearPhysicsForce)(JNIEnv *env,
                                             jobject obj,
                                             jlong nativeRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->clearForces();
        }
    });
}

JNI_METHOD(void, nativeApplyPhysicsForce)(JNIEnv *env,
                                       jobject obj,
                                       jlong nativeRef,
                                       jfloatArray forceArray,
                                       jfloatArray positionArray) {
    // Grab the physics force to be applied
    jfloat *force = env->GetFloatArrayElements(forceArray, 0);
    VROVector3f vectorForce = VROVector3f(force[0], force[1], force[2]);
    env->ReleaseFloatArrayElements(forceArray, force, 0);

    // Grab the position at which to apply the force at
    jfloat *position = env->GetFloatArrayElements(positionArray, 0);
    VROVector3f vectorPosition = VROVector3f(position[0], position[1], position[2]);
    env->ReleaseFloatArrayElements(positionArray, force, 0);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, vectorForce, vectorPosition] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyForce(vectorForce, vectorPosition);
        }
    });
}


JNI_METHOD(void, nativeApplyPhysicsTorque)(JNIEnv *env,
                                       jobject obj,
                                       jlong nativeRef,
                                       jfloatArray torqueArray) {
    jfloat *torque = env->GetFloatArrayElements(torqueArray, 0);
    VROVector3f vectorTorque = VROVector3f(torque[0], torque[1], torque[2]);
    env->ReleaseFloatArrayElements(torqueArray, torque, 0);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, vectorTorque] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyTorque(vectorTorque);
        }
    });
}


JNI_METHOD(void, nativeApplyPhysicsImpulse)(JNIEnv *env,
                                       jobject obj,
                                       jlong nativeRef,
                                       jfloatArray forceArray,
                                       jfloatArray positionArray) {
    // Grab the physics impulse to be applied
    jfloat *force = env->GetFloatArrayElements(forceArray, 0);
    VROVector3f vectorForce = VROVector3f(force[0], force[1], force[2]);
    env->ReleaseFloatArrayElements(forceArray, force, 0);

    // Grab the position at which to apply the impulse at
    jfloat *jPos = env->GetFloatArrayElements(positionArray, 0);
    VROVector3f vectorPos = VROVector3f(jPos[0], jPos[1], jPos[2]);
    env->ReleaseFloatArrayElements(positionArray, jPos, 0);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, vectorForce, vectorPos] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyImpulse(vectorForce, vectorPos);
        }
    });
}


JNI_METHOD(void, nativeApplyPhysicsTorqueImpulse)(JNIEnv *env,
                                       jobject obj,
                                       jlong nativeRef,
                                       jfloatArray torqueArray) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    jfloat *torque = env->GetFloatArrayElements(torqueArray, 0);
    VROVector3f vectorTorque = VROVector3f(torque[0], torque[1], torque[2]);
    env->ReleaseFloatArrayElements(torqueArray, torque, 0);

    VROPlatformDispatchAsyncRenderer([node_w, vectorTorque] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyTorqueImpulse(vectorTorque);
        }
    });
}

JNI_METHOD(jstring, nativeIsValidBodyType)(JNIEnv *env,
                                             jobject obj,
                                             jstring bodyType,
                                             jfloat mass) {
    // Grab the physics body type
    const char *cStrBodyType = env->GetStringUTFChars(bodyType, NULL);
    std::string strBodyType(cStrBodyType);
    env->ReleaseStringUTFChars(bodyType, cStrBodyType);

    // Verify if the physics body type is valid and return
    std::string errorMsg;
    bool isValid = VROPhysicsBody::isValidType(strBodyType, mass, errorMsg);
    if (isValid) {
        return nullptr;
    } else {
        return env->NewStringUTF(errorMsg.c_str());
    }
}

JNI_METHOD(jstring, nativeIsValidShapeType)(JNIEnv *env,
                                             jobject obj,
                                             jstring shapeType,
                                             jfloatArray shapeParams) {
    // Grab the shape type
    const char *cStrShapeType = env->GetStringUTFChars(shapeType, NULL);
    std::string strShapeType(cStrShapeType);
    env->ReleaseStringUTFChars(shapeType, cStrShapeType);

    // Grab the shape params
    int paramsLength = env->GetArrayLength(shapeParams);
    jfloat *pointArray = env->GetFloatArrayElements(shapeParams, 0);
    std::vector<float> params;
    for (int i = 0; i < paramsLength; i ++) {
        params.push_back(pointArray[i]);
    }
    env->ReleaseFloatArrayElements(shapeParams, pointArray, 0);

    // Verify if the shape type and params are valid and return
    std::string errorMsg;
    bool isValid = VROPhysicsShape::isValidShape(strShapeType, params, errorMsg);
    if (isValid) {
        return nullptr;
    } else {
        return env->NewStringUTF(errorMsg.c_str());
    }
}

JNI_METHOD(jlong, nativeSetPhysicsDelegate)(JNIEnv *env,
                                             jobject obj,
                                             jlong nativeRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    std::shared_ptr<PhysicsDelegate_JNI> delegate = std::make_shared<PhysicsDelegate_JNI>(obj);

    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setPhysicsDelegate(delegate);
        }
    });

    return PhysicsDelegate_JNI::jptr(delegate);
}

JNI_METHOD(jlong, nativeClearPhysicsDelegate)(JNIEnv *env,
                                            jobject obj,
                                            jlong nativeRef,
                                            jlong delegateRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setPhysicsDelegate(nullptr);
        }
    });
    delete reinterpret_cast<PersistentRef<PhysicsDelegate_JNI> *>(delegateRef);
}


JNI_METHOD(void, nativeSetPhysicsVelocity)(JNIEnv *env,
                                           jobject obj,
                                           jlong nativeRef,
                                           jfloatArray velocityArray,
                                           jboolean isConstant) {
    jfloat *jVelocity = env->GetFloatArrayElements(velocityArray, 0);
    VROVector3f velocity = VROVector3f(jVelocity[0], jVelocity[1], jVelocity[2]);
    env->ReleaseFloatArrayElements(velocityArray, jVelocity, 0);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, velocity, isConstant] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setVelocity(velocity, isConstant);
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

}  // extern "C"
