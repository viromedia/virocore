//
//  PhysicsDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_PHYSICS_DELEGATE_JNI_H
#define ANDROID_PHYSICS_DELEGATE_JNI_H

#include "VROPhysicsBodyDelegate.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

/**
 * PhysicsDelegate_JNI implements a JNI abstraction of the VROPhysicsBodyDelegate to
 * both allow java objects to register for, and to notify them of physics delegate
 * events across the JNI bridge.
 */
class PhysicsDelegate_JNI : public VROPhysicsBodyDelegate {

public:
    PhysicsDelegate_JNI(VRO_OBJECT videoJavaObject);
    ~PhysicsDelegate_JNI();

    void onCollided(std::string key, VROPhysicsBody::VROCollision collision);

private:
        VRO_OBJECT _javaObject;
};
#endif
