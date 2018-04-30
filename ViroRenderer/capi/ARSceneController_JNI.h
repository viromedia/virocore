//
//  ARSceneController_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ARSceneController_JNI_h
#define ARSceneController_JNI_h

#include <memory>
#include <VROARSceneController.h>
#include <VROARDeclarativeSession.h>
#include <VROARImperativeSession.h>
#include <VROPlatformUtil.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ARSceneController {
    inline VRO_REF jptr(std::shared_ptr<VROARSceneController> sharedARSceneController) {
        PersistentRef<VROARSceneController> *arSceneController =
                new PersistentRef<VROARSceneController>(sharedARSceneController);
        return reinterpret_cast<intptr_t>(arSceneController);
    }

    inline std::shared_ptr<VROARSceneController> native(VRO_REF ptr) {
        PersistentRef<VROARSceneController> *persistentARSceneController =
                reinterpret_cast<PersistentRef<VROARSceneController> *>(ptr);
        return persistentARSceneController->get();
    }
}

class ARDeclarativeSceneDelegate : public VROARSceneDelegate, public VROARDeclarativeSessionDelegate {
public:
    ARDeclarativeSceneDelegate(VRO_OBJECT arSceneJavaObject, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(arSceneJavaObject)) {
    }

    virtual ~ARDeclarativeSceneDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    static VRO_REF jptr(std::shared_ptr<ARDeclarativeSceneDelegate> arSceneDelegate) {
        PersistentRef<ARDeclarativeSceneDelegate> *persistentDelegate = new PersistentRef<ARDeclarativeSceneDelegate>(arSceneDelegate);
        return reinterpret_cast<intptr_t>(persistentDelegate);
    }

    static std::shared_ptr<ARDeclarativeSceneDelegate> native(VRO_REF ptr) {
        PersistentRef<ARDeclarativeSceneDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<ARDeclarativeSceneDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onTrackingUpdated(VROARTrackingState state, VROARTrackingStateReason reason);
    void onAmbientLightUpdate(float ambientLightIntensity, float colorTemperature);
    void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor);

private:
    VRO_OBJECT _javaObject;
};

class ARImperativeSceneDelegate : public VROARSceneDelegate, public VROARImperativeSessionDelegate {
public:
    ARImperativeSceneDelegate(VRO_OBJECT arSceneJavaObject, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(arSceneJavaObject))    {
    }

    virtual ~ARImperativeSceneDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    static VRO_REF jptr(std::shared_ptr<ARImperativeSceneDelegate> arSceneDelegate) {
        PersistentRef<ARImperativeSceneDelegate> *persistentDelegate = new PersistentRef<ARImperativeSceneDelegate>(arSceneDelegate);
        return reinterpret_cast<intptr_t>(persistentDelegate);
    }

    static std::shared_ptr<ARImperativeSceneDelegate> native(VRO_REF ptr) {
        PersistentRef<ARImperativeSceneDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<ARImperativeSceneDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onTrackingUpdated(VROARTrackingState state, VROARTrackingStateReason reason);
    void onAmbientLightUpdate(float ambientLightIntensity, float colorTemperature);
    void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);

private:
    VRO_OBJECT _javaObject;
};

#endif