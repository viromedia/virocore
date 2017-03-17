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

    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->removeFromParentNode();
        }
    });

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
                geometryPtr->getMaterials() = tempMaterials;
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

}  // extern "C"
