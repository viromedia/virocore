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
#include "VROImagePostProcess.h"
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
        _recorderDisplay->setViewport({0, 0, input->getWidth(), input->getHeight()});

        driver->bindRenderTarget(_recorderDisplay, VRORenderTargetUnbindOp::Invalidate);
        _recordingPostProcess->blit({ input->getTexture(0) }, driver);
    }

    if (_scheduledScreenShot) {
        std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
        if (jRecorder) {
            passert (driver->getRenderTarget() == input);

            // If we were using hardware gamma correction, then we have to emulate that when taking the
            // screenshot by using software gamma correction
            if (driver->getColorRenderingMode() == VROColorRenderingMode::Linear) {
                std::shared_ptr<VRORenderTarget> ldrTarget = bindScreenshotLDRTarget(input->getWidth(), input->getHeight(), driver);
                getGammaPostProcess(driver)->blit({ input->getTexture(0) }, driver);
                ldrTarget->bindRead();
            }

            // Otherwise the texture is already tone-mapped so we can perform a direct read or blit
            else {
                // If the input was an LDR color target, we can directly read from it
                if (input->getType() == VRORenderTargetType::ColorTexture) {
                    input->bindRead();
                }
                    // Otherwise create an LDR render target and render to it
                else {
                    std::shared_ptr<VRORenderTarget> ldrTarget = bindScreenshotLDRTarget(input->getWidth(), input->getHeight(), driver);
                    input->blitColor(ldrTarget, false, driver);
                    ldrTarget->bindRead();
                }
            }



            // This will call glReadPixels up in Java, reading from the currently bound (READ) framebuffer
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

void VROAVRecorderAndroid::unbindFromEGLSurface() {
    std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
    if (!jRecorder) {
        return;
    }

    jRecorder->onUnbindFromEGLSurface();
}

void VROAVRecorderAndroid::eglSwap() {
    std::shared_ptr<MediaRecorder_JNI> jRecorder = _w_mediaRecorderJNI.lock();
    if (!jRecorder) {
        return;
    }

    jRecorder->onEglSwap();
}

std::shared_ptr<VRORenderTarget> VROAVRecorderAndroid::bindScreenshotLDRTarget(int width, int height,
                                                                               std::shared_ptr<VRODriver> &driver) {
    if (!_screenshotLDRTarget) {
        pinfo("Creating screenshot LDR render target");
        _screenshotLDRTarget = driver->newRenderTarget( VRORenderTargetType::ColorTexture, 1, 1, false);
    }
    _screenshotLDRTarget->setViewport({0, 0, width, height});
    driver->bindRenderTarget(_screenshotLDRTarget, VRORenderTargetUnbindOp::Invalidate);
    return _screenshotLDRTarget;
}

std::shared_ptr<VROImagePostProcess> VROAVRecorderAndroid::getGammaPostProcess(std::shared_ptr<VRODriver> driver) {
    if (!_gammaPostProcess) {
        std::vector<std::string> samplers = { "hdr_texture", "tone_mapping_mask" };
        std::vector<std::string> code = {
                "const highp float gamma = 2.2;",
                "uniform sampler2D srgb_texture;",
                "highp vec4 srgb_color = texture(srgb_texture, v_texcoord);",
                "highp vec3 gamma_color = pow(srgb_color.xyz, vec3(1.0 / gamma));",
                "frag_color = vec4(gamma_color, srgb_color.a);",
        };

        std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
        std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
        std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
        _gammaPostProcess = driver->newImagePostProcess(shader);
    }

    return _gammaPostProcess;
}