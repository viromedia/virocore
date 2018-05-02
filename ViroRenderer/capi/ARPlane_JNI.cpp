//
//  ARPlane_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "ARPlane_JNI.h"
#include "ARUtils_JNI.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARPlane_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARPlane_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROARPlaneNode), nativeCreateARPlane)(VRO_ARGS
                                                         VRO_FLOAT minWidth,
                                                         VRO_FLOAT minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = std::make_shared<VROARPlaneNode>(minWidth, minHeight);
    return VRO_REF_NEW(VROARPlaneNode, arPlane);
}

VRO_METHOD(void, nativeDestroyARPlane)(VRO_ARGS
                                       VRO_REF(VROARPlaneNode) nativeARPlane) {
    VRO_REF_DELETE(VROARPlaneNode, nativeARPlane);
}

VRO_METHOD(VRO_REF(ARPlaneDelegate), nativeCreateARPlaneDelegate)(VRO_ARGS
                                                 VRO_REF(VROARPlaneNode) nativeNodeRef) {
    std::shared_ptr<ARPlaneDelegate> delegate = std::make_shared<ARPlaneDelegate>(object, env);
    std::shared_ptr<VROARPlaneNode> arPlane = VRO_REF_GET(VROARPlaneNode, nativeNodeRef);
    arPlane->setARNodeDelegate(delegate);
    return VRO_REF_NEW(ARPlaneDelegate, delegate);
}

VRO_METHOD(void, nativeDestroyARPlaneDelegate)(VRO_ARGS
                                               VRO_REF(ARPlaneDelegate) delegateRef) {
    VRO_REF_DELETE(ARPlaneDelegate, delegateRef);
}


VRO_METHOD(void, nativeSetMinWidth)(VRO_ARGS
                                    VRO_REF(VROARPlaneNode) nativeARPlane,
                                    VRO_FLOAT minWidth) {
    std::shared_ptr<VROARPlaneNode> arPlane = VRO_REF_GET(VROARPlaneNode, nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

VRO_METHOD(void, nativeSetMinHeight)(VRO_ARGS
                                     VRO_REF(VROARPlaneNode) nativeARPlane,
                                     VRO_FLOAT minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = VRO_REF_GET(VROARPlaneNode, nativeARPlane);
    arPlane->setMinHeight(minHeight);
}

VRO_METHOD(void, nativeSetAnchorId)(VRO_ARGS
                                    VRO_REF(VROARPlaneNode) nativeARPlane,
                                    VRO_STRING id) {
    std::shared_ptr<VROARPlaneNode> arPlane = VRO_REF_GET(VROARPlaneNode, nativeARPlane);
    arPlane->setId(VRO_STRING_STL(id));
}

VRO_METHOD(void, nativeSetPauseUpdates)(VRO_ARGS
                                        VRO_REF(VROARPlaneNode) nativeARPlane,
                                        VRO_BOOL pauseUpdates) {
    std::weak_ptr<VROARPlaneNode> arPlane_w = VRO_REF_GET(VROARPlaneNode, nativeARPlane);
    VROPlatformDispatchAsyncRenderer([arPlane_w, pauseUpdates] {
        std::shared_ptr<VROARPlaneNode> arPlane = arPlane_w.lock();
        arPlane->setPauseUpdates(pauseUpdates);
    });
}


} // extern "C"

void ARPlaneDelegate::onARAnchorAttached(std::shared_ptr<VROARAnchor> anchor) {
    std::weak_ptr<VROARPlaneAnchor> planeAnchor_w = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObject_w = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([this, jObject_w, planeAnchor_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObject_w);
        std::shared_ptr<VROARPlaneAnchor> planeAnchor = planeAnchor_w.lock();
        if (VRO_IS_OBJECT_NULL(localObj) || !planeAnchor) {
            return;
        }

        // create the Java ARAnchor POJO
        VRO_OBJECT anchorObj = ARUtilsCreateJavaARAnchorFromAnchor(planeAnchor);

        // Yes, the function in the bridge is onAnchorFound.
        VROPlatformCallHostFunction(localObj, "onAnchorFound", "(Lcom/viro/core/ARAnchor;)V",
                                    anchorObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObject_w);
    });
}

void ARPlaneDelegate::onARAnchorUpdated(std::shared_ptr<VROARAnchor> anchor) {
    std::weak_ptr<VROARPlaneAnchor> planeAnchor_w = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObject_w = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([this, jObject_w, planeAnchor_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObject_w);
        std::shared_ptr<VROARPlaneAnchor> planeAnchor = planeAnchor_w.lock();
        if (VRO_IS_OBJECT_NULL(localObj) || !planeAnchor) {
            return;
        }

        // create the Java ARAnchor POJO
        VRO_OBJECT anchorObj = ARUtilsCreateJavaARAnchorFromAnchor(planeAnchor);

        // Yes, the function in the bridge is onAnchorUpdated.
        VROPlatformCallHostFunction(localObj, "onAnchorUpdated", "(Lcom/viro/core/ARAnchor;)V",
                                    anchorObj);

        VRO_DELETE_WEAK_GLOBAL_REF(jObject_w);
    });
}

void ARPlaneDelegate::onARAnchorRemoved() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObject_w = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObject_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObject_w);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        // Yes, the function in the bridge is onAnchorRemoved.
        VROPlatformCallHostFunction(localObj, "onAnchorRemoved", "()V");
        VRO_DELETE_WEAK_GLOBAL_REF(jObject_w);
    });
}

