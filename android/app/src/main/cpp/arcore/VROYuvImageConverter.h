//
//  VROYuvImageConverter.h
//  Viro
//
//  Created by Raj Advani on 4/9/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROYUVIMAGECONVERTER_H
#define ANDROID_VROYUVIMAGECONVERTER_H

#include <stdint.h>
#include "ARCore_API.h"

class VROYuvImageConverter {
public:

    // Converts the given arcore::Image from YCbCr to RGBA.
    // Conversion rotation amounts are counterclockwise
    static void convertImage(arcore::Image *image, uint8_t *data);
    static void convertImage90(arcore::Image *image, uint8_t *data);
    static void convertImage180(arcore::Image *image, uint8_t *data);
    static void convertImage270(arcore::Image *image, uint8_t *data);

};


#endif //ANDROID_VROYUVIMAGECONVERTER_H
