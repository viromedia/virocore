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

void initBlankTexture(const VRORenderContext &context) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:@"blank" ofType:@"png"];
    UIImage *image = [UIImage imageWithContentsOfFile:path];
    
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageiOS>(image, VROTextureInternalFormat::RGBA8);
    staticBlankTexture = std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8, false, VROMipmapMode::None, wrapper);
}

#elif VRO_PLATFORM_ANDROID

#include "VROImageAndroid.h"

void initBlankTexture(const VRORenderContext &context) {
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageAndroid>("blank.png", VROTextureInternalFormat::RGBA8);
    staticBlankTexture = std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8, false, VROMipmapMode::None, wrapper);
}

#endif
