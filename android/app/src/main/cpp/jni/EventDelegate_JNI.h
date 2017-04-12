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
#include "Node_JNI.h"

/**
 * EventDelegate_JNI implements a JNI abstraction of the VROEventDelegate to
 * both allow java objects to register for events, and to notify them of
 * delegate events across the JNI bridge.
 */
class EventDelegate_JNI : public VROEventDelegate{
public:
    EventDelegate_JNI(jobject sceneJavaObject, JNIEnv *env) {
        _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(sceneJavaObject));
    }

    ~EventDelegate_JNI() {
        JNIEnv *env = VROPlatformGetJNIEnv();
        env->DeleteGlobalRef(_javaObject);
    }

    /**
     * Java event delegates to be triggered across the JNI bridge.
     */
    void onHover(int source, bool isHovering);
    void onClick(int source, ClickState clickState);
    void onTouch(int source, TouchState touchState, float x, float y);
    void onMove(int source, VROVector3f rotation, VROVector3f position, VROVector3f forwardVec);
    void onControllerStatus(int source, ControllerStatus status);
    void onGazeHit(int source, float distance, VROVector3f hitLocation);
    void onSwipe(int source, SwipeState swipeState);
    void onScroll(int source, float x, float y);
    void onDrag(int source, VROVector3f newPosition);
    void onFuse(int source, float timeToFuseRatio);
private:
    jobject _javaObject;
    void callJavaFunction(std::string functionName, std::string methodID, ...);
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