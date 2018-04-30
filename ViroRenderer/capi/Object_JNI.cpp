//
// Object_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include <memory>
#include <VROModelIOUtil.h>
#include "VROOBJLoader.h"
#include "VROFBXLoader.h"
#include "VROGeometry.h"
#include "VRONode.h"
#include "Node_JNI.h"
#include "OBJLoaderDelegate_JNI.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Object3D_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeLoadModelFromURL)(VRO_ARGS
                                         VRO_STRING jURL,
                                         VRO_REF node_j,
                                         VRO_BOOL isFBX,
                                         VRO_LONG requestId) {
    VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized

    std::string URL = VROPlatformGetString(jURL, env);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(obj, env);
    std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish =
            [delegateRef, isFBX, requestId](std::shared_ptr<VRONode> node, bool success) {
                if (!success) {
                    delegateRef->objFailed("Failed to load OBJ");
                }
                else {
                    delegateRef->objLoaded(node, isFBX, requestId);
                }
            };

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([isFBX, node, URL, onFinish] {
        if (isFBX) {
            VROFBXLoader::loadFBXFromResource(URL, VROResourceType::URL, node, onFinish);
        } else {
            VROOBJLoader::loadOBJFromResource(URL, VROResourceType::URL, node, onFinish);
        }
    });
}

VRO_METHOD(void, nativeLoadModelFromResources)(VRO_ARGS
                                               VRO_STRING jresource,
                                               VRO_OBJECT resourceMap_j,
                                               VRO_REF node_j,
                                               VRO_BOOL isFBX,
                                               VRO_LONG requestId) {
    VROPlatformSetEnv(env); // Invoke in case renderer has not yet initialized

    std::string resource = VROPlatformGetString(jresource, env);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(obj, env);
    std::function<void(std::shared_ptr<VRONode> node, bool success)> onFinish =
            [delegateRef, isFBX, requestId](std::shared_ptr<VRONode> node, bool success) {
                if (!success) {
                    delegateRef->objFailed("Failed to load OBJ");
                }
                else {
                    delegateRef->objLoaded(node, isFBX, requestId);
                }
            };

    std::map<std::string, std::string> resourceMap;
    bool hasResourceMap = false;
    if (!VRO_IS_OBJECT_NULL(resourceMap_j)) {
        resourceMap = VROPlatformConvertFromJavaMap(resourceMap_j);
        hasResourceMap = true;
    }

    std::shared_ptr<VRONode> node = Node::native(node_j);
    VROPlatformDispatchAsyncRenderer([isFBX, resource, hasResourceMap, resourceMap, node, onFinish] {
        if (isFBX) {
            if (!hasResourceMap) {
                VROFBXLoader::loadFBXFromResource(resource, VROResourceType::BundledResource, node,
                                                  onFinish);
            } else {
                VROFBXLoader::loadFBXFromResources(resource, VROResourceType::BundledResource, node,
                                                   resourceMap, onFinish);
            }
        } else {
            if (!hasResourceMap) {
                VROOBJLoader::loadOBJFromResource(resource, VROResourceType::BundledResource, node,
                                                  onFinish);
            } else {
                VROOBJLoader::loadOBJFromResources(resource, VROResourceType::BundledResource, node,
                                                   resourceMap, onFinish);
            }
        }
    });
}

} // extern "C"