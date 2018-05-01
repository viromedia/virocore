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

VRO_METHOD(VRO_REF, nativeCreateARPlane)(VRO_ARGS
                                         VRO_FLOAT minWidth,
                                         VRO_FLOAT minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = std::make_shared<VROARPlaneNode>(minWidth, minHeight);
    return ARPlane::jptr(arPlane);
}

VRO_METHOD(void, nativeDestroyARPlane)(VRO_ARGS
                                       VRO_REF nativeARPlane) {
    delete reinterpret_cast<PersistentRef<VROARPlaneNode> *>(nativeARPlane);
}

VRO_METHOD(VRO_REF, nativeCreateARPlaneDelegate)(VRO_ARGS
                                                 VRO_REF nativeNodeRef) {
    std::shared_ptr<ARPlaneDelegate> delegate = std::make_shared<ARPlaneDelegate>(object, env);
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeNodeRef);
    arPlane->setARNodeDelegate(delegate);
    return ARPlaneDelegate::jptr(delegate);
}

VRO_METHOD(void, nativeDestroyARPlaneDelegate)(VRO_ARGS
                                               VRO_REF delegateRef) {
    delete reinterpret_cast<PersistentRef<ARPlaneDelegate> *>(delegateRef);
}


VRO_METHOD(void, nativeSetMinWidth)(VRO_ARGS
                                    VRO_REF nativeARPlane,
                                    VRO_FLOAT minWidth) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinWidth(minWidth);
}

VRO_METHOD(void, nativeSetMinHeight)(VRO_ARGS
                                     VRO_REF nativeARPlane,
                                     VRO_FLOAT minHeight) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setMinHeight(minHeight);
}

VRO_METHOD(void, nativeSetAnchorId)(VRO_ARGS
                                    VRO_REF nativeARPlane,
                                    VRO_STRING id) {
    std::shared_ptr<VROARPlaneNode> arPlane = ARPlane::native(nativeARPlane);
    arPlane->setId(VRO_STRING_STL(id));
}

VRO_METHOD(void, nativeSetPauseUpdates)(VRO_ARGS
                                        VRO_REF nativeARPlane,
                                        VRO_BOOL pauseUpdates) {
    std::weak_ptr<VROARPlaneNode> arPlane_w = ARPlane::native(nativeARPlane);
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
        VROPlatformCallJavaFunction(localObj, "onAnchorFound", "(Lcom/viro/core/ARAnchor;)V",
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
        VROPlatformCallJavaFunction(localObj, "onAnchorUpdated", "(Lcom/viro/core/ARAnchor;)V",
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
        VROPlatformCallJavaFunction(localObj, "onAnchorRemoved", "()V");
        VRO_DELETE_WEAK_GLOBAL_REF(jObject_w);
    });
}

