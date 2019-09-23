//
//  VROImagePreprocessor.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
     degree rotation.
     */
    static CVPixelBufferRef rotateImage(CVPixelBufferRef image, uint8_t rotation,
                                        size_t *outResultWidth, size_t *outResultHeight);
    
    /*
     Crops an image to the given rectangle, then resizes it to the given square size. The
     resize operation will fit the long side, the maintain aspect ratio while scaling down
     the short side. This will result in empty bars on either side of the image, which will
     be filled grey.
     
     The scratch buffer is used for temp operations and must be large enough to fit the
     original image: image_width * image_height * bytes_per_pixel. If NULL, a scratch buffer
     will be allocated and deallocated (slow).
     */
    static CVPixelBufferRef cropAndResize(CVPixelBufferRef image, int cropX, int cropY,
                                          int cropWidth, int cropHeight, int size,
                                          uint8_t *scratchBuffer);
    
    /*
     Perform the crop and resize using CoreImage insead of vImage.
     
     TODO: not working.
     */
    static CVPixelBufferRef cropAndResizeCI(CVPixelBufferRef image, int cropX, int cropY,
                                            int cropWidth, int cropHeight, int size);
    
    /*
     Write the given image to the photos album. This is for debugging purposes.
     */
    static void writeImageToPhotos(CVPixelBufferRef image);
    
};

#endif /* VROImagePreprocessor_h */
