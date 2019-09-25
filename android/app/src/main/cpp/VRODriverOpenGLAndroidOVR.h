//
//  Created by Raj Advani on 8/23/17.
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
     OVR devices will always use sRGB texture buffers (this is setup in
     VROSceneRendererOVR).
     */
    virtual VROColorRenderingMode getColorRenderingMode() {
        return VROColorRenderingMode::Linear;
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
