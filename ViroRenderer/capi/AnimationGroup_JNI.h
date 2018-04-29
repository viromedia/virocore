//
//  AnimationGroup_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef AnimationGroup_JNI_h
#define AnimationGroup_JNI_h

#include <memory>
#include <VROAnimationGroup.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace AnimationGroup {
    inline VRO_REF jptr(std::shared_ptr<VROAnimationGroup> ptr) {
        PersistentRef<VROAnimationGroup> *native_node = new PersistentRef<VROAnimationGroup>(ptr);
        return reinterpret_cast<intptr_t>(native_node);
    }

    inline std::shared_ptr<VROAnimationGroup> native(VRO_REF ptr) {
        PersistentRef<VROAnimationGroup> *persistentNode = reinterpret_cast<PersistentRef<VROAnimationGroup> *>(ptr);
        return persistentNode->get();
    }
}

#endif