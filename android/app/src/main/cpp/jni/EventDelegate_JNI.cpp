//
//  EventDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "EventDelegate_JNI.h"
#include "Node_JNI.h"
#include "ARUtils_JNI.h"
#include "VROARPointCloud.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_EventDelegate_##method_name
#endif

extern "C" {

VRO_METHOD(VRO_REF, nativeCreateDelegate)(VRO_NO_ARGS) {
   std::shared_ptr<EventDelegate_JNI> delegate
           = std::make_shared<EventDelegate_JNI>(obj, env);
   return EventDelegate::jptr(delegate);
}

VRO_METHOD(void, nativeDestroyDelegate)(VRO_ARGS
                                        VRO_REF native_node_ref) {
    // TODO: figure out why this is needed
    VROPlatformDispatchAsyncRenderer([native_node_ref]{
        delete reinterpret_cast<PersistentRef<VRONode> *>(native_node_ref);
    });
}

VRO_METHOD(void, nativeEnableEvent)(VRO_ARGS
                                    VRO_REF native_node_ref,
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

VRO_METHOD(void, nativeSetTimeToFuse)(VRO_ARGS
                                      VRO_REF native_node_ref,
                                      VRO_FLOAT durationInMillis) {
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

/*
 This integer represents a value which no Node should return when getUniqueID()
 is invoked. This value is derived from the behavior of sUniqueIDGenerator in
 VRONode.h.
 */
static int sNullNodeID = -1;

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

            VRO_FLOAT tempArr[returnLength];
            tempArr[0] = position.at(0);
            tempArr[1] = position.at(1);
            tempArr[2] = position.at(2);
            env->SetFloatArrayRegion(positionArray, 0, 3, tempArr);
        } else {
            positionArray = nullptr;
        }

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onHover", "(IIZ[F)V", source, nodeId, isHovering, positionArray);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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
            VRO_FLOAT tempArr[returnLength];
            tempArr[0] = position.at(0);
            tempArr[1] = position.at(1);
            tempArr[2] = position.at(2);
            env->SetFloatArrayRegion(positionArray, 0, 3, tempArr);
        } else {
            positionArray = nullptr;
        }

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onClick", "(III[F)V", source, nodeId, clickState, positionArray);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onTouch", "(IIIFF)V", source, nodeId, touchState, x, y);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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
        env->DeleteWeakGlobalRef(weakObj);
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

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onSwipe", "(III)V", source, nodeId, swipeState);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onScroll", "(IIFF)V", source, nodeId, x, y);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onDrag", "(IIFFF)V", source, nodeId, newPosition.x, newPosition.y,
                                    newPosition.z);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onFuse", "(II)V", source, nodeId, timeToFuseRatio);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onPinch", "(IIFI)V", source, nodeId, scaleFactor, pinchState);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
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

        int nodeId = node != nullptr ? node->getUniqueID() : sNullNodeID;
        VROPlatformCallJavaFunction(localObj,
                                    "onRotate", "(IIFI)V", source, nodeId, rotationRadians, rotateState);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}

void EventDelegate_JNI::onCameraARHitTest(std::vector<VROARHitTestResult> results) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, results] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jclass arHitTestResultClass = env->FindClass("com/viro/core/ARHitTestResult");

        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }
        //start code to convert VROARHitTestResult to ARHitTestResult
        jobjectArray resultsArray = env->NewObjectArray(results.size(), arHitTestResultClass, NULL);
        for (int i = 0; i < results.size(); i++) {
            jobject jresult = ARUtilsCreateARHitTestResult(results[i]);
            env->SetObjectArrayElement(resultsArray, i, jresult);
        }

        VROPlatformCallJavaFunction(localObj,
                                    "onCameraARHitTest", "([Lcom/viro/core/ARHitTestResult;)V", resultsArray);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}

void EventDelegate_JNI::onARPointCloudUpdate(std::shared_ptr<VROARPointCloud> pointCloud) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    // So it turns out that ARCore returns us garbage values (NaN) when the camera is obscured
    // so, rather than waste all the time going up to Java, check 1 value right here and
    // return if it is a NaN.
    if (isnan(pointCloud->getPoints()[0].x)) {
        return;
    }

    VROPlatformDispatchAsyncApplication([weakObj, pointCloud] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        jobject jPointCloud = ARUtilsCreateARPointCloud(pointCloud);
        VROPlatformCallJavaFunction(localObj, "onARPointCloudUpdate", "(Lcom/viro/core/ARPointCloud;)V", jPointCloud);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}
