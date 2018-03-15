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
    
    // We always convert to RGBA8 because sRGB8 is not compatible with automatic mipmap
    // generation in OpenGL 3.0).
    int bytesPerPixel = _surface->format->BytesPerPixel;
    if (bytesPerPixel == 3) {
        SDL_Surface *rgbaSurface = convertToRGBA8(_surface);
        SDL_FreeSurface(_surface);
        _surface = rgbaSurface;
        
        if (_surface == NULL) {
            pinfo("Failed to convert surface to RGBA8 [%s]", SDL_GetError());
        }
    }
    
    if (_surface == NULL) {
        pinfo("Failed to load image at path [%s]", file.c_str());
    }
    
    _format = VROTextureFormat::RGBA8;
    _internalFormat = VROTextureInternalFormat::RGBA8;
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

void VROImageWasm::lock() {
    SDL_LockSurface(_surface);
}

void VROImageWasm::unlock() {
    SDL_UnlockSurface(_surface);
}

unsigned char *VROImageWasm::getData(size_t *length) {
    if (_format == VROTextureFormat::RGB8) {
        *length = getHeight() * getWidth() * 3;
    } else {
        *length = getHeight() * getWidth() * 4;
    }
    return (unsigned char *)_surface->pixels;
}

SDL_Surface *VROImageWasm::convertToRGBA8(SDL_Surface *image) {
    SDL_PixelFormat fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.BitsPerPixel = 32;
    fmt.BytesPerPixel = 4;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    fmt.Rmask = 0xff000000;
    fmt.Gmask = 0x00ff0000;
    fmt.Bmask = 0x0000ff00;
    fmt.Amask = 0x000000ff;
#else
    fmt.Rmask = 0x000000ff;
    fmt.Gmask = 0x0000ff00;
    fmt.Bmask = 0x00ff0000;
    fmt.Amask = 0xff000000;
#endif
    return SDL_ConvertSurface(image, &fmt, SDL_SWSURFACE);
}
