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
static std::shared_ptr<VROTexture> staticBlankCubeTexture = nullptr;
static std::shared_ptr<VROTexture> staticPointCloudTexture = nullptr;

std::shared_ptr<VROTexture> getBlankTexture(VROTextureType type) {
    if (type == VROTextureType::None || type == VROTextureType::Texture2D || type == VROTextureType::TextureEGLImage) {
        return staticBlankTexture;
    }
    else {
        return staticBlankCubeTexture;
    }
}

std::shared_ptr<VROTexture> getPointCloudTexture() {
    if (!staticPointCloudTexture){
        initPointCloudTexture();
    }
    return staticPointCloudTexture;
}

#if VRO_PLATFORM_IOS

#include "VROImageiOS.h"

void initBlankTexture(const VRORenderContext &context) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:@"blank" ofType:@"png"];
    UIImage *image = [UIImage imageWithContentsOfFile:path];
    
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageiOS>(image, VROTextureInternalFormat::RGBA8);
    staticBlankTexture = std::make_shared<VROTexture>(true, VROMipmapMode::None, wrapper);
    
    std::vector<std::shared_ptr<VROImage>> cubeImages = { wrapper, wrapper, wrapper, wrapper, wrapper, wrapper };
    staticBlankCubeTexture = std::make_shared<VROTexture>(true, cubeImages);
}

void initPointCloudTexture() {
    if (staticPointCloudTexture) {
        return;
    }

    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:@"point_cloud" ofType:@"png"];
    UIImage *image = [UIImage imageWithContentsOfFile:path];
    
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageiOS>(image, VROTextureInternalFormat::RGBA8);
    staticPointCloudTexture = std::make_shared<VROTexture>(true, VROMipmapMode::None, wrapper);
}

#elif VRO_PLATFORM_ANDROID

#include "VROImageAndroid.h"

void initBlankTexture(const VRORenderContext &context) {
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageAndroid>("blank.png", VROTextureInternalFormat::RGBA8);
    staticBlankTexture = std::make_shared<VROTexture>(true, VROMipmapMode::None, wrapper);
    
    std::vector<std::shared_ptr<VROImage>> cubeImages = { wrapper, wrapper, wrapper, wrapper, wrapper, wrapper };
    staticBlankCubeTexture = std::make_shared<VROTexture>(true, cubeImages);
}

void initPointCloudTexture() {
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageAndroid>("point_cloud.png", VROTextureInternalFormat::RGBA8);
    staticPointCloudTexture = std::make_shared<VROTexture>(true, VROMipmapMode::None, wrapper);
}

#elif VRO_PLATFORM_WASM

#include "VROImageWasm.h"

void initBlankTexture(const VRORenderContext &context) {
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageWasm>("blank.png", VROTextureInternalFormat::RGBA8);
    staticBlankTexture = std::make_shared<VROTexture>(true, VROMipmapMode::None, wrapper);
    
    std::vector<std::shared_ptr<VROImage>> cubeImages = { wrapper, wrapper, wrapper, wrapper, wrapper, wrapper };
    staticBlankCubeTexture = std::make_shared<VROTexture>(true, cubeImages);
}

void initPointCloudTexture() {
    
}

#endif
