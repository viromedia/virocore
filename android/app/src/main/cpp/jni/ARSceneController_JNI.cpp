//
//  ARSceneController_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "ARSceneController_JNI.h"
#include "ARPlane_JNI.h"
#include "Node_JNI.h"
#include "ARUtils_JNI.h"
#include <VROPlatformUtil.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ARScene_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARSceneController) (JNIEnv *env,
                                        jclass clazz) {
    std::shared_ptr<VROARSceneController> arSceneController = std::make_shared<VROARSceneController>();
    return ARSceneController::jptr(arSceneController);
}

JNI_METHOD(jlong, nativeCreateARSceneDelegate) (JNIEnv *env,
                                               jobject object,
                                               jlong arSceneControllerPtr) {
    std::shared_ptr<ARSceneDelegate> delegate = std::make_shared<ARSceneDelegate>(object, env);
    std::shared_ptr<VROARScene> arScene =
            std::dynamic_pointer_cast<VROARScene>(ARSceneController::native(arSceneControllerPtr)->getScene());
    arScene->setDelegate(delegate);
    return ARSceneDelegate::jptr(delegate);
}

JNI_METHOD(void, nativeDestroyARSceneController) (JNIEnv *env,
                                        jobject object,
                                        jlong arSceneControllerPtr) {
    delete reinterpret_cast<PersistentRef<VROARSceneController> *>(arSceneControllerPtr);
}

JNI_METHOD(void, nativeDestroyARSceneDelegate) (JNIEnv *env,
                                                jobject object,
                                                jlong arSceneDelegatePtr) {
    delete reinterpret_cast<PersistentRef<VROARSceneDelegate> *>(arSceneDelegatePtr);
}

JNI_METHOD(void, nativeAddARPlane) (JNIEnv *env,
                                    jobject object,
                                    jlong arSceneControllerPtr,
                                    jlong arPlanePtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARPlaneNode> arPlane_w = ARPlane::native(arPlanePtr);

    VROPlatformDispatchAsyncRenderer([arPlane_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARPlaneNode> arPlane = arPlane_w.lock();

        if (arScene && arPlane) {
            arScene->addARPlane(arPlane);
        }
    });
}

JNI_METHOD(void, nativeUpdateARPlane) (JNIEnv *env,
                                       jobject object,
                                       jlong arSceneControllerPtr,
                                       jlong arPlanePtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARPlaneNode> arPlane_w = ARPlane::native(arPlanePtr);

    VROPlatformDispatchAsyncRenderer([arPlane_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARPlaneNode> arPlane = arPlane_w.lock();

        if (arScene && arPlane) {
            arScene->updateARPlane(arPlane);
        }
    });
}

JNI_METHOD(void, nativeRemoveARPlane) (JNIEnv *env,
                                       jobject object,
                                       jlong arSceneControllerPtr,
                                       jlong arPlanePtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARPlaneNode> arPlane_w = ARPlane::native(arPlanePtr);

    VROPlatformDispatchAsyncRenderer([arPlane_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARPlaneNode> arPlane = arPlane_w.lock();

        if (arScene && arPlane) {
            arScene->removeARPlane(arPlane);
        }
    });
}

}  // extern "C"

void ARSceneDelegate::onTrackingInitialized() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onTrackingInitialized", "()V");
        env->DeleteLocalRef(localObj);
    });
}

void ARSceneDelegate::onAmbientLightUpdate(float ambientLightIntensity, float colorTemperature) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, ambientLightIntensity, colorTemperature] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onAmbientLightUpdate", "(FF)V",
                                    ambientLightIntensity, colorTemperature);
        env->DeleteLocalRef(localObj);
    });
}

void ARSceneDelegate::onAnchorFound(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);

        VROPlatformCallJavaFunction(localObj, "onAnchorFound", "(Lcom/viro/renderer/ARAnchor;)V",
                                    janchor);
        env->DeleteLocalRef(localObj);
    });
}

void ARSceneDelegate::onAnchorUpdated(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);

        VROPlatformCallJavaFunction(localObj, "onAnchorUpdated", "(Lcom/viro/renderer/ARAnchor;)V",
                                    janchor);
        env->DeleteLocalRef(localObj);
    });
}

void ARSceneDelegate::onAnchorRemoved(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);

        VROPlatformCallJavaFunction(localObj, "onAnchorRemoved", "(Lcom/viro/renderer/ARAnchor;)V",
                                    janchor);
        env->DeleteLocalRef(localObj);
    });
}
