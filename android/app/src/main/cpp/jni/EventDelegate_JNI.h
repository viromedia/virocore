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
    void onControllerStatus(ControllerStatus status);
    void onButtonEvent(EventSource type, EventAction event);
    void onTouchPadEvent(EventSource type, EventAction event, float x, float y);
    void onRotate(VROVector3f rotation);
    void onPosition(VROVector3f location);
    void onGaze(bool isGazing);
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