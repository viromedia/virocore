//
//  VROImageUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROImageUtil.h"
#include "VROLog.h"
#include "VROTexture.h"
#include "VROImage.h"

static thread_local std::shared_ptr<VROTexture> staticBlankTexture = nullptr;
static thread_local std::shared_ptr<VROTexture> staticBlankCubeTexture = nullptr;
static thread_local std::shared_ptr<VROTexture> staticPointCloudTexture = nullptr;

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

#elif VRO_PLATFORM_MACOS

#include "VROImageMacOS.h"

void initBlankTexture(const VRORenderContext &context) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:@"blank" ofType:@"png"];
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];
    
    std::shared_ptr<VROImage> wrapper = std::make_shared<VROImageMacOS>(image, VROTextureInternalFormat::RGBA8);
    staticBlankTexture = std::make_shared<VROTexture>(true, VROMipmapMode::None, wrapper);
    
    std::vector<std::shared_ptr<VROImage>> cubeImages = { wrapper, wrapper, wrapper, wrapper, wrapper, wrapper };
    staticBlankCubeTexture = std::make_shared<VROTexture>(true, cubeImages);
}

void initPointCloudTexture() {
    
}

#endif
