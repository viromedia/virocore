//
//  VROImageAndroid.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROIMAGEANDROID_H
#define ANDROID_VROIMAGEANDROID_H

#include "VROImage.h"
#include <jni.h>
#include <string>

class VROImageAndroid : public VROImage {

public:

    // This constructor can only really be used by the Renderer projects
    VROImageAndroid(std::string asset, VROTextureInternalFormat internalFormat);

    // This is the constructor that should be called from JNI
    VROImageAndroid(jobject jbitmap, VROTextureInternalFormat internalFormat);

    // This constructor attempts to get the format based on the bitmap properties/info
    VROImageAndroid(jobject jbitmap);


    virtual ~VROImageAndroid();

    int getWidth() const;
    int getHeight() const;
    unsigned char *getData(size_t *length);
    unsigned char *getGrayscaleData(size_t *length, size_t *stride);

private:

    int _width, _height;
    int _dataLength;
    unsigned char *_data;
    unsigned char *_grayscaleData;

    void convertRgbaToGrayscale(int32_t stride, uint8_t** out_grayscale_buffer);

};

#endif //ANDROID_VROIMAGEANDROID_H
