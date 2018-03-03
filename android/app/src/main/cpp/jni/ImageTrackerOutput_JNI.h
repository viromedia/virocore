//
// Created by Andy Chu on 6/1/17.
//

#ifndef ANDROID_IMAGETRACKEROUTPUT_JNI_H
#define ANDROID_IMAGETRACKEROUTPUT_JNI_H

#include <jni.h>
#include <memory>

#include "VROARImageTrackerOutput.h"
#include "PersistentRef.h"


namespace ImageTrackerOutput {
    inline jlong jptr(std::shared_ptr<VROARImageTrackerOutput> tracker) {
        PersistentRef<VROARImageTrackerOutput> *nativeTracker = new PersistentRef<VROARImageTrackerOutput>(tracker);
        return reinterpret_cast<intptr_t>(nativeTracker);
    }

    inline std::shared_ptr<VROARImageTrackerOutput> native(jlong ptr) {
        PersistentRef<VROARImageTrackerOutput> *persistentOutput = reinterpret_cast<PersistentRef<VROARImageTrackerOutput> *>(ptr);
        return persistentOutput->get();
    }
}

#endif //ANDROID_IMAGETRACKEROUTPUT_JNI_H
