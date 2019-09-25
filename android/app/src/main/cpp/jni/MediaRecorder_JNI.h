//
//  MediaRecorder_JNI.h
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef MEDIA_RECORDER_JNI_H
#define MEDIA_RECORDER_JNI_H

#include <PersistentRef.h>
#include "VROAVRecorderAndroid.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

namespace MediaRecorder {
    inline jlong jptr(std::shared_ptr<MediaRecorder_JNI> delegate) {
        PersistentRef<MediaRecorder_JNI> *delegateRef
                = new PersistentRef<MediaRecorder_JNI>(delegate);
        return reinterpret_cast<intptr_t>(delegateRef);
    }

    inline std::shared_ptr<MediaRecorder_JNI> native(jlong ptr) {
        PersistentRef<MediaRecorder_JNI> *persistentDelegate
                = reinterpret_cast<PersistentRef<MediaRecorder_JNI> *>(ptr);
        return persistentDelegate->get();
    }
}

class VROSceneRenderer;
class VROChoreographer;

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
    void onUnbindFromEGLSurface();
    void onEnableFrameRecording(bool enabled);
    void onEglSwap();
    void onTakeScreenshot();

private:
    std::shared_ptr<VROAVRecorderAndroid> _nativeMediaRecorder;
    std::weak_ptr<VROChoreographer> _choreographer;
    std::weak_ptr<VRODriver> _driver;
    VRO_OBJECT _javaMediaRecorder;
};

#endif //MEDIA_RECORDER_JNI_H