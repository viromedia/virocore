//
//  SoundDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIDEO_DELEGATE_JNI_H
#define ANDROID_VIDEO_DELEGATE_JNI_H

#include <jni.h>
#include <memory>
#include <PersistentRef.h>
#include <VROSoundDelegateInternal.h>

class SoundDelegate : public VROSoundDelegateInternal {

public:
    SoundDelegate(jobject soundObjectJava);
    ~SoundDelegate();

    static jlong jptr(std::shared_ptr<SoundDelegate> delegate) {
        return reinterpret_cast<intptr_t>(new PersistentRef<SoundDelegate>(delegate));
    }

    static std::shared_ptr<SoundDelegate> native(jlong ptr) {
        return reinterpret_cast<PersistentRef<SoundDelegate> *>(ptr)->get();
    }

    // VROSoundDelegateInternal
    virtual void soundIsReady();
    virtual void soundDidFinish();

private:
    jobject _javaObject;
};

#endif