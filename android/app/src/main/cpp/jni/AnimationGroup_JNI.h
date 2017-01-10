//
//  AnimationGroup_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef AnimationGroup_JNI_h
#define AnimationGroup_JNI_h

#include <jni.h>
#include <memory>
#include <VROAnimationGroup.h>
#include "PersistentRef.h"

namespace AnimationGroup {
    inline jlong jptr(std::shared_ptr<VROAnimationGroup> ptr) {
        PersistentRef<VROAnimationGroup> *native_node = new PersistentRef<VROAnimationGroup>(ptr);
        return reinterpret_cast<intptr_t>(native_node);
    }

    inline std::shared_ptr<VROAnimationGroup> native(jlong ptr) {
        PersistentRef<VROAnimationGroup> *persistentNode = reinterpret_cast<PersistentRef<VROAnimationGroup> *>(ptr);
        return persistentNode->get();
    }
}

#endif