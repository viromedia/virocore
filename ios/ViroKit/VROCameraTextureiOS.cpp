//
//  VROCameraTextureiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/22/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROCameraTextureiOS.h"
#include "VRORenderContext.h"
#include "VROFrameSynchronizer.h"
#include "VROTextureSubstrate.h"
#include "VROLog.h"
#include "VROVideoTextureCache.h"
#include "VRODriver.h"

VROCameraOrientation VROCameraTextureiOS::toCameraOrientation(UIInterfaceOrientation orientation) {
    if (orientation == UIInterfaceOrientationPortrait) {
        return VROCameraOrientation::Portrait;
    }
    else if (orientation == UIInterfaceOrientationLandscapeLeft) {
        return VROCameraOrientation::LandscapeLeft;
    }
    else if (orientation == UIInterfaceOrientationLandscapeRight) {
        return VROCameraOrientation::LandscapeRight;
    }
    else {
        return VROCameraOrientation::PortraitUpsideDown;
    }
}

VROCameraTextureiOS::VROCameraTextureiOS(VROTextureType type) :
    VROCameraTexture(type),
    _paused(true) {
    
}

VROCameraTextureiOS::~VROCameraTextureiOS() {
    
}

bool VROCameraTextureiOS::initCamera(VROCameraPosition position, VROCameraOrientation orientation,
                                     std::shared_ptr<VRODriver> driver) {
    pause();
    std::shared_ptr<VROCameraTextureiOS> shared = std::dynamic_pointer_cast<VROCameraTextureiOS>(shared_from_this());
    
    _videoTextureCache = std::shared_ptr<VROVideoTextureCache>(driver->newVideoTextureCache());
    _delegate = [[VROCameraCaptureDelegate alloc] initWithCameraTexture:shared cache:_videoTextureCache];
    
    // Create a capture session
    _captureSession = [[AVCaptureSession alloc] init];
    if (!_captureSession) {
        pinfo("Error: Could not create a capture session");
        return false;
    }
    
    [_captureSession beginConfiguration];
    [_captureSession setSessionPreset:AVCaptureSessionPresetHigh];
    
    // Get the a video device with the requested camera
    AVCaptureDevice *videoDevice = nil;
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if (position == VROCameraPosition::Front && [device position] == AVCaptureDevicePositionFront) {
            videoDevice = device;
        }
        else if (position == VROCameraPosition::Back && [device position] == AVCaptureDevicePositionBack) {
            videoDevice = device;
        }
    }
    
    if (videoDevice == nil) {
        videoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }
    if (videoDevice == nil) {
        pinfo("Error: could not create AVCaptureDevice");
        return false;
    }
    
    NSLog(@"Camera FOV [%f]", videoDevice.activeFormat.videoFieldOfView);
    
    NSError *error;
    
    // Device input
    AVCaptureDeviceInput *deviceInput = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&error];
    if (error) {
        pinfo("Error: could not create AVCaptureDeviceInput");
        return false;
    }
    
    [_captureSession addInput:deviceInput];
    
    // Create the output for the capture session.
    AVCaptureVideoDataOutput *dataOutput = [[AVCaptureVideoDataOutput alloc] init];
    [dataOutput setAlwaysDiscardsLateVideoFrames:YES];
    [dataOutput setVideoSettings:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]
                                                             forKey:(id)kCVPixelBufferPixelFormatTypeKey]];
    
    // Set dispatch to be on the main thread to create the texture in memory
    // and allow OpenGL to use it for rendering
    [dataOutput setSampleBufferDelegate:_delegate queue:dispatch_get_main_queue()];
    
    [_captureSession addOutput:dataOutput];
    updateOrientation(orientation);
    [_captureSession commitConfiguration];
    
    _orientationListener = [[VROCameraOrientationListener alloc] initWithCameraTexture:shared];
    [[NSNotificationCenter defaultCenter] addObserver:_orientationListener
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    return true;
}

float VROCameraTextureiOS::getHorizontalFOV() const {
    AVCaptureDeviceInput *input = [_captureSession.inputs firstObject];
    return input.device.activeFormat.videoFieldOfView;
}

void VROCameraTextureiOS::updateOrientation(VROCameraOrientation orientation) {
    AVCaptureVideoDataOutput *output = [[_captureSession outputs] objectAtIndex:0];
    
    AVCaptureConnection *connection = [output connectionWithMediaType:AVMediaTypeVideo];
    if (orientation == VROCameraOrientation::Portrait) {
        [connection setVideoOrientation:AVCaptureVideoOrientationPortrait];
    }
    else if (orientation == VROCameraOrientation::LandscapeLeft) {
        [connection setVideoOrientation:AVCaptureVideoOrientationLandscapeLeft];
    }
    else if (orientation == VROCameraOrientation::LandscapeRight) {
        [connection setVideoOrientation:AVCaptureVideoOrientationLandscapeRight];
    }
    else {
        [connection setVideoOrientation:AVCaptureVideoOrientationPortraitUpsideDown];
    }
}

void VROCameraTextureiOS::pause() {
    _paused = true;
    [_captureSession stopRunning];
}

void VROCameraTextureiOS::play() {
    _paused = false;
    [_captureSession startRunning];
}

bool VROCameraTextureiOS::isPaused() {
    return _paused;
}

void VROCameraTextureiOS::displayPixelBuffer(std::unique_ptr<VROTextureSubstrate> substrate) {
    setSubstrate(std::move(substrate));
}

#pragma mark - VROCameraCaptureDelegate

@interface VROCameraCaptureDelegate ()

@property (readwrite, nonatomic) std::weak_ptr<VROCameraTextureiOS> texture;
@property (readwrite, nonatomic) std::weak_ptr<VROVideoTextureCache> cache;

@end

@implementation VROCameraCaptureDelegate

- (id)initWithCameraTexture:(std::shared_ptr<VROCameraTextureiOS>)texture
                      cache:(std::shared_ptr<VROVideoTextureCache>)cache {
    self = [super init];
    if (self) {
        self.texture = texture;
        self.cache = cache;
    }
    
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    
    std::shared_ptr<VROCameraTextureiOS> texture = self.texture.lock();
    std::shared_ptr<VROVideoTextureCache> cache = self.cache.lock();
    if (texture && cache) {
        texture->displayPixelBuffer(cache->createTextureSubstrate(sampleBuffer));
    }
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    
}

@end

@interface VROCameraOrientationListener ()

@property (readwrite, nonatomic) std::weak_ptr<VROCameraTextureiOS> texture;

@end

@implementation VROCameraOrientationListener

- (id)initWithCameraTexture:(std::shared_ptr<VROCameraTextureiOS>)texture {
    self = [super init];;
    if (self) {
        self.texture = texture;
    }
    return self;
}

- (void)orientationDidChange:(NSNotification *)notification {
    std::shared_ptr<VROCameraTextureiOS> texture = self.texture.lock();
    if (!texture) {
        return;
    }
    
    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    texture->updateOrientation(VROCameraTextureiOS::toCameraOrientation(orientation));
}

@end
