//
//  VROVideoTexture.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoSurface.h"
#include "VRORenderContextMetal.h"
#include "VROSurface.h"
#include "VROLog.h"
#include "VROMaterialSubstrateMetal.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

std::shared_ptr<VROVideoSurface> VROVideoSurface::createVideoSurface(float width, float height) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    VROSurface::buildGeometry(width, height, sources, elements);
    
    std::shared_ptr<VROVideoSurface> surface = std::shared_ptr<VROVideoSurface>(new VROVideoSurface(sources, elements));
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setWritesToDepthBuffer(true);
    material->setReadsFromDepthBuffer(true);
    
    surface->getMaterials().push_back(material);
    return surface;
}

VROVideoSurface::VROVideoSurface(std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                 std::vector<std::shared_ptr<VROGeometryElement>> &elements) :
    VROSurface(sources, elements),
    _currentTextureIndex(0) {

}

VROVideoSurface::~VROVideoSurface() {

}

void VROVideoSurface::captureFrontCamera(VRORenderContext &context) {
     context.addFrameListener(shared_from_this());
    _videoDelegate = [[VROVideoCaptureDelegate alloc] initWithVROVideoSurface:this];
    
    id <MTLDevice> device = ((VRORenderContextMetal &)context).getDevice();
    
    CVReturn textureCacheError = CVMetalTextureCacheCreate(kCFAllocatorDefault, NULL, device,
                                                           NULL, &_videoTextureCache);
    
    if (textureCacheError) {
        pinfo("ERROR: Couldnt create a texture cache");
        pabort();
    }
    
    // Create a capture session
    _captureSession = [[AVCaptureSession alloc] init];
    if (!_captureSession) {
        pinfo("ERROR: Couldnt create a capture session");
        pabort();
    }
    
    [_captureSession beginConfiguration];
    [_captureSession setSessionPreset:AVCaptureSessionPresetLow];
    
    // Get the a video device with preference to the front facing camera
    AVCaptureDevice *videoDevice = nil;
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if ([device position] == AVCaptureDevicePositionFront) {
            videoDevice = device;
        }
    }
    
    if (videoDevice == nil) {
        videoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }
    
    if (videoDevice == nil) {
        pinfo("ERROR: Couldnt create a AVCaptureDevice");
        pabort();
    }
    
    NSError *error;
    
    // Device input
    AVCaptureDeviceInput *deviceInput = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&error];
    
    if (error) {
        pinfo("ERROR: Couldnt create AVCaptureDeviceInput");
        pabort();
    }
    
    [_captureSession addInput:deviceInput];
    
    // Create the output for the capture session.
    AVCaptureVideoDataOutput *dataOutput = [[AVCaptureVideoDataOutput alloc] init];
    [dataOutput setAlwaysDiscardsLateVideoFrames:YES];
    
    // Set the color space.
    [dataOutput setVideoSettings:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]
                                                             forKey:(id)kCVPixelBufferPixelFormatTypeKey]];
    
    // Set dispatch to be on the main thread to create the texture in memory and allow Metal to use it for rendering
    [dataOutput setSampleBufferDelegate:_videoDelegate queue:dispatch_get_main_queue()];
    
    [_captureSession addOutput:dataOutput];
    [_captureSession commitConfiguration];
    [_captureSession startRunning];
}

void VROVideoSurface::displayVideo(NSURL *url, VRORenderContext &context) {
    
}

@interface VROVideoCaptureDelegate ()

@property (readonly) VROVideoSurface *surface;

@end

@implementation VROVideoCaptureDelegate {
    
    id <MTLTexture> _videoTexture[3];
    
}

- (id)initWithVROVideoSurface:(VROVideoSurface *)surface {
    self = [super init];
    if (self) {
        _surface = surface;
    }
    
    return self;
}


- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    
    CVMetalTextureCacheRef videoTextureCache = _surface->getVideoTextureCache();
    int currentTextureIndex = _surface->getCurrentTextureIndex();
    
    CVReturn error;
    
    CVImageBufferRef sourceImageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    size_t width = CVPixelBufferGetWidth(sourceImageBuffer);
    size_t height = CVPixelBufferGetHeight(sourceImageBuffer);
    
    CVMetalTextureRef textureRef;
    error = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault, videoTextureCache, sourceImageBuffer,
                                                      NULL, MTLPixelFormatBGRA8Unorm, width, height, 0, &textureRef);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    _videoTexture[currentTextureIndex] = CVMetalTextureGetTexture(textureRef);
    if (!_videoTexture[currentTextureIndex]) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->getDiffuse().setContents(std::make_shared<VROTexture>(_videoTexture[currentTextureIndex]));
    _surface->getMaterials()[0] = material;
    
    CVBufferRelease(textureRef);
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {

}

@end