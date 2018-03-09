//
//  VROImageWasm.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/8/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROImageWasm.h"
#include "VROLog.h"
#include "VROTime.h"

VROImageWasm::VROImageWasm(std::string file, VROTextureInternalFormat internalFormat) {
    _surface = IMG_Load(file.c_str());
    _format = VROTextureFormat::RGBA8;
    if (_surface == NULL) {
        pinfo("Failed to load image at path [%s]", file.c_str);
    }
}

VROImageWasm::~VROImageWasm() {
    SDL_FreeSurface(_surface);
}

int VROImageWasm::getHeight() const {
    return _surface->h;
}

int VROImageWasm::getWidth() const {
    return _surface->w;
}

unsigned char *VROImageWasm::getData(size_t *length) {
    // TODO support RGB images (bytes per pixel 3)
    *length = getHeight() * getWidth() * 4;
    return (unsigned char *)_surface->pixels;
}
