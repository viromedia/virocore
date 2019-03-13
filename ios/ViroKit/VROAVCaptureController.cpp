//
//  VROAVCaptureController.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/12/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROAVCaptureController.h"
#include "VRORenderContext.h"
#include "VROFrameSynchronizer.h"
#include "VROTextureSubstrate.h"
#include "VROLog.h"
#include "VROVideoTextureCache.h"
#include "VRODriver.h"
#include "VROConvert.h"
#include "VROImagePreprocessor.h"
#include "VRODriverOpenGLiOS.h"

VROAVCaptureController::VROAVCaptureController() :
    _paused(true),
    _lastSampleBuffer(nil) {
    
}

void VROAVCaptureController::initCapture(VROCameraPosition position, VROCameraOrientation orientation,
                                         bool renderPreview,
                                         std::shared_ptr<VRODriver> driver) {
    pause();
    std::shared_ptr<VROAVCaptureController> shared = std::dynamic_pointer_cast<VROAVCaptureController>(shared_from_this());
    
    _delegate = [[VROCameraCaptureDelegate alloc] initWithCaptureController:shared];
    
    // Create a capture session
    _captureSession = [[AVCaptureSession alloc] init];
    if (!_captureSession) {
        pinfo("Error: Could not create a capture session");
        return;
    }
    
    [_captureSession beginConfiguration];
    [_captureSession setSessionPreset:AVCaptureSessionPresetHigh];
    
    // Get the a video device with the requested camera
    AVCaptureDevicePosition avPosition = (position == VROCameraPosition::Front) ? AVCaptureDevicePositionFront : AVCaptureDevicePositionBack;
    AVCaptureDevice *videoDevice = [[[AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera]
                                                                                            mediaType:AVMediaTypeVideo
                                                                                             position:avPosition] devices] firstObject];

    NSError *error;
    
    // Device input
    AVCaptureDeviceInput *deviceInput = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&error];
    if (error) {
        pinfo("Error: could not create AVCaptureDeviceInput");
        return;
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
    
    _orientationListener = [[VROCameraOrientationListener alloc] initWithCaptureController:shared];
    [[NSNotificationCenter defaultCenter] addObserver:_orientationListener
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    
    // Render a preview layer
    if (renderPreview) {
        AVCaptureVideoPreviewLayer *previewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:_captureSession];
        previewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
        
        std::shared_ptr<VRODriverOpenGLiOS> driveriOS = std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver);
        UIView *previewView = driveriOS->getView();
        CALayer *rootLayer = previewView.layer;
        previewLayer.frame = rootLayer.bounds;
        previewLayer.opacity = 1.0;
        
        [rootLayer addSublayer:previewLayer];
    }
}

VROAVCaptureController::~VROAVCaptureController() {
    if (_lastSampleBuffer) {
        CFRelease(_lastSampleBuffer);
    }
    [[NSNotificationCenter defaultCenter] removeObserver:_orientationListener];
}

float VROAVCaptureController::getHorizontalFOV() const {
    AVCaptureDeviceInput *input = [_captureSession.inputs firstObject];
    return input.device.activeFormat.videoFieldOfView;
}

VROVector3f VROAVCaptureController::getImageSize() const {
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

void VROAVCaptureController::updateOrientation(VROCameraOrientation orientation) {
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

void VROAVCaptureController::pause() {
    _paused = true;
    [_captureSession stopRunning];
}

void VROAVCaptureController::play() {
    _paused = false;
    [_captureSession startRunning];
}

bool VROAVCaptureController::isPaused() {
    return _paused;
}

void VROAVCaptureController::update(CMSampleBufferRef sampleBuffer) {
    if (_lastSampleBuffer) {
        CFRelease(_lastSampleBuffer);
    }
    CFRetain(sampleBuffer);
    _lastSampleBuffer = sampleBuffer;
    
    if (_updateListener) {
        _updateListener(sampleBuffer);
    }
}

#pragma mark - VROCameraCaptureDelegate

@interface VROCameraCaptureDelegate ()

@property (readwrite, nonatomic) std::weak_ptr<VROAVCaptureController> controller;

@end

@implementation VROCameraCaptureDelegate

- (id)initWithCaptureController:(std::shared_ptr<VROAVCaptureController>)controller {
    self = [super init];
    if (self) {
        self.controller = controller;
    }
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
fromConnection:(AVCaptureConnection *)connection {
    
    std::shared_ptr<VROAVCaptureController> controller = self.controller.lock();
    if (controller) {
        controller->update(sampleBuffer);
    }
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
fromConnection:(AVCaptureConnection *)connection {
    
}

@end

@interface VROCameraOrientationListener ()

@property (readwrite, nonatomic) std::weak_ptr<VROAVCaptureController> controller;

@end

@implementation VROCameraOrientationListener

- (id)initWithCaptureController:(std::shared_ptr<VROAVCaptureController>)controller {
    self = [super init];;
    if (self) {
        self.controller = controller;
    }
    return self;
}

- (void)orientationDidChange:(NSNotification *)notification {
    std::shared_ptr<VROAVCaptureController> controller = self.controller.lock();
    if (!controller) {
        return;
    }
    
    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    controller->updateOrientation(VROConvert::toCameraOrientation(orientation));
}

@end
