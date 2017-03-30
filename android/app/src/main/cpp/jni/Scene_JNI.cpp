//
//  Scene_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
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
#include "Scene_JNI.h"
#include "Texture_JNI.h"
#include "RenderContext_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SceneJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateScene)(JNIEnv *env,
                                        jobject object,
                                        jlong root_node_ref) {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();

    PersistentRef<VRONode> *persistentNode = reinterpret_cast<PersistentRef<VRONode> *>(root_node_ref);
    std::weak_ptr<VRONode> node_w = persistentNode->get();
    std::weak_ptr<VROScene> scene_w = sceneController->getScene();

    VROPlatformDispatchAsyncRenderer([node_w, scene_w] {
        std::shared_ptr<VROScene> scene = scene_w.lock();
        if (scene) {
            scene->setThreadRestriction(pthread_self());

            std::shared_ptr<VRONode> node = node_w.lock();
            if (node) {
                scene->addNode(node);
            }
        }
    });

    return Scene::jptr(sceneController);
}

JNI_METHOD(jlong, nativeCreateSceneDelegate)(JNIEnv *env,
                                            jobject object,
                                            jlong native_object_ref) {
    std::shared_ptr<SceneDelegate> delegate = std::make_shared<SceneDelegate>(object, env);
    Scene::native(native_object_ref)->setDelegate(delegate);
    return SceneDelegate::jptr(delegate);
}

JNI_METHOD(void, nativeDestroyScene)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_object_ref) {
    delete reinterpret_cast<PersistentRef<VROSceneController> *>(native_object_ref);
}

JNI_METHOD(void, nativeDestroySceneDelegate)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_delegate_object_ref) {
    delete reinterpret_cast<PersistentRef<SceneDelegate> *>(native_delegate_object_ref);
}

JNI_METHOD(void, nativeSetBackgroundVideoTexture)(JNIEnv *env,
                                     jclass clazz,
                                     jlong sceneRef,
                                     jlong textureRef) {
    std::weak_ptr<VROSceneController> sceneController_w = Scene::native(sceneRef);
    std::weak_ptr<VROVideoTextureAVP> videoTexture_w = VideoTexture::native(textureRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, videoTexture_w] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        std::shared_ptr<VROVideoTextureAVP> videoTexture = videoTexture_w.lock();

        if (sceneController && videoTexture) {
            sceneController->getScene()->setBackgroundSphere(videoTexture);
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundImageTexture)(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong sceneRef,
                                                  jlong imageRef) {
    std::weak_ptr<VROTexture> image_w = Texture::native(imageRef);
    std::weak_ptr<VROSceneController> sceneController_w = Scene::native(sceneRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, image_w] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        std::shared_ptr<VROTexture> image = image_w.lock();

        if (sceneController && image) {
            sceneController->getScene()->setBackgroundSphere(image);
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundRotation)(JNIEnv *env,
                                              jclass clazz,
                                              jlong sceneRef,
                                              jfloat rotationDegreeX,
                                              jfloat rotationDegreeY,
                                              jfloat rotationDegreeZ) {
    std::weak_ptr<VROSceneController> sceneController_w = Scene::native(sceneRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, rotationDegreeX, rotationDegreeY, rotationDegreeZ] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (sceneController) {
            sceneController->getScene()->setBackgroundRotation({toRadians(rotationDegreeX),
                                                                toRadians(rotationDegreeY),
                                                                toRadians(rotationDegreeZ)});
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundCubeImageTexture)(JNIEnv *env,
                                          jclass clazz,
                                          jlong sceneRef,
                                          jlong textureRef) {
    std::weak_ptr<VROSceneController> sceneController_w = Scene::native(sceneRef);
    std::weak_ptr<VROTexture> texture_w = Texture::native(textureRef);

    VROPlatformDispatchAsyncRenderer([sceneController_w, texture_w] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        std::shared_ptr<VROTexture> texture = texture_w.lock();
        if (sceneController && texture) {
            sceneController->getScene()->setBackgroundCube(texture);
        }
    });
}

JNI_METHOD(void, nativeSetBackgroundCubeWithColor)(JNIEnv *env,
                                                   jclass clazz,
                                                   jlong sceneRef,
                                                   jlong color) {
    std::weak_ptr<VROSceneController> sceneController_w = Scene::native(sceneRef);
    VROPlatformDispatchAsyncRenderer([sceneController_w, color] {
        std::shared_ptr<VROSceneController> sceneController = sceneController_w.lock();
        if (!sceneController) {
            return;
        }
        // Get the color
        float a = ((color >> 24) & 0xFF) / 255.0;
        float r = ((color >> 16) & 0xFF) / 255.0;
        float g = ((color >> 8) & 0xFF) / 255.0;
        float b = (color & 0xFF) / 255.0;

        VROVector4f vecColor(r, g, b, a);
        sceneController->getScene()->setBackgroundCube(vecColor);
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

}  // extern "C"

/*
 *   Scene delegates for triggering Java methods.
 */
void SceneDelegate::onSceneWillAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneWillAppear");
}
void SceneDelegate::onSceneDidAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneDidAppear");
}
void SceneDelegate::onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneWillDisappear");
}
void SceneDelegate::onSceneDidDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    callVoidFunctionWithName("onSceneDidDisappear");
}

void SceneDelegate::callVoidFunctionWithName(std::string functionName) {
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
