//
//  VideoTexture_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIDEO_TEXTURE_JNI_H
#define ANDROID_VIDEO_TEXTURE_JNI_H

#include "VROVideoDelegateInternal.h"
#include "VROVideoTextureAVP.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace VideoTexture {
    inline VRO_REF jptr(std::shared_ptr<VROVideoTextureAVP> ptr) {
        PersistentRef<VROVideoTextureAVP> *persistentRef = new PersistentRef<VROVideoTextureAVP>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROVideoTextureAVP> native(VRO_REF ptr) {
        PersistentRef<VROVideoTextureAVP> *persistentRef = reinterpret_cast<PersistentRef<VROVideoTextureAVP> *>(ptr);
        return persistentRef->get();
    }
}
#endif
