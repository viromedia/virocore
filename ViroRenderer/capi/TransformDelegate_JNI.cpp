//
//  TransformDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROPlatformUtil.h>
#include "TransformDelegate_JNI.h"

TransformDelegate_JNI::TransformDelegate_JNI(VRO_OBJECT javaDelegateObject, double distanceFilter):VROTransformDelegate(distanceFilter){
    _javaObject  = reinterpret_cast<jclass>(VROPlatformGetJNIEnv()->NewWeakGlobalRef(javaDelegateObject));
}

TransformDelegate_JNI::~TransformDelegate_JNI() {
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_javaObject);
}


void TransformDelegate_JNI::onPositionUpdate(VROVector3f position){
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, position] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        VROPlatformCallJavaFunction(localObj, "onPositionUpdate", "(FFF)V",
                                    position.x, position.y, position.z);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}