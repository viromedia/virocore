//
//  Image_JNI.h
//  ViroRenderer
//
//  Created by Andy Chu on 12/01/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include <memory>

#include "VROImageAndroid.h"
#include "PersistentRef.h"

namespace Image {
    inline jlong jptr(std::shared_ptr<VROImageAndroid> ptr) {
        PersistentRef<VROImageAndroid> *persistentRef = new PersistentRef<VROImageAndroid>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROImageAndroid> native(jlong ptr) {
        PersistentRef<VROImageAndroid> *persistentRef = reinterpret_cast<PersistentRef<VROImageAndroid> *>(ptr);
        return persistentRef->get();
    }
}