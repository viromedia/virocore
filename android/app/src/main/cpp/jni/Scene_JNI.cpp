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

    /**
     * TODO:
     * Update lighting model once we have created it's corresponding managers.
     */
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.4, 0.4, 0.4 });
    rootNode->addLight(ambient);
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

}  // extern "C"

/*
 *   Scene delegates for triggering Java methods.
 */
void SceneDelegate::onSceneWillAppear(VRORenderContext &context, VRODriver &driver) {
    callVoidFunctionWithName("onSceneWillAppear");
}
void SceneDelegate::onSceneDidAppear(VRORenderContext &context, VRODriver &driver) {
    callVoidFunctionWithName("onSceneDidAppear");
}
void SceneDelegate::onSceneWillDisappear(VRORenderContext &context, VRODriver &driver) {
    callVoidFunctionWithName("onSceneWillDisappear");
}
void SceneDelegate::onSceneDidDisappear(VRORenderContext &context, VRODriver &driver) {
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
