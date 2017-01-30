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
    VROSceneController *sceneController = new VROSceneController();

    PersistentRef<VRONode> *persistentNode = reinterpret_cast<PersistentRef<VRONode> *>(root_node_ref);
    std::shared_ptr<VRONode> rootNode = persistentNode->get();

    sceneController->getScene()->addNode(rootNode);

    std::shared_ptr<SceneDelegate> delegate = std::make_shared<SceneDelegate>(object, env);
    sceneController->setDelegate(delegate);

    return Scene::jptr(sceneController);
}

JNI_METHOD(void, nativeDestroyScene)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_object_ref) {
  delete reinterpret_cast<PersistentRef<VROScene> *>(native_object_ref);
}

JNI_METHOD(void, nativeSetBackgroundVideoTexture)(JNIEnv *env,
                                     jclass clazz,
                                     jlong sceneRef,
                                     jlong textureRef) {
    VROSceneController *sceneController = Scene::native(sceneRef);
    sceneController->getScene()->setBackgroundSphere(VideoTexture::native(textureRef));
}

JNI_METHOD(void, nativeSetBackgroundImageTexture)(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong sceneRef,
                                                  jlong imageRef) {
    VROSceneController *sceneController = Scene::native(sceneRef);
    sceneController->getScene()->setBackgroundSphere(Texture::native(imageRef));
}

JNI_METHOD(void, nativeSetBackgroundRotation)(JNIEnv *env,
                                              jclass clazz,
                                              jlong sceneRef,
                                              jfloat rotationDegreeX,
                                              jfloat rotationDegreeY,
                                              jfloat rotationDegreeZ) {

    VROSceneController *sceneController = Scene::native(sceneRef);
    sceneController->getScene()->setBackgroundRotation({toRadians(rotationDegreeX),
                                                        toRadians(rotationDegreeY),
                                                        toRadians(rotationDegreeZ)});
}

JNI_METHOD(void, nativeSetBackgroundCubeImageTexture)(JNIEnv *env,
                                          jclass clazz,
                                          jlong sceneRef,
                                          jlong textureRef) {

    VROSceneController *sceneController = Scene::native(sceneRef);
    sceneController->getScene()->setBackgroundCube(Texture::native(textureRef));
}

JNI_METHOD(void, nativeSetBackgroundCubeWithColor)(JNIEnv *env,
                                                   jclass clazz,
                                                   jlong sceneRef,
                                                   jlong color) {
    // Get the color
    float a = ((color >> 24) & 0xFF) / 255.0;
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;

    VROVector4f vecColor(r, g, b, a);
    VROSceneController *sceneController = Scene::native(sceneRef);
    sceneController->getScene()->setBackgroundCube(vecColor);
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

    RenderContext::native(renderContextRef)->getDriver()->setSoundRoom(sizeX, sizeY, sizeZ,
                                                                       strWallMaterial, strCeilingMaterial,
                                                                       strFloorMaterial);
    env->ReleaseStringUTFChars(wallMaterial, cWallMaterial);
    env->ReleaseStringUTFChars(ceilingMaterial, cCeilingMaterial);
    env->ReleaseStringUTFChars(floorMaterial, cFloorMaterial);

}

}  // extern "C"

/*
 *   Scene delegates for triggering Java methods.
 */
void SceneDelegate::onSceneWillAppear(VRORenderContext *context, VRODriver *driver) {
    callVoidFunctionWithName("onSceneWillAppear");
}
void SceneDelegate::onSceneDidAppear(VRORenderContext *context, VRODriver *driver) {
    callVoidFunctionWithName("onSceneDidAppear");
}
void SceneDelegate::onSceneWillDisappear(VRORenderContext *context, VRODriver *driver) {
    callVoidFunctionWithName("onSceneWillDisappear");
}
void SceneDelegate::onSceneDidDisappear(VRORenderContext *context, VRODriver *driver) {
    callVoidFunctionWithName("onSceneDidDisappear");
}

void SceneDelegate::callVoidFunctionWithName(std::string functionName) {
    _env->ExceptionClear();
    jclass viroClass = _env->FindClass("com/viro/renderer/jni/SceneJni");
    if (viroClass == nullptr) {
        perr("Unable to find SceneJni class for to trigger callbacks.");
        return;
    }

    jmethodID method = _env->GetMethodID(viroClass, functionName.c_str(), "()V");
    if (method == nullptr) {
        perr("Unable to find method %s callback.", functionName.c_str());
        return;
    }

    _env->CallVoidMethod(_javaObject, method);
    if (_env->ExceptionOccurred()) {
        perr("Exception occured when calling %s.", functionName.c_str());
        _env->ExceptionClear();
    }
    _env->DeleteLocalRef(viroClass);
}
