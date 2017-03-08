//
//  Scene_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>
#include <VROSceneController.h>

#include "PersistentRef.h"

namespace Scene{
    inline jlong jptr(std::shared_ptr<VROSceneController> ptr) {
        PersistentRef<VROSceneController> *persistentRef = new PersistentRef<VROSceneController>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSceneController> native(jlong ptr) {
        PersistentRef<VROSceneController> *persistentRef = reinterpret_cast<PersistentRef<VROSceneController> *>(ptr);
        return persistentRef->get();
    }
}

class SceneDelegate : public VROSceneController::VROSceneControllerDelegate{
public:
    SceneDelegate(jobject sceneJavaObject, JNIEnv *env) {
        _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(sceneJavaObject));
        _env = env;
    }

    ~SceneDelegate() {
        _env->DeleteGlobalRef(_javaObject);
    }

    static jlong jptr(std::shared_ptr<SceneDelegate> shared_node) {
        PersistentRef<SceneDelegate> *native_delegate = new PersistentRef<SceneDelegate>(shared_node);
        return reinterpret_cast<intptr_t>(native_delegate);
    }

    static std::shared_ptr<SceneDelegate> native(jlong ptr) {
        PersistentRef<SceneDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<SceneDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onSceneWillAppear(VRORenderContext * context, VRODriver *driver);
    void onSceneDidAppear(VRORenderContext * context, VRODriver *driver);
    void onSceneWillDisappear(VRORenderContext * context, VRODriver *driver);
    void onSceneDidDisappear(VRORenderContext * context, VRODriver *driver);
private:
    void callVoidFunctionWithName(std::string functionName);
    jobject _javaObject;
    JNIEnv *_env;

};
