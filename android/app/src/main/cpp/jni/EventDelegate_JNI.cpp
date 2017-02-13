//
//  EventDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "EventDelegate_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_EventDelegateJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateDelegate)(JNIEnv *env,
                                        jobject obj) {
   std::shared_ptr<EventDelegate_JNI> delegate
           = std::make_shared<EventDelegate_JNI>(obj, env);
   return EventDelegate::jptr(delegate);
}

JNI_METHOD(void, nativeDestroyDelegate)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_node_ref) {
  delete reinterpret_cast<PersistentRef<VRONode> *>(native_node_ref);
}

JNI_METHOD(void, nativeEnableEvent)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_node_ref,
                                        jint eventTypeId,
                                        jboolean enabled) {
    VROPlatformDispatchAsyncRenderer([native_node_ref, eventTypeId, enabled] {
        VROEventDelegate::EventAction eventType
                = static_cast<VROEventDelegate::EventAction>(eventTypeId);
        EventDelegate::native(native_node_ref)->setEnabledEvent(eventType, enabled);
    });
}

}  // extern "C"
void EventDelegate_JNI::onHover(int source, bool isHovering) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, isHovering]{
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onHover", "(IZ)V", source, isHovering);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onClick(int source, ClickState clickState) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, clickState] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onClick", "(II)V", source, clickState);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onTouch(int source, TouchState touchState, float x, float y){
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, touchState, x, y] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onTouch", "(IIFF)V", source, touchState, x, y);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onMove(int source, VROVector3f rotation, VROVector3f position) {
    //No-op
}
void EventDelegate_JNI::onControllerStatus(int source, ControllerStatus status) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, status] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onControllerStatus", "(II)V", source, status);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onGazeHit(int source, float distance, VROVector3f hitLocation) {
    //No-op
}

void EventDelegate_JNI::onSwipe(int source, SwipeState swipeState) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, swipeState] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onSwipe", "(II)V", source, swipeState);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onScroll(int source, float x, float y) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, x, y] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onScroll", "(IFF)V", source, x, y);
        env->DeleteLocalRef(localObj);
    });
}