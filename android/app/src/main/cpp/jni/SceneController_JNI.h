//
//  SceneController_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VROSceneController.h>

#include "PersistentRef.h"

namespace SceneController {
    inline jlong jptr(std::shared_ptr<VROSceneController> ptr) {
        PersistentRef<VROSceneController> *persistentRef = new PersistentRef<VROSceneController>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSceneController> native(jlong ptr) {
        PersistentRef<VROSceneController> *persistentRef = reinterpret_cast<PersistentRef<VROSceneController> *>(ptr);
        return persistentRef->get();
    }
}

class SceneControllerDelegate : public VROSceneController::VROSceneControllerDelegate {
public:
    SceneControllerDelegate(jobject sceneJavaObject, JNIEnv *env) {
        _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(sceneJavaObject));
        _env = env;
    }

    ~SceneControllerDelegate() {
        _env->DeleteGlobalRef(_javaObject);
    }

    static jlong jptr(std::shared_ptr<SceneControllerDelegate> shared_node) {
        PersistentRef<SceneControllerDelegate> *native_delegate = new PersistentRef<SceneControllerDelegate>(shared_node);
        return reinterpret_cast<intptr_t>(native_delegate);
    }

    static std::shared_ptr<SceneControllerDelegate> native(jlong ptr) {
        PersistentRef<SceneControllerDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<SceneControllerDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onSceneWillAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneWillDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
private:
    void callVoidFunctionWithName(std::string functionName);
    jobject _javaObject;
    JNIEnv *_env;

};
