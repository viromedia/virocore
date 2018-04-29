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

#include "VRODefines.h"
#include VRO_C_INCLUDE

class VROSoundDataDelegate_JNI: public VROSoundDataDelegate {
public:
    VROSoundDataDelegate_JNI(VRO_OBJECT soundDataObject, VRO_ENV env);
    ~VROSoundDataDelegate_JNI();

    void dataIsReady();
    void dataError(std::string error);
private:
    VRO_OBJECT _javaObject;
};

namespace SoundData {
    inline VRO_REF jptr(std::shared_ptr<VROSoundDataGVR> ptr) {
        PersistentRef<VROSoundDataGVR> *persistentRef = new PersistentRef<VROSoundDataGVR>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSoundDataGVR> native(VRO_REF ptr) {
        PersistentRef<VROSoundDataGVR> *persistentRef = reinterpret_cast<PersistentRef<VROSoundDataGVR> *>(ptr);
        return persistentRef->get();
    }
}

namespace SoundDataDelegate {
    inline VRO_REF jptr(std::shared_ptr<VROSoundDataDelegate_JNI> ptr) {
        PersistentRef<VROSoundDataDelegate_JNI> *persistentRef = new PersistentRef<VROSoundDataDelegate_JNI>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSoundDataDelegate_JNI> native(VRO_REF ptr) {
        PersistentRef<VROSoundDataDelegate_JNI> *persistentRef = reinterpret_cast<PersistentRef<VROSoundDataDelegate_JNI> *>(ptr);
        return persistentRef->get();
    }
}

#endif