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
    static CVPixelBufferRef cropAndResize(CVPixelBufferRef image, float cropX, float cropY,
                                          float cropWidth, float cropHeight, float size,
                                          uint8_t *scratchBuffer);
    
    /*
     Perform the crop and resize using CoreImage insead of vImage.
     
     TODO: not working.
     */
    static CVPixelBufferRef cropAndResizeCI(CVPixelBufferRef image, float cropX, float cropY,
                                          float cropWidth, float cropHeight, float size);
    
    /*
     Write the given image to the photos album. This is for debugging purposes.
     */
    static void writeImageToPhotos(CVPixelBufferRef image);
    
};

#endif /* VROImagePreprocessor_h */
