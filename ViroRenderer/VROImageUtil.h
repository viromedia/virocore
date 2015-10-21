//
//  VROImageUtil.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROImageUtil_h
#define VROImageUtil_h

#include <stdio.h>
#import <UIKit/UIKit.h>

void *VROImageLoadTextureDataRGBA8888(const char *resource, size_t *bitmapLength, int *width, int *height);

#endif /* VROImageUtil_h */
