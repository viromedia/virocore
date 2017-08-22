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
#include "VROConvert.h"

VROCameraTextureiOS::VROCameraTextureiOS(VROTextureType type) :
    VROCameraTexture(type),
    _paused(true) {
    
}

VROCameraTextureiOS::~VROCameraTextureiOS() {
    [[NSNotificationCenter defaultCenter] removeObserver:_orientationListener];
}

bool VROCameraTextureiOS::initCamera(VROCameraPosition position, VROCameraOrientation orientation,
                                     std::shared_ptr<VRODriver> driver) {
    pause();
    std::shared_ptr<VROCameraTextureiOS> shared = std::dynamic_pointer_cast<VROCameraTextureiOS>(shared_from_this());
    
    _videoTextureCache = driver->newVideoTextureCache();
    _delegate = [[VROCameraCaptureDelegate alloc] initWithCameraTexture:shared cache:_videoTextureCache driver:driver];
    
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

VROVector3f VROCameraTextureiOS::getImageSize() const {
    AVCaptureDeviceInput *input = [_captureSession.inputs firstObject];
    
    CMFormatDescriptionRef desc = input.device.activeFormat.formatDescription;
    CGSize dim = CMVideoFormatDescriptionGetPresentationDimensions(desc, true, true);

    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    if (UIInterfaceOrientationIsPortrait(orientation)) {
        return { (float)dim.height, (float)dim.width,  0 };
    }
    else {
        return { (float)dim.width,  (float)dim.height, 0 };
    }
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
    setSubstrate(0, std::move(substrate));
}

#pragma mark - VROCameraCaptureDelegate

@interface VROCameraCaptureDelegate ()

@property (readwrite, nonatomic) std::weak_ptr<VROCameraTextureiOS> texture;
@property (readwrite, nonatomic) std::weak_ptr<VROVideoTextureCache> cache;
@property (readwrite, nonatomic) std::weak_ptr<VRODriver> driver;

@end

@implementation VROCameraCaptureDelegate

- (id)initWithCameraTexture:(std::shared_ptr<VROCameraTextureiOS>)texture
                      cache:(std::shared_ptr<VROVideoTextureCache>)cache
                     driver:(std::shared_ptr<VRODriver>)driver {
    self = [super init];
    if (self) {
        self.texture = texture;
        self.cache = cache;
        self.driver = driver;
        self.trackingHelper = [[VROTrackingHelper alloc] init];
    }
    
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    
    std::shared_ptr<VROCameraTextureiOS> texture = self.texture.lock();
    std::shared_ptr<VROVideoTextureCache> cache = self.cache.lock();
    std::shared_ptr<VRODriver> driver = self.driver.lock();
    if (texture && cache && driver) {
        texture->displayPixelBuffer(cache->createTextureSubstrate(sampleBuffer, driver->isGammaCorrectionEnabled()));
    }

    // Uncomment this line to enable image detection
    // [_trackingHelper processBuffer:sampleBuffer];
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
    texture->updateOrientation(VROConvert::toCameraOrientation(orientation));
}

@end
