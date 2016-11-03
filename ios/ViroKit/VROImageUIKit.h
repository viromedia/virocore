//
//  VROImageUIKit.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROImageUIKit_h
#define VROImageUIKit_h

#import "VROImage.h"
#import <UIKit/UIKit.h>

class VROImageUIKit : public VROImage {
    
public:
    
    VROImageUIKit(UIImage *image) :
        _image(image) {}
    virtual ~VROImageUIKit();
    
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

#endif /* VROImageUIKit_h */
