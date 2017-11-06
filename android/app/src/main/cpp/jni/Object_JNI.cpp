//
// Object_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include <jni.h>
#include <memory>
#include <VROModelIOUtil.h>
#include "VROOBJLoader.h"
#include "VROFBXLoader.h"
#include "VROGeometry.h"
#include "VRONode.h"
#include "Node_JNI.h"
#include "OBJLoaderDelegate_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Object3D_##method_name

extern "C" {

JNI_METHOD(void, nativeLoadModelFromURL)(JNIEnv *env,
                                         jobject object,
                                         jstring jURL,
                                         jlong node_j,
                                         jboolean isFBX,
                                         jlong requestId) {
    std::string URL = VROPlatformGetString(jURL, env);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(object, env);
    std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish =
            [delegateRef, isFBX, requestId](std::shared_ptr<VRONode> node, bool success) {
                if (!success) {
                    delegateRef->objFailed("Failed to load OBJ");
                }
                else {
                    delegateRef->objLoaded(node, isFBX, requestId);
                }
            };

    // We can run this on the UI thread because it's async, so it will immediately dispatch
    // to the background anyway
    std::shared_ptr<VRONode> node = Node::native(node_j);
    if (isFBX) {
        VROFBXLoader::loadFBXFromResource(URL, VROResourceType::URL, node, true, onFinish);
    }
    else {
        VROOBJLoader::loadOBJFromResource(URL, VROResourceType::URL, node, true, onFinish);
    }
}

JNI_METHOD(void, nativeLoadModelFromResources)(JNIEnv *env,
                                                jobject object,
                                                jstring jresource,
                                                jobject resourceMap,
                                                jlong node_j,
                                                jboolean isFBX,
                                                jlong requestId) {
    std::string resource = VROPlatformGetString(jresource, env);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(object, env);
    std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish =
            [delegateRef, isFBX, requestId](std::shared_ptr<VRONode> node, bool success) {
                if (!success) {
                    delegateRef->objFailed("Failed to load OBJ");
                }
                else {
                    delegateRef->objLoaded(node, isFBX, requestId);
                }
            };

    // We can run this on the UI thread because it's async, so it will immediately dispatch
    // to the background anyway
    std::shared_ptr<VRONode> node = Node::native(node_j);
    if (isFBX) {
        if (resourceMap == nullptr) {
            VROFBXLoader::loadFBXFromResource(resource, VROResourceType::BundledResource, node,
                                              true, onFinish);
        }
        else {
            VROFBXLoader::loadFBXFromResources(resource, VROResourceType::BundledResource, node,
                                               VROPlatformConvertFromJavaMap(resourceMap),
                                               true, onFinish);
        }
    }
    else {
        if (resourceMap == nullptr) {
            VROOBJLoader::loadOBJFromResource(resource, VROResourceType::BundledResource, node,
                                                     true, onFinish);
        }
        else {
            VROOBJLoader::loadOBJFromResources(resource, VROResourceType::BundledResource, node,
                                               VROPlatformConvertFromJavaMap(resourceMap),
                                               true, onFinish);
        }
    }
}

} // extern "C"