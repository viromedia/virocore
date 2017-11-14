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
    _javaObject = reinterpret_cast<jclass>(env->NewWeakGlobalRef(nodeJavaObject));
}

OBJLoaderDelegate::~OBJLoaderDelegate() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteWeakGlobalRef(_javaObject);
}

void OBJLoaderDelegate::objLoaded(std::shared_ptr<VRONode> node, bool isFBX, jlong requestId) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    // If the request is antiquated, clear the node
    jlong activeRequestID = VROPlatformCallJavaLongFunction(_javaObject, "getActiveRequestID", "()J");
    if (activeRequestID != requestId) {
        pinfo("Received antiquated Object3D load, discarding");
        node->removeAllChildren();
        return;
    }

    VROPlatformDispatchAsyncApplication([weakObj, node, isFBX] {
        JNIEnv *env = VROPlatformGetJNIEnv();

        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(weakObj);
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
        env->DeleteWeakGlobalRef(weakObj);
    });
}

void OBJLoaderDelegate::objFailed(std::string error) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, error] {
        JNIEnv *env = VROPlatformGetJNIEnv();

        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(weakObj);
            return;
        }

        jstring jerror = env->NewStringUTF(error.c_str());
        VROPlatformCallJavaFunction(localObj, "nodeDidFailOBJLoad", "(Ljava/lang/String;)V", jerror);

        env->DeleteLocalRef(localObj);
        env->DeleteLocalRef(jerror);
        env->DeleteWeakGlobalRef(weakObj);
    });
}