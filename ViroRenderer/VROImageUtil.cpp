//
//  VROImageUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROImageUtil.h"
#include "VROLog.h"
#include "VROTexture.h"
#include "VROImage.h"

static std::shared_ptr<VROTexture> staticBlankTexture = nullptr;

std::shared_ptr<VROTexture> getBlankTexture() {
    return staticBlankTexture;
}

#if VRO_PLATFORM_IOS

#include "VROImageiOS.h"

void *VROImageLoadTextureDataRGBA8888(const char *resource, size_t *bitmapLength, int *width, int *height) {
    NSString *file = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:resource] ofType:@"png"];
    UIImage *image = [UIImage imageWithContentsOfFile:file];
    passert (image != nullptr);

    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageiOS>(image);
    *width = wrapper->getWidth();
    *height = wrapper->getHeight();
    
    return wrapper->extractRGBA8888(bitmapLength);
}

void initBlankTexture(const VRORenderContext &context) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:@"blank" ofType:@"png"];
    UIImage *image = [UIImage imageWithContentsOfFile:path];
    
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageiOS>(image);
    staticBlankTexture = std::make_shared<VROTexture>(wrapper);
}

#elif VRO_PLATFORM_ANDROID

void *VROImageLoadTextureDataRGBA8888(const char *resource, size_t *bitmapLength, int *width, int *height) {
    //TODO Android
    return nullptr;
}

void initBlankTexture(const VRORenderContext &context) {
    //TODO Android
}

#endif
