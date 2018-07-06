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
#include "arcore/VROARSessionARCore.h"
#include "arcore/VROARCameraARCore.h"
#include "ViroContextAndroid_JNI.h"

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

VRO_METHOD(VRO_REF(VROARNode), nativeCreateAnchoredNode)(VRO_ARGS
                                                         VRO_REF(VROARSceneController) sceneController_j,
                                                         float posX, float posY, float posZ,
                                                         float quatX, float quatY, float quatZ, float quatW) {

    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, sceneController_j)->getScene());

    std::shared_ptr<VROARNode> node = std::make_shared<VROARNode>();

    // Set the position and rotation of the ARNode so this data can be accessed
    // immediately from the application (UI) thread. This node is added to the root
    // node so we can compute its transforms with identity parent matrices.
    node->setPositionAtomic({ posX, posY, posZ });
    node->setRotationAtomic({ quatX, quatY, quatZ, quatW });
    node->computeTransformsAtomic({}, {});

    // Acquire the anchor from the session. If tracking is limited then this can
    // fail, in which case we return null.
    std::shared_ptr<VROARSessionARCore> session = std::dynamic_pointer_cast<VROARSessionARCore>(scene->getARSession());
    arcore::Pose *pose = session->getSessionInternal()->createPose(posX, posY, posZ,
                                                                   quatX, quatY, quatZ, quatW);
    std::shared_ptr<arcore::Anchor> anchor_arc = std::shared_ptr<arcore::Anchor>(
            session->getSessionInternal()->acquireNewAnchor(pose));
    delete (pose);

    if (anchor_arc) {
        // Create a Viro|ARCore anchor
        std::string key = VROStringUtil::toString64(anchor_arc->getId());
        std::shared_ptr<VROARAnchorARCore> anchor = std::make_shared<VROARAnchorARCore>(key, anchor_arc, nullptr, session);
        node->setAnchor(anchor);

        std::weak_ptr<VROARSessionARCore> session_w = session;
        VROPlatformDispatchAsyncRenderer([node, anchor, session_w] {
            std::shared_ptr<VROARSessionARCore> session_s = session_w.lock();
            if (!session_s) {
                return;
            }

            // Set the node *after* the sync so that the anchor has the latest transforms to pass to the node
            anchor->sync();
            anchor->setARNode(node);

            // Add the anchor to the session so all updates are propagated to Viro
            session_s->addAnchor(anchor);
        });
        return VRO_REF_NEW(VROARNode, node);
    } else {
        pinfo("Failed to acquire anchor from world position: no anchored node will be created");
        return 0;
    }
}

VRO_METHOD(void, nativeHostCloudAnchor)(VRO_ARGS
                                        VRO_REF(VROARSceneController) sceneController_j,
                                        VRO_STRING anchorId_j) {
    VRO_METHOD_PREAMBLE;

    std::weak_ptr<VROARScene> scene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, sceneController_j)->getScene());

    std::string localAnchorId = VRO_STRING_STL(anchorId_j);
    VRO_WEAK obj_w = VRO_NEW_WEAK_GLOBAL_REF(obj);

    VROPlatformDispatchAsyncRenderer([obj_w, localAnchorId, scene_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();

        std::shared_ptr<VROARScene> scene = scene_w.lock();
        if (!scene) {
            VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
            return;
        }
        std::shared_ptr<VROARSessionARCore> session = std::dynamic_pointer_cast<VROARSessionARCore>(scene->getARSession());
        if (!session) {
            VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
            return;
        }

        std::shared_ptr<VROARAnchor> anchor = session->getAnchorWithId(localAnchorId);
        scene->getARSession()->hostCloudAnchor(anchor,
           [obj_w, localAnchorId](std::shared_ptr<VROARAnchor> cloudAnchor) {
               VROPlatformDispatchAsyncApplication([obj_w, localAnchorId, cloudAnchor] {
                   // Success callback
                   VRO_ENV env = VROPlatformGetJNIEnv();

                   VRO_OBJECT obj_j = VRO_NEW_LOCAL_REF(obj_w);
                   if (VRO_IS_OBJECT_NULL(obj_j)) {
                       VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
                       return;
                   }

                   VRO_STRING localAnchorId_j = VRO_NEW_STRING(localAnchorId.c_str());
                   VRO_OBJECT anchor_j = ARUtilsCreateJavaARAnchorFromAnchor(cloudAnchor);
                   int nodeId = cloudAnchor->getARNode()->getUniqueID();
                   VROPlatformCallHostFunction(obj_j, "onHostSuccess",
                                               "(Ljava/lang/String;Lcom/viro/core/ARAnchor;I)V",
                                               localAnchorId_j, anchor_j, nodeId);

                   VRO_DELETE_LOCAL_REF(obj_j);
                   VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
               });
           },
           [obj_w, localAnchorId](std::string error) {
               VROPlatformDispatchAsyncApplication([obj_w, localAnchorId, error] {
                   // Failure callback
                   VRO_ENV env = VROPlatformGetJNIEnv();

                   VRO_OBJECT obj_j = VRO_NEW_LOCAL_REF(obj_w);
                   if (VRO_IS_OBJECT_NULL(obj_j)) {
                       VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
                       return;
                   }

                   VRO_STRING localAnchorId_j = VRO_NEW_STRING(localAnchorId.c_str());
                   VRO_STRING error_j = VRO_NEW_STRING(error.c_str());
                   VROPlatformCallHostFunction(obj_j, "onHostFailure",
                                               "(Ljava/lang/String;Ljava/lang/String;)V",
                                               localAnchorId_j, error_j);

                   VRO_DELETE_LOCAL_REF(obj_j);
                   VRO_DELETE_LOCAL_REF(error_j);
                   VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
               });
           });
    });
}

VRO_METHOD(void, nativeResolveCloudAnchor)(VRO_ARGS
                                           VRO_REF(VROARSceneController) sceneController_j,
                                           VRO_STRING cloudAnchorId_j) {
    VRO_METHOD_PREAMBLE;

    std::weak_ptr<VROARScene> scene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, sceneController_j)->getScene());

    std::string cloudAnchorId = VRO_STRING_STL(cloudAnchorId_j);
    VRO_WEAK obj_w = VRO_NEW_WEAK_GLOBAL_REF(obj);

    VROPlatformDispatchAsyncRenderer([obj_w, cloudAnchorId, scene_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();

        std::shared_ptr<VROARScene> scene = scene_w.lock();
        if (!scene) {
            VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
            return;
        }
        std::shared_ptr<VROARSessionARCore> session = std::dynamic_pointer_cast<VROARSessionARCore>(scene->getARSession());
        if (!session) {
            VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
            return;
        }

        scene->getARSession()->resolveCloudAnchor(cloudAnchorId,
           [obj_w, cloudAnchorId](std::shared_ptr<VROARAnchor> cloudAnchor) {
               // Success callback
               VROPlatformDispatchAsyncApplication([obj_w, cloudAnchorId, cloudAnchor] {
                   VRO_ENV env = VROPlatformGetJNIEnv();

                   VRO_OBJECT obj_j = VRO_NEW_LOCAL_REF(obj_w);
                   if (VRO_IS_OBJECT_NULL(obj_j)) {
                       VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
                       return;
                   }

                   VRO_STRING cloudAnchorId_j = VRO_NEW_STRING(cloudAnchorId.c_str());
                   VRO_OBJECT anchor_j = ARUtilsCreateJavaARAnchorFromAnchor(cloudAnchor);
                   int nodeId = cloudAnchor->getARNode()->getUniqueID();

                   VROPlatformCallHostFunction(obj_j, "onResolveSuccess",
                                               "(Ljava/lang/String;Lcom/viro/core/ARAnchor;I)V",
                                               cloudAnchorId_j, anchor_j, nodeId);

                   VRO_DELETE_LOCAL_REF(obj_j);
                   VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
               });
           },
           [obj_w, cloudAnchorId](std::string error) {
               // Failure callback
               VROPlatformDispatchAsyncApplication([obj_w, cloudAnchorId, error] {
                   VRO_ENV env = VROPlatformGetJNIEnv();

                   VRO_OBJECT obj_j = VRO_NEW_LOCAL_REF(obj_w);
                   if (VRO_IS_OBJECT_NULL(obj_j)) {
                       VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
                       return;
                   }

                   VRO_STRING cloudAnchorId_j = VRO_NEW_STRING(cloudAnchorId.c_str());
                   VRO_STRING error_j = VRO_NEW_STRING(error.c_str());
                   VROPlatformCallHostFunction(obj_j, "onResolveFailure",
                   "(Ljava/lang/String;Ljava/lang/String;)V",
                   cloudAnchorId_j, error_j);

                   VRO_DELETE_LOCAL_REF(obj_j);
                   VRO_DELETE_LOCAL_REF(error_j);
                   VRO_DELETE_WEAK_GLOBAL_REF(obj_w);
               });
           });
    });
}

VRO_METHOD(void, nativeSetCameraImageListener)(VRO_ARGS
                                               VRO_REF(VROARSceneController) sceneController_j,
                                               VRO_REF(ViroContext) context_j,
                                               VRO_OBJECT listener_j) {
    VRO_METHOD_PREAMBLE;
    std::weak_ptr<VROARScene> scene_w = std::dynamic_pointer_cast<VROARScene>(
            VRO_REF_GET(VROARSceneController, sceneController_j)->getScene());
    std::weak_ptr<ViroContext> context_w = VRO_REF_GET(ViroContext, context_j);

    if (VRO_IS_OBJECT_NULL(listener_j)) {
        VROPlatformDispatchAsyncRenderer([scene_w, context_w] {
            VRO_ENV env = VROPlatformGetJNIEnv();
            std::shared_ptr<ViroContext> context = context_w.lock();
            if (!context) {
                return;
            }
            std::shared_ptr<ViroContextAndroid> context_a = std::dynamic_pointer_cast<ViroContextAndroid>(context);
            if (!context_a) {
                return;
            }
            context_a->setCameraImageFrameListener(nullptr);
        });
    }
    else {
        VRO_OBJECT listener_g = VRO_NEW_GLOBAL_REF(listener_j);
        VROPlatformDispatchAsyncRenderer([listener_g, scene_w, context_w] {
            VRO_ENV env = VROPlatformGetJNIEnv();
            std::shared_ptr<VROARScene> scene = scene_w.lock();
            if (!scene) {
                VRO_DELETE_GLOBAL_REF(listener_g);
                return;
            }
            std::shared_ptr<ViroContext> context = context_w.lock();
            if (!context) {
                VRO_DELETE_GLOBAL_REF(listener_g);
                return;
            }
            std::shared_ptr<ViroContextAndroid> context_a = std::dynamic_pointer_cast<ViroContextAndroid>(context);
            if (!context_a) {
                VRO_DELETE_GLOBAL_REF(listener_g);
                return;
            }

            std::shared_ptr<CameraImageFrameListener> imageListener =
                    std::make_shared<CameraImageFrameListener>(listener_g, scene, env);
            context_a->setCameraImageFrameListener(imageListener);
            VRO_DELETE_GLOBAL_REF(listener_g);
        });
    }
}

}  // extern "C"

// +---------------------------------------------------------------------------+
// | Camera Image Frame Listener
// +---------------------------------------------------------------------------+

void CameraImageFrameListener::onFrameWillRender(const VRORenderContext &context) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    std::shared_ptr<VROARScene> scene = _scene.lock();
    if (!scene) {
        return;
    }
    std::shared_ptr<VROARSession> session = scene->getARSession();
    if (!session) {
        return;
    }

    std::unique_ptr<VROARFrame> &frame = session->getLastFrame();
    if (!frame) {
        return;
    }
    std::shared_ptr<VROARCameraARCore> camera = std::dynamic_pointer_cast<VROARCameraARCore>(frame->getCamera());
    if (!camera || !camera->isImageDataAvailable()) {
        return;
    }
    int bufferIndex = _bufferIndex;
    _bufferIndex = (_bufferIndex + 1) % 3;

    VROVector3f size = camera->getImageSize();
    int width = (int) size.x;
    int height = (int) size.y;
    if (width <= 0 || height <= 0) {
        return;
    }

    int dataLength = width * height * 4;
    if (!_data[bufferIndex] || _data[bufferIndex]->getDataLength() < dataLength) {
        uint8_t *data = (uint8_t *) malloc(dataLength);
        _data[bufferIndex] = std::make_shared<VROData>(data, dataLength, VRODataOwnership::Move);
        _buffers[bufferIndex] = VRO_NEW_GLOBAL_REF(env->NewDirectByteBuffer(data, dataLength));
    }

    camera->getImageData((uint8_t *) _data[bufferIndex]->getData());

    VRO_WEAK listener_w = VRO_NEW_WEAK_GLOBAL_REF(_listener_j);
    jobject buffer_w = VRO_NEW_WEAK_GLOBAL_REF(_buffers[bufferIndex]);

    VROPlatformDispatchAsyncApplication([listener_w, width, height, buffer_w] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT listener = VRO_NEW_LOCAL_REF(listener_w);
        if (VRO_IS_OBJECT_NULL(listener)) {
            VRO_DELETE_WEAK_GLOBAL_REF(listener_w);
            VRO_DELETE_WEAK_GLOBAL_REF(buffer_w);
            return;
        }

        VRO_OBJECT buffer = VRO_NEW_LOCAL_REF(buffer_w);
        if (VRO_IS_OBJECT_NULL(buffer_w)) {
            VRO_DELETE_WEAK_GLOBAL_REF(listener_w);
            VRO_DELETE_WEAK_GLOBAL_REF(buffer_w);
            return;
        }

        VROPlatformCallHostObjectFunction(buffer, "rewind", "()Ljava/nio/Buffer;");
        VROPlatformCallHostObjectFunction(buffer, "limit", "(I)Ljava/nio/Buffer;", width * height * 4);
        VROPlatformCallHostFunction(listener, "onCameraImageUpdated", "(Ljava/nio/ByteBuffer;II)V", buffer, width, height);
        VRO_DELETE_LOCAL_REF(listener);
        VRO_DELETE_WEAK_GLOBAL_REF(listener_w);
    });
}

void CameraImageFrameListener::onFrameDidRender(const VRORenderContext &context) {

}

// +---------------------------------------------------------------------------+
// | Declarative Delegate
// +---------------------------------------------------------------------------+

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