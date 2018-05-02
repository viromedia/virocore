//
//  SoundDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIDEO_DELEGATE_JNI_H
#define ANDROID_VIDEO_DELEGATE_JNI_H

#include <memory>
#include "VROSoundDelegateInternal.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class SoundDelegate : public VROSoundDelegateInternal {

public:
    SoundDelegate(VRO_OBJECT soundObjectJava);
    ~SoundDelegate();

    // VROSoundDelegateInternal
    virtual void soundIsReady();
    virtual void soundDidFinish();
    virtual void soundDidFail(std::string error);

private:
    VRO_OBJECT _javaObject;
};

#endif