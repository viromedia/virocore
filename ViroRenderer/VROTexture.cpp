//
//  VROTexture.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROTexture.h"
#include "VROTextureSubstrate.h"
#include "VRODriver.h"
#include "VROTextureSubstrateMetal.h"
#include "VROLog.h"
#include "VROAllocationTracker.h"
#include "VROData.h"
#include "VROImage.h"
#include "VROMaterialVisual.h"
#include "VROFrameScheduler.h"
#include "VROStringUtil.h"
#include <atomic>

static std::atomic_int sTextureId;

VROTexture::VROTexture(VROTextureType type, VROTextureInternalFormat internalFormat, VROStereoMode stereoMode) :
    _textureId(sTextureId++),
    _type(type),
    _format(VROTextureFormat::RGBA8),
    _internalFormat(internalFormat),
    _stereoMode(stereoMode),

    // Note these parameters are irrelevent for this constructor, since they are used
    // to generate a substrate, and for this constructor the substrate is injected externally
    _sRGB(false),
    _wrapS(VROWrapMode::Clamp),
    _wrapT(VROWrapMode::Clamp),
    _minificationFilter(VROFilterMode::Linear),
    _magnificationFilter(VROFilterMode::Linear),
    _mipFilter(VROFilterMode::Linear) {
    
    setNumSubstrates(getNumSubstratesForFormat(internalFormat));
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type, VROTextureInternalFormat internalFormat,
                       std::unique_ptr<VROTextureSubstrate> substrate, VROStereoMode stereoState) :
    _textureId(sTextureId++),
    _type(type),
    _format(VROTextureFormat::RGBA8),
    _internalFormat(internalFormat),
    _stereoMode(stereoState),

    // Note these parameters are irrelevant for this constructor, since they are used
    // to generate a substrate, and for this constructor the substrate is already supplied
    _sRGB(false),
    _wrapS(VROWrapMode::Clamp),
    _wrapT(VROWrapMode::Clamp),
    _minificationFilter(VROFilterMode::Linear),
    _magnificationFilter(VROFilterMode::Linear),
    _mipFilter(VROFilterMode::Linear) {
    
    _substrates.push_back(std::move(substrate));
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(bool sRGB,
                       VROMipmapMode mipmapMode,
                       std::shared_ptr<VROImage> image,
                       VROStereoMode stereoMode) :
    _textureId(sTextureId++),
    _type(VROTextureType::Texture2D),
    _images( {image} ),
    _format(image->getFormat()),
    _internalFormat(image->getInternalFormat()),
    _width(image->getWidth()),
    _height(image->getHeight()),
    _mipmapMode(mipmapMode),
    _sRGB(sRGB),
    _stereoMode(stereoMode),
    _wrapS(VROWrapMode::Clamp),
    _wrapT(VROWrapMode::Clamp),
    _minificationFilter(VROFilterMode::Linear),
    _magnificationFilter(VROFilterMode::Linear),
    _mipFilter(VROFilterMode::Linear) {
    
    setNumSubstrates(getNumSubstratesForFormat(_internalFormat));
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(bool sRGB,
                       std::vector<std::shared_ptr<VROImage>> &images,
                       VROStereoMode stereoMode) :
    _textureId(sTextureId++),
    _type(VROTextureType::TextureCube),
    _images(images),
    _format(images.front()->getFormat()),
    _internalFormat(images.front()->getInternalFormat()),
    _width(images.front()->getWidth()),
    _height(images.front()->getHeight()),
    _mipmapMode(VROMipmapMode::None), // No mipmapping for cube textures
    _sRGB(sRGB),
    _stereoMode(stereoMode),
    _wrapS(VROWrapMode::Clamp),
    _wrapT(VROWrapMode::Clamp),
    _minificationFilter(VROFilterMode::Linear),
    _magnificationFilter(VROFilterMode::Linear),
    _mipFilter(VROFilterMode::Linear) {
    
    setNumSubstrates(getNumSubstratesForFormat(_internalFormat));
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type,
                       VROTextureFormat format,
                       VROTextureInternalFormat internalFormat, bool sRGB,
                       VROMipmapMode mipmapMode,
                       std::vector<std::shared_ptr<VROData>> &data,
                       int width, int height,
                       std::vector<uint32_t> mipSizes,
                       VROStereoMode stereoMode) :
    _textureId(sTextureId++),
    _type(type),
    _data(data),
    _format(format),
    _internalFormat(internalFormat),
    _width(width),
    _height(height),
    _mipmapMode(mipmapMode),
    _mipSizes(mipSizes),
    _sRGB(sRGB),
    _stereoMode(stereoMode),
    _wrapS(VROWrapMode::Clamp),
    _wrapT(VROWrapMode::Clamp),
    _minificationFilter(VROFilterMode::Linear),
    _magnificationFilter(VROFilterMode::Linear),
    _mipFilter(VROFilterMode::Linear) {
    
    setNumSubstrates(getNumSubstratesForFormat(internalFormat));
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::~VROTexture() {
    ALLOCATION_TRACKER_SUB(Textures, 1);
}

int VROTexture::getNumSubstrates() const {
    return (int)_substrates.size();
}

void VROTexture::hydrateAsync(std::function<void()> callback,
                              std::shared_ptr<VRODriver> &driver) {
    if (isHydrated()) {
        return;
    }
    
    _hydrationCallbacks.push_back(callback);
    
    const std::shared_ptr<VROFrameScheduler> &scheduler = driver->getFrameScheduler();
    std::string key = getHydrationTaskKey();
    if (!scheduler->isTaskQueued(key)) {
        std::function<void()> hydrationTask = createHydrationTask(driver);
        scheduler->scheduleTask(key, hydrationTask);
    }
}

std::string VROTexture::getHydrationTaskKey() const {
    return "th_" + VROStringUtil::toString(_textureId);
}

std::function<void()> VROTexture::createHydrationTask(std::shared_ptr<VRODriver> &driver) {
    // Only hold weak pointers: we don't want queued hydration
    // to prolong the lifetime of these objects
    std::weak_ptr<VRODriver> driver_w = driver;
    std::weak_ptr<VROTexture> texture_w = shared_from_this();
    
    std::function<void()> hydrationTask = [driver_w, texture_w]() {
        std::shared_ptr<VROTexture> texture_s = texture_w.lock();
        std::shared_ptr<VRODriver> driver_s = driver_w.lock();
        
        if (texture_s && driver_s) {
            texture_s->hydrate(driver_s);
        }
    };
    return hydrationTask;
}

bool VROTexture::isHydrated() const {
    return _substrates[0] != nullptr;
}

VROTextureSubstrate *VROTexture::getSubstrate(int index, std::shared_ptr<VRODriver> &driver, bool immediate) {
    passert (index <= _substrates.size());
    if (!_substrates[index]) {
        // Hydration only works for single-substrate textures. Multi-substrate
        // textures need to inject the substrates manually via setSubstrate().
        passert (index == 0);
        if (immediate) {
            hydrate(driver);
        }
        else {
            std::string key = getHydrationTaskKey();
            if (!driver->getFrameScheduler()->isTaskQueued(key)) {
                std::function<void()> hydrationTask = createHydrationTask(driver);
                driver->getFrameScheduler()->scheduleTask(key, hydrationTask);
            }
        }
    }
    
    return _substrates[index].get();
}

void VROTexture::setNumSubstrates(int numSubstrates) {
    _substrates.resize(numSubstrates);
}

void VROTexture::setSubstrate(int index, std::unique_ptr<VROTextureSubstrate> substrate) {
    passert_msg(index < _substrates.size(), "Cannot set substrate %d, numSubstrates only %d",
                index, (int) _substrates.size());
    _substrates[index] = std::move(substrate);
}

void VROTexture::prewarm(std::shared_ptr<VRODriver> driver) {
    hydrate(driver);
}

void VROTexture::hydrate(std::shared_ptr<VRODriver> &driver) {
    passert (_images.empty() || _data.empty());
    
    if (!_images.empty()) {
        std::vector<std::shared_ptr<VROData>> data;
        for (std::shared_ptr<VROImage> &image : _images) {
            passert (image->getFormat() == _format);
            passert (image->getInternalFormat() == _internalFormat);
            
            image->lock();
            {
                size_t length;
                void *bytes = image->getData(&length);
                data.push_back(std::make_shared<VROData>(bytes, length, VRODataOwnership::Wrap));
            }
            image->unlock();
        }
        
        std::vector<uint32_t> mipSizes;
        _substrates[0] = std::unique_ptr<VROTextureSubstrate>(driver->newTextureSubstrate(_type, _format, _internalFormat, _sRGB, _mipmapMode,
                                                                                          data, _width, _height, _mipSizes, _wrapS, _wrapT,
                                                                                          _minificationFilter, _magnificationFilter, _mipFilter));
        _images.clear();
    }
    else if (!_data.empty()) {
        _substrates[0] = std::unique_ptr<VROTextureSubstrate>(driver->newTextureSubstrate(_type, _format, _internalFormat, _sRGB, _mipmapMode,
                                                                                          _data, _width, _height, _mipSizes, _wrapS, _wrapT,
                                                                                          _minificationFilter, _magnificationFilter, _mipFilter));
        _data.clear();
    }
    
    for (auto &callback : _hydrationCallbacks) {
        callback();
    }
    _hydrationCallbacks.clear();
}

int VROTexture::getNumSubstratesForFormat(VROTextureInternalFormat format) const {
    if (format == VROTextureInternalFormat::YCBCR) {
        return 2;
    } else {
        return 1;
    }
}

bool VROTexture::hasAlpha() const {
    return _format != VROTextureFormat::RGB565 && _format != VROTextureFormat::RGB8;
}


