//
// Created by Raj Advani on 8/23/17.
//

#ifndef ANDROID_VRODRIVEROPENGLANDROIDOVR_H
#define ANDROID_VRODRIVEROPENGLANDROIDOVR_H

#include "VRODriverOpenGLAndroid.h"
#include "VRODisplayOpenGLOVR.h"

class VRODriverOpenGLAndroidOVR : public VRODriverOpenGLAndroid {
public:

    VRODriverOpenGLAndroidOVR(std::shared_ptr<gvr::AudioApi> gvrAudio) :
            VRODriverOpenGLAndroid(gvrAudio) {
    }
    virtual ~VRODriverOpenGLAndroidOVR() { }

    /*
     OVR most likely works with an sRGB framebuffer but we have not yet
     enabled this, so use software conversion.

     TODO VIRO-1937 Enable OVR hardware sRGB conversion
     */
    virtual VROColorRenderingMode getColorRenderingMode() {
        return VROColorRenderingMode::LinearSoftware;
    }

    /*
     On OVR the primary framebuffer (display) is tied to an OVR
     swapchain.
     */
    std::shared_ptr<VRORenderTarget> getDisplay() {
        if (!_display) {
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            _display = std::make_shared<VRODisplayOpenGLOVR>(driver);
        }
        return _display;
    }

private:

};

#endif //ANDROID_VRODRIVEROPENGLANDROIDOVR_H
