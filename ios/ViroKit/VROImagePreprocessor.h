//
//  VROImagePreprocessor.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#ifndef VROImagePreprocessor_h
#define VROImagePreprocessor_h

#include <stdio.h>
#import <AVFoundation/AVFoundation.h>

/*
 Preprocesses images from the AR system prior feeding them into neural networks.
 */
class VROImagePreprocessor {
    
public:
    
    /*
     Convert a CVImageBufferRef in YCbCr color space into a CVPixelBufferRef in RGBA
     color space.
     */
    static CVPixelBufferRef convertYCbCrToRGB(CVImageBufferRef image);
    
    /*
     Rotate the given image. For the rotation parameter, [0, 1, 2, 3] map to [0, 90, 180, 270]
     degree rotation. The resultWidth and resultHeight should be the width and height of the
     image after the rotation is complete (note these need to be passed in, these are not
     output variables).
     */
    static CVPixelBufferRef rotateImage(CVPixelBufferRef image, uint8_t rotation, size_t resultWidth, size_t resultHeight);
    
    /*
     Write the given image to the photos album. This is for debugging purposes.
     */
    static void writeImageToPhotos(CVPixelBufferRef image);
    
};

#endif /* VROImagePreprocessor_h */
