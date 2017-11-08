//
//  EventDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "EventDelegate_JNI.h"
#include "Node_JNI.h"

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
    // TODO: figure out why this is needed
    VROPlatformDispatchAsyncRenderer([native_node_ref]{
        delete reinterpret_cast<PersistentRef<VRONode> *>(native_node_ref);
    });
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
        VROEventDelegate::EventAction eventType = static_cast<VROEventDelegate::EventAction>(eventTypeId);
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
void EventDelegate_JNI::onHover(int source, std::shared_ptr<VRONode> node, bool isHovering, std::vector<float> position) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, isHovering, position] {
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

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onHover", "(IIZ[F)V", source, nodeId, isHovering, positionArray);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState, std::vector<float> position) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, clickState, position] {
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

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onClick", "(III[F)V", source, nodeId, clickState, positionArray);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onTouch(int source, std::shared_ptr<VRONode> node, TouchState touchState, float x, float y){
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, touchState, x, y] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onTouch", "(IIIFF)V", source, nodeId, touchState, x, y);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onMove(int source, std::shared_ptr<VRONode> node, VROVector3f rotation, VROVector3f position, VROVector3f forwardVec) {
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

void EventDelegate_JNI::onGazeHit(int source, std::shared_ptr<VRONode> node, float distance, VROVector3f hitLocation) {
    //No-op
}

void EventDelegate_JNI::onSwipe(int source, std::shared_ptr<VRONode> node, SwipeState swipeState) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, swipeState] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onSwipe", "(III)V", source, nodeId, swipeState);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onScroll(int source, std::shared_ptr<VRONode> node, float x, float y) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, x, y] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onScroll", "(IIFF)V", source, nodeId, x, y);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onDrag(int source, std::shared_ptr<VRONode> node, VROVector3f newPosition) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, newPosition] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onDrag", "(IIFFF)V", source, nodeId, newPosition.x, newPosition.y,
                                    newPosition.z);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onFuse(int source, std::shared_ptr<VRONode> node, float timeToFuseRatio){
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

    VROPlatformDispatchAsyncApplication([weakObj, source, node, timeToFuseRatio] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onFuse", "(II)V", source, nodeId, timeToFuseRatio);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onPinch(int source, std::shared_ptr<VRONode> node, float scaleFactor, PinchState pinchState) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, scaleFactor, pinchState] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onPinch", "(IIFI)V", source, nodeId, scaleFactor, pinchState);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onRotate(int source, std::shared_ptr<VRONode> node, float rotationRadians, RotateState rotateState) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, node, rotationRadians, rotateState] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        int nodeId = node->getUniqueID();
        VROPlatformCallJavaFunction(localObj,
                                    "onRotate", "(IIFI)V", source, nodeId, rotationRadians, rotateState);
        env->DeleteLocalRef(localObj);
    });
}

void EventDelegate_JNI::onCameraARHitTest(int source, std::vector<VROARHitTestResult> results) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, source, results] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jclass arHitTestResultClass = env->FindClass("com/viro/renderer/ARHitTestResult");

        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }
        //start code to convert VROARHitTestResult to ARHitTestResult
        jobjectArray resultsArray = env->NewObjectArray(results.size(), arHitTestResultClass, NULL);
        for (int i = 0; i < results.size(); i++) {
            VROARHitTestResult result = results[i];

            jmethodID constructorMethod = env->GetMethodID(arHitTestResultClass, "<init>",
                                                           "(Ljava/lang/String;[F[F[F)V");
            jstring jtypeString;
            jfloatArray jposition = env->NewFloatArray(3);
            jfloatArray jscale = env->NewFloatArray(3);
            jfloatArray jrotation = env->NewFloatArray(3);

            VROVector3f positionVec = result.getWorldTransform().extractTranslation();
            VROVector3f scaleVec = result.getWorldTransform().extractScale();
            VROVector3f rotationVec = result.getWorldTransform().extractRotation(
                    scaleVec).toEuler();

            float position[3] = {positionVec.x, positionVec.y, positionVec.z};
            float scale[3] = {scaleVec.x, scaleVec.y, scaleVec.z};
            float rotation[3] = {rotationVec.x, rotationVec.y, rotationVec.z};

            env->SetFloatArrayRegion(jposition, 0, 3, position);
            env->SetFloatArrayRegion(jscale, 0, 3, scale);
            env->SetFloatArrayRegion(jrotation, 0, 3, rotation);

            const char *typeString;
                    // Note: ARCore currently only supports Plane & FeaturePoint. See VROARFrameARCore::hitTest.
            switch (result.getType()) {
                case VROARHitTestResultType::ExistingPlaneUsingExtent:
                    typeString = "ExistingPlaneUsingExtent";
                    break;
                default: // FeaturePoint
                    typeString = "FeaturePoint";
                    break;
            }

            jtypeString = env->NewStringUTF(typeString);

            jobject jresult = env->NewObject(arHitTestResultClass, constructorMethod, jtypeString,
                                             jposition, jscale, jrotation);

            env->SetObjectArrayElement(resultsArray, i, jresult);
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onCameraARHitTest", "(I[Lcom/viro/renderer/ARHitTestResult;)V", source, resultsArray);
        env->DeleteLocalRef(localObj);
    });
}
