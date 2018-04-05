//
//  VROVideoTextureiOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoTextureiOS.h"
#include "VRORenderContext.h"
#include "VROFrameSynchronizer.h"
#include "VROLog.h"
#include "VROTime.h"
#include "VROAllocationTracker.h"
#include "VROVideoTextureCache.h"
#include "VRODriver.h"
#include "VROTextureSubstrate.h"
#include "VROVideoDelegateiOS.h"

# define ONE_FRAME_DURATION 0.03

static NSString *const kStatusKey = @"status";
static NSString *const kPlaybackKeepUpKey = @"playbackLikelyToKeepUp";

VROVideoTextureiOS::VROVideoTextureiOS(VROStereoMode stereoMode) :
    VROVideoTexture(VROTextureType::Texture2D, stereoMode),
    _paused(true) {
    
    ALLOCATION_TRACKER_ADD(VideoTextures, 1);
}

VROVideoTextureiOS::~VROVideoTextureiOS() {
    // Remove observers from the player's item when this is deallocated
    if (_avPlayerDelegate) {
        [_player.currentItem removeObserver:_avPlayerDelegate forKeyPath:kStatusKey context:this];
        [_player.currentItem removeObserver:_avPlayerDelegate forKeyPath:kPlaybackKeepUpKey context:this];
    }
    [_player.currentItem removeObserver:_videoNotificationListener forKeyPath:kStatusKey context:this];
    
    ALLOCATION_TRACKER_SUB(VideoTextures, 1);
}

void VROVideoTextureiOS::deleteGL() {    
    // Remove observers from the player's item when this is deallocated
    if (_avPlayerDelegate) {
        [_player.currentItem removeObserver:_avPlayerDelegate forKeyPath:kStatusKey context:this];
        [_player.currentItem removeObserver:_avPlayerDelegate forKeyPath:kPlaybackKeepUpKey context:this];
    }
    
    // Deletes the CVOpenGLTextureCache
    _avPlayerDelegate = nil;
}

#pragma mark - Recorded Video Playback

void VROVideoTextureiOS::prewarm() {
    [_player play];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
      if (_paused || _player.status != AVPlayerStatusReadyToPlay || _player.currentItem.status != AVPlayerItemStatusReadyToPlay) {
        [_player pause];
      }
    });
}

void VROVideoTextureiOS::play() {
    _paused = false;
    [_player play];
}

void VROVideoTextureiOS::pause() {
    [_player pause];
    _paused = true;
}

void VROVideoTextureiOS::seekToTime(float seconds) {
    seconds = clamp(seconds, 0, getVideoDurationInSeconds());
    // need to use toleranceBefore/After or we lose some precision, ie. 2.5 seconds become 0 seconds for a 147s video.
    [_player seekToTime:CMTimeMakeWithSeconds(seconds, 1000) toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

float VROVideoTextureiOS::getCurrentTimeInSeconds() {
    AVPlayerItem *currentItem = _player.currentItem;
    return CMTimeGetSeconds(currentItem.currentTime);
}

float VROVideoTextureiOS::getVideoDurationInSeconds(){
    AVPlayerItem *currentItem = _player.currentItem;
    return CMTimeGetSeconds(currentItem.duration);
}

bool VROVideoTextureiOS::isPaused() {
    return _paused;
}

void VROVideoTextureiOS::setMuted(bool muted) {
    _player.muted = muted;
}

void VROVideoTextureiOS::setVolume(float volume) {
    _player.volume = volume;
}

void VROVideoTextureiOS::setLoop(bool loop) {
    _loop = loop;
    if (_videoNotificationListener) {
        [_videoNotificationListener shouldLoop:loop];
    }
}

void VROVideoTextureiOS::setDelegate(std::shared_ptr<VROVideoDelegateInternal> delegate) {
    VROVideoTexture::setDelegate(delegate);
    if (_videoNotificationListener) {
        [_videoNotificationListener setDelegate:delegate];
    }
}

void VROVideoTextureiOS::playerWillBuffer() {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->videoWillBuffer();
    }
}

void VROVideoTextureiOS::playerDidBuffer() {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->videoDidBuffer();
    }
}

void VROVideoTextureiOS::loadVideo(std::string url,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {
    
    frameSynchronizer->removeFrameListener(std::dynamic_pointer_cast<VROVideoTexture>(shared_from_this()));
    frameSynchronizer->addFrameListener(std::dynamic_pointer_cast<VROVideoTexture>(shared_from_this()));
    
    _player = [AVPlayer playerWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]];
    _avPlayerDelegate = [[VROAVPlayerDelegate alloc] initWithVideoTexture:this
                                                                   player:_player
                                                                   driver:driver];
    
    [_player.currentItem addObserver:_avPlayerDelegate
                          forKeyPath:kStatusKey
                             options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                             context:this];
    
    [_player.currentItem addObserver:_avPlayerDelegate
                          forKeyPath:kPlaybackKeepUpKey
                             options:NSKeyValueObservingOptionNew
                             context:this];
    
    _videoNotificationListener = [[VROVideoNotificationListener alloc] initWithVideoPlayer:_player
                                                                                      loop:_loop
                                                                             videoDelegate:_delegate.lock()];
    
    [_player.currentItem addObserver:_videoNotificationListener
                          forKeyPath:kStatusKey
                             options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
                             context:this];
}

void VROVideoTextureiOS::onFrameWillRender(const VRORenderContext &context) {
    [_avPlayerDelegate renderFrame];
    VROVideoTexture::updateVideoTime();
}

void VROVideoTextureiOS::onFrameDidRender(const VRORenderContext &context) {
    
}

void VROVideoTextureiOS::displayPixelBuffer(std::unique_ptr<VROTextureSubstrate> substrate) {
    setSubstrate(0, std::move(substrate));
}

#pragma mark - AVPlayer Video Playback Delegate

@interface VROAVPlayerDelegate () {
    
    dispatch_queue_t _videoQueue;
    int _currentTextureIndex;
    std::weak_ptr<VRODriver> _driver;
    std::shared_ptr<VROVideoTextureCache> _videoTextureCache;
    
}

@property (readonly) VROVideoTextureiOS *texture;
@property (readonly) AVPlayer *player;
@property (readwrite) AVPlayerItemVideoOutput *output;
@property (readwrite) BOOL mediaReady;
@property (readwrite) BOOL playerReady;
@property (readwrite) BOOL buffering;

@property (readwrite) id notificationToken;

@end

@implementation VROAVPlayerDelegate

- (id)initWithVideoTexture:(VROVideoTextureiOS *)texture
                    player:(AVPlayer *)player
                    driver:(std::shared_ptr<VRODriver>)driver {
    
    self = [super init];
    if (self) {
        _texture = texture;
        _player = player;
        _driver = driver;
        _currentTextureIndex = 0;
        _mediaReady = NO;
        _playerReady = NO;
        _buffering = NO;
        
        _videoTextureCache = driver->newVideoTextureCache();
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
            [path isEqualToString:kStatusKey] &&
            self.player.status == AVPlayerStatusReadyToPlay &&
            self.player.currentItem.status == AVPlayerItemStatusReadyToPlay) {
            self.playerReady = YES;

            // It's unclear what thread we receive this notification on,
            // so return to main
            dispatch_async(dispatch_get_main_queue(), ^{
                [self attachVideoOutput];
            });

        }
        else if ([path isEqualToString:kPlaybackKeepUpKey]) {
            if ([object isKindOfClass:[AVPlayerItem class]]) {
                AVPlayerItem *item = object;
                // call play if playback is likely to keep up and video isn't paused.
                if (item.playbackLikelyToKeepUp && !self.texture->isPaused()) {
                    [self.player play];
                }
                if (_buffering == !item.playbackLikelyToKeepUp) {
                    // If the state didn't change, then do nothing!
                    return;
                }
                _buffering = !item.playbackLikelyToKeepUp;
                if (item.playbackLikelyToKeepUp) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        _texture->playerDidBuffer();
                    });
                } else {
                    [self.player pause];
                    dispatch_async(dispatch_get_main_queue(), ^{
                        _texture->playerWillBuffer();
                    });
                }
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
        
        std::shared_ptr<VRODriver> driver = _driver.lock();
        if (pixelBuffer && driver) {
            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            self.texture->displayPixelBuffer(_videoTextureCache->createTextureSubstrate(pixelBuffer,
                                                                                        driver->getColorRenderingMode() != VROColorRenderingMode::NonLinear));
            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            CFRelease(pixelBuffer);
        }
    }
}

@end

#pragma mark - Video Notification Listener

@interface VROVideoNotificationListener ()

@property (nonatomic, weak, readonly) AVPlayer *player;
@property (nonatomic, assign) BOOL loop;

@end

@implementation VROVideoNotificationListener {
    std::weak_ptr<VROVideoDelegateInternal> _delegate;
}

- (id)initWithVideoPlayer:(AVPlayer *)player loop:(BOOL)loop
            videoDelegate:(std::shared_ptr<VROVideoDelegateInternal>)videoDelegate {
    self = [super init];
    if (self) {
        _player = player;
        _loop = loop;
        _delegate = videoDelegate;
        [self registerForPlayerFinish];
    }
    return self;
}

- (void)shouldLoop:(BOOL)loop {
    _loop = loop;
    _player.actionAtItemEnd = loop ? AVPlayerActionAtItemEndNone : AVPlayerActionAtItemEndPause;
}

- (void)setDelegate:(std::shared_ptr<VROVideoDelegateInternal>)videoDelegate {
    _delegate = videoDelegate;
    [self checkForErrorAndNotifyDelegate];
}

- (void)registerForPlayerFinish {
    if (self.player) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(playerDidFinish:)
                                                     name:AVPlayerItemDidPlayToEndTimeNotification
                                                   object:[self.player currentItem]];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(playerDidFail:)
                                                     name:AVPlayerItemFailedToPlayToEndTimeNotification
                                                   object:[self.player currentItem]];
        
    }
}

- (void)dealloc {
    // make sure we stop observing when we're dealloc'd
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)playerDidFinish:(NSNotification *)notification {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    
    // when a video finishes, either loop or let the delegate know that we're done playing.
    if (self.loop) {
        AVPlayerItem *playerItem = [notification object];
        [playerItem seekToTime:kCMTimeZero];
    }
    
    if (delegate) {
        delegate->videoDidFinish();
    }
}

- (void)playerDidFail:(NSNotification *)notification {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    
    NSError *error = [notification.userInfo objectForKey:AVPlayerItemFailedToPlayToEndTimeErrorKey];
    if (delegate) {
        delegate->videoDidFail(std::string([error.localizedDescription UTF8String]));
    }
}

- (void)observeValueForKeyPath:(NSString *)path ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if ([path isEqualToString:kStatusKey]) {
        [self checkForErrorAndNotifyDelegate];
    }
}

- (void)checkForErrorAndNotifyDelegate {
    if (self.player.currentItem.status == AVPlayerItemStatusFailed) {
        std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
        
        NSError *error = self.player.currentItem.error;
        if (delegate && error) {
            delegate->videoDidFail(std::string([error.localizedDescription UTF8String]));
        }
    }
}

@end
