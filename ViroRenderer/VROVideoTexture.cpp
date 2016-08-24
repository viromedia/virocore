//
//  VROVideoTexture.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/8/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoTexture.h"
#include "VRORenderContext.h"
#include "VROFrameSynchronizer.h"
#include "VRODriverMetal.h"
#include "VROLog.h"
#include "VROTextureSubstrateMetal.h"
#include "VROTime.h"
#include "VROAllocationTracker.h"
#include "VROVideoTextureCache.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

# define ONE_FRAME_DURATION 0.03

VROVideoTexture::VROVideoTexture() :
    _paused(true) {
    
    ALLOCATION_TRACKER_ADD(VideoTextures, 1);
}

VROVideoTexture::~VROVideoTexture() {
    ALLOCATION_TRACKER_SUB(VideoTextures, 1);
}

#pragma mark - Recorded Video Playback

void VROVideoTexture::prewarm() {
    [_player play];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [_player pause];
    });
}

void VROVideoTexture::play() {
    _paused = false;
    [_player play];
}

void VROVideoTexture::pause() {
    [_player pause];
    _paused = true;
}

bool VROVideoTexture::isPaused() {
    return _paused;
}

void VROVideoTexture::loadVideo(NSURL *url,
                                std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                VRODriver &driver) {
    
    frameSynchronizer->addFrameListener(shared_from_this());
    
    _player = [AVPlayer playerWithURL:url];
    _videoPlaybackDelegate = [[VROVideoPlaybackDelegate alloc] initWithVideoTexture:this
                                                                             player:_player
                                                                             driver:driver];
    
    [_player.currentItem addObserver:_videoPlaybackDelegate
           forKeyPath:@"status"
              options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
              context:this];
    
    [_player.currentItem addObserver:_videoPlaybackDelegate
                          forKeyPath:@"playbackLikelyToKeepUp"
                             options:NSKeyValueObservingOptionNew
                             context:this];
}

void VROVideoTexture::onFrameWillRender(const VRORenderContext &context) {
    [_videoPlaybackDelegate renderFrame];
}

void VROVideoTexture::onFrameDidRender(const VRORenderContext &context) {
   
}

void VROVideoTexture::displayPixelBuffer(std::unique_ptr<VROTextureSubstrate> substrate) {
    setSubstrate(VROTextureType::Quad, std::move(substrate));
}

#pragma mark - Video Playback Delegate

@interface VROVideoPlaybackDelegate () {
    
    dispatch_queue_t _videoQueue;
    int _currentTextureIndex;
    VROVideoTextureCache *_videoTextureCache;

}

@property (readonly) VROVideoTexture *texture;
@property (readonly) AVPlayer *player;

@property (readwrite) AVPlayerItemVideoOutput *output;
@property (readwrite) BOOL mediaReady;
@property (readwrite) BOOL playerReady;

@property (readwrite) id notificationToken;

@end

@implementation VROVideoPlaybackDelegate

- (id)initWithVideoTexture:(VROVideoTexture *)texture
                    player:(AVPlayer *)player
                    driver:(VRODriver &)driver {
                        
    self = [super init];
    if (self) {
        _texture = texture;
        _player = player;
        
        _currentTextureIndex = 0;
        _mediaReady = false;
        _playerReady = false;
        
        _videoTextureCache = driver.newVideoTextureCache();
        _videoQueue = dispatch_queue_create("video_output_queue", DISPATCH_QUEUE_SERIAL);
    }
    
    return self;
}

- (void)outputMediaDataWillChange:(AVPlayerItemOutput *)sender {
    _mediaReady = true;
}

- (void)observeValueForKeyPath:(NSString *)path ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if (context == _texture) {
        
        if (!self.playerReady &&
            [path isEqualToString:@"status"] &&
            self.player.status == AVPlayerStatusReadyToPlay &&
            self.player.currentItem.status == AVPlayerItemStatusReadyToPlay) {
            
            self.playerReady = true;
            
            // It's unclear what thread we receive this notification on,
            // so return to main
            dispatch_async(dispatch_get_main_queue(), ^{
                [self attachVideoOutput];
            });
        }
        else if ([path isEqualToString:@"playbackLikelyToKeepUp"]) {
            if (!self.texture->isPaused()) {
                [self.player play];
            }
        }
    }
    else {
        [super observeValueForKeyPath:path ofObject:object change:change context:context];
    }
}

- (void)attachVideoOutput {
    NSDictionary *pixBuffAttributes = @{(id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)};
    self.output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixBuffAttributes];
    [self addLoopNotification:self.player.currentItem];

    [self.output setDelegate:self queue:_videoQueue];
    [self.output requestNotificationOfMediaDataChangeWithAdvanceInterval:ONE_FRAME_DURATION];
    
    [self.player.currentItem addOutput:self.output];
}

- (void)renderFrame {
    /*
     Stuttering is significantly reduced by placing this code in willRender() as opposed
     to didRender(). Reason unknown: contention of resources somewhere?
     */
    if (!_mediaReady) {
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
    CMTime outputItemTime = [_output itemTimeForHostTime:nextVSync];
    
    if ([_output hasNewPixelBufferForItemTime:outputItemTime]) {
        CMTime presentationTime = kCMTimeZero;
        CVPixelBufferRef pixelBuffer = [_output copyPixelBufferForItemTime:outputItemTime
                                                             itemTimeForDisplay:&presentationTime];
        
        if (pixelBuffer != nullptr) {
            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            
            self.texture->displayPixelBuffer(std::move(_videoTextureCache->createTextureSubstrate(pixelBuffer)));
            
            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            CFRelease(pixelBuffer);
        }
    }
}

-(void) addLoopNotification:(AVPlayerItem *)item {
    if (_notificationToken) {
        _notificationToken = nil;
    }
    
    /*
     Setting actionAtItemEnd to None prevents the movie from getting paused at item end. 
     A very simplistic, and not gapless, looped playback.
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

@end