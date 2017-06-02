//
// Created by Andy Chu on 6/1/17.
//

#ifndef ANDROID_IMAGETRACKEROUTPUT_JNI_H
#define ANDROID_IMAGETRACKEROUTPUT_JNI_H

#include <jni.h>
#include <memory>

#include "VROImageTrackerOutput.h"
#include "PersistentRef.h"


namespace ImageTrackerOutput {
    inline jlong jptr(std::shared_ptr<VROImageTrackerOutput> tracker) {
        PersistentRef<VROImageTrackerOutput> *nativeTracker = new PersistentRef<VROImageTrackerOutput>(tracker);
        return reinterpret_cast<intptr_t>(nativeTracker);
    }

    inline std::shared_ptr<VROImageTrackerOutput> native(jlong ptr) {
        PersistentRef<VROImageTrackerOutput> *persistentOutput = reinterpret_cast<PersistentRef<VROImageTrackerOutput> *>(ptr);
        return persistentOutput->get();
    }
}

#endif //ANDROID_IMAGETRACKEROUTPUT_JNI_H
