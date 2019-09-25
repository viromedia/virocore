//
//  VROAVRecorderAndroid.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
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

#ifndef VRO_AVRECORDER_ANDROID_H
#define VRO_AVRECORDER_ANDROID_H

#include "VROVideoTexture.h"
#include "VROOpenGL.h"

class VRODriverOpenGL;
class VROImagePostProcess;
class VRORecorderEglSurfaceDisplay;
class MediaRecorder_JNI;
class VRORenderTarget;
class VRORenderToTextureDelegateAndroid;

/*
 VROAVRecorderAndroid contains the native implementation of ViroMediaRecorder.java that
 coordinates callbacks required for handling screen capturing and recording.
 */
class VROAVRecorderAndroid : public std::enable_shared_from_this<VROAVRecorderAndroid> {
public:
    VROAVRecorderAndroid(std::shared_ptr<MediaRecorder_JNI> jRecorder);

    virtual ~VROAVRecorderAndroid();

    void init(std::shared_ptr<VRODriver> driver);

    /*
     Java callbacks for acknowledging scheduled video recording or screen capturing
     tasks made by ViroMediaRecorder.java
     */
    void setEnableVideoFrameRecording(bool isRecording);

    void scheduleScreenCapture();

    /*
     True if this recorder is currently recording video.
     */
    bool isRecordingVideo() const { return _isRecording; }

    /*
     Returns a VRORenderToTextureDelegate, implemented by this VROAVRecorderAndroid,
     to be set on VROChoreographer for driving video input when recording.
     */
    std::shared_ptr<VRORenderToTextureDelegateAndroid> getRenderToTextureDelegate();

    /*
     Called by by VROChoreographer when a frame is rendered onto the texture of the
     given VRORenderTarget output, where we then handle the swapping of framebuffer data needed
     for screen capturing and recording.
     */
    bool onRenderedFrameTexture(std::shared_ptr<VRORenderTarget> output,
                                std::shared_ptr<VRODriver> driver);

    /*
     Binds and unbinds the underlying egl _recorderDisplay for recording.
     */
    void bindToEglSurface();

    void unbindFromEGLSurface();

    /*
     Performs an implicit flush operation onto the bounded egl _recorderDisplay that effectively
     provides our recorders with a 'rendered frame'.
     */
    void eglSwap();

private:
    /*
     True if a video recording currently occurring and we are binding egl surfaces and swapping them.
     */
    bool _isRecording;

    /*
     True if a screen shot capture has been scheduled for the next frame by ViroMediaRecorder.java.
     */
    bool _scheduledScreenShot;

    /*
     The render target representing the eglSurface (MediaRecorderSurface.java) on which the
     VROChoreographer renders a "textured represented scene" that would be used for video recording.
     */
    std::shared_ptr<VRORecorderEglSurfaceDisplay> _recorderDisplay;

    /*
     ImagePostProcess that blits the final rendered scene as a texture onto the _recorderDisplay.
     */
    std::shared_ptr<VROImagePostProcess> _recordingPostProcess;

    /*
     A RenderToTextureDelegate that is implemented by this VROAVRecorderAndroid
     to drive data input when recording.
     */
    std::shared_ptr<VRORenderToTextureDelegateAndroid> _renderToTextureDelegate;

    /*
     Render target used for taking screenshots. This is used if the input textures are HDR;
     we have to render them to this LDR target so we can run glReadPixels.
     */
    std::shared_ptr<VRORenderTarget> _screenshotLDRTarget;

    /*
     Post process to gamma correct screen shots.
     */
    std::shared_ptr<VROImagePostProcess> _gammaPostProcess;

    /*
     Weak reference to the native-to-java jni interface for triggering java callbacks.
     */
    std::weak_ptr<MediaRecorder_JNI> _w_mediaRecorderJNI;

    /*
     Create or retrieve the screenshot LDR target.
     */
    std::shared_ptr<VRORenderTarget> bindScreenshotLDRTarget(int width, int height,
                                                             std::shared_ptr<VRODriver> &driver);

    /*
     Create or retrieve the gamma correction post-process.
     */
    std::shared_ptr<VROImagePostProcess> getGammaPostProcess(std::shared_ptr<VRODriver> driver);

};

#endif //VRO_AVRECORDER_ANDROID_H
