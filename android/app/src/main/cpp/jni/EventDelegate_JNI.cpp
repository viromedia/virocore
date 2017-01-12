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
    VROEventDelegate::EventSource eventType
            = static_cast<VROEventDelegate::EventSource>(eventTypeId);
    EventDelegate::native(native_node_ref)->setEnabledEvent(eventType, enabled);
}

}  // extern "C"

void EventDelegate_JNI::onControllerStatus(ControllerStatus status) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onControllerStatus", "(I)V", status);
}

void EventDelegate_JNI::onButtonEvent(EventSource type, EventAction event) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onButtonEvent", "(II)V", type, event);
}

void EventDelegate_JNI::onTouchPadEvent(EventSource type, EventAction event, float x, float y){
    VROPlatformCallJavaFunction(_javaObject,
                                "onTouchpadEvent", "(IIFF)V", type, event, x, y);
}

void EventDelegate_JNI::onRotate(VROVector3f rotation) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onRotate", "(FFF)V", rotation.x,rotation.y, rotation.z);
}

void EventDelegate_JNI::onPosition(VROVector3f location) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onPosition", "(FFF)V", location.x, location.y, location.z);
}

void EventDelegate_JNI::onGaze(bool isGazing){
    VROPlatformCallJavaFunction(_javaObject,
                                "onGaze", "(Z)V", isGazing);
}