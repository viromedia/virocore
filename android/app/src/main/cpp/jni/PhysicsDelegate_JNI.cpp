//
//  PhysicsDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <jni.h>
#include <memory>
#include <VROPlatformUtil.h>
#include "VRONode.h"
#include "PhysicsDelegate_JNI.h"
#include "VROLog.h"

PhysicsDelegate_JNI::PhysicsDelegate_JNI(jobject obj){
    _javaObject = VROPlatformGetJNIEnv()->NewWeakGlobalRef(obj);
}

PhysicsDelegate_JNI::~PhysicsDelegate_JNI() {
    VROPlatformGetJNIEnv()->DeleteWeakGlobalRef(_javaObject);
}

void PhysicsDelegate_JNI::onCollided(std::string key, VROPhysicsBody::VROCollision collision) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jweak weakObj = _javaObject;

    VROPlatformDispatchAsyncApplication([weakObj, collision] {
        JNIEnv *env = VROPlatformGetJNIEnv();
        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            return;
        }

        jstring jCollidedBodyTag = env->NewStringUTF(collision.collidedBodyTag.c_str());
        VROPlatformCallJavaFunction(localObj, "onCollided", "(Ljava/lang/String;FFFFFF)V",
                                    jCollidedBodyTag,
                                    collision.collidedPoint.x,
                                    collision.collidedPoint.y,
                                    collision.collidedPoint.z,
                                    collision.collidedNormal.x,
                                    collision.collidedNormal.y,
                                    collision.collidedNormal.z);

        env->DeleteLocalRef(localObj);
    });
}
