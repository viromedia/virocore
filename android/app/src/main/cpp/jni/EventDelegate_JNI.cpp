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
    VROEventDelegate::EventType eventType
            = static_cast<VROEventDelegate::EventType>(eventTypeId);
    EventDelegate::native(native_node_ref)->setEnabledEvent(eventType, enabled);
}

}  // extern "C"

void EventDelegate_JNI::onTapped() {
    VROPlatformCallJavaFunction(_javaObject, "onTapped", "()V");
}

void EventDelegate_JNI::onGaze(bool isGazing) {
    VROPlatformCallJavaFunction(_javaObject, "onGaze", "(Z)V", isGazing);
}
