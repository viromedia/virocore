//
//  SceneController_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROBox.h>
#include <VROImageAndroid.h>

#include "VRONode.h"
#include "VROScene.h"
#include "VROSceneController.h"
#include "Viro.h"
#include "PersistentRef.h"
#include "VideoTexture_JNI.h"
#include "SceneController_JNI.h"
#include "Texture_JNI.h"
#include "RenderContext_JNI.h"
#include "Node_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SceneControllerJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateSceneController)(JNIEnv *env,
                                     jobject object,
                                     jlong root_node_ref) {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();

    std::weak_ptr<VRONode> node_w = Node::native(root_node_ref);
    std::weak_ptr<VROScene> scene_w = sceneController->getScene();

    VROPlatformDispatchAsyncRenderer([node_w, scene_w] {
        std::shared_ptr<VROScene> scene = scene_w.lock();
        if (scene) {
            std::shared_ptr<VRONode> node = node_w.lock();
            if (node) {
                scene->getRootNode()->addChildNode(node);
            }
        }
    });

    return SceneController::jptr(sceneController);
}

JNI_METHOD(jlong, nativeCreateSceneControllerDelegate)(JNIEnv *env,
                                                       jobject object,
                                                       jlong native_object_ref) {
    std::shared_ptr<SceneControllerDelegate> delegate = std::make_shared<SceneControllerDelegate>(object, env);
    SceneController::native(native_object_ref)->setDelegate(delegate);
    return SceneControllerDelegate::jptr(delegate);
}

JNI_METHOD(void, nativeDestroySceneController)(JNIEnv *env,
                                               jclass clazz,
                                               jlong native_object_ref) {
    delete reinterpret_cast<PersistentRef<VROSceneController> *>(native_object_ref);
}

JNI_METHOD(void, nativeDestroySceneControllerDelegate)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_delegate_object_ref) {
    delete reinterpret_cast<PersistentRef<SceneControllerDelegate> *>(native_delegate_object_ref);
}

JNI_METHOD(void, nativeSetBackgroundVideoTexture)(JNIEnv *env,
                                     jclass clazz,
                                     jlong sceneRef,
                                     jlong textureRef) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, videoTexture_w] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();

        if (sceneController && videoTexture) {
            sceneController->getScene()->getRootNode()->setBackgroundSphere(videoTexture);
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundImageTexture)(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong sceneRef,
                                                  jlong imageRef) {
    std::weak_ptr<VROTexture> image_w = Texture::native(imageRef);
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, image_w] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        std::shared_ptr<VROTexture> image = image_w.lock();

        if (sceneController && image) {
            sceneController->getScene()->getRootNode()->setBackgroundSphere(image);
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundRotation)(JNIEnv *env,
                                              jclass clazz,
                                              jlong sceneRef,
                                              jfloat rotationDegreeX,
                                              jfloat rotationDegreeY,
                                              jfloat rotationDegreeZ) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, rotationDegreeX, rotationDegreeY, rotationDegreeZ] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (sceneController) {
            sceneController->getScene()->getRootNode()->setBackgroundRotation({toRadians(rotationDegreeX),
                                                                               toRadians(rotationDegreeY),
                                                                               toRadians(rotationDegreeZ)});
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundCubeImageTexture)(JNIEnv *env,
                                          jclass clazz,
                                          jlong sceneRef,
                                          jlong textureRef) {
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

JNI_METHOD(void, nativeSetBackgroundCubeWithColor)(JNIEnv *env,
                                                   jclass clazz,
                                                   jlong sceneRef,
                                                   jlong color) {
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

JNI_METHOD(void, nativeSetSoundRoom)(JNIEnv *env, jobject obj, jlong sceneRef, jlong renderContextRef,
                                     jfloat sizeX, jfloat sizeY, jfloat sizeZ, jstring wallMaterial,
                                     jstring ceilingMaterial, jstring floorMaterial) {
    const char *cWallMaterial = env->GetStringUTFChars(wallMaterial, NULL);
    const char *cCeilingMaterial = env->GetStringUTFChars(ceilingMaterial, NULL);
    const char *cFloorMaterial = env->GetStringUTFChars(floorMaterial, NULL);

    std::string strWallMaterial(cWallMaterial);
    std::string strCeilingMaterial(cCeilingMaterial);
    std::string strFloorMaterial(cFloorMaterial);

    std::weak_ptr<RenderContext> renderContext_w = RenderContext::native(renderContextRef);

    VROPlatformDispatchAsyncRenderer([renderContext_w,
                                             sizeX, sizeY, sizeZ,
                                             strWallMaterial,
                                             strCeilingMaterial,
                                             strFloorMaterial] {
        std::shared_ptr<RenderContext> renderContext = renderContext_w.lock();
        if (!renderContext) {
            return;
        }

        renderContext->getDriver()->setSoundRoom(sizeX, sizeY, sizeZ,
                                                 strWallMaterial,
                                                 strCeilingMaterial,
                                                 strFloorMaterial);
    });

    env->ReleaseStringUTFChars(wallMaterial, cWallMaterial);
    env->ReleaseStringUTFChars(ceilingMaterial, cCeilingMaterial);
    env->ReleaseStringUTFChars(floorMaterial, cFloorMaterial);

}

JNI_METHOD(void, nativeSetPhysicsWorldGravity)(JNIEnv *env,
                                     jclass clazz,
                                     jlong sceneRef,
                                     jfloatArray gravityArray) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    jfloat *gravityArrayf = env->GetFloatArrayElements(gravityArray, 0);
    VROVector3f gravity = VROVector3f(gravityArrayf[0], gravityArrayf[1], gravityArrayf[2]);

    VROPlatformDispatchAsyncRenderer([sceneController_w, gravity] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (sceneController){
            sceneController->getScene()->getPhysicsWorld()->setGravity(gravity);
        }
    });
}

JNI_METHOD(void, nativeSetPhysicsWorldDebugDraw)(JNIEnv *env,
                                               jclass clazz,
                                               jlong sceneRef,
                                               jboolean debugDraw) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    VROPlatformDispatchAsyncRenderer([sceneController_w, debugDraw] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (sceneController) {
            sceneController->getScene()->getPhysicsWorld()->setDebugDrawVisible(debugDraw);
        }
    });
}

JNI_METHOD(void, nativeAttachToPhysicsWorld)(JNIEnv *env,
                                     jclass clazz,
                                     jlong sceneRef,
                                     jlong nodeRef) {
    std::weak_ptr<VROSceneController> sceneController_w = SceneController::native(sceneRef);
    std::weak_ptr<VRONode> node_w = Node::native(nodeRef);
    VROPlatformDispatchAsyncRenderer([sceneController_w, node_w] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody() && sceneController) {
            sceneController->getScene()->getPhysicsWorld()->addPhysicsBody(node->getPhysicsBody());
        }
    });
}

JNI_METHOD(void, nativeDetachFromPhysicsWorld)(JNIEnv *env,
                                     jclass clazz,
                                     jlong sceneRef,
                                     jlong nodeRef) {
    std::shared_ptr<VROSceneController> sceneController = SceneController::native(sceneRef);
    std::shared_ptr<VRONode> node = Node::native(nodeRef);
    VROPlatformDispatchAsyncRenderer([sceneController, node] {
        if (node && node->getPhysicsBody() && sceneController) {
            sceneController->getScene()->getPhysicsWorld()->removePhysicsBody(node->getPhysicsBody());
        }
    });
}

JNI_METHOD(void, findCollisionsWithRayAsync)(JNIEnv *env,
                                     jobject obj,
                                     jlong sceneRef,
                                     jfloatArray fromPos,
                                     jfloatArray toPos,
                                     jboolean closest,
                                     jstring tag,
                                     jobject callback) {

    // Grab start position from which to perform the collision test
    jfloat *fromPosf = env->GetFloatArrayElements(fromPos, 0);
    VROVector3f from = VROVector3f(fromPosf[0], fromPosf[1], fromPosf[2]);
    env->ReleaseFloatArrayElements(fromPos, fromPosf, 0);

    // Grab end position to which to perform the test to.
    jfloat *toPosf = env->GetFloatArrayElements(toPos, 0);
    VROVector3f to = VROVector3f(toPosf[0], toPosf[1], toPosf[2]);
    env->ReleaseFloatArrayElements(toPos, toPosf, 0);

    // Get the ray tag used to notify collided objects with.
    const char *cStrTag = env->GetStringUTFChars(tag, NULL);
    std::string strTag(cStrTag);
    env->ReleaseStringUTFChars(tag, cStrTag);

    // If no ray tag is given, set it to the default tag.
    if (strTag.empty()) {
        strTag = kDefaultNodeTag;
    }

    jweak weakCallback = env->NewWeakGlobalRef(callback);
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
            JNIEnv *env = VROPlatformGetJNIEnv();
            jobject jCallback = env->NewLocalRef(weakCallback);
            if (jCallback == NULL) {
                return;
            }

            VROPlatformCallJavaFunction(jCallback, "onComplete", "(Z)V", hitSomething);
            env->DeleteLocalRef(jCallback);
            env->DeleteWeakGlobalRef(weakCallback);
        });
    });
}

JNI_METHOD(void, findCollisionsWithShapeAsync)(JNIEnv *env,
                                       jobject obj,
                                       jlong sceneRef,
                                       jfloatArray posStart,
                                       jfloatArray posEnd,
                                       jstring shapeType,
                                       jfloatArray shapeParams,
                                       jstring tag,
                                       jobject callback) {

    // Grab start position from which to perform the collision test
    jfloat *posStartf = env->GetFloatArrayElements(posStart, 0);
    VROVector3f from = VROVector3f(posStartf[0], posStartf[1], posStartf[2]);
    env->ReleaseFloatArrayElements(posStart, posStartf, 0);

    // Grab end position to which to perform the test to.
    jfloat *posEndf = env->GetFloatArrayElements(posEnd, 0);
    VROVector3f to = VROVector3f(posEndf[0], posEndf[1], posEndf[2]);
    env->ReleaseFloatArrayElements(posStart, posEndf, 0);

    // Grab the shape type
    const char *cStrShapeType = env->GetStringUTFChars(shapeType, NULL);
    std::string strShapeType(cStrShapeType);
    env->ReleaseStringUTFChars(shapeType, cStrShapeType);

    // Grab the shape params
    int paramsLength = env->GetArrayLength(shapeParams);
    jfloat *pointArray = env->GetFloatArrayElements(shapeParams, 0);
    std::vector<float> params;
    for (int i = 0; i < paramsLength; i ++) {
        params.push_back(pointArray[i]);
    }
    env->ReleaseFloatArrayElements(shapeParams, pointArray, 0);

    // Get the ray tag used to notify collided objects with.
    const char *cStrTag = env->GetStringUTFChars(tag, NULL);
    std::string strTag(cStrTag);
    env->ReleaseStringUTFChars(tag, cStrTag);

    // If no ray tag is given, set it to the default tag.
    if (strTag.empty()) {
        strTag = kDefaultNodeTag;
    }

    jweak weakCallback = env->NewWeakGlobalRef(callback);
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
            JNIEnv *env = VROPlatformGetJNIEnv();
            jobject jCallback = env->NewLocalRef(weakCallback);
            if (jCallback == NULL) {
                return;
            }

            VROPlatformCallJavaFunction(jCallback, "onComplete", "(Z)V", hitSomething);
            env->DeleteLocalRef(jCallback);
            env->DeleteWeakGlobalRef(weakCallback);
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
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak jObjWeak = env->NewWeakGlobalRef(_javaObject);
    VROPlatformDispatchAsyncApplication([jObjWeak, functionName] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(jObjWeak);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    functionName, "()V");
        env->DeleteLocalRef(localObj);
    });
}
