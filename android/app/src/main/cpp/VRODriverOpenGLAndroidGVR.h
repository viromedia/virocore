//
// Created by Raj Advani on 8/23/17.
//

#ifndef ANDROID_VRODRIVEROPENGLANDROIDGVR_H
#define ANDROID_VRODRIVEROPENGLANDROIDGVR_H

#include "VRODriverOpenGLAndroid.h"
#include "VRODisplayOpenGLGVR.h"

class VRODriverOpenGLAndroidGVR : public VRODriverOpenGLAndroid {
public:

    VRODriverOpenGLAndroidGVR(std::shared_ptr<gvr::AudioApi> gvrAudio) :
        VRODriverOpenGLAndroid(gvrAudio) {
    }
    virtual ~VRODriverOpenGLAndroidGVR() { }

    /*
     GVR does not yet provide a way to allocate an sRGB framebuffer
     so we must do gamma conversion manually.
     */
    virtual VROColorRenderingMode getColorRenderingMode() {
        // TODO VIRO-2278: restore this to LinearSoftware after bottleneck is found
        return VROColorRenderingMode::NonLinear;
    }

    virtual bool isBloomEnabled() {
        // TODO VIRO-2278: restore this to true after bottleneck is found
        return false;
    }

    /*
     On GVR the primary framebuffer (display) is tied to a GVR
     swapchain.
     */
    std::shared_ptr<VRORenderTarget> getDisplay() {
        if (!_display) {
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            _display = std::make_shared<VRODisplayOpenGLGVR>(driver);
        }
        return _display;
    }

private:

};

#endif //ANDROID_VRODRIVEROPENGLANDROIDGVR_H
