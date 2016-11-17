//
//  VRORenderContextMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRORenderContextMetal_h
#define VRORenderContextMetal_h

#include "VRODefines.h"
#if VRO_METAL

#include <stdio.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include "VRODriver.h"
#include "VRORenderTarget.h"
#include "VROMatrix4f.h"
#include "VROSoundEffectiOS.h"
#include <memory>
#include "VROGeometrySubstrateMetal.h"
#include "VROMaterialSubstrateMetal.h"
#include "VROTextureSubstrateMetal.h"
#include "VROVideoTextureCacheMetal.h"

/*
 Driver for Metal.
 */
class VRODriverMetal : public VRODriver {
    
public:
    
    VRODriverMetal(id <MTLDevice> device) {
        _device = device;
        _commandQueue = [device newCommandQueue];
        
        NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
        NSString *shaders = [bundle pathForResource:@"default" ofType:@"metallib"];
        
        _library = [device newLibraryWithFile:shaders error:nil];
    }
    
    void setRenderTarget(std::shared_ptr<VRORenderTarget> renderTarget) {
        _renderTarget = renderTarget;
    }
    
    id <MTLDevice> getDevice() const {
        return _device;
    }
    id <MTLCommandQueue> getCommandQueue() const {
        return _commandQueue;
    }
    id <MTLLibrary> getLibrary() const {
        return _library;
    }
    std::shared_ptr<VRORenderTarget> getRenderTarget() const {
        return _renderTarget;
    }
    
    VROGeometrySubstrate *newGeometrySubstrate(const VROGeometry &geometry) {
        return new VROGeometrySubstrateMetal(geometry, *this);
    }
    
    VROMaterialSubstrate *newMaterialSubstrate(VROMaterial &material) {
        return new VROMaterialSubstrateMetal(material, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, std::vector<std::shared_ptr<VROImage>> &images) {
        return new VROTextureSubstrateMetal(type, images, *this);
    }
    
    VROTextureSubstrate *newTextureSubstrate(VROTextureType type, VROTextureFormat format, std::shared_ptr<VROData> data,
                                             int width, int height) {
        return new VROTextureSubstrateMetal(type, format, data, width, height, *this);
    }
    
    VROVideoTextureCache *newVideoTextureCache() {
        return new VROVideoTextureCacheMetal(_device);
    }
    
    std::shared_ptr<VROSoundEffect> newSoundEffect(std::string fileName) {
        NSURL *fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fileName.c_str()]];
        std::string url = std::string([[fileURL description] UTF8String]);
        
        return std::make_shared<VROSoundEffectiOS>(url);
    }

private:
    
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
    id <MTLLibrary> _library;
    
    std::shared_ptr<VRORenderTarget> _renderTarget;
    
};

#endif
#endif /* VRORenderContextMetal_h */
