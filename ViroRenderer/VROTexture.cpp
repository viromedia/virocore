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

VROTexture::VROTexture(VROTextureType type) :
    _textureId(sTextureId++),
    _type(type),
    _substrate(nullptr) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type, std::unique_ptr<VROTextureSubstrate> substrate) :
    _textureId(sTextureId++),
    _type(type),
    _substrate(std::move(substrate)) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureInternalFormat internalFormat,
                       VROMipmapMode mipmapMode,
                       std::shared_ptr<VROImage> image) :
    _textureId(sTextureId++),
    _type(VROTextureType::Texture2D),
    _images( {image} ),
    _format(image->getFormat()),
    _internalFormat(internalFormat),
    _width(image->getWidth()),
    _height(image->getHeight()),
    _mipmapMode(mipmapMode),
    _substrate(nullptr) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureInternalFormat internalFormat,
                       std::vector<std::shared_ptr<VROImage>> &images) :
    _textureId(sTextureId++),
    _type(VROTextureType::TextureCube),
    _images(images),
    _format(images.front()->getFormat()),
    _internalFormat(internalFormat),
    _width(images.front()->getWidth()),
    _height(images.front()->getHeight()),
    _mipmapMode(VROMipmapMode::None), // No mipmapping for cube textures
    _substrate(nullptr) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type,
                       VROTextureFormat format,
                       VROTextureInternalFormat internalFormat,
                       VROMipmapMode mipmapMode,
                       std::vector<std::shared_ptr<VROData>> &data,
                       int width, int height,
                       std::vector<uint32_t> mipSizes) :
    _textureId(sTextureId++),
    _type(type),
    _data(data),
    _format(format),
    _internalFormat(internalFormat),
    _width(width),
    _height(height),
    _mipmapMode(mipmapMode),
    _mipSizes(mipSizes),
    _substrate(nullptr) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::~VROTexture() {
    ALLOCATION_TRACKER_SUB(Textures, 1);
}

VROTextureSubstrate *const VROTexture::getSubstrate(std::shared_ptr<VRODriver> &driver, VROFrameScheduler *scheduler) {
    if (!_substrate) {
        if (!scheduler) {
            hydrate(driver);
        }
        else {
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
            
            std::string key = "th_" + VROStringUtil::toString(_textureId);
            scheduler->scheduleTask(key, hydrationTask);
        }
    }
    
    return _substrate.get();
}

void VROTexture::setSubstrate(std::unique_ptr<VROTextureSubstrate> substrate) {
    _substrate = std::move(substrate);
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
            
            size_t length;
            void *bytes = image->getData(&length);
            data.push_back(std::make_shared<VROData>(bytes, length, VRODataOwnership::Wrap));
        }
        
        std::vector<uint32_t> mipSizes;
        _substrate = std::unique_ptr<VROTextureSubstrate>(driver->newTextureSubstrate(_type, _format, _internalFormat, _mipmapMode,
                                                                                      data, _width, _height, _mipSizes));
        _images.clear();
    }
    else if (!_data.empty()) {
        _substrate = std::unique_ptr<VROTextureSubstrate>(driver->newTextureSubstrate(_type, _format, _internalFormat, _mipmapMode,
                                                                                      _data, _width, _height, _mipSizes));
        _data.clear();
    }
}

