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
#include "VROMaterialVisual.h"
#include <atomic>

static std::atomic_int sTextureId;

VROTexture::VROTexture(VROTextureType type) :
    _textureId(sTextureId++),
    _type(type),
    _image(nullptr),
    _substrate(nullptr) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type, std::unique_ptr<VROTextureSubstrate> substrate) :
    _textureId(sTextureId++),
    _type(type),
    _image(nullptr),
    _substrate(std::move(substrate)) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(std::shared_ptr<VROImage> image, VRODriver *driver) :
    _textureId(sTextureId++),
    _type(VROTextureType::Texture2D),
    _image(image),
    _substrate(nullptr) {
    
    if (driver) {
        prewarm(*driver);
    }
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(std::vector<std::shared_ptr<VROImage>> &images, VRODriver *driver) :
    _textureId(sTextureId++),
    _type(VROTextureType::TextureCube),
    _image(nullptr),
    _substrate(nullptr) {
    
    _imagesCube = images;
    if (driver) {
        prewarm(*driver);
    }
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type, VROTextureFormat format,
                       std::shared_ptr<VROData> data, int width, int height,
                       VRODriver *driver) :
    _textureId(sTextureId++),
    _type(type),
    _image(nullptr),
    _data(data),
    _format(format),
    _width(width),
    _height(height),
    _substrate(nullptr) {
    
    if (driver) {
        prewarm(*driver);
    }
    ALLOCATION_TRACKER_ADD(Textures, 1);
}


VROTexture::~VROTexture() {
    ALLOCATION_TRACKER_SUB(Textures, 1);
}

VROTextureSubstrate *const VROTexture::getSubstrate(VRODriver &driver) {
    if (!_substrate) {
        hydrate(driver);
    }
    
    return _substrate.get();
}

void VROTexture::setSubstrate(VROTextureType type, std::unique_ptr<VROTextureSubstrate> substrate) {
    _type = type;
    _substrate = std::move(substrate);
}

void VROTexture::prewarm(VRODriver &driver) {
    hydrate(driver);
}

void VROTexture::hydrate(VRODriver &driver) {
    if (_type == VROTextureType::Texture2D) {
        if (_image) {
            std::vector<std::shared_ptr<VROImage>> images = { _image };
            _substrate = std::unique_ptr<VROTextureSubstrate>(driver.newTextureSubstrate(_type, images));
            _image = nullptr;
        }
        else if (_data) {
            _substrate = std::unique_ptr<VROTextureSubstrate>(driver.newTextureSubstrate(_type, _format, _data, _width, _height));
            _data = nullptr;
        }
    }
    
    // VROTextureType::Cube with 6 separated images
    else if (_type == VROTextureType::TextureCube && _imagesCube.size() == 6) {
        _substrate = std::unique_ptr<VROTextureSubstrate>(driver.newTextureSubstrate(_type, _imagesCube));
        _imagesCube.clear();
    }
    
    // VROTextureType::Cube with a holistic cube image (not yet divided)
    else if (_type == VROTextureType::TextureCube && _image){
        
        _image = nullptr;
    }
    
    else {
        pinfo("Invalid texture format [type %d]", _type);
    }
}

void VROTexture::setImage(std::shared_ptr<VROImage> image) {
    _type = VROTextureType::Texture2D;
    _image = image;
}

void VROTexture::setImageCube(std::shared_ptr<VROImage> image) {
    _type = VROTextureType::TextureCube;
    _image = image;
}

void VROTexture::setImageCube(std::vector<std::shared_ptr<VROImage>> &images) {
    _type = VROTextureType::TextureCube;
    _imagesCube = images;
}

