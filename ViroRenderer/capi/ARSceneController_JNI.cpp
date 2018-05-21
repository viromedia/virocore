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
#include "VROPlatformUtil.h"

#if VRO_PLATFORM_ANDROID
#include "VROARImageTargetAndroid.h"

#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_ARScene_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type ARScene_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF(VROARSceneController), nativeCreateARSceneController)(VRO_NO_ARGS) {
    std::shared_ptr<VROARSceneController> arSceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(arSceneController->getScene());
    scene->initImperativeSession();

    return VRO_REF_NEW(VROARSceneController, arSceneController);
}

VRO_METHOD(VRO_REF(VROARSceneController), nativeCreateARSceneControllerDeclarative)(VRO_NO_ARGS) {
    std::shared_ptr<VROARSceneController> arSceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(arSceneController->getScene());
    scene->initDeclarativeSession();

    return VRO_REF_NEW(VROARSceneController, arSceneController);
}

VRO_METHOD(VRO_REF(VROARSceneDelegate), nativeCreateARSceneDelegate)(VRO_ARGS
                                                                     VRO_REF(VROARSceneController) arSceneControllerPtr) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    if (arScene->getDeclarativeSession()) {
        std::shared_ptr<ARDeclarativeSceneDelegate> delegate = std::make_shared<ARDeclarativeSceneDelegate>(obj, env);
        arScene->setDelegate(delegate);
        arScene->getDeclarativeSession()->setDelegate(delegate);

        return VRO_REF_NEW(VROARSceneDelegate, delegate);
    }
    else {
        passert (arScene->getImperativeSession() != nullptr);
        std::shared_ptr<ARImperativeSceneDelegate> delegate = std::make_shared<ARImperativeSceneDelegate>(obj, env);
        arScene->setDelegate(delegate);
        arScene->getImperativeSession()->setDelegate(delegate);

        return VRO_REF_NEW(VROARSceneDelegate, delegate);
    }
}

VRO_METHOD(void, nativeDestroyARSceneDelegate)(VRO_ARGS
                                               VRO_REF(VROARSceneDelegate) arSceneDelegatePtr) {
    VRO_REF_DELETE(VROARSceneDelegate, arSceneDelegatePtr);
}

VRO_METHOD(void, nativeDisplayPointCloud)(VRO_ARGS
                                          VRO_REF(VROARSceneController) arSceneControllerPtr,
                                          VRO_BOOL displayPointCloud) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w, displayPointCloud] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->displayPointCloud(displayPointCloud);
        }
    });
}

VRO_METHOD(void, nativeResetPointCloudSurface)(VRO_ARGS
                                               VRO_REF(VROARSceneController) arSceneControllerPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->resetPointCloudSurface();
        }
    });
}

VRO_METHOD(void, nativeSetPointCloudSurface)(VRO_ARGS
                                             VRO_REF(VROARSceneController) arSceneControllerPtr,
                                             VRO_REF(VROSurface) pointCloudSurface) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    std::weak_ptr<VROSurface> surface_w = VRO_REF_GET(VROSurface, pointCloudSurface);
    VROPlatformDispatchAsyncRenderer([arScene_w, surface_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROSurface> surface = surface_w.lock();

        if (arScene && surface) {
            arScene->setPointCloudSurface(surface);
        }
    });
}

VRO_METHOD(void, nativeSetPointCloudSurfaceScale)(VRO_ARGS
                                                  VRO_REF(VROARSceneController) arSceneControllerPtr,
                                                  VRO_FLOAT scaleX, VRO_FLOAT scaleY, VRO_FLOAT scaleZ) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w, scaleX, scaleY, scaleZ] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->setPointCloudSurfaceScale({scaleX, scaleY, scaleZ});
        }
    });
}

VRO_METHOD(void, nativeSetPointCloudMaxPoints)(VRO_ARGS
                                               VRO_REF(VROARSceneController) arSceneControllerPtr,
                                               VRO_INT maxPoints) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    VROPlatformDispatchAsyncRenderer([arScene_w, maxPoints] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();

        if (arScene) {
            arScene->setPointCloudMaxPoints(maxPoints);
        }
    });
}

VRO_METHOD(void, nativeSetAnchorDetectionTypes)(VRO_ARGS
                                                VRO_REF(VROARSceneController) sceneRef,
                                                VRO_STRING_ARRAY typeStrArray) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, sceneRef)->getScene());
    std::set<VROAnchorDetection> types;

    int stringCount = VRO_ARRAY_LENGTH(typeStrArray);
    for (int i = 0; i < stringCount; i++) {
        std::string typeString = VRO_STRING_STL(VRO_STRING_ARRAY_GET(typeStrArray, i));
        if (VROStringUtil::strcmpinsensitive(typeString, "PlanesHorizontal")) {
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
                                  VRO_REF(VROARSceneController) scene_j,
                                  VRO_REF(VROARDeclarativeNode) node_j) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, scene_j)->getScene());
    std::weak_ptr<VROARDeclarativeNode> node_w = VRO_REF_GET(VROARDeclarativeNode, node_j);

    VROPlatformDispatchAsyncRenderer([node_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARDeclarativeNode> node = node_w.lock();

        if (arScene && node) {
            arScene->getDeclarativeSession()->addARNode(node);
        }
    });
}

VRO_METHOD(void, nativeUpdateARNode)(VRO_ARGS
                                     VRO_REF(VROARSceneController) scene_j,
                                     VRO_REF(VROARDeclarativeNode) node_j) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, scene_j)->getScene());
    std::weak_ptr<VROARDeclarativeNode> node_w = VRO_REF_GET(VROARDeclarativeNode, node_j);

    VROPlatformDispatchAsyncRenderer([node_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARDeclarativeNode> node = node_w.lock();

        if (arScene && node) {
            arScene->getDeclarativeSession()->updateARNode(node);
        }
    });
}

VRO_METHOD(void, nativeRemoveARNode)(VRO_ARGS
                                     VRO_REF(VROARSceneController) arSceneControllerPtr,
                                     VRO_REF(VROARDeclarativeNode) arPlanePtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARDeclarativeNode> arPlane_w = VRO_REF_GET(VROARDeclarativeNode, arPlanePtr);

    VROPlatformDispatchAsyncRenderer([arPlane_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARDeclarativeNode> node = arPlane_w.lock();

        if (arScene && node) {
            arScene->getDeclarativeSession()->removeARNode(node);
        }
    });
}

VRO_METHOD(void, nativeAddARImageTarget)(VRO_ARGS
                                         VRO_REF(VROARSceneController) arSceneControllerPtr,
                                         VRO_REF(VROARImageTarget) arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTarget> arImageTarget_w = VRO_REF_GET(VROARImageTarget, arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTarget> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getImperativeSession()->addARImageTarget(arImageTarget);
        }
    });
}

VRO_METHOD(void, nativeRemoveARImageTarget)(VRO_ARGS
                                            VRO_REF(VROARSceneController) arSceneControllerPtr,
                                            VRO_REF(VROARImageTarget) arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTarget> arImageTarget_w = VRO_REF_GET(VROARImageTarget, arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTarget> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getImperativeSession()->removeARImageTarget(arImageTarget);
        }
    });
}

VRO_METHOD(void, nativeAddARImageTargetDeclarative)(VRO_ARGS
                                                    VRO_REF(VROARSceneController) arSceneControllerPtr,
                                                    VRO_REF(VROARImageTarget) arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTarget> arImageTarget_w = VRO_REF_GET(VROARImageTarget, arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTarget> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getDeclarativeSession()->addARImageTarget(arImageTarget);
        }
    });
}

VRO_METHOD(void, nativeRemoveARImageTargetDeclarative)(VRO_ARGS
                                                       VRO_REF(VROARSceneController) arSceneControllerPtr,
                                                       VRO_REF(VROARImageTarget) arImageTargetPtr) {
    std::weak_ptr<VROARScene> arScene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, arSceneControllerPtr)->getScene());
    std::weak_ptr<VROARImageTarget> arImageTarget_w = VRO_REF_GET(VROARImageTarget, arImageTargetPtr);

    VROPlatformDispatchAsyncRenderer([arImageTarget_w, arScene_w] {
        std::shared_ptr<VROARScene> arScene = arScene_w.lock();
        std::shared_ptr<VROARImageTarget> arImageTarget = arImageTarget_w.lock();

        if (arScene && arImageTarget) {
            arScene->getDeclarativeSession()->removeARImageTarget(arImageTarget);
        }
    });
}

VRO_METHOD(VRO_FLOAT, nativeGetAmbientLightIntensity)(VRO_ARGS
                                                      VRO_REF(VROARSceneController) sceneController_j) {
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, sceneController_j)->getScene());
    return scene->getAmbientLightIntensity();
}

VRO_METHOD(VRO_FLOAT_ARRAY, nativeGetAmbientLightColor)(VRO_ARGS
                                                        VRO_REF(VROARSceneController) sceneController_j) {
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, sceneController_j)->getScene());
    return ARUtilsCreateFloatArrayFromVector3f(scene->getAmbientLightColor());
}

}  // extern "C"

void ARDeclarativeSceneDelegate::onTrackingUpdated(VROARTrackingState state,
                                                   VROARTrackingStateReason reason) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, state, reason] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
            return;
        }

        VROPlatformCallHostFunction(localObj, "onTrackingUpdated", "(II)V", state, reason);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::onAmbientLightUpdate(float intensity, VROVector3f color) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, intensity, color] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
            return;
        }

        VROPlatformCallHostFunction(localObj, "onAmbientLightUpdate", "(FFFF)V",
                                    intensity, color.x, color.y, color.z);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
            return;
        }

        VRO_OBJECT janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VRO_REF(VROARNode) nodeNativeRef = 0;
        VROPlatformCallHostFunction(localObj, "onAnchorFound",
                                    "(Lcom/viro/core/ARAnchor;J)V",
                                    janchor, nodeNativeRef);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor) {

}

void ARDeclarativeSceneDelegate::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
            return;
        }

        VRO_OBJECT janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VROPlatformCallHostFunction(localObj, "onAnchorUpdated",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    janchor, 0);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
    });
}

void ARDeclarativeSceneDelegate::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, anchor] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
            return;
        }

        VRO_OBJECT janchor = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VROPlatformCallHostFunction(localObj, "onAnchorRemoved",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    janchor, 0);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
    });
}

// +---------------------------------------------------------------------------+
// | Imperative Delegate
// +---------------------------------------------------------------------------+

void ARImperativeSceneDelegate::onTrackingUpdated(VROARTrackingState state,
                                                  VROARTrackingStateReason reason) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, state, reason] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
            return;
        }

        VROPlatformCallHostFunction(localObj, "onTrackingUpdated", "(II)V", state, reason);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
    });
}

void ARImperativeSceneDelegate::onAmbientLightUpdate(float intensity,
                                                     VROVector3f color) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, intensity, color] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
            return;
        }

        VROPlatformCallHostFunction(localObj, "onAmbientLightUpdate", "(FFFF)V",
                                    intensity, color.x, color.y, color.z);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(jObjWeak);
    });
}

void ARImperativeSceneDelegate::anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {
    VRO_ENV env = VROPlatformGetJNIEnv();

    VRO_WEAK object_w = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    std::weak_ptr<VROARAnchor> anchor_w = anchor;

    VROPlatformDispatchAsyncApplication([object_w, anchor_w, node] {
        VRO_ENV env = VROPlatformGetJNIEnv();

        VRO_OBJECT object = VRO_NEW_LOCAL_REF(object_w);
        if (VRO_IS_OBJECT_NULL(object)) {
            VRO_DELETE_WEAK_GLOBAL_REF(object_w);
            return;
        }
        std::shared_ptr<VROARAnchor> anchor_s = anchor_w.lock();
        if (!anchor_s) {
            VRO_DELETE_WEAK_GLOBAL_REF(object_w);
            return;
        }

        VRO_OBJECT anchor_j = ARUtilsCreateJavaARAnchorFromAnchor(anchor_s);
        VRO_REF(VROARNode) node_j = VRO_REF_NEW(VROARNode, node);
        VROPlatformCallHostFunction(object, "onAnchorFound",
                                    "(Lcom/viro/core/ARAnchor;J)V",
                                    anchor_j, node_j);
        VRO_DELETE_LOCAL_REF(object);
        VRO_DELETE_WEAK_GLOBAL_REF(object_w);
    });
}

void ARImperativeSceneDelegate::anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {

}

void ARImperativeSceneDelegate::anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    passert (node != nullptr);

    VRO_WEAK object_w = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    std::weak_ptr<VROARAnchor> anchor_w = anchor;

    VROPlatformDispatchAsyncApplication([object_w, anchor_w, node] {
        VRO_ENV env = VROPlatformGetJNIEnv();

        VRO_OBJECT object = VRO_NEW_LOCAL_REF(object_w);
        if (VRO_IS_OBJECT_NULL(object)) {
            VRO_DELETE_WEAK_GLOBAL_REF(object_w);
            return;
        }
        std::shared_ptr<VROARAnchor> anchor_s = anchor_w.lock();
        if (!anchor_s) {
            VRO_DELETE_WEAK_GLOBAL_REF(object_w);
            return;
        }

        VRO_OBJECT anchor_j = ARUtilsCreateJavaARAnchorFromAnchor(anchor_s);
        VROPlatformCallHostFunction(object, "onAnchorUpdated",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    anchor_j, node->getUniqueID());
        VRO_DELETE_LOCAL_REF(object);
        VRO_DELETE_WEAK_GLOBAL_REF(object_w);
    });
}

void ARImperativeSceneDelegate::anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK object_w = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([object_w, anchor, node] {
        VRO_ENV env = VROPlatformGetJNIEnv();

        VRO_OBJECT object = VRO_NEW_LOCAL_REF(object_w);
        if (VRO_IS_OBJECT_NULL(object)) {
            VRO_DELETE_WEAK_GLOBAL_REF(object_w);
            return;
        }

        VRO_OBJECT anchor_j = ARUtilsCreateJavaARAnchorFromAnchor(anchor);
        VROPlatformCallHostFunction(object, "onAnchorRemoved",
                                    "(Lcom/viro/core/ARAnchor;I)V",
                                    anchor_j, node->getUniqueID());
        VRO_DELETE_LOCAL_REF(object);
        VRO_DELETE_WEAK_GLOBAL_REF(object_w);
    });
}
