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
  Node::native(native_node_ref)->removeFromParentNode();
  delete reinterpret_cast<PersistentRef<VRONode> *>(native_node_ref);
}

JNI_METHOD(void, nativeAddChildNode)(JNIEnv *env,
                             jobject obj,
                             jlong native_node_ref,
                             jlong child_node_ref) {
    Node::native(native_node_ref)->addChildNode(Node::native(child_node_ref));
}

JNI_METHOD(void, nativeRemoveFromParent)(JNIEnv *env,
                             jobject obj,
                             jlong native_node_ref) {
    Node::native(native_node_ref)->removeFromParentNode();
}

JNI_METHOD(void, nativeSetPosition)(JNIEnv *env,
                                    jobject obj,
                                    jlong native_node_ref,
                                    jfloat positionX,
                                    jfloat positionY,
                                    jfloat positionZ) {
    Node::native(native_node_ref)->setPosition({positionX, positionY, positionZ});
}

JNI_METHOD(void, nativeSetRotation)(JNIEnv *env,
                                    jobject obj,
                                    jlong native_node_ref,
                                    jfloat rotationDegreesX,
                                    jfloat rotationDegreesY,
                                    jfloat rotationDegreesZ) {
    Node::native(native_node_ref)->setRotation({toRadians(rotationDegreesX),
                                                toRadians(rotationDegreesY),
                                                toRadians(rotationDegreesZ)});
}

JNI_METHOD(void, nativeSetScale)(JNIEnv *env,
                                    jobject obj,
                                    jlong native_node_ref,
                                    jfloat scaleX,
                                    jfloat scaleY,
                                    jfloat scaleZ) {
    Node::native(native_node_ref)->setScale({scaleX, scaleY, scaleZ});
}

JNI_METHOD(void, nativeSetOpacity)(JNIEnv *env,
                                 jobject obj,
                                 jlong native_node_ref,
                                 jfloat opacity) {
    Node::native(native_node_ref)->setOpacity(opacity);
}

JNI_METHOD(void, nativeSetVisible)(JNIEnv *env,
                                   jobject obj,
                                   jlong native_node_ref,
                                   jfloat opacity) {
    Node::native(native_node_ref)->setOpacity(opacity);
}

JNI_METHOD(void, nativeSetMaterials)(JNIEnv *env,
                                     jobject obj,
                                     jlong nativeNodeRef,
                                     jlongArray longArrayRef) {
    std::shared_ptr<VROGeometry> geometryPtr = Node::native(nativeNodeRef)->getGeometry();

    jlong *longArray = env->GetLongArrayElements(longArrayRef, 0);
    jsize len = env->GetArrayLength(longArrayRef);

    if (geometryPtr != nullptr) {
        std::vector<std::shared_ptr<VROMaterial>> tempMaterials;
        for (int i = 0; i < len; i++) {
            tempMaterials.push_back(Material::native(longArray[i]));
        }

        geometryPtr->getMaterials() = tempMaterials;
    }

    env->ReleaseLongArrayElements(longArrayRef, longArray, 0);
}

JNI_METHOD(void, nativeSetTransformBehaviors)(JNIEnv *env,
                                              jobject obj,
                                              jlong nativeNodeRef,
                                              jobjectArray stringArrayRef) {
    int length = env->GetArrayLength(stringArrayRef);

    Node::native(nativeNodeRef)->removeAllConstraints();

    for (int i = 0; i < length; i++) {
        jstring string = (jstring) (env->GetObjectArrayElement(stringArrayRef, i));
        const char *rawString = env->GetStringUTFChars(string, NULL);
        std::string transformBehavior(rawString);

        if (VROStringUtil::strcmpinsensitive(transformBehavior, "billboard")) {
            Node::native(nativeNodeRef)->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        } else if (VROStringUtil::strcmpinsensitive(transformBehavior, "billboardX")) {
            Node::native(nativeNodeRef)->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::X));
        } else if (VROStringUtil::strcmpinsensitive(transformBehavior, "billboardY")) {
            Node::native(nativeNodeRef)->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::Y));
        } else if (VROStringUtil::strcmpinsensitive(transformBehavior, "billboardZ")) {
            Node::native(nativeNodeRef)->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::Z));
        }
        env->ReleaseStringUTFChars(string, rawString);
    }
    env->DeleteLocalRef(stringArrayRef);
}

}  // extern "C"
