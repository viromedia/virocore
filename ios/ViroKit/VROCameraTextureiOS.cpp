//
//  VROCameraTextureiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/22/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

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
    
    _controller->setUpdateListener([shared_w, driver_w] (CMSampleBufferRef sampleBuffer) {
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

