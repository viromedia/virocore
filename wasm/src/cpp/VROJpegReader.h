//
//  VROJpegReader.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/28/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROJpegReader_h
#define VROJpegReader_h

#include <string>
#include "SDL_image.h"

class VROJpegReader {

public:

    static bool isJPG(void *data, int length);
    static SDL_Surface *loadJPG(void *data, int length);

};


#endif //ANDROID_VROTEXTFACEWASM_H
