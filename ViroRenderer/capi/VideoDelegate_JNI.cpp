//
//  VideoDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <memory>
#include "VROVideoSurface.h"
#include "VROPlatformUtil.h"
#include "VRONode.h"
#include "VideoDelegate_JNI.h"
#include "VROLog.h"

VideoDelegate::VideoDelegate(VRO_OBJECT obj) :
    _javaObject(VRO_OBJECT_NULL) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    _javaObject = VRO_NEW_WEAK_GLOBAL_REF(obj);
}

VideoDelegate::~VideoDelegate() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
}

void VideoDelegate::videoWillBuffer() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerWillBuffer", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::videoDidBuffer() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerDidBuffer", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::videoDidFinish() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerDidFinishPlaying", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::videoDidFail(std::string error) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, error] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VRO_STRING jerror = VRO_NEW_STRING(error.c_str());
        VROPlatformCallJavaFunction(localObj, "onVideoFailed", "(Ljava/lang/String;)V", VRO_STRING_POD(jerror));
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_LOCAL_REF(jerror);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::onReady() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VROPlatformCallJavaFunction(weakObj, "onReady", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::onVideoUpdatedTime(float currentTimeInSeconds, float totalTimeInSeconds){
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, currentTimeInSeconds, totalTimeInSeconds] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VROPlatformCallJavaFunction(weakObj, "onVideoUpdatedTime", "(FF)V",
                                    currentTimeInSeconds, totalTimeInSeconds);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}