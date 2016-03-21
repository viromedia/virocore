//
//  VROTexture.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROTexture.h"
#include "VROTextureSubstrate.h"
#include "VRORenderContext.h"
#include "VROTextureSubstrateMetal.h"
#include "VROLog.h"
#include "VROAllocationTracker.h"
#include "VROData.h"

VROTexture::VROTexture() :
    _image(nullptr),
    _substrate(nullptr) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type, std::unique_ptr<VROTextureSubstrate> substrate) :
    _type(type),
    _image(nullptr),
    _substrate(std::move(substrate)) {
    
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(UIImage *image, const VRORenderContext *context) :
    _type(VROTextureType::Quad),
    _image(image),
    _substrate(nullptr) {
    
    if (context) {
        prewarm(*context);
    }
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(std::vector<UIImage *> &images, const VRORenderContext *context) :
    _type(VROTextureType::Cube),
    _image(nullptr),
    _substrate(nullptr) {
    
    _imagesCube = images;
    if (context) {
        prewarm(*context);
    }
    ALLOCATION_TRACKER_ADD(Textures, 1);
}

VROTexture::VROTexture(VROTextureType type, VROTextureFormat format,
                       std::shared_ptr<VROData> data, int width, int height,
                       const VRORenderContext *context) :
    _type(type),
    _image(nullptr),
    _data(data),
    _format(format),
    _width(width),
    _height(height),
    _substrate(nullptr) {
    
    if (context) {
        prewarm(*context);
    }
    ALLOCATION_TRACKER_ADD(Textures, 1);
}


VROTexture::~VROTexture() {
    ALLOCATION_TRACKER_SUB(Textures, 1);
}

VROTextureSubstrate *const VROTexture::getSubstrate(const VRORenderContext &context) {
    if (!_substrate) {
        hydrate(context);
    }
    
    return _substrate.get();
}

void VROTexture::setSubstrate(VROTextureType type, std::unique_ptr<VROTextureSubstrate> substrate) {
    _type = type;
    _substrate = std::move(substrate);
}

void VROTexture::prewarm(const VRORenderContext &context) {
    hydrate(context);
}

void VROTexture::hydrate(const VRORenderContext &context) {
    if (_type == VROTextureType::Quad) {
        if (_image) {
            std::vector<UIImage *> images = { _image };
            _substrate = std::unique_ptr<VROTextureSubstrate>(context.newTextureSubstrate(_type, images));
            _image = nullptr;
        }
        else if (_data) {
            _substrate = std::unique_ptr<VROTextureSubstrate>(context.newTextureSubstrate(_type, _format, _data, _width, _height));
            _data = nullptr;
        }
    }
    
    // VROTextureType::Cube with 6 separated images
    else if (_type == VROTextureType::Cube && _imagesCube.size() == 6) {
        _substrate = std::unique_ptr<VROTextureSubstrate>(context.newTextureSubstrate(_type, _imagesCube));
        _imagesCube.clear();
    }
    
    // VROTextureType::Cube with a holistic cube image (not yet divided)
    else if (_type == VROTextureType::Cube && _image){
        
        _image = nullptr;
    }
    
    else {
        pinfo("Invalid texture format [type %d]", _type);
    }
}

void VROTexture::setImage(UIImage *image) {
    _type = VROTextureType::Quad;
    _image = image;
}

void VROTexture::setImageCube(UIImage *image) {
    _type = VROTextureType::Cube;
    _image = image;
}

void VROTexture::setImageCube(std::vector<UIImage *> &images) {
    _type = VROTextureType::Cube;
    _imagesCube = images;
}




