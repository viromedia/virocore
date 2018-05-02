//
// Created by Raj Advani on 10/16/17.
//

#ifndef ANDROID_PHYSICSBODY_JNI_H
#define ANDROID_PHYSICSBODY_JNI_H

#include <memory>
#include "VROPhysicsBody.h"
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace PhysicsBody {
    inline VRO_REF(VROPhysicsBody) jptr(std::shared_ptr<VROPhysicsBody> physicsBody) {
        PersistentRef<VROPhysicsBody> *nativePhysics = new PersistentRef<VROPhysicsBody>(physicsBody);
        return reinterpret_cast<intptr_t>(nativePhysics);
    }

    inline std::shared_ptr<VROPhysicsBody> native(VRO_REF(VROPhysicsBody) ptr) {
        PersistentRef<VROPhysicsBody> *persistentPhysics = reinterpret_cast<PersistentRef<VROPhysicsBody> *>(ptr);
        return persistentPhysics->get();
    }
}

#endif //ANDROID_PHYSICSBODY_JNI_H
