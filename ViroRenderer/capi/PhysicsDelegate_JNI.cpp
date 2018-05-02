//
//  PhysicsDelegate_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <memory>
#include <VROPlatformUtil.h>
#include "VRONode.h"
#include "PhysicsDelegate_JNI.h"
#include "VROLog.h"

PhysicsDelegate_JNI::PhysicsDelegate_JNI(VRO_OBJECT obj) :
    _javaObject(VRO_OBJECT_NULL) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    _javaObject = VRO_NEW_WEAK_GLOBAL_REF(obj);
}

PhysicsDelegate_JNI::~PhysicsDelegate_JNI() {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_DELETE_WEAK_GLOBAL_REF(_javaObject);
}

void PhysicsDelegate_JNI::onCollided(std::string key, VROPhysicsBody::VROCollision collision) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_WEAK weakObj = VRO_NEW_WEAK_GLOBAL_REF(_javaObject);

    VROPlatformDispatchAsyncApplication([weakObj, collision] {
        VRO_ENV env = VROPlatformGetJNIEnv();
        VRO_OBJECT localObj = VRO_NEW_LOCAL_REF(weakObj);
        if (VRO_IS_OBJECT_NULL(localObj)) {
            return;
        }

        VRO_STRING jCollidedBodyTag = VRO_NEW_STRING(collision.collidedBodyTag.c_str());
        VROPlatformCallHostFunction(localObj, "onCollided", "(Ljava/lang/String;FFFFFF)V",
                                    jCollidedBodyTag,
                                    collision.collidedPoint.x,
                                    collision.collidedPoint.y,
                                    collision.collidedPoint.z,
                                    collision.collidedNormal.x,
                                    collision.collidedNormal.y,
                                    collision.collidedNormal.z);

        VRO_DELETE_LOCAL_REF(localObj);
        VRO_DELETE_WEAK_GLOBAL_REF(weakObj);
    });
}
