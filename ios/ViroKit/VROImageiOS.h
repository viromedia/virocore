//
//  VROImageiOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROImageiOS_h
#define VROImageiOS_h

#import "VROImage.h"
#import <UIKit/UIKit.h>

class VROImageiOS : public VROImage {
    
public:
    
    VROImageiOS(UIImage *image) :
        _image(image) {}
    virtual ~VROImageiOS();
    
    int getWidth() const {
        return _image.size.width * _image.scale;
    }
    int getHeight() const {
        return _image.size.height * _image.scale;
    }
    unsigned char *extractRGBA8888(size_t *length);
    
private:
    
    UIImage *_image;
    
};

#endif /* VROImageiOS_h */
