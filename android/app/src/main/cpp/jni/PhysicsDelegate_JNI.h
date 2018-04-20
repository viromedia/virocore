//
//  PhysicsDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_PHYSICS_DELEGATE_JNI_H
#define ANDROID_PHYSICS_DELEGATE_JNI_H

#include <PersistentRef.h>
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
    PhysicsDelegate_JNI(jobject videoJavaObject);
    ~PhysicsDelegate_JNI();

    static VRO_REF jptr(std::shared_ptr<PhysicsDelegate_JNI> shared_node) {
        PersistentRef<PhysicsDelegate_JNI> *native_surface = new PersistentRef<PhysicsDelegate_JNI>(shared_node);
        return reinterpret_cast<intptr_t>(native_surface);
    }

    static std::shared_ptr<PhysicsDelegate_JNI> native(VRO_REF ptr) {
        PersistentRef<PhysicsDelegate_JNI> *persistentSurface = reinterpret_cast<PersistentRef<PhysicsDelegate_JNI> *>(ptr);
        return persistentSurface->get();
    }

    void onCollided(std::string key, VROPhysicsBody::VROCollision collision);

private:
        jobject _javaObject;
};
#endif
