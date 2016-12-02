//
//  Texture_JNI.h
//  ViroRenderer
//
//  Created by Andy Chu on 12/01/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>

#include "VROTexture.h"
#include "PersistentRef.h"

namespace Texture {
    inline jlong jptr(std::shared_ptr<VROTexture> ptr) {
        PersistentRef<VROTexture> *persistentRef = new PersistentRef<VROTexture>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROTexture> native(jlong ptr) {
        PersistentRef<VROTexture> *persistentRef = reinterpret_cast<PersistentRef<VROTexture> *>(ptr);
        return persistentRef->get();
    }
}