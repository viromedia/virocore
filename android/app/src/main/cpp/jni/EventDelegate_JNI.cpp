//
//  EventDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "EventDelegate_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_EventDelegate_##method_name

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

    std::weak_ptr<EventDelegate_JNI> delegate_w = EventDelegate::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([delegate_w, eventTypeId, enabled] {
        std::shared_ptr<EventDelegate_JNI> delegate = delegate_w.lock();
        if (!delegate) {
            return;
        }
        VROEventDelegate::EventAction eventType
                = static_cast<VROEventDelegate::EventAction>(eventTypeId);
        delegate->setEnabledEvent(eventType, enabled);
    });
}

JNI_METHOD(void, nativeSetTimeToFuse)(JNIEnv *env,
                                        jclass clazz,
                                        jlong native_node_ref,
                                      jfloat durationInMillis) {
    std::weak_ptr<EventDelegate_JNI> delegate_w = EventDelegate::native(native_node_ref);
    VROPlatformDispatchAsyncRenderer([delegate_w, durationInMillis] {
        std::shared_ptr<EventDelegate_JNI> delegate = delegate_w.lock();
        if (!delegate) {
            return;
        }
        delegate->setTimeToFuse(durationInMillis);
    });
}

}  // extern "C"
void EventDelegate_JNI::onHover(int source, bool isHovering, std::vector<float> position) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, isHovering, position] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        jfloatArray positionArray;
        if (position.size() == 3) {
            int returnLength = 3;
            positionArray = env->NewFloatArray(returnLength);

            jfloat tempArr[returnLength];
            tempArr[0] = position.at(0);
            tempArr[1] = position.at(1);
            tempArr[2] = position.at(2);
            env->SetFloatArrayRegion(positionArray, 0, 3, tempArr);
        } else {
            positionArray = nullptr;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onHover", "(IZ[F)V", source, isHovering, positionArray);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onClick(int source, ClickState clickState, std::vector<float> position) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, clickState, position] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        jfloatArray positionArray;
        if (position.size() == 3) {
            int returnLength = 3;
            positionArray = env->NewFloatArray(returnLength);
            jfloat tempArr[returnLength];
            tempArr[0] = position.at(0);
            tempArr[1] = position.at(1);
            tempArr[2] = position.at(2);
            env->SetFloatArrayRegion(positionArray, 0, 3, tempArr);
        } else {
            positionArray = nullptr;
        }
        VROPlatformCallJavaFunction(localObj,
                                    "onClick", "(II[F)V", source, clickState, positionArray);
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

void EventDelegate_JNI::onMove(int source, VROVector3f rotation, VROVector3f position, VROVector3f forwardVec) {
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

void EventDelegate_JNI::onDrag(int source, VROVector3f newPosition) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, newPosition] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onDrag", "(IFFF)V", source, newPosition.x, newPosition.y,
                                    newPosition.z);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onFuse(int source, float timeToFuseRatio){
    /**
     * As onFuse is also used by internal components to update ui based
     * on timeToFuse ratio, we only want to notify our bridge components
     * if we have successfully fused (if timeToFuseRatio has counted down to 0).
     */
    if (timeToFuseRatio > 0.0f){
        return;
    }

    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, timeToFuseRatio] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onFuse", "(I)V", source, timeToFuseRatio);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onPinch(int source, float scaleFactor, PinchState pinchState) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, scaleFactor, pinchState] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onPinch", "(IFI)V", source, scaleFactor, pinchState);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onRotate(int source, float rotateDegrees, RotateState rotateState) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, rotateDegrees, rotateState] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onRotate", "(IFI)V", source, rotateDegrees, rotateState);
        env->DeleteLocalRef(localObj);
    });
}