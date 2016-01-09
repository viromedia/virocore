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

VROTexture::VROTexture() :
    _image(nullptr) {
    
}

VROTexture::VROTexture(UIImage *image) :
    _image(image),
    _substrate(nullptr) {
        
}

VROTexture::~VROTexture() {

}

VROTextureSubstrate *const VROTexture::getSubstrate(const VRORenderContext &context) {
    if (!_substrate) {
        hydrate(context);
    }
    
    return _substrate.get();
}

void VROTexture::setSubstrate(std::unique_ptr<VROTextureSubstrate> substrate) {
    _substrate = std::move(substrate);
}

void VROTexture::hydrate(const VRORenderContext &context) {
    if (_image) {
        _substrate = std::unique_ptr<VROTextureSubstrate>(context.newTextureSubstrate(_image));
        _image = NULL;
    }
}




