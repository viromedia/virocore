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

VROTexture::VROTexture(UIImage *image) :
    _image(image),
    _substrate(nullptr) {
        
}

VROTexture::~VROTexture() {
    delete (_substrate);
}

void VROTexture::hydrate(const VRORenderContext &context) {
    if (_image) {
        _substrate = context.newTextureSubstrate(_image);
        _image = NULL;
    }
}




