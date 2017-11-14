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
#include <VROARDeclarativeSession.h>
#include <VROARImperativeSession.h>
#include <VROPlatformUtil.h>
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

class ARDeclarativeSceneDelegate : public VROARSceneDelegate, public VROARDeclarativeSessionDelegate {
public:
    ARDeclarativeSceneDelegate(jobject arSceneJavaObject, JNIEnv *env) {
        _javaObject = env->NewWeakGlobalRef(arSceneJavaObject);
    }

    virtual ~ARDeclarativeSceneDelegate() {
        JNIEnv *env = VROPlatformGetJNIEnv();
        env->DeleteWeakGlobalRef(_javaObject);
    }

    static jlong jptr(std::shared_ptr<ARDeclarativeSceneDelegate> arSceneDelegate) {
        PersistentRef<ARDeclarativeSceneDelegate> *persistentDelegate = new PersistentRef<ARDeclarativeSceneDelegate>(arSceneDelegate);
        return reinterpret_cast<intptr_t>(persistentDelegate);
    }

    static std::shared_ptr<ARDeclarativeSceneDelegate> native(jlong ptr) {
        PersistentRef<ARDeclarativeSceneDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<ARDeclarativeSceneDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onTrackingInitialized();
    void onAmbientLightUpdate(float ambientLightIntensity, float colorTemperature);
    void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor);

private:
    jweak _javaObject;
};

class ARImperativeSceneDelegate : public VROARSceneDelegate, public VROARImperativeSessionDelegate {
public:
    ARImperativeSceneDelegate(jobject arSceneJavaObject, JNIEnv *env) {
        _javaObject = env->NewWeakGlobalRef(arSceneJavaObject);
    }

    virtual ~ARImperativeSceneDelegate() {
        JNIEnv *env = VROPlatformGetJNIEnv();
        env->DeleteWeakGlobalRef(_javaObject);
    }

    static jlong jptr(std::shared_ptr<ARImperativeSceneDelegate> arSceneDelegate) {
        PersistentRef<ARImperativeSceneDelegate> *persistentDelegate = new PersistentRef<ARImperativeSceneDelegate>(arSceneDelegate);
        return reinterpret_cast<intptr_t>(persistentDelegate);
    }

    static std::shared_ptr<ARImperativeSceneDelegate> native(jlong ptr) {
        PersistentRef<ARImperativeSceneDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<ARImperativeSceneDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onTrackingInitialized();
    void onAmbientLightUpdate(float ambientLightIntensity, float colorTemperature);
    void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);

private:
    jweak _javaObject;
};

#endif