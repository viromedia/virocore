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
class VRODriver;
class VROMaterial;
class VROVideoTextureCache;

static const long kInFlightVideoTextures = 3;

class VROVideoTexture : public VROTexture, public VROFrameListener, public std::enable_shared_from_this<VROVideoTexture> {
    
public:
    
    VROVideoTexture();
    ~VROVideoTexture();
    
    /*
     Use this video texture to display the contents of the given URL. The video
     will not run until play() is invoked.
     */
    void loadVideo(NSURL *url,
                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                   VRODriver &driver);
    
    /*
     Perform video initialization (which causes a stutter) early.
     */
    void prewarm();
    
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);
    
    void pause();
    void play();
    bool isPaused();
    
    /*
     Internal: invoked by the delegate.
     */
    void displayPixelBuffer(std::unique_ptr<VROTextureSubstrate> substrate);
    
private:
    
    /*
     AVPlayer for recorded video playback.
     */
    AVPlayer *_player;
    bool _paused;
    VROVideoPlaybackDelegate *_videoPlaybackDelegate;
    
};

/*
 Delegate for receiving video output from URLs.
 */
@interface VROVideoPlaybackDelegate : NSObject <AVPlayerItemOutputPullDelegate>

- (id)initWithVideoTexture:(VROVideoTexture *)texture
                    player:(AVPlayer *)player
                    driver:(VRODriver &)driver;

- (void)renderFrame;

@end

#endif /* VROVideoTexture_h */
