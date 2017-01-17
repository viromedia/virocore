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
    VROEventDelegate::EventAction eventType
            = static_cast<VROEventDelegate::EventAction>(eventTypeId);
    EventDelegate::native(native_node_ref)->setEnabledEvent(eventType, enabled);
}

}  // extern "C"
void EventDelegate_JNI::onHover(int source, bool isHovering) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onHover", "(IZ)V", source, isHovering);
}

void EventDelegate_JNI::onClick(int source, ClickState clickState) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onClick", "(II)V", source, clickState);
}

void EventDelegate_JNI::onTouch(int source, TouchState touchState, float x, float y){
    VROPlatformCallJavaFunction(_javaObject,
                                "onTouch", "(IIFF)V", source, touchState, x, y);
}

void EventDelegate_JNI::onMove(int source, VROVector3f rotation, VROVector3f position) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onMove", "(IFFFFFF)V", source, rotation.x,rotation.y, rotation.z,
                                                            position.x, position.y, position.z);
}
void EventDelegate_JNI::onControllerStatus(int source, ControllerStatus status) {
    VROPlatformCallJavaFunction(_javaObject,
                                "onControllerStatus", "(II)V", source, status);
}

void EventDelegate_JNI::onGazeHit(int source, float distance, VROVector3f hitLocation) {
    //No-op
}