//
//  VROTextureSubstrateOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTextureSubstrateOpenGL.h"
#include "VROImageUtil.h"
#include "VROTexture.h"
#include "VROData.h"
#include "VRODriverOpenGL.h"
#include "VROLog.h"
#include "VROAllocationTracker.h"

// Constants for ETC2 ripped from NDKr9 headers
#define GL_COMPRESSED_RGB8_ETC2                          0x9274
#define GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278

VROTextureSubstrateOpenGL::VROTextureSubstrateOpenGL(int width, int height, CGContextRef bitmapContext,
                                                   const VRODriver &driver) {
    
    glGenTextures(1, &_texture);
    
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, CGBitmapContextGetData(bitmapContext));
    
    ALLOCATION_TRACKER_ADD(TextureSubstrates, 1);
}

VROTextureSubstrateOpenGL::VROTextureSubstrateOpenGL(VROTextureType type, std::vector<UIImage *> &images,
                                                   const VRODriver &driver) {
    
    glGenTextures(1, &_texture);
    
    if (type == VROTextureType::Quad) {
        glBindTexture(GL_TEXTURE_2D, _texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        UIImage *image = images.front();
        int width = image.size.width * image.scale;
        int height = image.size.height * image.scale;
        
        size_t dataLength;
        void *data = VROExtractRGBA8888FromImage(image, &dataLength);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        free (data);
    }
    
    else if (type == VROTextureType::Cube && images.size() == 6) {
        passert_msg(images.size() == 6,
                    "Cube texture can only be created from exactly six images");
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        UIImage *firstImage = images.front();
        const CGFloat cubeSize = firstImage.size.width * firstImage.scale;
        
        for (int slice = 0; slice < 6; ++slice) {
            UIImage *image = images[slice];
            
            size_t dataLength;
            void *data = VROExtractRGBA8888FromImage(image, &dataLength);
            
            passert_msg(image.size.width == cubeSize && image.size.height == cubeSize,
                        "Cube map images must be square and uniformly-sized");
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice, 0, GL_RGB, cubeSize, cubeSize, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, data);
            free(data);
        }
    }
    
    else {
        pabort("Invalid texture images received, could not convert to OpenGL");
    }
    
    ALLOCATION_TRACKER_ADD(TextureSubstrates, 1);
}

VROTextureSubstrateOpenGL::VROTextureSubstrateOpenGL(VROTextureType type, VROTextureFormat format,
                                                   std::shared_ptr<VROData> data, int width, int height,
                                                   const VRODriver &driver) {
    
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (format == VROTextureFormat::ETC2) {
        if (type == VROTextureType::Quad) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA8_ETC2_EAC, width, height, 0, data->getDataLength(), data->getData());
        }
        else {
            pabort();
        }
    }
    else if (format == VROTextureFormat::ASTC_4x4_LDR) {
        if (type == VROTextureType::Quad) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_ASTC_4x4_KHR, width, height, 0, data->getDataLength(), data->getData());
        }
        else {
            pabort();
        }
    }
    else {
        pabort();
    }
    
}

VROTextureSubstrateOpenGL::~VROTextureSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(TextureSubstrates, 1);
}