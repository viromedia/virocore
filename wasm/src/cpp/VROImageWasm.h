//
//  VROImageWasm.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/8/18.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROImageWasm_h
#define VROImageWasm_h

#include "VROImage.h"
#include "SDL_image.h"
#include <string>

class VROImageWasm : public VROImage {
    
public:
    
    /*
     Construct a new VROImage from the given file that's been preloaded
     or downloaded.
     */
    VROImageWasm(std::string file, VROTextureInternalFormat format);
    virtual ~VROImageWasm();
    
    int getWidth() const;
    int getHeight() const;
    unsigned char *getData(size_t *length);
    
private:
    
    SDL_Surface *_surface;
    
};

#endif /* VROImageWasm_h */
