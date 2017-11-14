//
//  OBJLoaderDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//


#include <jni.h>
#include <memory>
#include "VROPlatformUtil.h"
#include "OBJLoaderDelegate_JNI.h"
#include "Node_JNI.h"
#include "Geometry_JNI.h"

OBJLoaderDelegate::OBJLoaderDelegate(jobject nodeJavaObject, JNIEnv *env) {
    _javaObject = env->NewWeakGlobalRef(nodeJavaObject);
}

OBJLoaderDelegate::~OBJLoaderDelegate() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteWeakGlobalRef(_javaObject);
}

void OBJLoaderDelegate::objLoaded(std::shared_ptr<VRONode> node, bool isFBX, jlong requestId) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = _javaObject;

    if (weakObj == NULL){
        return;
    }

    jobject ref =env->NewGlobalRef(_javaObject);

    // If the request is antiquated, clear the node
    jlong activeRequestID = VROPlatformCallJavaLongFunction(ref, "getActiveRequestID", "()J");
    if (activeRequestID != requestId) {
        pinfo("Received antiquated Object3D load, discarding");
        node->removeAllChildren();
        env->DeleteGlobalRef(ref);
        return;
    }

    VROPlatformDispatchAsyncApplication([ref, node, isFBX] {
        JNIEnv *env = VROPlatformGetJNIEnv();

        jobject localObj = env->NewLocalRef(ref);
        if (localObj == NULL) {
            return;
        }

        // If the request was for an OBJ, create a persistent ref for the Java Geometry and
        // pass that up as well. This enables Java SDK users to set materials on the Geometry
        long geometryRef = 0;
        if (!isFBX && node->getGeometry()) {
            geometryRef = Geometry::jptr(node->getGeometry());
        }

        VROPlatformCallJavaFunction(localObj, "nodeDidFinishCreation", "(ZJ)V", isFBX, geometryRef);
        env->DeleteLocalRef(localObj);
        env->DeleteGlobalRef(ref);
    });
}

void OBJLoaderDelegate::objFailed(std::string error) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jobject ref = env->NewGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([ref, error] {
        JNIEnv *env = VROPlatformGetJNIEnv();

        jobject localObj = env->NewLocalRef(ref);
        if (localObj == NULL) {
            return;
        }

        jstring jerror = env->NewStringUTF(error.c_str());
        VROPlatformCallJavaFunction(localObj, "nodeDidFailOBJLoad", "(Ljava/lang/String;)V", jerror);

        env->DeleteLocalRef(jerror);
        env->DeleteLocalRef(localObj);
        env->DeleteGlobalRef(ref);
    });
}