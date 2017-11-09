/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

#include "FrameListener_JNI.h"
#include "SceneController_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_PortalTraversalListener_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreatePortalTraversalListener)(JNIEnv *env,
                                                       jclass clazz, long refSceneController) {

    std::shared_ptr<VROSceneController> sceneController = SceneController::native(
            refSceneController);
    std::shared_ptr<VROPortalTraversalListener> node = std::make_shared<VROPortalTraversalListener>(
            sceneController->getScene());
    return PortalTraversalListener::jptr(node);
}

JNI_METHOD(void, nativeDestroyPortalTraversalListener)(JNIEnv *env,
                                                        jclass clazz, long refPortalTraversalListener) {
    delete reinterpret_cast<PersistentRef<VROPortalTraversalListener> *>(refPortalTraversalListener);
}


}