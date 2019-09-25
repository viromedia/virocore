//
//  VROCameraTextureiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/22/17.
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

#include "VROCameraTextureiOS.h"
#include "VRORenderContext.h"
#include "VROFrameSynchronizer.h"
#include "VROTextureSubstrate.h"
#include "VROLog.h"
#include "VROVideoTextureCache.h"
#include "VRODriver.h"
#include "VROConvert.h"
#include "VRODriverOpenGLiOS.h"
#include "VROAVCaptureController.h"

VROCameraTextureiOS::VROCameraTextureiOS(VROTextureType type) :
    VROCameraTexture(type) {
    
}

VROCameraTextureiOS::~VROCameraTextureiOS() {
    
}

bool VROCameraTextureiOS::initCamera(VROCameraPosition position, VROCameraOrientation orientation,
                                     std::shared_ptr<VRODriver> driver) {
    _videoTextureCache = driver->newVideoTextureCache();
    _controller = std::make_shared<VROAVCaptureController>();
    _controller->initCapture(position, orientation, false, driver);
    pause();

    std::weak_ptr<VROCameraTextureiOS> shared_w = std::dynamic_pointer_cast<VROCameraTextureiOS>(shared_from_this());
    std::weak_ptr<VRODriver> driver_w = driver;
    
    _controller->setUpdateListener([shared_w, driver_w] (CMSampleBufferRef sampleBuffer, std::vector<float> intrinsics) {
        std::shared_ptr<VROCameraTextureiOS> texture = shared_w.lock();
        std::shared_ptr<VROVideoTextureCache> cache = texture->_videoTextureCache;
        std::shared_ptr<VRODriver> driver = driver_w.lock();
        if (cache && driver) {
            texture->setSubstrate(0, cache->createTextureSubstrate(sampleBuffer,
                                                                   driver->getColorRenderingMode() != VROColorRenderingMode::NonLinear));
        }
    });
    
    return true;
}

float VROCameraTextureiOS::getHorizontalFOV() const {
    return _controller->getHorizontalFOV();
}

VROVector3f VROCameraTextureiOS::getImageSize() const {
    return _controller->getImageSize();
}

void VROCameraTextureiOS::pause() {
    _controller->pause();
}

void VROCameraTextureiOS::play() {
    _controller->play();
}

bool VROCameraTextureiOS::isPaused() {
    return _controller->isPaused();
}

CMSampleBufferRef VROCameraTextureiOS::getSampleBuffer() const {
    return _controller->getSampleBuffer();
}

std::vector<float> VROCameraTextureiOS::getCameraIntrinsics() const {
    return _controller->getCameraIntrinsics();
}

