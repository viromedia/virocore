//
//  SoundData_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef SoundData_JNI_h
#define SoundData_JNI_h

#include <memory>
#include <VROSoundDataGVR.h>
#include "PersistentRef.h"

namespace SoundData {
    inline jlong jptr(std::shared_ptr<VROSoundDataGVR> ptr) {
        PersistentRef<VROSoundDataGVR> *persistentRef = new PersistentRef<VROSoundDataGVR>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSoundDataGVR> native(jlong ptr) {
        PersistentRef<VROSoundDataGVR> *persistentRef = reinterpret_cast<PersistentRef<VROSoundDataGVR> *>(ptr);
        return persistentRef->get();
    }
}

#endif