//
//  SceneController_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <memory>
#include <VROBox.h>
#include "VRONode.h"
#include "VROScene.h"
#include "VROSceneController.h"
#include "PersistentRef.h"
#include "VideoTexture_JNI.h"
#include "SceneController_JNI.h"
#include "Texture_JNI.h"
#include "ViroContext_JNI.h"
#include "Node_JNI.h"
#include "ParticleEmitter_JNI.h"
#include "VROPostProcessEffectFactory.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Scene_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Scene_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateSceneController)(VRO_ARGS
                                                 VRO_REF root_node_ref) {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    return SceneController::jptr(sceneController);
}

VRO_METHOD(VRO_REF, nativeGetSceneNodeRef)(VRO_ARGS
                                           VRO_REF root_node_ref) {
    std::shared_ptr<VROSceneController> sceneController = SceneController::native(root_node_ref);
    std::shared_ptr<VRONode> node = std::static_pointer_cast<VRONode>(sceneController->getScene()->getRootNode());
    return Node::jptr(node);
}

VRO_METHOD(VRO_REF, nativeCreateSceneControllerDelegate)(VRO_ARGS
                                                         VRO_REF native_object_ref) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<SceneControllerDelegate> delegate = std::make_shared<SceneControllerDelegate>(obj, env);
    SceneController::native(native_object_ref)->setDelegate(delegate);
    return SceneControllerDelegate::jptr(delegate);
}

VRO_METHOD(void, nativeDestroySceneController)(VRO_ARGS
                                               VRO_REF native_object_ref) {
    delete reinterpret_cast<PersistentRef<VROSceneController> *>(native_object_ref);
}

VRO_METHOD(void, nativeDestroySceneControllerDelegate)(VRO_ARGS
                                                       VRO_REF native_delegate_object_ref) {
    delete reinterpret_cast<PersistentRef<SceneControllerDelegate> *>(native_delegate_object_ref);
}

VRO_METHOD(void, nativeSetBackgroundTexture)(VRO_ARGS
                                             VRO_REF scene_j,
                                             VRO_REF texture_j) {
    std::weak_ptr<VROSceneController> scene_w = SceneController::native(scene_j);
    std::weak_ptr<VROTexture> texture_w = Texture::native(texture_j);

    VROPlatformDispatchAsyncRenderer([scene_w, texture_w] {
        std::shared_ptr<VROSceneController> scene = scene_w.lock();
        std::shared_ptr<VROTexture> texture = texture_w.lock();

        if (scene && texture) {
            scene->getScene()->getRootNode()->setBackgroundSphere(texture);
        }
    });
}

VRO_METHOD(void, nativeSetBackgroundRotation)(VRO_ARGS
                                              VRO_REF sceneRef,
                                              VRO_FLOAT rotationRadiansX,
                                              VRO_FLOAT rotationRadiansY,
                                              VRO_FLOAT rotationRadiansZ) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, rotationRadiansX, rotationRadiansY, rotationRadiansZ] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (sceneController) {
            sceneController->getScene()->getRootNode()->setBackgroundRotation({rotationRadiansX,
                                                                               rotationRadiansY,
                                                                               rotationRadiansZ});
        }
    });
}

VRO_METHOD(void, nativeSetBackgroundCubeImageTexture)(VRO_ARGS
                                                      VRO_REF sceneRef,
                                                      VRO_REF textureRef) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    std::weak_ptr<VROTexture> texture_w = Texture::native(textureRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, texture_w] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        std::shared_ptr<VROTexture> texture = texture_w.lock();
        if (sceneController && texture) {
            sceneController->getScene()->getRootNode()->setBackgroundCube(texture);
        }
    });
}

VRO_METHOD(void, nativeSetBackgroundCubeWithColor)(VRO_ARGS
                                                   VRO_REF sceneRef,
                                                   VRO_LONG color) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    VROPlatformDispatchAsyncRenderer([sceneController_w, color] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (!sceneController) {
            return;
        }
        // Get the color
        float a = ((color >> 24) & 0xFF) / 255.0f;
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = (color & 0xFF) / 255.0f;

        VROVector4f vecColor(r, g, b, a);
        sceneController->getScene()->getRootNode()->setBackgroundCube(vecColor);
    });
}

VRO_METHOD(void, nativeSetLightingEnvironment)(VRO_ARGS
                                               VRO_REF scene_j,
                                               VRO_REF texture_j) {
    std::weak_ptr<VROSceneController> scene_w = SceneController::native(scene_j);
    long texture_ref = texture_j;
    VROPlatformDispatchAsyncRenderer([scene_w, texture_ref] {
        std::shared_ptr<VROSceneController> scene = scene_w.lock();
        if (!scene){
            return;
        }

        if (texture_ref != 0){
            scene->getScene()->getRootNode()->setLightingEnvironment(Texture::native(texture_ref));
        } else {
            scene->getScene()->getRootNode()->setLightingEnvironment(nullptr);
        }
    });
}

VRO_METHOD(void, nativeSetSoundRoom)(VRO_ARGS
                                     VRO_REF sceneRef, VRO_REF context_j,
                                     VRO_FLOAT sizeX, VRO_FLOAT sizeY, VRO_FLOAT sizeZ, VRO_STRING wallMaterial,
                                     VRO_STRING ceilingMaterial, VRO_STRING floorMaterial) {
    VRO_METHOD_PREAMBLE;
    std::string strWallMaterial = VRO_STRING_STL(wallMaterial);
    std::string strCeilingMaterial = VRO_STRING_STL(ceilingMaterial);
    std::string strFloorMaterial = VRO_STRING_STL(floorMaterial);

    std::weak_ptr<ViroContext> context_w = ViroContext::native(context_j);

    VROPlatformDispatchAsyncRenderer([context_w,
                                             sizeX, sizeY, sizeZ,
                                             strWallMaterial,
                                             strCeilingMaterial,
                                             strFloorMaterial] {
        std::shared_ptr<ViroContext> context = context_w.lock();
        if (!context) {
            return;
        }

        context->getDriver()->setSoundRoom(sizeX, sizeY, sizeZ,
                                           strWallMaterial,
                                           strCeilingMaterial,
                                           strFloorMaterial);
    });
}

VRO_METHOD(bool, nativeSetEffects)(VRO_ARGS
                                   VRO_REF sceneRef,
                                   VRO_STRING_ARRAY jEffects) {
    VRO_METHOD_PREAMBLE;

    std::vector<std::string> effects;
    if (jEffects != NULL) {
        int numberOfValues = VRO_ARRAY_LENGTH(jEffects);
        for (int i = 0; i < numberOfValues; i++) {
            VRO_STRING jEffect = VRO_STRING_ARRAY_GET(jEffects, i);
            std::string strEffect = VRO_STRING_STL(jEffect);
            VROPostProcessEffect postEffect = VROPostProcessEffectFactory::getEffectForString(strEffect);
            effects.push_back(strEffect);
        }
    }
    
    std::shared_ptr<VROSceneController> sceneController = SceneController::native(sceneRef);
    VROPlatformDispatchAsyncRenderer([sceneController, effects] {
        if (sceneController->getScene()){
            sceneController->getScene()->setPostProcessingEffects(effects);
        }
    });
    return true;
}


VRO_METHOD(void, nativeSetPhysicsWorldGravity)(VRO_ARGS
                                               VRO_REF sceneRef,
                                               VRO_FLOAT_ARRAY gravityArray) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    VRO_FLOAT *gravityArrayf = VRO_FLOAT_ARRAY_GET_ELEMENTS(gravityArray);
    VROVector3f gravity = VROVector3f(gravityArrayf[0], gravityArrayf[1], gravityArrayf[2]);

    VROPlatformDispatchAsyncRenderer([sceneController_w, gravity] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (sceneController){
            sceneController->getScene()->getPhysicsWorld()->setGravity(gravity);
        }
    });
}

VRO_METHOD(void, nativeSetPhysicsWorldDebugDraw)(VRO_ARGS
                                                 VRO_REF sceneRef,
                                                 VRO_BOOL debugDraw) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    VROPlatformDispatchAsyncRenderer([sceneController_w, debugDraw] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (sceneController) {
            sceneController->getScene()->getPhysicsWorld()->setDebugDrawVisible(debugDraw);
        }
    });
}

VRO_METHOD(void, findCollisionsWithRayAsync)(VRO_ARGS
                                             VRO_REF sceneRef,
                                             VRO_FLOAT_ARRAY fromPos,
                                             VRO_FLOAT_ARRAY toPos,
                                             VRO_BOOL closest,
                                             VRO_STRING tag,
                                             VRO_OBJECT callback) {
    VRO_METHOD_PREAMBLE;

    // Grab start position from which to perform the collision test
    VRO_FLOAT *fromPosf = VRO_FLOAT_ARRAY_GET_ELEMENTS(fromPos);
    VROVector3f from = VROVector3f(fromPosf[0], fromPosf[1], fromPosf[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(fromPos, fromPosf);

    // Grab end position to which to perform the test to.
    VRO_FLOAT *toPosf = VRO_FLOAT_ARRAY_GET_ELEMENTS(toPos);
    VROVector3f to = VROVector3f(toPosf[0], toPosf[1], toPosf[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(toPos, toPosf);

    // Get the ray tag used to notify collided objects with.
    std::string strTag = VRO_STRING_STL(tag);

    // If no ray tag is given, set it to the default tag.
    if (strTag.empty()) {
        strTag = kDefaultNodeTag;
    }

    VRO_WEAK weakCallback = VRO_NEW_WEAK_GLOBAL_REF(callback);
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);

    // Perform the collision ray test asynchronously.
    VROPlatformDispatchAsyncRenderer([sceneController_w, weakCallback, from, to, closest, strTag] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (!sceneController) {
            return;
        }

        bool hitSomething = sceneController->getScene()->getPhysicsWorld()->findCollisionsWithRay(
                from,
                to,
                closest,
                strTag);

        // Notify the bridge after collision tests are complete
        VROPlatformDispatchAsyncApplication([hitSomething, weakCallback] {
            VRO_ENV env = VROPlatformGetJNIEnv();
            VRO_OBJECT jCallback = VRO_NEW_LOCAL_REF(weakCallback);
            if (VRO_IS_OBJECT_NULL(jCallback)) {
                return;
            }

            VROPlatformCallJavaFunction(jCallback, "onComplete", "(Z)V", hitSomething);
            VRO_DELETE_LOCAL_REF(jCallback);
            VRO_DELETE_WEAK_GLOBAL_REF(weakCallback);
        });
    });
}

VRO_METHOD(void, findCollisionsWithShapeAsync)(VRO_ARGS
                                               VRO_REF sceneRef,
                                               VRO_FLOAT_ARRAY posStart,
                                               VRO_FLOAT_ARRAY posEnd,
                                               VRO_STRING shapeType,
                                               VRO_FLOAT_ARRAY shapeParams,
                                               VRO_STRING tag,
                                               VRO_OBJECT callback) {
    VRO_METHOD_PREAMBLE;

    // Grab start position from which to perform the collision test
    VRO_FLOAT *posStartf = VRO_FLOAT_ARRAY_GET_ELEMENTS(posStart);
    VROVector3f from = VROVector3f(posStartf[0], posStartf[1], posStartf[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(posStart, posStartf);

    // Grab end position to which to perform the test to.
    VRO_FLOAT *posEndf = VRO_FLOAT_ARRAY_GET_ELEMENTS(posEnd);
    VROVector3f to = VROVector3f(posEndf[0], posEndf[1], posEndf[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(posStart, posEndf);

    // Grab the shape type
    std::string strShapeType = VRO_STRING_STL(shapeType);

    // Grab the shape params
    int paramsLength = VRO_ARRAY_LENGTH(shapeParams);
    VRO_FLOAT *pointArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(shapeParams);
    std::vector<float> params;
    for (int i = 0; i < paramsLength; i ++) {
        params.push_back(pointArray[i]);
    }
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(shapeParams, pointArray);

    // Get the ray tag used to notify collided objects with.
    std::string strTag = VRO_STRING_STL(tag);

    // If no ray tag is given, set it to the default tag.
    if (strTag.empty()) {
        strTag = kDefaultNodeTag;
    }

    VRO_WEAK weakCallback = VRO_NEW_WEAK_GLOBAL_REF(callback);
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);

    // Perform the collision shape test asynchronously.
    VROPlatformDispatchAsyncRenderer([sceneController_w, weakCallback, from, to, strShapeType, params, strTag] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (!sceneController) {
            return;
        }

        // Create a VROPhysicsShape and perform collision tests.
        VROPhysicsShape::VROShapeType propShapeType = VROPhysicsShape::getTypeForString(strShapeType);
        std::shared_ptr<VROPhysicsShape> shape = std::make_shared<VROPhysicsShape>(propShapeType, params);
        bool hitSomething = sceneController->getScene()->getPhysicsWorld()->findCollisionsWithShape(
                from, to, shape, strTag);

        // Notify the bridge after collision tests are complete
        VROPlatformDispatchAsyncApplication([hitSomething, weakCallback] {
            VRO_ENV env = VROPlatformGetJNIEnv();
            VRO_OBJECT jCallback = VRO_NEW_LOCAL_REF(weakCallback);
            if (VRO_IS_OBJECT_NULL(jCallback)) {
                return;
            }

            VROPlatformCallJavaFunction(jCallback, "onComplete", "(Z)V", hitSomething);
            VRO_DELETE_LOCAL_REF(jCallback);
            VRO_DELETE_WEAK_GLOBAL_REF(weakCallback);
        });
    });
}

}  // extern "C"

/*
 *   Scene delegates for triggering Java methods.
 */
void SceneControllerDelegate::onSceneWillAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneWillAppear");
}
void SceneControllerDelegate::onSceneDidAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneDidAppear");
}
void SceneControllerDelegate::onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneWillDisappear");
}
void SceneControllerDelegate::onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneDidDisappear");
}

void SceneControllerDelegate::callVoidFunctionWithName(std::string functionName) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK jObjWeak = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, functionName] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(jObjWeak);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    functionName, "()V");
        VRO_DELETE_LOCAL_REF(localObj);
    });
}
