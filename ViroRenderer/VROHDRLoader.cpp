//
//  VROHDRLoader.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/22/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROHDRLoader.h"
#include "VROLog.h"
#include "VROTexture.h"
#include "VROData.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm/gtc/packing.hpp"
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <stdint.h>
#include <iostream>
#include <array>

// Set to true to compress HDR textures into RGB9_E5 format; false to
// store HDR textures in memory in fully expanded RGB16F.
static bool kCompressHDR = true;

std::shared_ptr<VROTexture> VROHDRLoader::loadRadianceHDRTexture(std::string hdrPath) {
    int width, height, n;

    pinfo("Loading Radiance HDR file [%s]...", hdrPath.c_str());
    float *data = stbi_loadf(hdrPath.c_str(), &width, &height, &n, 0);
    if (data == nullptr) {
        pinfo("Error loading Radiance HDR file");
        return nullptr;
    }
    
    pinfo("Load successful [width: %d, height %d, components per pixel %d]", width, height, n);
    
    // Note the data float* will be freed by loadTexture, if necessary
    std::shared_ptr<VROTexture> texture = loadTexture(data, width, height, n);
    return texture;
}

std::shared_ptr<VROTexture> VROHDRLoader::loadTexture(float *data, int width, int height, int componentsPerPixel) {
    passert (componentsPerPixel == 3 || componentsPerPixel == 4);
    int numPixels = width * height;
    
    if (kCompressHDR) {
        int packedLength = numPixels * sizeof(uint32_t);
        uint32_t *packedF9E5 = (uint32_t *) malloc(packedLength);
        for (int i = 0; i < numPixels; i++) {
            float r = data[i * componentsPerPixel + 0];
            float g = data[i * componentsPerPixel + 1];
            float b = data[i * componentsPerPixel + 2];
            // alpha is disregarded
            
            const glm::vec3 v(r, g, b);
            packedF9E5[i] = glm::packF3x9_E1x5(v);
        }
        free (data);
        
        std::vector<uint32_t> mipSizes;
        std::shared_ptr<VROData> texData = std::make_shared<VROData>(packedF9E5, packedLength, VRODataOwnership::Move);
        std::vector<std::shared_ptr<VROData>> dataVec = { texData };
        
        return std::make_shared<VROTexture>(VROTextureType::Texture2D,
                                            VROTextureFormat::RGB9_E5,
                                            VROTextureInternalFormat::RGB9_E5, true,
                                            VROMipmapMode::None,
                                            dataVec, width, height, mipSizes);
    }
    else {
        int length = numPixels * componentsPerPixel * sizeof(float);
        
        std::vector<uint32_t> mipSizes;
        std::shared_ptr<VROData> texData = std::make_shared<VROData>(data, length, VRODataOwnership::Move);
        std::vector<std::shared_ptr<VROData>> dataVec = { texData };
        
        return std::make_shared<VROTexture>(VROTextureType::Texture2D,
                                            VROTextureFormat::RGB16F,
                                            VROTextureInternalFormat::RGB16F, true,
                                            VROMipmapMode::None,
                                            dataVec, width, height, mipSizes);
    }
}
