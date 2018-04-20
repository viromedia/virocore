//
//  Image_JNI.h
//  ViroRenderer
//
//  Created by Andy Chu on 12/01/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef Image_JNI_h
#define Image_JNI_h

#include <memory>
#include "VROImageAndroid.h"
#include "PersistentRef.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Image {
    inline VRO_REF jptr(std::shared_ptr<VROImageAndroid> ptr) {
        PersistentRef<VROImageAndroid> *persistentRef = new PersistentRef<VROImageAndroid>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROImageAndroid> native(VRO_REF ptr) {
        PersistentRef<VROImageAndroid> *persistentRef = reinterpret_cast<PersistentRef<VROImageAndroid> *>(ptr);
        return persistentRef->get();
    }
}

#endif