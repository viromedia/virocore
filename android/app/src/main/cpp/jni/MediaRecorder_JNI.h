/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
#ifndef MEDIA_RECORDER_JNI_H
#define MEDIA_RECORDER_JNI_H

#include <PersistentRef.h>
#include "VROAVRecorderAndroid.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace MediaRecorder {
    inline VRO_REF jptr(std::shared_ptr<MediaRecorder_JNI> delegate) {
        PersistentRef<MediaRecorder_JNI> *delegateRef
                = new PersistentRef<MediaRecorder_JNI>(delegate);
        return reinterpret_cast<intptr_t>(delegateRef);
    }

    inline std::shared_ptr<MediaRecorder_JNI> native(VRO_REF ptr) {
        PersistentRef<MediaRecorder_JNI> *persistentDelegate
                = reinterpret_cast<PersistentRef<MediaRecorder_JNI> *>(ptr);
        return persistentDelegate->get();
    }
}
class VROSceneRenderer;

class MediaRecorder_JNI : public std::enable_shared_from_this<MediaRecorder_JNI> {
public:
    MediaRecorder_JNI(VRO_OBJECT nodeJavaObject, JNIEnv *env);
    ~MediaRecorder_JNI();

    // Java to native calls;
    void nativeCreateRecorder(std::shared_ptr<VROSceneRenderer> renderer);
    void nativeEnableFrameRecording(bool isRecording);
    void nativeScheduleScreenCapture();

    // Native to java calls
    void onBindToEGLSurface();
    void onUnBindFromEGLSurface();
    void onEnableFrameRecording(bool enabled);
    void onEglSwap();
    void onTakeScreenshot();

private:
    std::shared_ptr<VROAVRecorderAndroid> _nativeMediaRecorder;
    VRO_OBJECT _javaMediaRecorder;
};

#endif //MEDIA_RECORDER_JNI_H