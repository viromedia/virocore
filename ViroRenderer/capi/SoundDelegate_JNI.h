//
//  SoundDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIDEO_DELEGATE_JNI_H
#define ANDROID_VIDEO_DELEGATE_JNI_H

#include <memory>
#include "PersistentRef.h"
#include "VROSoundDelegateInternal.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class SoundDelegate : public VROSoundDelegateInternal {

public:
    SoundDelegate(VRO_OBJECT soundObjectJava);
    ~SoundDelegate();

    static VRO_REF jptr(std::shared_ptr<SoundDelegate> delegate) {
        return reinterpret_cast<intptr_t>(new PersistentRef<SoundDelegate>(delegate));
    }

    static std::shared_ptr<SoundDelegate> native(VRO_REF ptr) {
        return reinterpret_cast<PersistentRef<SoundDelegate> *>(ptr)->get();
    }

    // VROSoundDelegateInternal
    virtual void soundIsReady();
    virtual void soundDidFinish();
    virtual void soundDidFail(std::string error);

private:
    VRO_OBJECT _javaObject;
};

#endif