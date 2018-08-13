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

#include "VRODefines.h"
#include VRO_C_INCLUDE

class ARDeclarativeSceneDelegate : public VROARSceneDelegate, public VROARDeclarativeSessionDelegate {
public:
    ARDeclarativeSceneDelegate(VRO_OBJECT arSceneJavaObject, VRO_ENV env) :
        _javaObject(VRO_NEW_WEAK_GLOBAL_REF(arSceneJavaObject)) {
    }

    virtual ~ARDeclarativeSceneDelegate() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
    }

    void onTrackingUpdated(VROARTrackingState state, VROARTrackingStateReason reason);
    void onAmbientLightUpdate(float intensity, VROVector3f color);
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

    void onTrackingUpdated(VROARTrackingState state, VROARTrackingStateReason reason);
    void onAmbientLightUpdate(float intensity, VROVector3f color);
    void anchorWasDetected(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWillUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorDidUpdate(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);
    void anchorWasRemoved(std::shared_ptr<VROARAnchor> anchor, std::shared_ptr<VROARNode> node);

private:
    VRO_OBJECT _javaObject;
};

#endif