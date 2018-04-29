//
//  VideoDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROVideoSurface.h>
#include <VROVideoTextureAVP.h>
#include <VROPlatformUtil.h>
#include "VRONode.h"
#include "VideoDelegate_JNI.h"
#include "VROLog.h"

VideoDelegate::VideoDelegate(VRO_OBJECT javaVideoObject){
    _javaObject = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewWeakGlobalRef(javaVideoObject));
}

VideoDelegate::~VideoDelegate() {
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_javaObject);
}

void VideoDelegate::videoWillBuffer() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    jweak weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerWillBuffer", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::videoDidBuffer() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    jweak weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerDidBuffer", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::videoDidFinish() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    jweak weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "playerDidFinishPlaying", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::videoDidFail(std::string error) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    jweak weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, error] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VRO_STRING jerror = VRO_NEW_STRING(error.c_str());
        VROPlatformCallJavaFunction(localObj, "onVideoFailed", "(Ljava/lang/String;)V", jerror);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_LOCAL_REF(jerror);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::onReady() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    jweak weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(weakObj, "onReady", "()V");
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}

void VideoDelegate::onVideoUpdatedTime(float currentTimeInSeconds, float totalTimeInSeconds){
    VRO_ENV env = VROPlatformGetJNIEnv();
    jweak weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, currentTimeInSeconds, totalTimeInSeconds] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(weakObj, "onVideoUpdatedTime", "(FF)V",
                                    currentTimeInSeconds, totalTimeInSeconds);
        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}