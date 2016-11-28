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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_SceneJni_##method_name

namespace {
  inline jlong jptr(VROSceneController *native_sceneController) {
    return reinterpret_cast<intptr_t>(native_sceneController);
  }

  inline VROSceneController *native(jlong ptr) {
    return reinterpret_cast<VROSceneController *>(ptr);
  }
}  // anonymous namespace

extern "C" {

JNI_METHOD(jlong, nativeCreateScene)(JNIEnv *env,
                                        jclass clazz,
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

    /**
     * TODO:
     * Update setBackground for scene once we have impelmented the texture managers.
     * For now display a standard default background.
     */
    std::vector<std::shared_ptr<VROImage>> cubeImages = {
            std::make_shared<VROImageAndroid>("px.png"),
            std::make_shared<VROImageAndroid>("nx.png"),
            std::make_shared<VROImageAndroid>("py.png"),
            std::make_shared<VROImageAndroid>("ny.png"),
            std::make_shared<VROImageAndroid>("pz.png"),
            std::make_shared<VROImageAndroid>("nz.png")
    };

    sceneController->getScene()->setBackgroundCube(std::make_shared<VROTexture>(cubeImages));
    return jptr(sceneController);
}

JNI_METHOD(void, nativeDestroyScene)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_object_ref) {
  delete reinterpret_cast<PersistentRef<VROScene> *>(native_object_ref);
}

}  // extern "C"
