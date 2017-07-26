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

OBJLoaderDelegate::OBJLoaderDelegate(jobject nodeJavaObject, JNIEnv *env) {
    _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(nodeJavaObject));
}

OBJLoaderDelegate::~OBJLoaderDelegate() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteGlobalRef(_javaObject);
}

void OBJLoaderDelegate::objLoaded(std::shared_ptr<VRONode> node) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = env->NewWeakGlobalRef(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, node] {
        JNIEnv *env = VROPlatformGetJNIEnv();

        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(weakObj);
            return;
        }

        // Create a new persistent ref for the node so we can manipulate
        // it on the Java side
        jlong nodeRef = Node::jptr(node);

        VROPlatformCallJavaFunction(localObj, "nodeDidFinishCreation", "(J)V", nodeRef);
        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
}

void OBJLoaderDelegate::objAttached() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    VROPlatformCallJavaFunction(_javaObject, "nodeDidAttachOBJ", "()V");
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