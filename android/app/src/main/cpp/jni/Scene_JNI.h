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
    inline jlong jptr(VROSceneController *native_sceneController) {
        return reinterpret_cast<intptr_t>(native_sceneController);
    }

    inline VROSceneController *native(jlong ptr) {
        return reinterpret_cast<VROSceneController *>(ptr);
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
    void onSceneWillAppear(VRORenderContext &context, VRODriver &driver);
    void onSceneDidAppear(VRORenderContext &context, VRODriver &driver);
    void onSceneWillDisappear(VRORenderContext &context, VRODriver &driver);
    void onSceneDidDisappear(VRORenderContext &context, VRODriver &driver);

private:
    void callVoidFunctionWithName(std::string functionName);
    jobject _javaObject;
    JNIEnv *_env;

};