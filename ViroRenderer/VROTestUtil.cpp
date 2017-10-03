//
//  VROTestUtil.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROTestUtil.h"
#include "VRODefines.h"
#include "VROTexture.h"
#include "VRONode.h"
#include "VROVector3f.h"
#include "VROGeometry.h"
#include "VROFBXLoader.h"
#include "VROExecutableAnimation.h"

#if VRO_PLATFORM_IOS
#include <UIKit/UIKit.h>
#include "VROImageiOS.h"
#include "VROVideoTextureiOS.h"
#endif

#if VRO_PLATFORM_ANDROID
#include "VROPlatformUtil.h"
#include "VROImageAndroid.h"
#include "VROVideoTextureAVP.h"
#endif

std::string VROTestUtil::getURLForResource(std::string resource, std::string type) {
#if VRO_PLATFORM_IOS
    NSString *objPath = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:resource.c_str()]
                                                        ofType:[NSString stringWithUTF8String:type.c_str()]];
    NSURL *objURL = [NSURL fileURLWithPath:objPath];
    return std::string([[objURL description] UTF8String]);
#else
    return "file:///android_asset/" + resource  + "." + type;
#endif
}

void *VROTestUtil::loadDataForResource(std::string resource, std::string type, int *outLength) {
#if VRO_PLATFORM_IOS
    NSString *path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:resource.c_str()]
                                                        ofType:[NSString stringWithUTF8String:type.c_str()]];
    NSURL *url = [NSURL fileURLWithPath:path];
    NSData *data = [NSData dataWithContentsOfURL:url];
    
    void *outBytes = malloc([data length]);
    memcpy(outBytes, [data bytes], [data length]);
    
    *outLength = (int)[data length];
    return outBytes;
#else
    std::string path = VROPlatformCopyAssetToFile(resource + "." + type);
    return VROPlatformLoadFile(path, outLength);
#endif
}

std::shared_ptr<VROTexture> VROTestUtil::loadCloudBackground() {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
#if VRO_PLATFORM_IOS
    std::vector<std::shared_ptr<VROImage>> cubeImages =  {
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"px1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nx1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"py1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"ny1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"pz1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nz1.jpg"], format)
    };
    
    return std::make_shared<VROTexture>(format, true, cubeImages);
#else
    std::vector<std::shared_ptr<VROImage>> cubeImages = {
            std::make_shared<VROImageAndroid>("px1.jpg", format),
            std::make_shared<VROImageAndroid>("nx1.jpg", format),
            std::make_shared<VROImageAndroid>("py1.jpg", format),
            std::make_shared<VROImageAndroid>("ny1.jpg", format),
            std::make_shared<VROImageAndroid>("pz1.jpg", format),
            std::make_shared<VROImageAndroid>("nz1.jpg", format)
    };

    return std::make_shared<VROTexture>(format, true, cubeImages);
#endif
}

std::shared_ptr<VROTexture> VROTestUtil::loadNiagaraBackground() {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
#if VRO_PLATFORM_IOS
    std::vector<std::shared_ptr<VROImage>> cubeImages =  {
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"px"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nx"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"py"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"ny"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"pz"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nz"], format)
    };
    
    return std::make_shared<VROTexture>(format, true, cubeImages);
#else
    std::vector<std::shared_ptr<VROImage>> cubeImages = {
        std::make_shared<VROImageAndroid>("px.png", format),
        std::make_shared<VROImageAndroid>("nx.png", format),
        std::make_shared<VROImageAndroid>("py.png", format),
        std::make_shared<VROImageAndroid>("ny.png", format),
        std::make_shared<VROImageAndroid>("pz.png", format),
        std::make_shared<VROImageAndroid>("nz.png", format)
    };

    return std::make_shared<VROTexture>(format, true, cubeImages);
#endif
}

std::shared_ptr<VROTexture> VROTestUtil::loadWestlakeBackground() {
#if VRO_PLATFORM_IOS
    return std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8, true, VROMipmapMode::None,
                                        std::make_shared<VROImageiOS>([UIImage imageNamed:@"360_westlake.jpg"],
                                                                      VROTextureInternalFormat::RGBA8));
#else
    return std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8, true, VROMipmapMode::None,
                                        std::make_shared<VROImageAndroid>("360_westlake.jpg",
                                                                          VROTextureInternalFormat::RGBA8));
#endif
}

std::shared_ptr<VROTexture> VROTestUtil::loadDiffuseTexture(std::string texture, VROMipmapMode mipmap, VROStereoMode stereo) {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;

#if VRO_PLATFORM_IOS
    return std::make_shared<VROTexture>(format, true, mipmap,
                                        std::make_shared<VROImageiOS>([UIImage imageNamed:[NSString stringWithUTF8String:texture.c_str()]], format), stereo);
#else
    if (texture.find(".") == std::string::npos) {
        texture = texture + ".png";
    }
    return std::make_shared<VROTexture>(format, true, mipmap,
                                        std::make_shared<VROImageAndroid>(texture.c_str(), format), stereo);
#endif
}

std::shared_ptr<VROTexture> VROTestUtil::loadSpecularTexture(std::string texture) {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
#if VRO_PLATFORM_IOS
    return std::make_shared<VROTexture>(format, true, VROMipmapMode::Runtime,
                                        std::make_shared<VROImageiOS>([UIImage imageNamed:[NSString stringWithUTF8String:texture.c_str()]], format));
#else
    if (texture.find(".") == std::string::npos) {
        texture = texture + ".png";
    }
    return std::make_shared<VROTexture>(format, true, VROMipmapMode::Runtime,
                                        std::make_shared<VROImageAndroid>(texture.c_str(), format));

#endif
}

std::shared_ptr<VROTexture> VROTestUtil::loadNormalTexture(std::string texture) {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
#if VRO_PLATFORM_IOS
    return std::make_shared<VROTexture>(format, false, VROMipmapMode::Runtime,
                                        std::make_shared<VROImageiOS>([UIImage imageNamed:[NSString stringWithUTF8String:texture.c_str()]], format));
#else
    if (texture.find(".") == std::string::npos) {
        texture = texture + ".png";
    }
    return std::make_shared<VROTexture>(format, false, VROMipmapMode::Runtime,
                                        std::make_shared<VROImageAndroid>(texture.c_str(), format));
#endif
}

std::shared_ptr<VRONode> VROTestUtil::loadFBXModel(std::string model, VROVector3f position, VROVector3f scale,
                                                   int lightMask, std::string animation) {
    std::string url;
    std::string base;
    
#if VRO_PLATFORM_IOS
    NSString *fbxPath = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:model.c_str()]
                                                        ofType:@"vrx"];
    NSURL *fbxURL = [NSURL fileURLWithPath:fbxPath];
    url = std::string([[fbxURL description] UTF8String]);
    
    NSString *basePath = [fbxPath stringByDeletingLastPathComponent];
    NSURL *baseURL = [NSURL fileURLWithPath:basePath];
    base = std::string([[baseURL description] UTF8String]);
#else
    url = "file:///android_asset/" + model + ".vrx";
    base = url.substr(0, url.find_last_of('/'));
#endif
    
    return VROFBXLoader::loadFBXFromURL(url, base, true,
                                        [scale, position, lightMask, animation](std::shared_ptr<VRONode> node, bool success) {
                                            if (!success) {
                                                return;
                                            }
                                            
                                            node->setScale(scale);
                                            node->setPosition(position);
                                            setLightMasks(node, lightMask);
                                            
                                            if (node->getGeometry()) {
                                                node->getGeometry()->setName("FBX Root Geometry");
                                            }
                                            for (std::shared_ptr<VRONode> &child : node->getChildNodes()) {
                                                if (child->getGeometry()) {
                                                    child->getGeometry()->setName("FBX Geometry");
                                                }
                                            }
                                            
                                            std::set<std::string> animations = node->getAnimationKeys(true);
                                            for (std::string animation : animations) {
                                                pinfo("Loaded animation [%s]", animation.c_str());
                                            }
                                            animateTake(node, animation);
                                        });
}

void VROTestUtil::animateTake(std::shared_ptr<VRONode> node, std::string name) {
    node->getAnimation(name.c_str(), true)->execute(node, [node, name] {
        animateTake(node, name);
    });
}

void VROTestUtil::setLightMasks(std::shared_ptr<VRONode> node, int value) {
    node->setLightReceivingBitMask(value);
    node->setShadowCastingBitMask(value);
    
    for (std::shared_ptr<VRONode> child : node->getChildNodes()) {
        setLightMasks(child, value);
    }
}

std::shared_ptr<VROVideoTexture> VROTestUtil::loadVideoTexture(VROStereoMode stereo) {
#if VRO_PLATFORM_IOS
    return std::make_shared<VROVideoTextureiOS>(stereo);
#else
    return std::make_shared<VROVideoTextureAVP>(stereo);
#endif
}

