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
#include "PersistentRef.h"
#include "VROImage.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace Image {
    inline VRO_REF(VROImage) jptr(std::shared_ptr<VROImage> ptr) {
        PersistentRef<VROImage> *persistentRef = new PersistentRef<VROImage>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROImage> native(VRO_REF(VROImage) ptr) {
        PersistentRef<VROImage> *persistentRef = reinterpret_cast<PersistentRef<VROImage> *>(ptr);
        return persistentRef->get();
    }
}

#endif