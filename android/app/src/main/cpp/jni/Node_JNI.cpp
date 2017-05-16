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

}  // extern "C"
