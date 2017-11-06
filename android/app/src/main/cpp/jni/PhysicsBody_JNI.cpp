//
// Created by Raj Advani on 10/16/17.
//

#include "PhysicsBody_JNI.h"
#include "Node_JNI.h"
#include "VRONode.h"
#include "VROPhysicsBody.h"
#include "PhysicsDelegate_JNI.h"
#include "VROPlatformUtil.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_PhysicsBody_##method_name

extern "C" {

JNI_METHOD(void, nativeInitPhysicsBody)(JNIEnv *env,
                                        jobject obj,
                                        jlong nativeRef,
                                        jstring bodyTypeStr,
                                        jfloat mass,
                                        jstring shapeTypeStr,
                                        jfloatArray shapeParams) {
    // Get Physics Body type
    std::string strBodyType = VROPlatformGetString(bodyTypeStr, env);
    VROPhysicsBody::VROPhysicsBodyType bodyType = VROPhysicsBody::getBodyTypeForString(strBodyType);

    // Build a VROPhysicsShape if possible
    std::shared_ptr<VROPhysicsShape> propPhysicsShape = nullptr;
    if (shapeTypeStr != NULL) {
        std::string strShapeType = VROPlatformGetString(shapeTypeStr, env);
        VROPhysicsShape::VROShapeType shapeType = VROPhysicsShape::getTypeForString(strShapeType);

        int paramsLength = env->GetArrayLength(shapeParams);
        jfloat *pointArray = env->GetFloatArrayElements(shapeParams, 0);
        std::vector<float> params;
        for (int i = 0; i < paramsLength; i++) {
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
    if (shapeTypeStr != NULL) {
        // Get the shape type
        std::string strShapeType = VROPlatformGetString(shapeTypeStr, env);
        VROPhysicsShape::VROShapeType shapeType = VROPhysicsShape::getTypeForString(strShapeType);

        // Get the shape params
        int paramsLength = env->GetArrayLength(shapeParams);
        jfloat *pointArray = env->GetFloatArrayElements(shapeParams, 0);
        std::vector<float> params;
        for (int i = 0; i < paramsLength; i++) {
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

JNI_METHOD(void, nativeSetPhysicsUseGravity)(JNIEnv *env,
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
                                           jclass clazz,
                                           jstring bodyType,
                                           jfloat mass) {
    // Grab the physics body type
    std::string strBodyType = VROPlatformGetString(bodyType, env);

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
                                            jclass clazz,
                                            jstring shapeType,
                                            jfloatArray shapeParams) {
    // Grab the shape type
    std::string strShapeType = VROPlatformGetString(shapeType, env);

    // Grab the shape params
    int paramsLength = env->GetArrayLength(shapeParams);
    jfloat *pointArray = env->GetFloatArrayElements(shapeParams, 0);
    std::vector<float> params;
    for (int i = 0; i < paramsLength; i++) {
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

JNI_METHOD(void, nativeClearPhysicsDelegate)(JNIEnv *env,
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
}