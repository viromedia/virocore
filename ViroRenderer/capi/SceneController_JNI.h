//
//  SceneController_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <memory>
#include <VROSceneController.h>
#include <VROPlatformUtil.h>

#include "VRODefines.h"
#include VRO_C_INCLUDE

class SceneControllerDelegate : public VROSceneController::VROSceneControllerDelegate {
public:
    SceneControllerDelegate(VRO_OBJECT obj, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(obj)) {
    }

    ~SceneControllerDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    void onSceneWillAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidAppear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneWillDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
    void onSceneDidDisappear(VRORenderContext * context, std::shared_ptr<VRODriver> driver);
private:
    void callVoidFunctionWithName(std::string functionName);
    VRO_OBJECT _javaObject;
};
