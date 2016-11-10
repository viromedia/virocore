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
#include <string>

class VROImageAndroid : public VROImage {

public:

    VROImageAndroid(std::string resource);
    virtual ~VROImageAndroid();

    int getWidth() const;
    int getHeight() const;
    unsigned char *extractRGBA8888(size_t *length);

private:

    int _width, _height;
    int _dataLength;
    unsigned char *_data;

};

#endif //ANDROID_VROIMAGEANDROID_H
