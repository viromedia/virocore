//
//  SceneController_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <memory>
#include <VROSceneController.h>
#include <VROPlatformUtil.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace SceneController {
    inline VRO_REF jptr(std::shared_ptr<VROSceneController> ptr) {
        PersistentRef<VROSceneController> *persistentRef = new PersistentRef<VROSceneController>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSceneController> native(VRO_REF ptr) {
        PersistentRef<VROSceneController> *persistentRef = reinterpret_cast<PersistentRef<VROSceneController> *>(ptr);
        return persistentRef->get();
    }
}

class SceneControllerDelegate : public VROSceneController::VROSceneControllerDelegate {
public:
    SceneControllerDelegate(VRO_OBJECT sceneJavaObject, VRO_ENV env) {
        _javaObject = reinterpret_cast<jclass>(VRO_NEW_WEAK_GLOBAL_REF(sceneJavaObject));
    }

    ~SceneControllerDelegate() {
        VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_javaObject);
    }

    static VRO_REF jptr(std::shared_ptr<SceneControllerDelegate> shared_node) {
        PersistentRef<SceneControllerDelegate> *native_delegate = new PersistentRef<SceneControllerDelegate>(shared_node);
        return reinterpret_cast<intptr_t>(native_delegate);
    }

    static std::shared_ptr<SceneControllerDelegate> native(VRO_REF ptr) {
        PersistentRef<SceneControllerDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<SceneControllerDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onSceneWillAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneWillDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
private:
    void callVoidFunctionWithName(std::string functionName);
    VRO_OBJECT _javaObject;
};
