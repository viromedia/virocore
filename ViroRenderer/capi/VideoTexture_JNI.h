//
//  VideoTexture_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIDEO_TEXTURE_JNI_H
#define ANDROID_VIDEO_TEXTURE_JNI_H

#include "VROVideoDelegateInternal.h"
#include "VROVideoTexture.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace VideoTexture {
    inline VRO_REF jptr(std::shared_ptr<VROVideoTexture> ptr) {
        PersistentRef<VROVideoTexture> *persistentRef = new PersistentRef<VROVideoTexture>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROVideoTexture> native(VRO_REF ptr) {
        PersistentRef<VROVideoTexture> *persistentRef = reinterpret_cast<PersistentRef<VROVideoTexture> *>(ptr);
        return persistentRef->get();
    }
}
#endif
