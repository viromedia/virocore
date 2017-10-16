//
//  ARSceneController_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ARSceneController_JNI_h
#define ARSceneController_JNI_h

#include <jni.h>
#include <memory>
#include <VROARSceneController.h>
#include "PersistentRef.h"

namespace ARSceneController {
    inline jlong jptr(std::shared_ptr<VROARSceneController> sharedARSceneController) {
        PersistentRef<VROARSceneController> *arSceneController =
                new PersistentRef<VROARSceneController>(sharedARSceneController);
        return reinterpret_cast<intptr_t>(arSceneController);
    }

    inline std::shared_ptr<VROARSceneController> native(jlong ptr) {
        PersistentRef<VROARSceneController> *persistentARSceneController =
                reinterpret_cast<PersistentRef<VROARSceneController> *>(ptr);
        return persistentARSceneController->get();
    }
}

class ARSceneDelegate : public VROARSceneDelegate {
public:
    ARSceneDelegate(jobject arSceneJavaObject, JNIEnv *env) {
        _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(arSceneJavaObject));
        _env = env;
    }

    ~ARSceneDelegate() {
        _env->DeleteGlobalRef(_javaObject);
    }

    static jlong jptr(std::shared_ptr<ARSceneDelegate> arSceneDelegate) {
        PersistentRef<ARSceneDelegate> *persistentDelegate = new PersistentRef<ARSceneDelegate>(arSceneDelegate);
        return reinterpret_cast<intptr_t>(persistentDelegate);
    }

    static std::shared_ptr<ARSceneDelegate> native(jlong ptr) {
        PersistentRef<ARSceneDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<ARSceneDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onTrackingInitialized();
    void onAmbientLightUpdate(float ambientLightIntensity, float colorTemperature);
    void onAnchorFound(std::shared_ptr<VROARAnchor> anchor);
    void onAnchorUpdated(std::shared_ptr<VROARAnchor> anchor);
    void onAnchorRemoved(std::shared_ptr<VROARAnchor> anchor);

private:
    jobject _javaObject;
    JNIEnv *_env;
};

#endif