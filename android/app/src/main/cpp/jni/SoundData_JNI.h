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

class VROSoundDataDelegate_JNI: public VROSoundDataDelegate {
public:
    VROSoundDataDelegate_JNI(jobject soundDataObject, JNIEnv *env);
    ~VROSoundDataDelegate_JNI();

    void dataIsReady();
    void dataError(std::string error);
private:
    jobject _javaObject;
};

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

namespace SoundDataDelegate {
    inline jlong jptr(std::shared_ptr<VROSoundDataDelegate_JNI> ptr) {
        PersistentRef<VROSoundDataDelegate_JNI> *persistentRef = new PersistentRef<VROSoundDataDelegate_JNI>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    inline std::shared_ptr<VROSoundDataDelegate_JNI> native(jlong ptr) {
        PersistentRef<VROSoundDataDelegate_JNI> *persistentRef = reinterpret_cast<PersistentRef<VROSoundDataDelegate_JNI> *>(ptr);
        return persistentRef->get();
    }
}

#endif