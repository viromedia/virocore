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

    virtual ~VROImageAndroid();

    int getWidth() const;
    int getHeight() const;
    unsigned char *getData(size_t *length);

private:

    int _width, _height;
    int _dataLength;
    unsigned char *_data;

};

#endif //ANDROID_VROIMAGEANDROID_H
