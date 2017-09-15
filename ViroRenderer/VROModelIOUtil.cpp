//
//  VROModelIOUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROModelIOUtil.h"
#include "VROPlatformUtil.h"
#include "VROImage.h"
#include "VROTexture.h"
#include "VROLog.h"
#include "VROTextureUtil.h"
#include "VROStringUtil.h"

std::shared_ptr<VROTexture> VROModelIOUtil::loadTexture(const std::string &name, std::string &base, bool isBaseURL, bool sRGB,
                                                        const std::map<std::string, std::string> *resourceMap,
                                                        std::map<std::string, std::shared_ptr<VROTexture>> &cache) {
    std::shared_ptr<VROTexture> texture;

    auto it = cache.find(name);
    if (it == cache.end()) {
        bool isTempTextureFile = false;
        std::string textureFile;
        
        if (resourceMap == nullptr) {
            textureFile = base + "/" + name;
        } else {
            textureFile = VROPlatformFindValueInResourceMap(name, *resourceMap);
        }

        if (isBaseURL) {
            bool success = false;
            textureFile = VROPlatformDownloadURLToFile(textureFile, &isTempTextureFile, &success);
        }

        // Abort (return empty texture) if the file wasn't found
        if (textureFile.length() == 0) {
            return texture;
        }
        
        if (VROStringUtil::endsWith(name, "ktx")) {
            int dataLength;
            void *data = VROPlatformLoadFile(textureFile, &dataLength);
            
            VROTextureFormat format;
            int texWidth;
            int texHeight;
            std::vector<uint32_t> mipSizes;
            std::shared_ptr<VROData> texData = VROTextureUtil::readKTXHeader((uint8_t *) data, (uint32_t) dataLength,
                                                                             &format, &texWidth, &texHeight, &mipSizes);
            std::vector<std::shared_ptr<VROData>> dataVec = { texData };
            
            texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, format,
                                                   VROTextureInternalFormat::RGBA8, true,
                                                   VROMipmapMode::Pregenerated,
                                                   dataVec, texWidth, texHeight, mipSizes);
        }
        else {
            std::shared_ptr<VROImage> image = VROPlatformLoadImageFromFile(textureFile,
                                                                           VROTextureInternalFormat::RGBA8);
            if (isTempTextureFile) {
                VROPlatformDeleteFile(textureFile);
            }
            if (!image) {
                pinfo("Failed to load texture [%s] at path [%s]", name.c_str(), textureFile.c_str());
                return {};
            }
            texture = std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8, sRGB, VROMipmapMode::Runtime, image);
        }
        cache.insert(std::make_pair(name, texture));
    }
    else {
        texture = it->second;
    }
    
    return texture;
}
