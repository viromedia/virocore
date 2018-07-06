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

class CameraImageFrameListener : public VROFrameListener {
public:
    CameraImageFrameListener(VRO_OBJECT listener_j, std::shared_ptr<VROARScene> scene, VRO_ENV env) :
            _listener_j(VRO_NEW_GLOBAL_REF(listener_j)),
            _scene(scene),
            _bufferIndex(0) {

        for (int i = 0; i < 3; i++) {
            _buffers[i] = NULL;
        }
    }

    virtual ~CameraImageFrameListener() {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_DELETE_GLOBAL_REF(_listener_j);

        for (int i = 0; i < 3; i++) {
            if (_buffers[i] != NULL) {
                VRO_DELETE_GLOBAL_REF(_buffers[i]);
            }
        }
    }

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:
    VRO_OBJECT _listener_j;
    std::weak_ptr<VROARScene> _scene;
    int _bufferIndex;
    std::shared_ptr<VROData> _data[3];
    jobject _buffers[3];
};

#endif