//
//  VROVideoTexture.h
//  ViroRenderer
//
//  Created by Raj Advani on 1/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROVideoTexture_h
#define VROVideoTexture_h

#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CVMetalTextureCache.h>
#import "VROFrameListener.h"
#import "VROTexture.h"
#import <memory>

@class VROVideoCaptureDelegate;
@class VROVideoPlaybackDelegate;
class VRORenderContext;
class VROFrameSynchronizer;
class VRODriverContext;
class VROMaterial;

static const long kInFlightVideoTextures = 3;

class VROVideoTexture : public VROTexture, public VROFrameListener, public std::enable_shared_from_this<VROVideoTexture> {
    
public:
    
    VROVideoTexture();
    ~VROVideoTexture();
    
    /*
     Use this video texture to display the contents of the front-facing camera.
     */
    void displayCamera(AVCaptureDevicePosition position,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       const VRODriverContext &driverContext);
    
    /*
     Use this video texture to display the contents of the given URL. The video
     will not run until play() is invoked.
     */
    void loadVideo(NSURL *url,
                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                   const VRODriverContext &driverContext);
    
    /*
     Perform video initialization (which causes a stutter) early.
     */
    void prewarm();
    
    int getCurrentTextureIndex() const {
        return _currentTextureIndex;
    }
    CVMetalTextureCacheRef getVideoTextureCache() {
        return _videoTextureCache;
    }
    
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);
    
    void setMediaReady(bool mediaReady) {
        _mediaReady = mediaReady;
    }
    
    void pause();
    void play();
    bool isPaused();
    
private:
    
    /*
     Capture session and delegate used for live video playback.
     */
    AVCaptureSession *_captureSession;
    VROVideoCaptureDelegate *_videoDelegate;
    
    /*
     AVPlayer for recorded video playback.
     */
    AVPlayer *_player;
    AVPlayerItemVideoOutput *_videoOutput;
    float _preferredRotation;
    id _notificationToken;
    bool _mediaReady;
    bool _paused;
    dispatch_queue_t _videoQueue;
    VROVideoPlaybackDelegate *_videoPlaybackDelegate;
    
    /*
     Video texture cache used for both live and recorded playback.
     */
    int _currentTextureIndex;
    CVMetalTextureCacheRef _videoTextureCache;
    
    void addLoopNotification(AVPlayerItem *item);
    void displayPixelBuffer(CVPixelBufferRef pixelBuffer);
    
};

/*
 Delegate for capturing video from cameras.
 */
@interface VROVideoCaptureDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

- (id)initWithVROVideoTexture:(VROVideoTexture *)texture;

@end

/*
 Delegate for receiving video output from URLs.
 */
@interface VROVideoPlaybackDelegate : NSObject <AVPlayerItemOutputPullDelegate>

- (id)initWithVROVideoTexture:(VROVideoTexture *)texture;

@end

#endif /* VROVideoTexture_h */
