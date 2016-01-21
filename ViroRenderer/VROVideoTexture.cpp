//
//  VROVideoTexture.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoTexture.h"
#include "VRORenderContextMetal.h"
#include "VROLog.h"
#include "VROTextureSubstrateMetal.h"
#include "VROTime.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

# define ONE_FRAME_DURATION 0.03

VROVideoTexture::VROVideoTexture() :
    _notificationToken(nullptr),
    _paused(true),
    _currentTextureIndex(0) {
    
    _player = [[AVPlayer alloc] init];
    
    NSDictionary *pixBuffAttributes = @{(id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)};
    _videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixBuffAttributes];
}

VROVideoTexture::~VROVideoTexture() {
    
}

#pragma mark - Recorded Video Playback

void VROVideoTexture::displayVideo(NSURL *url, VRORenderContext &context) {
    _paused = true;
    
    id <MTLDevice> device = ((VRORenderContextMetal &)context).getDevice();
    
    CVReturn textureCacheError = CVMetalTextureCacheCreate(kCFAllocatorDefault, NULL, device,
                                                           NULL, &_videoTextureCache);
    if (textureCacheError) {
        pinfo("ERROR: Couldnt create a texture cache");
        pabort();
    }
    
    _videoQueue = dispatch_queue_create("video_output_queue", DISPATCH_QUEUE_SERIAL);
    _videoPlaybackDelegate = [[VROVideoPlaybackDelegate alloc] initWithVROVideoTexture:this];
    [_videoOutput setDelegate:_videoPlaybackDelegate queue:_videoQueue];
    
    context.addFrameListener(shared_from_this());
    
    /*
     Sets up player item and adds video output to it. The tracks property of an asset is
     loaded via asynchronous key value loading, to access the preferred transform of a
     video track used to orientate the video while rendering.
     
     After adding the video output, we request a notification of media change in order
     to restart the CADisplayLink.
     */
    [[_player currentItem] removeOutput:_videoOutput];
    
    AVPlayerItem *item = [AVPlayerItem playerItemWithURL:url];
    AVAsset *asset = [item asset];
    
    [asset loadValuesAsynchronouslyForKeys:@[@"tracks"] completionHandler:^{
        
        if ([asset statusOfValueForKey:@"tracks" error:nil] == AVKeyValueStatusLoaded) {
            NSArray *tracks = [asset tracksWithMediaType:AVMediaTypeVideo];
            if ([tracks count] > 0) {
                
                // Choose the first video track.
                AVAssetTrack *videoTrack = [tracks objectAtIndex:0];
                [videoTrack loadValuesAsynchronouslyForKeys:@[@"preferredTransform"] completionHandler:^{
                    
                    if ([videoTrack statusOfValueForKey:@"preferredTransform" error:nil] == AVKeyValueStatusLoaded) {
                        CGAffineTransform preferredTransform = [videoTrack preferredTransform];
                        
                        /*
                         The orientation of the camera while recording affects the orientation
                         of the images received from an AVPlayerItemVideoOutput. Here we compute a
                         rotation that is used to correctly orientate the video.
                         */
                        _preferredRotation = -1 * atan2(preferredTransform.b, preferredTransform.a);
                        addLoopNotification(item);
                        
                        dispatch_async(dispatch_get_main_queue(), ^{
                            [item addOutput:_videoOutput];
                            [_player replaceCurrentItemWithPlayerItem:item];
                            [_videoOutput requestNotificationOfMediaDataChangeWithAdvanceInterval:ONE_FRAME_DURATION];
                            [_player play];
                        });
                    }
                }];
            }
        }
    }];
}

void VROVideoTexture::addLoopNotification(AVPlayerItem *item) {
    if (_notificationToken) {
        _notificationToken = nil;
    }
    
    /*
     Setting actionAtItemEnd to None prevents the movie from getting paused at item end. A very simplistic, and not gapless, looped playback.
     */
    _player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
    _notificationToken = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification
                                                                           object:item
                                                                            queue:[NSOperationQueue mainQueue]
                                                                       usingBlock:^(NSNotification *note) {
                          [[_player currentItem] seekToTime:kCMTimeZero];
                          }
                          ];
}

void VROVideoTexture::onFrameWillRender() {
    if (_paused) {
        return;
    }
}

void VROVideoTexture::onFrameDidRender() {
    if (_paused) {
        return;
    }
    
    _currentTextureIndex = (_currentTextureIndex + 1) % kInFlightVideoTextures;
    
    double timestamp = CACurrentMediaTime();
    double duration = .01667;
    
    /*
     The callback gets called once every frame. Compute the next time the screen will be
     refreshed, and copy the pixel buffer for that time. This pixel buffer can then be processed
     and later rendered on screen.
     */
    CFTimeInterval nextVSync = timestamp + duration;
    CMTime outputItemTime = [_videoOutput itemTimeForHostTime:nextVSync];
    
    if ([_videoOutput hasNewPixelBufferForItemTime:outputItemTime]) {
        CVPixelBufferRef pixelBuffer = [_videoOutput copyPixelBufferForItemTime:outputItemTime
                                                             itemTimeForDisplay:NULL];
        
        displayPixelBuffer(pixelBuffer);
        if (pixelBuffer != nullptr) {
            CFRelease(pixelBuffer);
        }
    }
}

void VROVideoTexture::displayPixelBuffer(CVPixelBufferRef pixelBuffer) {
    if (pixelBuffer == nullptr) {
        return;
    }
    
    CVReturn error;
    
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    
    CVMetalTextureRef textureRef;
    error = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _videoTextureCache, pixelBuffer,
                                                      NULL, MTLPixelFormatBGRA8Unorm, width, height, 0, &textureRef);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    id <MTLTexture> videoTexture = CVMetalTextureGetTexture(textureRef);
    if (!videoTexture) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateMetal>(new VROTextureSubstrateMetal(videoTexture));
    setSubstrate(VROTextureType::Quad, std::move(substrate));
    
    CVBufferRelease(textureRef);
}

@interface VROVideoPlaybackDelegate ()

@property (readonly) VROVideoTexture *texture;

@end

@implementation VROVideoPlaybackDelegate

- (id)initWithVROVideoTexture:(VROVideoTexture *)texture {
    self = [super init];
    if (self) {
        _texture = texture;
    }
    
    return self;
}

- (void)outputMediaDataWillChange:(AVPlayerItemOutput *)sender {
    self.texture->setPaused(false);
}

@end

#pragma mark - Live Video Playback

void VROVideoTexture::displayCamera(AVCaptureDevicePosition position, VRORenderContext &context) {
    context.addFrameListener(shared_from_this());
    _videoDelegate = [[VROVideoCaptureDelegate alloc] initWithVROVideoTexture:this];
    
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
        if ([device position] == position) {
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
    
    _paused = false;
}

@interface VROVideoCaptureDelegate ()

@property (readonly) VROVideoTexture *texture;

@end

@implementation VROVideoCaptureDelegate {
    
    id <MTLTexture> _videoTexture[3];
    
}

- (id)initWithVROVideoTexture:(VROVideoTexture *)texture {
    self = [super init];
    if (self) {
        _texture = texture;
    }
    
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
fromConnection:(AVCaptureConnection *)connection {
    
    CVMetalTextureCacheRef videoTextureCache = _texture->getVideoTextureCache();
    int currentTextureIndex = _texture->getCurrentTextureIndex();
    
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
    
    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateMetal>(new VROTextureSubstrateMetal(_videoTexture[currentTextureIndex]));
    _texture->setSubstrate(VROTextureType::Quad, std::move(substrate));
    
    CVBufferRelease(textureRef);
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
fromConnection:(AVCaptureConnection *)connection {
    
}

@end