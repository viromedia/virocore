//
//  ExecutableAnimation_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ExecutableAnimation_JNI_h
#define ExecutableAnimation_JNI_h

#include <memory>
#include <VROExecutableAnimation.h>
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace ExecutableAnimation {
    inline VRO_REF(VROExecutableAnimation) jptr(std::shared_ptr<VROExecutableAnimation> ptr) {
        PersistentRef<VROExecutableAnimation> *native = new PersistentRef<VROExecutableAnimation>(ptr);
        return reinterpret_cast<intptr_t>(native);
    }

    inline std::shared_ptr<VROExecutableAnimation> native(VRO_REF(VROExecutableAnimation) ptr) {
        PersistentRef<VROExecutableAnimation> *persistentAnim = reinterpret_cast<PersistentRef<VROExecutableAnimation> *>(ptr);
        return persistentAnim->get();
    }
}

#endif
