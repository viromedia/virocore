//
//  VROTextureSubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/4/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROTextureSubstrateMetal.h"
#include "VROImageUtil.h"
#include "VROTexture.h"
#include "VRORenderContextMetal.h"
#include "VROLog.h"

VROTextureSubstrateMetal::VROTextureSubstrateMetal(int width, int height, CGContextRef bitmapContext,
                                                   const VRORenderContext &context) {
    id <MTLDevice> device = ((VRORenderContextMetal &)context).getDevice();

    int bytesPerPixel = 4;
    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:width
                                                                                         height:height
                                                                                   mipmapped:NO];
    
    _texture = [device newTextureWithDescriptor:descriptor];
    [_texture replaceRegion:MTLRegionMake2D(0, 0, width, height)
                mipmapLevel:0
                  withBytes:CGBitmapContextGetData(bitmapContext)
                bytesPerRow:bytesPerPixel * width];
    
    ALLOCATION_TRACKER_ADD(TextureSubstrates, 1);
}

VROTextureSubstrateMetal::VROTextureSubstrateMetal(VROTextureType type, std::vector<UIImage *> &images,
                                                   const VRORenderContext &context) {
    
    VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    id <MTLDevice> device = metal.getDevice();
    
    if (type == VROTextureType::Quad) {
        UIImage *image = images.front();
        int width = image.size.width * image.scale;
        int height = image.size.height * image.scale;
        
        size_t dataLength;
        void *data = VROExtractRGBA8888FromImage(image, &dataLength);
        
        int bytesPerPixel = 4;
        MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                              width:width
                                                                                             height:height
                                                                                          mipmapped:YES];
        _texture = [device newTextureWithDescriptor:descriptor];
        
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        [_texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bytesPerPixel * width];
        
        id <MTLCommandBuffer> textureCommandBuffer = [metal.getCommandQueue() commandBuffer];
        id<MTLBlitCommandEncoder> commandEncoder = [textureCommandBuffer blitCommandEncoder];
        [commandEncoder generateMipmapsForTexture:_texture];
        [commandEncoder endEncoding];
        [textureCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {

        }];
        [textureCommandBuffer commit];
        
        free (data);
    }
    
    else if (type == VROTextureType::Cube && images.size() == 6) {
        passert_msg(images.size() == 6,
                    "Cube texture can only be created from exactly six images");
        
        UIImage *firstImage = images.front();
        const CGFloat cubeSize = firstImage.size.width * firstImage.scale;
        
        const NSUInteger bytesPerPixel = 4;
        const NSUInteger bytesPerRow = bytesPerPixel * cubeSize;
        const NSUInteger bytesPerImage = bytesPerRow * cubeSize;
        
        MTLRegion region = MTLRegionMake2D(0, 0, cubeSize, cubeSize);
        
        MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                        size:cubeSize
                                                                                                   mipmapped:NO];
        _texture = [device newTextureWithDescriptor:textureDescriptor];
        
        for (size_t slice = 0; slice < 6; ++slice) {
            UIImage *image = images[slice];
            
            size_t dataLength;
            void *data = VROExtractRGBA8888FromImage(image, &dataLength);
            
            passert_msg(image.size.width == cubeSize && image.size.height == cubeSize,
                        "Cube map images must be square and uniformly-sized");
            
            [_texture replaceRegion:region
                        mipmapLevel:0
                              slice:slice
                          withBytes:data
                        bytesPerRow:bytesPerRow
                      bytesPerImage:bytesPerImage];
            free(data);
        }
    }
    
    else {
        pabort("Invalid texture images received, could not convert to Metal");
    }
    
    ALLOCATION_TRACKER_ADD(TextureSubstrates, 1);
}

VROTextureSubstrateMetal::VROTextureSubstrateMetal(VROTextureType type, VROTextureFormat format,
                                                   std::shared_ptr<VROData> data, int width, int height, bool mipmap,
                                                   const VRORenderContext &context) {
    
    if (format == VROTextureFormat::ETC2) {
        VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
        id <MTLDevice> device = metal.getDevice();
        
        if (type == VROTextureType::Quad) {
            int bytesPerRow = width / 4 * 8; // TODO This is not working
            MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatETC2_RGB8
                                                                                                  width:width
                                                                                                 height:height
                                                                                              mipmapped:mipmap];
            _texture = [device newTextureWithDescriptor:descriptor];
     
            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            [_texture replaceRegion:region mipmapLevel:0 withBytes:data->getData() bytesPerRow:bytesPerRow];
            
            if (mipmap) {
                id <MTLCommandBuffer> textureCommandBuffer = [metal.getCommandQueue() commandBuffer];
                id<MTLBlitCommandEncoder> commandEncoder = [textureCommandBuffer blitCommandEncoder];
                [commandEncoder generateMipmapsForTexture:_texture];
                [commandEncoder endEncoding];
                [textureCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                    
                }];
                [textureCommandBuffer commit];
            }
        }
        else {
            pabort();
        }
    }
    else if (format == VROTextureFormat::ASTC_4x4_LDR) {
        VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
        id <MTLDevice> device = metal.getDevice();
        
        if (type == VROTextureType::Quad) {
            int bytesPerRow = width / 4 * 16; // texels per row / block footprint (4) / * 16 (each ASTC block is 16 bytes)
            MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatASTC_4x4_LDR
                                                                                                  width:width
                                                                                                 height:height
                                                                                              mipmapped:mipmap];
            _texture = [device newTextureWithDescriptor:descriptor];
            
            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            [_texture replaceRegion:region mipmapLevel:0 withBytes:data->getData() bytesPerRow:bytesPerRow];
            
            if (mipmap) {
                id <MTLCommandBuffer> textureCommandBuffer = [metal.getCommandQueue() commandBuffer];
                id<MTLBlitCommandEncoder> commandEncoder = [textureCommandBuffer blitCommandEncoder];
                [commandEncoder generateMipmapsForTexture:_texture];
                [commandEncoder endEncoding];
                [textureCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                    
                }];
                [textureCommandBuffer commit];
            }
        }
        else {
            pabort();
        }
    }
    else {
        pabort();
    }
    
}

VROTextureSubstrateMetal::~VROTextureSubstrateMetal() {
    ALLOCATION_TRACKER_SUB(TextureSubstrates, 1);
}

