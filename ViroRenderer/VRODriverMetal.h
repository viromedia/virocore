//
//  VRORenderContextMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
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
    
    void willRenderFrame(VRORenderContext &context) {
        
    }
    void didRenderFrame(const VROFrameTimer &timer, const VRORenderContext &context) {
        
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
    
    std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() {
        return new VROVideoTextureCacheMetal(_device);
    }
    
    std::shared_ptr<VROSound> newSound(std::string resource, VROResourceType resourceType, VROSoundType type) {
        NSURL *fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fileName.c_str()]];
        std::string url = std::string([[fileURL description] UTF8String]);
        
        return std::make_shared<VROSoundEffectiOS>(url);
    }
    
    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string fileName, bool isLocal) {
        NSURL *fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:fileName.c_str()]];
        std::string url = std::string([[fileURL description] UTF8String]);
        
        return std::make_shared<VROAudioPlayeriOS>(url);
    }
    
    std::shared_ptr<VROTypeface> newTypeface(std::string typeface, int size) {
        // TODO Metal: Not supported
        pabort();
    }

private:
    
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
    id <MTLLibrary> _library;
    
    std::shared_ptr<VRORenderTarget> _renderTarget;
    
};

#endif
#endif /* VRORenderContextMetal_h */
