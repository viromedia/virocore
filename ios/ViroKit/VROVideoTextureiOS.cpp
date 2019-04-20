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
#include "VROCameraTexture.h"

# define ONE_FRAME_DURATION 0.03

static NSString *const kStatusKey = @"status";
static NSString *const kPlaybackKeepUpKey = @"playbackLikelyToKeepUp";

VROVideoTextureiOS::VROVideoTextureiOS(VROStereoMode stereoMode,
                                       bool enableCMSampleBuffer) :
    VROVideoTexture(VROTextureType::Texture2D, stereoMode),
    _paused(true),
    _isCMSampleBuffered(enableCMSampleBuffer) {
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

VROVector3f VROVideoTextureiOS::getVideoDimensions() {
    if (_player && _player.error == nil) {
        // First, try getting the dimensions from the current player item.
        AVPlayerItem *currentItem = [_player currentItem] ;
        if (currentItem != NULL && currentItem.videoComposition != NULL) {
            CGSize naturalSize = currentItem.videoComposition.renderSize;
            float width = (float) naturalSize.width;
            float height = (float) naturalSize.height;
            return VROVector3f(width, height, 0);
        }

        // Else grab the dimensions from the first track's preferred transform.
        NSArray<AVAssetTrack *> * tracks
                = [_player.currentItem.asset tracksWithMediaType:AVMediaTypeVideo];
        if (tracks.count == 0) {
            return VROVector3f(0,0,0);
        }

        AVAssetTrack *track = [tracks firstObject];
        if (track != nil) {
            CGSize naturalSize = [track naturalSize];
            naturalSize = CGSizeApplyAffineTransform(naturalSize, track.preferredTransform);
            float width = (float) naturalSize.width;
            float height = (float) naturalSize.height;
            return VROVector3f(abs(width), abs(height), 0);
        }
    }

    return VROVector3f(0,0,0);
}

void VROVideoTextureiOS::initVideoDimensions() {
    // First, ensure we have a valid track to create dimensions from
    NSArray<AVAssetTrack *> * tracks = [_player.currentItem.asset
                                        tracksWithMediaType:AVMediaTypeVideo];
    
    NSArray<AVAssetTrack *> * audioTracks = [_player.currentItem.asset
                                        tracksWithMediaType:AVMediaTypeAudio];
    if (tracks.count == 0) {
        return;
    }

    // Step 1: Create our mainComposition containing the original video feed.
    AVAssetTrack *videoTrack = [tracks firstObject];
    CMTime trackDuration = [[videoTrack asset] duration];
    CMTime insertionPoint = kCMTimeZero;
    NSError *error = nil;
    AVMutableComposition *mainComposition = [AVMutableComposition composition];
    AVMutableCompositionTrack *compositionVideoTrack = [mainComposition
                                                        addMutableTrackWithMediaType:AVMediaTypeVideo
                                                        preferredTrackID:kCMPersistentTrackID_Invalid];
    
    [compositionVideoTrack insertTimeRange:CMTimeRangeMake(kCMTimeZero, trackDuration)
                                   ofTrack:videoTrack atTime:insertionPoint error:&error];
    

    if ( audioTracks.count != 0) {
        AVAssetTrack *audioTrack = [audioTracks firstObject];
        CMTime audiotrackDuration = [[audioTrack asset] duration];
        AVMutableCompositionTrack *compositionAudioTrack = [mainComposition addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:kCMPersistentTrackID_Invalid];
        [compositionAudioTrack insertTimeRange:CMTimeRangeMake(kCMTimeZero,audiotrackDuration) ofTrack:audioTrack atTime:insertionPoint error:nil];
    }
    
    if (error) {
        return;
    }

    // Step 2: Calculate position and size of rendered video after rotating
    CGAffineTransform t1;
    CGAffineTransform t2;
    CGAffineTransform transform = [videoTrack preferredTransform];
    double radians = atan2(transform.b,transform.a);
    float degrees = radians_to_degrees(radians);
    float width = abs(videoTrack.naturalSize.width);
    float height = abs(videoTrack.naturalSize.height);
    float toDiagonal = sqrt(width*width+height*height);
    float toDiagonalAngle = radians_to_degrees(acosf(width / toDiagonal));
    float toDiagonalAngle2 = 90 - radians_to_degrees(acosf(width / toDiagonal));
    float toDiagonalAngleComple;
    float toDiagonalAngleComple2;
    float finalHeight;
    float finalWidth;

    if (degrees >= 0 && degrees <= 90) {
        toDiagonalAngleComple = toDiagonalAngle + degrees;
        toDiagonalAngleComple2 = toDiagonalAngle2 + degrees;
        finalHeight = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple)));
        finalWidth = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple2)));
        t1 = CGAffineTransformMakeTranslation(height * sinf(degrees_to_radians(degrees)), 0.0);
    } else if (degrees > 90 && degrees <= 180) {
        float degrees2 = degrees - 90;
        toDiagonalAngleComple = toDiagonalAngle + degrees2;
        toDiagonalAngleComple2 = toDiagonalAngle2 + degrees2;
        finalHeight = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple2)));
        finalWidth = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple)));
        t1 = CGAffineTransformMakeTranslation(width * sinf(degrees_to_radians(degrees2))+ height * cosf(degrees_to_radians(degrees2)), height * sinf(degrees_to_radians(degrees2)));
    } else if (degrees >= -90 && degrees < 0) {
        float degrees2 = degrees - 90;
        float degreesabs = abs(degrees);
        toDiagonalAngleComple = toDiagonalAngle+degrees2;
        toDiagonalAngleComple2 = toDiagonalAngle2+degrees2;
        finalHeight = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple2)));
        finalWidth = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple)));
        t1 = CGAffineTransformMakeTranslation(0, width * sinf(degrees_to_radians(degreesabs)));
    } else if (degrees >= -180 && degrees < -90) {
        float degreesabs = abs(degrees);
        float degreesplus = degreesabs - 90;
        toDiagonalAngleComple = toDiagonalAngle + degrees;
        toDiagonalAngleComple2 = toDiagonalAngle2 + degrees;
        finalHeight = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple)));
        finalWidth = abs(toDiagonal * sinf(degrees_to_radians(toDiagonalAngleComple2)));
        t1 = CGAffineTransformMakeTranslation(width * sinf(degrees_to_radians(degreesplus)), height * sinf(degrees_to_radians(degreesplus)) + width * cosf(degrees_to_radians(degreesplus)));
    } else {
        return;
    }
    t2 = CGAffineTransformRotate(t1, degrees_to_radians(degrees));

    // Step 3: Now make the video composition that will contain our rotation
    AVMutableVideoComposition *videoComposition = [AVMutableVideoComposition videoComposition];
    videoComposition.renderSize = CGSizeMake(finalWidth,finalHeight);
    videoComposition.frameDuration = CMTimeMake(1, 30);

    // Step 4: Create the rotation instructions
    AVMutableVideoCompositionInstruction *instructions = [AVMutableVideoCompositionInstruction videoCompositionInstruction];
    instructions.timeRange = CMTimeRangeMake(kCMTimeZero, trackDuration);
    AVMutableVideoCompositionLayerInstruction *layerInstruction = [AVMutableVideoCompositionLayerInstruction videoCompositionLayerInstructionWithAssetTrack:[mainComposition.tracks objectAtIndex:0]];
    [layerInstruction setTransform:t2 atTime:kCMTimeZero];
    instructions.layerInstructions = [NSArray arrayWithObject:layerInstruction];
    videoComposition.instructions = [NSArray arrayWithObject:instructions];

    // Step 5: Finally create the Player item and set it the videoComposition.
    AVPlayerItem *playerItem = [[AVPlayerItem alloc] initWithAsset:mainComposition];
    playerItem.videoComposition = videoComposition;

    // Set the player item with the rotated source.
    [_player replaceCurrentItemWithPlayerItem:playerItem];
    [_player seekToTime:kCMTimeZero toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
}

void VROVideoTextureiOS::loadVideo(std::string url,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {
    // If we no VROFrameSynchronizer is given, or _isCMSampleBuffered, do not attach provided synchronizer
    if (frameSynchronizer != nullptr && !_isCMSampleBuffered) {
        frameSynchronizer->removeFrameListener(std::dynamic_pointer_cast<VROVideoTexture>(shared_from_this()));
        frameSynchronizer->addFrameListener(std::dynamic_pointer_cast<VROVideoTexture>(shared_from_this()));
    }
    
    _player = [AVPlayer playerWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]];
    initVideoDimensions();
    
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

void VROVideoTextureiOS::updateFrame() {
    [_avPlayerDelegate renderFrame];
    VROVideoTexture::updateVideoTime();
}

void VROVideoTextureiOS::onFrameWillRender(const VRORenderContext &context) {
    updateFrame();
}

void VROVideoTextureiOS::onFrameDidRender(const VRORenderContext &context) {
    
}

void VROVideoTextureiOS::displayPixelBuffer(std::unique_ptr<VROTextureSubstrate> substrate) {
    setSubstrate(0, std::move(substrate));
}

CMSampleBufferRef VROVideoTextureiOS::getSampleBuffer() const {
    return [_avPlayerDelegate getSampleBuffer];
}

#pragma mark - AVPlayer Video Playback Delegate

@interface VROAVPlayerDelegate () {
    
    dispatch_queue_t _videoQueue;
    int _currentTextureIndex;
    std::weak_ptr<VRODriver> _driver;
    std::shared_ptr<VROVideoTextureCache> _videoTextureCache;

    /*
     The last received CMSampleBufferRef, which represents the current contents of the
     rendered video texture.
     */
    CMSampleBufferRef _lastSampleBuffer;
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

-(void)dealloc {
    if (_lastSampleBuffer) {
        CFRelease(_lastSampleBuffer);
        _lastSampleBuffer = nullptr;
    }
}

-(CMSampleBufferRef)getSampleBuffer {
    return _lastSampleBuffer;
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
            // If processing CMSampleBuffer data, save the last known video
            // frame into the _lastSampleBuffer if possible.
            if (self.texture->isCMSampleBufferEnabled()) {
                if (_lastSampleBuffer) {
                    CFRelease(_lastSampleBuffer);
                }

                CMVideoFormatDescriptionRef formatDescription;
                bool hasFormatDescription
                    = CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault,
                                                                   pixelBuffer,
                                                                   &formatDescription) == noErr;
                if (hasFormatDescription) {
                    CMSampleBufferCreateReadyWithImageBuffer(kCFAllocatorDefault,
                                                             pixelBuffer,
                                                             formatDescription,
                                                             &kCMTimingInfoInvalid,
                                                             &_lastSampleBuffer);
                    CFRelease(formatDescription);
                }
            }

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
