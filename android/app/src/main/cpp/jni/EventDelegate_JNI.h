//
//  EventDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef EventDelegate_JNI_h
#define EventDelegate_JNI_h

#include <jni.h>
#include <memory>
#include <stdarg.h>
#include <iostream>
#include <VRONode.h>
#include <VROBillboardConstraint.h>
#include <VROPlatformUtil.h>
#include "PersistentRef.h"
#include "VROGeometry.h"
#include "VRONode.h"
#include "VROARHitTestResult.h"
#include "Node_JNI.h"

/**
 * EventDelegate_JNI implements a JNI abstraction of the VROEventDelegate to
 * both allow java objects to register for events, and to notify them of
 * delegate events across the JNI bridge.
 */
class EventDelegate_JNI : public VROEventDelegate {
public:
    EventDelegate_JNI(jobject sceneJavaObject, JNIEnv *env) {
        _javaObject = reinterpret_cast<jclass>(env->NewWeakGlobalRef(sceneJavaObject));
    }

    ~EventDelegate_JNI() {
        JNIEnv *env = VROPlatformGetJNIEnv();
        env->DeleteWeakGlobalRef(_javaObject);
    }

    /*
     Java event delegates to be triggered across the JNI bridge.
     */
    void onHover(int source, std::shared_ptr<VRONode> node, bool isHovering, std::vector<float> position);
    void onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position);
    void onTouch(int source, std::shared_ptr<VRONode> node, TouchState touchState, float x, float y);
    void onMove(int source, std::shared_ptr<VRONode> node, VROVector3f rotation, VROVector3f position, VROVector3f forwardVec);
    void onControllerStatus(int source, ControllerStatus status);
    void onGazeHit(int source, std::shared_ptr<VRONode> node, float distance, VROVector3f hitLocation);
    void onSwipe(int source, std::shared_ptr<VRONode> node, SwipeState swipeState);
    void onScroll(int source, std::shared_ptr<VRONode> node, float x, float y);
    void onDrag(int source, std::shared_ptr<VRONode> node, VROVector3f newPosition);
    void onFuse(int source, std::shared_ptr<VRONode> node, float timeToFuseRatio);
    void onPinch(int source, std::shared_ptr<VRONode> node, float scaleFactor, PinchState pinchState);
    void onRotate(int source, std::shared_ptr<VRONode> node, float rotateDegrees, RotateState rotateState);
    void onCameraARHitTest(std::vector<VROARHitTestResult> results);
    void onARPointCloudUpdate(std::shared_ptr<VROARPointCloud> pointCloud);


private:
    jobject _javaObject;
};

namespace EventDelegate{
    inline jlong jptr(std::shared_ptr<EventDelegate_JNI> delegate) {
        PersistentRef<EventDelegate_JNI> *nativeDelegate
                = new PersistentRef<EventDelegate_JNI>(delegate);
        return reinterpret_cast<intptr_t>(nativeDelegate);
    }

    inline std::shared_ptr<EventDelegate_JNI> native(jlong ptr) {
        PersistentRef<EventDelegate_JNI> *persistentObject
                = reinterpret_cast<PersistentRef<EventDelegate_JNI> *>(ptr);
        return persistentObject->get();
    }
}

#endif