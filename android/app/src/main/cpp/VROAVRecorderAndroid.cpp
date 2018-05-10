//
//  VROAVRecorderAndroid.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VRORenderTarget.h>
#include <VROPlatformUtil.h>
#include "VROAVRecorderAndroid.h"
#include "VRODriverOpenGL.h"
#include "VROImageShaderProgram.h"
#include "VRORecorderEglSurfaceDisplay.h"
#include "VRORenderToTextureDelegateAndroid.h"
#include "jni/MediaRecorder_JNI.h"

VROAVRecorderAndroid::VROAVRecorderAndroid(std::shared_ptr<MediaRecorder_JNI> jRecorder) {
    _recorderDisplay = nullptr;
    _w_mediaRecorderJNI = jRecorder;
    _isRecording = false;
    _scheduledScreenShot = false;
}

VROAVRecorderAndroid::~VROAVRecorderAndroid() {
}

void VROAVRecorderAndroid::init(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> blitSamplers = { "source_texture" };
    std::vector<std::string> blitCode = {
            "uniform sampler2D source_texture;",
            "frag_color = texture(source_texture, v_texcoord);"
    };

    std::shared_ptr<VROShaderProgram> blitShader
            = VROImageShaderProgram::create(blitSamplers, blitCode, driver);
    _recordingPostProcess = driver->newImagePostProcess(blitShader);
}

std::shared_ptr<VRORenderToTextureDelegateAndroid> VROAVRecorderAndroid::getRenderToTextureDelegate() {
    if (_renderToTextureDelegate == nullptr) {
        _renderToTextureDelegate = std::make_shared<VRORenderToTextureDelegateAndroid>(shared_from_this());
    }
    return _renderToTextureDelegate;
}

void VROAVRecorderAndroid::setEnableVideoFrameRecording(bool isRecording) {
    std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
    if (!jRecorder) {
        return;
    }

    _isRecording = isRecording;
    jRecorder->onEnableFrameRecording(isRecording);
}

void VROAVRecorderAndroid::scheduleScreenCapture() {
    _scheduledScreenShot = true;
}

bool VROAVRecorderAndroid::onRenderedFrameTexture(std::shared_ptr<VRORenderTarget> input,
                                                  std::shared_ptr<VRODriver> driver) {
    if (_isRecording) {
        if (_recorderDisplay == nullptr) {
            std::shared_ptr<VRODriverOpenGL> openGLDriver = std::static_pointer_cast<VRODriverOpenGL>(driver);
            _recorderDisplay = std::make_shared<VRORecorderEglSurfaceDisplay>(openGLDriver, shared_from_this());
        }

        driver->bindRenderTarget(_recorderDisplay, VRORenderTargetUnbindOp::Invalidate);
        _recordingPostProcess->blit({ input->getTexture(0) }, driver);
    }

    if (_scheduledScreenShot) {
        std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
        if (jRecorder) {
            jRecorder->onTakeScreenshot();
        }
        _scheduledScreenShot = false;
    }
    return true;
}

void VROAVRecorderAndroid::bindToEglSurface() {
    std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
    if (!jRecorder) {
        return;
    }

    jRecorder->onBindToEGLSurface();
}

void VROAVRecorderAndroid::unBindFromEGLSurface() {
    std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
    if (!jRecorder) {
        return;
    }

    jRecorder->onUnBindFromEGLSurface();
}

void VROAVRecorderAndroid::eglSwap() {
    std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
    if (!jRecorder) {
        return;
    }

    jRecorder->onEglSwap();
}