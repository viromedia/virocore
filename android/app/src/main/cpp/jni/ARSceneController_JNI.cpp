//
//  ARSceneController_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//


#include "ARSceneController_JNI.h"
#include "ARDeclarativePlane_JNI.h"
#include "ARDeclarativeNode_JNI.h"
#include "VROARImperativeSession.h"
#include "Node_JNI.h"
#include "ARUtils_JNI.h"
#include "ARNode_JNI.h"
#include "Surface_JNI.h"
#include "ARImageTarget_JNI.h"
#include <VROPlatformUtil.h>
#include <VROARImageTargetAndroid.h>

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARScene_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateARSceneController)(VRO_NO_ARGS) {
    std::shared_ptr<VROARSceneController> arSceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(arSceneController->getScene());
    scene->initImperativeSession();

    return ARSceneController::jptr(arSceneController);
}

VRO_METHOD(VRO_REF, nativeCreateARSceneControllerDeclarative)(VRO_NO_ARGS) {
    std::shared_ptr<VROARSceneController> arSceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(arSceneController->getScene());
    scene->initDeclarativeSession();

    return ARSceneController::jptr(arSceneController);
}

VRO_METHOD(VRO_REF, nativeCreateARSceneDelegate)(VRO_ARGS
                                                 VRO_REF arSceneControllerPtr) {
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(ARSceneController::native(arSceneControllerPtr)->getScene());
    if (arScene->getDeclarativeSession()) {
        std::shared_ptr<ARDeclarativeSceneDelegate> delegate = std::make_shared<ARDeclarativeSceneDelegate>(obj, env);
        arScene->setDelegate(delegate);
        arScene->getDeclarativeSession()->setDelegate(delegate);

        return ARDeclarativeSceneDelegate::jptr(delegate);
    }
    else {
        passert (arScene->getImperativeSession() != nullptr);
        std::shared_ptr<ARImperativeSceneDelegate> delegate = std::make_shared<ARImperativeSceneDelegate>(obj, env);
        arScene->setDelegate(delegate);
        arScene->getImperativeSession()->setDelegate(delegate);

        return ARImperativeSceneDelegate::jptr(delegate);
    }
}

VRO_METHOD(void, nativeDestroyARSceneDelegate)(VRO_ARGS
                                               VRO_REF arSceneDelegatePtr) {
    delete reinterpret_cast<PersistentRef<VROARSceneDelegate> *>(arSceneDelegatePtr);
}

VRO_METHOD(void, nativeDisplayPointCloud)(VRO_ARGS
                                          VRO_REF arSceneControllerPtr,
                                          jboolean displayPointCloud) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w, displayPointCloud] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->displayPointCloud(displayPointCloud);
        }
    });
}

VRO_METHOD(void, nativeResetPointCloudSurface)(VRO_ARGS
                                               VRO_REF arSceneControllerPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->resetPointCloudSurface();
        }
    });
}

VRO_METHOD(void, nativeSetPointCloudSurface)(VRO_ARGS
                                             VRO_REF arSceneControllerPtr,
                                             VRO_REF pointCloudSurface) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROSurface> surface_w = Surface::native(pointCloudSurface);
    VROPlatformDispatchAsyncRenderer([arScene_w, surface_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROSurface> surface = surface_w.lock();

        if (arScene && surface) {
            arScene->setPointCloudSurface(surface);
        }
    });
}

VRO_METHOD(void, nativeSetPointCloudSurfaceScale)(VRO_ARGS
                                                  VRO_REF arSceneControllerPtr,
                                                  jfloat scaleX, jfloat scaleY, jfloat scaleZ) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w, scaleX, scaleY, scaleZ] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->setPointCloudSurfaceScale({scaleX, scaleY, scaleZ});
        }
    });
}

VRO_METHOD(void, nativeSetPointCloudMaxPoints)(VRO_ARGS
                                               VRO_REF arSceneControllerPtr,
                                               jint maxPoints) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w, maxPoints] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->setPointCloudMaxPoints(maxPoints);
        }
    });
}

VRO_METHOD(void, nativeSetAnchorDetectionTypes)(JNIEnv *env, VRO_REF sceneRef, jobjectArray typeStrArray) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(sceneRef)->getScene());
    std::set<VROAnchorDetection> types;

    int stringCount = env->GetArrayLength(typeStrArray);
    for (int i = 0; i < stringCount; i++) {
        std::string typeString = VROPlatformGetString((jstring) env->GetObjectArrayElement(typeStrArray, i), env);
        if (VROStringUtil::strcmpinsensitive(typeString, "None")) {
            types.insert(VROAnchorDetection::None);
        } else if (VROStringUtil::strcmpinsensitive(typeString, "PlanesHorizontal")) {
            types.insert(VROAnchorDetection::PlanesHorizontal);
        } else if (VROStringUtil::strcmpinsensitive(typeString, "PlanesVertical")) {
            types.insert(VROAnchorDetection::PlanesVertical);
        }
    }

    VROPlatformDispatchAsyncRenderer([arScene_w, types] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        if (arScene) {
            arScene->setAnchorDetectionTypes(types);
        }
    });

}

VRO_METHOD(void, nativeAddARNode)(VRO_ARGS
                                  VRO_REF scene_j,
                                  VRO_REF node_j) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(scene_j)->getScene());
    std::weak_ptr<VROARDeclarativeNode> node_w = ARDeclarativeNode::native(node_j);

    VROPlatformDispatchAsyncRenderer([node_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARDeclarativeNode> node = node_w.lock();

        if (arScene && node) {
            arScene->getDeclarativeSession()->addARNode(node);
        }
    });
}

VRO_METHOD(void, nativeUpdateARNode)(VRO_ARGS
                                     VRO_REF scene_j,
                                     VRO_REF node_j) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(scene_j)->getScene());
    std::weak_ptr<VROARDeclarativeNode> node_w = ARDeclarativeNode::native(node_j);

    VROPlatformDispatchAsyncRenderer([node_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARDeclarativeNode> node = node_w.lock();

        if (arScene && node) {
            arScene->getDeclarativeSession()->updateARNode(node);
        }
    });
}

VRO_METHOD(void, nativeRemoveARNode)(VRO_ARGS
                                     VRO_REF arSceneControllerPtr,
                                     VRO_REF arPlanePtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARDeclarativeNode> arPlane_w = ARDeclarativeNode::native(arPlanePtr);

    VROPlatformDispatchAsyncRenderer([arPlane_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARDeclarativeNode> node = arPlane_w.lock();

        if (arScene && node) {
            arScene->getDeclarativeSession()->removeARNode(node);
        }
    });
}

VRO_METHOD(void, nativeAddARImageTarget)(VRO_ARGS
                                         VRO_REF arSceneControllerPtr,
                                         VRO_REF arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTargetAndroid> arImageTarget_w = ARImageTarget::native(arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTargetAndroid> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getImperativeSession()->addARImageTarget(arImageTarget);
        }
    });
}

VRO_METHOD(void, nativeRemoveARImageTarget)(VRO_ARGS
                                            VRO_REF arSceneControllerPtr,
                                            VRO_REF arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTargetAndroid> arImageTarget_w = ARImageTarget::native(arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTargetAndroid> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getImperativeSession()->removeARImageTarget(arImageTarget);
        }
    });
}

VRO_METHOD(void, nativeAddARImageTargetDeclarative)(VRO_ARGS
                                                    VRO_REF arSceneControllerPtr,
                                                    VRO_REF arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTargetAndroid> arImageTarget_w = ARImageTarget::native(arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTargetAndroid> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getDeclarativeSession()->addARImageTarget(arImageTarget);
        }
    });
}

VRO_METHOD(void, nativeRemoveARImageTargetDeclarative)(VRO_ARGS
                                                       VRO_REF arSceneControllerPtr,
                                                       VRO_REF arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            ARSceneController::native(arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTargetAndroid> arImageTarget_w = ARImageTarget::native(arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTargetAndroid> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getDeclarativeSession()->removeARImageTarget(arImageTarget);
        }
    });
}

}  // extern "C"

void ARDeclarativeSceneDelegate::onTrackingUpdated(VROARTrackingState state,
                                                   VROARTrackingStateReason reason) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, state, reason] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onTrackingUpdated", "(II)V", state, reason);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::onAmbientLightUpdate(float ambientLightIntensity, float colorTemperature) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, ambientLightIntensity, colorTemperature] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onAmbientLightUpdate", "(FF)V",
                                    ambientLightIntensity, colorTemperature);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VRO_REF nodeNativeRef = 0;
        VROPlatformCallJavaFunction(localObj, "onAnchorFound",
                                    "(Lcom/viro/core/ARAnchor;J)V",
                                    janchor, nodeNativeRef);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {

}

void ARDeclarativeSceneDelegate::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VROPlatformCallJavaFunction(localObj, "onAnchorUpdated",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    janchor, 0);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VROPlatformCallJavaFunction(localObj, "onAnchorRemoved",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    janchor, 0);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

// +---------------------------------------------------------------------------+
// | Imperative Delegate
// +---------------------------------------------------------------------------+

void ARImperativeSceneDelegate::onTrackingUpdated(VROARTrackingState state,
                                                  VROARTrackingStateReason reason) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, state, reason] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onTrackingUpdated", "(II)V", state, reason);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARImperativeSceneDelegate::onAmbientLightUpdate(float ambientLightIntensity,
                                                     float colorTemperature) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, ambientLightIntensity, colorTemperature] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onAmbientLightUpdate", "(FF)V",
                                    ambientLightIntensity, colorTemperature);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARImperativeSceneDelegate::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor, node] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VRO_REF node_j = ARNode::jptr(node);
        VROPlatformCallJavaFunction(localObj, "onAnchorFound",
                                    "(Lcom/viro/core/ARAnchor;J)V",
                                    janchor, node_j);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARImperativeSceneDelegate::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {

}

void ARImperativeSceneDelegate::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor, node] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VROPlatformCallJavaFunction(localObj, "onAnchorUpdated",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    janchor, node->getUniqueID());
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}

void ARImperativeSceneDelegate::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor, node] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(jObjWeak);
            return;
        }

        jobject janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VROPlatformCallJavaFunction(localObj, "onAnchorRemoved",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    janchor, node->getUniqueID());
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(jObjWeak);
    });
}
