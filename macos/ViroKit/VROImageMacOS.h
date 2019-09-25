//
//  VROImageMacOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/3/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROImageMacOS_h
#define VROImageMacOS_h

#import "VROImage.h"
#import <AppKit/AppKit.h>

class VROImageMacOS : public VROImage {
    
public:
    
    /*
     Construct a new VROImage from the given UIImage. The data will be extracted
     from the UIImage into a format compatible with the given target internal format.
     */
    VROImageMacOS(NSImage *image, VROTextureInternalFormat format);
    virtual ~VROImageMacOS();
    
    int getWidth() const {
        return _width;
    }
    int getHeight() const {
        return _height;
    }
    unsigned char *getData(size_t *length);
    
private:
    
    bool hasAlpha(NSImage *image);
    
    int _width, _height;
    int _dataLength;
    unsigned char *_data;
    
};

#endif /* VROImageMacOS_h */
