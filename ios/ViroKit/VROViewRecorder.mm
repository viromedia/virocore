//
//  VROViewRecorder.m
//  ViroKit
//
//  Created by Raj Advani on 11/28/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROViewRecorder.h"
#import <memory>
#import "VROVideoTextureCache.h"
#import "VROTime.h"
#import "VRODriver.h"
#import "VRORenderer.h"
#import "VROTexture.h"
#import "VROTextureSubstrate.h"
#import "VROChoreographer.h"
#import "VRORenderTarget.h"

@interface VROViewRecorder () {
    
    VROViewRecordingErrorBlock _errorBlock;
    bool _isRecording;
    bool _isFirstRecordingFrame;
    bool _saveToCameraRoll;
    bool _addWatermark;
    UIImage *_watermarkImage;
    CGRect _watermarkFrame;
    CIImage *_resizedWatermarkImage;
    CVPixelBufferRef _compositePixelBuffer; // used for watermarking
    NSString *_videoFileName;
    NSURL *_tempVideoFilePath;
    AVAssetWriter *_videoWriter;
    AVAssetWriterInput *_videoWriterInput;
    AVAssetWriterInputPixelBufferAdaptor *_videoWriterPixelBufferAdaptor;
    AVAudioRecorder *_audioRecorder;
    double _startTimeMillis;
    NSURL *_audioFilePath;
    NSTimer *_videoLoopTimer;
    CVPixelBufferRef _videoPixelBuffer;
    std::shared_ptr<VROVideoTextureCache> _videoTextureCache;
    std::shared_ptr<VRODriver> _driver;
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROViewRecorderRTTDelegate> _rttDelegate;
    __weak GLKView *_view;
}

@end

@implementation VROViewRecorder

- (id)initWithView:(GLKView *)view
          renderer:(std::shared_ptr<VRORenderer>)renderer
            driver:(std::shared_ptr<VRODriver>)driver {
    self = [super init];
    if (self) {
        _view = view;
        _renderer = renderer;
        _driver = driver;
        _addWatermark = false;
    }
    return self;
}

- (void)deleteGL {
    if (_videoTextureCache) {
        _videoTextureCache.reset();
    }
}

#pragma mark - Recording and Screen Capture
// TODO: not a huge fan of the current implementation because
//   1) code will attempt to ask for permission before recording, "delaying" the actual recording
//   2) developer doesn't get any status updates other than complete failures (no delay notification)
//   3) developer doesn't get any "time" information.
// On the flip side, if the developer requests permissions FIRST, then recording starts immediately,
// so the developer can track recording time themselves and the only status will be "running", "error" and "done"
- (void)startVideoRecording:(NSString *)fileName
           saveToCameraRoll:(BOOL)saveToCamera
                 errorBlock:(VROViewRecordingErrorBlock)errorBlock {
    
    if (_isRecording) {
        NSLog(@"[Recording] Video is already being recorded, aborting...");
        if (errorBlock) {
            errorBlock(kVROViewErrorAlreadyRunning);
        }
        return;
    }
    
    // We MUST first check if a recording session is ongoing BEFORE we override state variables.
    _errorBlock = errorBlock;
    _saveToCameraRoll = saveToCamera;
    
    // Note, we don't need to ask for camera permissions here because
    //  1) we're grabbing image from the renderer
    //  2) in AR, the renderer accesses the camera and so it should ask for permission
    
    switch([[AVAudioSession sharedInstance] recordPermission]) {
            // continue executing if we have permissions
        case AVAudioSessionRecordPermissionGranted:
            NSLog(@"[Recording] Microphone permission granted.");
            break;
            // notify permission error and exit if we don't have permission
        case AVAudioSessionRecordPermissionDenied:
            NSLog(@"[Recording] Microphone permission denied.");
            if (_errorBlock) {
                _errorBlock(kVROViewErrorNoPermissions);
            }
            return;
            // if we dont have permissions, then attempt to get it
        case AVAudioSessionRecordPermissionUndetermined:
            NSLog(@"[Recording] Microphone permission undetermined.");
            [[AVAudioSession sharedInstance] requestRecordPermission:^(BOOL granted) {
                // just call this function again because we'll check for permission state again.
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self startVideoRecording:fileName saveToCameraRoll:saveToCamera errorBlock:_errorBlock];
                });
            }];
            return;
    }
    
    // If we're saving this file to camera roll, then also check for that permission before we start
    if (saveToCamera) {
        switch ([PHPhotoLibrary authorizationStatus]) {
            case PHAuthorizationStatusAuthorized:
                break;
            case PHAuthorizationStatusDenied:
            case PHAuthorizationStatusRestricted:
                if (_errorBlock) {
                    NSLog(@"[Recording] Photo library permission denied.");
                    _errorBlock(kVROViewErrorNoPermissions);
                }
                return;
            case PHAuthorizationStatusNotDetermined:
                [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        [self startVideoRecording:fileName saveToCameraRoll:saveToCamera errorBlock:_errorBlock];
                    });
                }];
                return;
        }
    }
    
    _videoFileName = fileName;
    _tempVideoFilePath = [self checkAndGetTempFileURL:[fileName stringByAppendingString:kVROViewTempVideoSuffix]];
    
    _audioFilePath = [self startAudioRecordingInternal:fileName];
    [self startVideoRecordingInternal:_tempVideoFilePath];
}

- (void)startVideoRecording:(NSString *)fileName
                  withWatermark:(UIImage *)watermarkImage
                  withFrame:(CGRect)watermarkFrame
           saveToCameraRoll:(BOOL)saveToCamera
                 errorBlock:(VROViewRecordingErrorBlock)errorBlock {
    _addWatermark = true;
    _watermarkImage = watermarkImage;
    _watermarkFrame = watermarkFrame;

    [self startVideoRecording:fileName saveToCameraRoll:saveToCamera errorBlock:errorBlock];
}

- (void)stopVideoRecordingWithHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    if (!_isRecording) {
        completionHandler(NO, nil, kVROViewErrorAlreadyStopped);
        return;
    }
    // this block will be called once the video writer in stopRecordingV1 finishes writing the vid
    VROViewWriteMediaFinishBlock wrappedCompleteHandler = ^(BOOL success, NSURL *filepath, NSInteger errorCode) {
        NSURL *videoURL = [self checkAndGetTempFileURL:[_videoFileName stringByAppendingString:kVROViewVideoSuffix]];
        
        // once the video finishes writing, then we need to merge the video w/ audio
        [self mergeAudio:_audioFilePath withVideo:filepath outputPath:videoURL completionHandler:^(BOOL success) {
            
            // delete the temp audio/video files.
            NSFileManager *fileManager = [NSFileManager defaultManager];
            [fileManager removeItemAtPath:[_tempVideoFilePath path] error:nil];
            [fileManager removeItemAtPath:[_audioFilePath path] error:nil];
            
            if (success) {
                if (_saveToCameraRoll) {
                    [self writeMediaToCameraRoll:videoURL isPhoto:NO withCompletionHandler:completionHandler];
                } else {
                    completionHandler(YES, videoURL, kVROViewErrorNone);
                }
            } else {
                completionHandler(NO, nil, kVROViewErrorUnknown);
            }
            _saveToCameraRoll = NO;
            _isRecording = NO;
            // we're done with watermarking!
            _addWatermark = NO;
        }];
    };
    [self stopAudioRecordingInternal];
    [self stopVideoRecordingInternal:wrappedCompleteHandler];
}

- (void)stopVideoRecordingWithError:(NSInteger)errorCode {
    if (_errorBlock) {
        _errorBlock(errorCode);
    }
    
    // stop all recording if error
    [self stopAudioRecordingInternal];
    [self stopVideoRecordingInternal:nil];
    
    // clean up the temp files
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (_tempVideoFilePath) [fileManager removeItemAtPath:[_tempVideoFilePath path] error:nil];
    if (_audioFilePath) [fileManager removeItemAtPath:[_audioFilePath path] error:nil];
}

- (void)takeScreenshot:(NSString *)fileName saveToCameraRoll:(BOOL)saveToCamera
 withCompletionHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    
    GLKView *view = _view;
    if (!view) {
        return;
    }
    
    // if we're saving this file to camera roll, then also check for that permission before we start
    if (saveToCamera) {
        switch ([PHPhotoLibrary authorizationStatus]) {
            case PHAuthorizationStatusAuthorized:
                break;
            case PHAuthorizationStatusDenied:
            case PHAuthorizationStatusRestricted:
                if (completionHandler) {
                    NSLog(@"[Recording] Photo library permission denied.");
                    completionHandler(NO, nil, kVROViewErrorNoPermissions);
                }
                return;
            case PHAuthorizationStatusNotDetermined:
                [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
                    [self takeScreenshot:fileName saveToCameraRoll:saveToCamera withCompletionHandler:completionHandler];
                }];
                return;
        }
    }
    
    NSURL *filePath = [self checkAndGetTempFileURL:[fileName stringByAppendingString:kVROViewImageSuffix]];
    [UIImagePNGRepresentation(view.snapshot) writeToFile:[filePath path] atomically:YES];
    
    if (saveToCamera) {
        [self writeMediaToCameraRoll:filePath isPhoto:YES withCompletionHandler:completionHandler];
    } else {
        completionHandler(YES, filePath, kVROViewErrorNone);
    }
}

- (void)writeMediaToCameraRoll:(NSURL *)filePath
                       isPhoto:(BOOL)isPhoto
         withCompletionHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    
    PHAssetResourceType type = isPhoto ? PHAssetResourceTypePhoto : PHAssetResourceTypeVideo;
    [[PHPhotoLibrary sharedPhotoLibrary] performChanges:^{
        [[PHAssetCreationRequest creationRequestForAsset] addResourceWithType:type fileURL:filePath options:nil];
    } completionHandler:^(BOOL success, NSError *error) {
        if (success) {
            NSLog(@"[Recording] saving media worked!");
            completionHandler(YES, filePath, kVROViewErrorNone);
        } else {
            NSLog(@"[Recording] saving media failed w/ error: %@", error.localizedDescription);
            completionHandler(NO, filePath, kVROViewErrorWriteToFile);
        }
    }];
}

- (BOOL)hasPhotoLibraryPermission {
    return [PHPhotoLibrary authorizationStatus] == PHAuthorizationStatusAuthorized;
}

- (void)mergeAudio:(NSURL *)audioPath withVideo:(NSURL *)videoPath outputPath:(NSURL *)outputPath completionHandler:(void (^)(BOOL))handler {
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:[audioPath path]] || ![fileManager fileExistsAtPath:[videoPath path]]) {
        NSLog(@"[Recording] Audio/Video merge failed because file does not exist.");
        handler(NO);
        return;
    }
    AVURLAsset *audioAsset = [[AVURLAsset alloc] initWithURL:audioPath options:nil];
    AVURLAsset *videoAsset = [[AVURLAsset alloc] initWithURL:videoPath options:nil];
    
    if (![audioAsset isPlayable] || ![videoAsset isPlayable]) {
        NSLog(@"[Recording] Audio/Video merge failed because a file is not playable.");
        handler(NO);
        return;
    }
    
    AVMutableComposition *mergedComposition = [AVMutableComposition composition];
    
    AVMutableCompositionTrack *compositionAudioTrack = [mergedComposition addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:kCMPersistentTrackID_Invalid];
    [compositionAudioTrack insertTimeRange:CMTimeRangeMake(kCMTimeZero, audioAsset.duration) ofTrack:[[audioAsset tracksWithMediaType:AVMediaTypeAudio] objectAtIndex:0] atTime:kCMTimeZero error:nil];
    
    AVMutableCompositionTrack *compositionVideoTrack = [mergedComposition addMutableTrackWithMediaType:AVMediaTypeVideo preferredTrackID:kCMPersistentTrackID_Invalid];
    [compositionVideoTrack insertTimeRange:CMTimeRangeMake(kCMTimeZero, audioAsset.duration) ofTrack:[[videoAsset tracksWithMediaType:AVMediaTypeVideo] objectAtIndex:0] atTime:kCMTimeZero error:nil];
    
    AVAssetExportSession *exportSession = [[AVAssetExportSession alloc] initWithAsset:mergedComposition presetName:AVAssetExportPresetPassthrough];
    
    exportSession.outputFileType = AVFileTypeMPEG4;
    exportSession.outputURL = outputPath;
    
    [exportSession exportAsynchronouslyWithCompletionHandler:^(void) {
        // let the handler know that the merge was successful if the status is completed
        handler(exportSession.status == AVAssetExportSessionStatusCompleted);
    }];
}

/*
 This function gets the temp file url for the given filename guaranteeing that it's a valid url. The given
 filename should already be suffixed with an extension.
 */
- (NSURL *)checkAndGetTempFileURL:(NSString *)fileName {
    // first check if the temp media directory exists
    BOOL isDirectory = NO;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *filePath = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingString:kVROViewTempMediaDirectory]];
    BOOL exists = [fileManager fileExistsAtPath:[filePath path] isDirectory:&isDirectory];
    
    // if it doesn't exist or isn't a directory, then turn it into a directory.
    if (!exists || !isDirectory) {
        if (!isDirectory) {
            [fileManager removeItemAtURL:filePath error:nil];
        }
        NSError *error;
        BOOL success = [fileManager createDirectoryAtURL:filePath withIntermediateDirectories:YES attributes:nil error:&error];
        if (!success) {
            NSLog(@"[Recording] failed to create directory w/ error: %@", [error localizedDescription]);
            return nil;
        }
    }
    
    filePath = [filePath URLByAppendingPathComponent:fileName];
    if ([fileManager fileExistsAtPath:[filePath path]]) {
        [fileManager removeItemAtURL:filePath error:nil];
    }
    
    return filePath;
}

#pragma mark - Audio Recording

- (NSURL *)startAudioRecordingInternal:(NSString *)fileName {
    NSDictionary *audioRecordSettings = @{
                                          AVFormatIDKey : @(kAudioFormatMPEG4AAC),
                                          AVSampleRateKey : @(16000), // 44.1k is CD (high) quality, we don't need that high, so 16kHz should be enough.
                                          AVNumberOfChannelsKey : @(1), // only 1 because 1 mic = mono sound
                                          };
    
    NSURL *url = [self checkAndGetTempFileURL:[fileName stringByAppendingString:kVROViewAudioSuffix]];
    
    NSError *error;
    _audioRecorder = [[AVAudioRecorder alloc] initWithURL:url settings:audioRecordSettings error:&error];

    if ([_audioRecorder prepareToRecord]) {
        [_audioRecorder record];
    } else {
        NSLog(@"[Recording] preparing to record audio failed with error: %@", [error localizedDescription]);
        [self stopVideoRecordingWithError:kVROViewErrorUnknown];
    }
    return url;
}

- (void)stopAudioRecordingInternal {
    if (_audioRecorder && [_audioRecorder isRecording]) {
        [_audioRecorder stop];
    }
    _audioRecorder = nil;
}

#pragma mark - Video Recording

/*
 The following functions encapsulate all the logic to grab and record the rendered frames to a file
 at the given filepath. Audio recording is done separately and will be merged with the video later.
 */
- (void)startVideoRecordingInternal:(NSURL *)filePath {
    GLKView *view = _view;
    if (!view) {
        return;
    }
    
    NSError *error = nil;
    _videoWriter = [[AVAssetWriter alloc] initWithURL:filePath fileType:AVFileTypeMPEG4 error:&error];
    int width  = view.frame.size.width  * view.contentScaleFactor;
    int height = view.frame.size.height * view.contentScaleFactor;
    
    /*
     * https://stackoverflow.com/questions/29505631/crop-video-in-ios-see-weird-green-line-around-video
     * The video width & height need to be even
     */
    if (width % 2 != 0) {
        width = width - 1;
    }
    if (height %2 != 0) {
        height = height - 1;
    }
    NSDictionary *videoSetting = @{
                                   AVVideoCodecKey : AVVideoCodecH264,
                                   AVVideoWidthKey : @(width),
                                   AVVideoHeightKey : @(height),
                                   AVVideoCompressionPropertiesKey:
                                     @{AVVideoAverageBitRateKey : @(7000000),
                                       }
                                   };
    
    _videoWriterInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:videoSetting];
    
    NSDictionary *pixelBufferAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                           @(kCVPixelFormatType_32BGRA), kCVPixelBufferPixelFormatTypeKey,
                                           @(width), kCVPixelBufferWidthKey,
                                           @(height), kCVPixelBufferHeightKey,
                                           nil];
    _videoWriterPixelBufferAdaptor = [AVAssetWriterInputPixelBufferAdaptor
                                      assetWriterInputPixelBufferAdaptorWithAssetWriterInput:_videoWriterInput
                                      sourcePixelBufferAttributes:pixelBufferAttributes];
    [_videoWriter addInput:_videoWriterInput];
    [_videoWriter startWriting];
    [_videoWriter startSessionAtSourceTime:kCMTimeZero];
    
    CVPixelBufferPoolRef pixelBufferPool = [_videoWriterPixelBufferAdaptor pixelBufferPool];
    CVPixelBufferPoolCreatePixelBuffer(NULL, pixelBufferPool, &_videoPixelBuffer);
    
    if (!_videoTextureCache) {
        _videoTextureCache = _driver->newVideoTextureCache();
    }
    
    if (_addWatermark) {
        
        // create the _compositePixelBuffer if it hasn't already been created!
        if (!_compositePixelBuffer) {
            NSDictionary *attr = @{
                                   (NSString*)kCVPixelBufferCGImageCompatibilityKey : @YES,
                                   (NSString*)kCVPixelBufferCGBitmapContextCompatibilityKey : @YES,
                                   };

            CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, width,
                                  height, kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef) attr,
                                  &_compositePixelBuffer);
            if (status != kCVReturnSuccess) {
                NSLog(@"VROViewRecorder - failed to create compositePixelBuffer for watermarking");
            }
        }

        // create a UIImage with size divided by the contentScaleFactor!
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(width/view.contentScaleFactor, height/view.contentScaleFactor), NO, 0.0);
        [_watermarkImage drawInRect:CGRectMake(_watermarkFrame.origin.x, _watermarkFrame.origin.y , _watermarkFrame.size.width, _watermarkFrame.size.height)];
        UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
        _resizedWatermarkImage = [CIImage imageWithCGImage:newImage.CGImage];
    }
    
    // We only need to use sRGB textures here if gamma correction is performed
    // on hardware. If gamma correction is performed in software, then the texture
    // we've received from the renderer is *already* gamma corrected. If gamma
    // correction is performed on hardware, then the image from the renderer has
    // not yet been gamma corrected, so we need our video texture to gamma correct
    // it here, just as the display will.
    bool sRGB = _driver->getColorRenderingMode() == VROColorRenderingMode::Linear;
    std::unique_ptr<VROTextureSubstrate> substrate = _videoTextureCache->createTextureSubstrate(_videoPixelBuffer, sRGB);
    std::shared_ptr<VROTexture> texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, VROTextureInternalFormat::RGBA8,
                                                                       std::move(substrate));
    
    _rttDelegate = std::make_shared<VROViewRecorderRTTDelegate>(self, texture, _renderer, _driver);
    _renderer->getChoreographer()->setRenderToTextureDelegate(_rttDelegate);

    _isRecording = YES;
    _isFirstRecordingFrame = YES;
    _videoLoopTimer = [NSTimer timerWithTimeInterval:0.03 target:self selector:@selector(recordFrame) userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:_videoLoopTimer forMode:NSRunLoopCommonModes];
}

- (void)recordFrame {
    if (_isRecording) {
        if ([_videoWriterInput isReadyForMoreMediaData]) {
            double currentTime;
            if (_isFirstRecordingFrame) {
                _startTimeMillis = VROTimeCurrentMillis();
                currentTime = 0;
                _isFirstRecordingFrame = false;
            }
            else {
                currentTime = VROTimeCurrentMillis() - _startTimeMillis;
            }
            
            BOOL success = NO;
            if (_addWatermark) {
                
                // grab the videoImage
                CIImage *videoImage = [CIImage imageWithCVPixelBuffer:_videoPixelBuffer];
                // write the resizedWatemarkImage onto the videoImage
                CIImage *outputImage = [_resizedWatermarkImage imageByCompositingOverImage:videoImage];

                // grab the CVPixelBuffer of the combined image
                CIContext *temporaryContext = [CIContext contextWithOptions:nil];
                [temporaryContext render:outputImage toCVPixelBuffer:_compositePixelBuffer];
                
                success = [_videoWriterPixelBufferAdaptor appendPixelBuffer:_compositePixelBuffer withPresentationTime:CMTimeMake(currentTime, 1000)];
            } else {
                success = [_videoWriterPixelBufferAdaptor appendPixelBuffer:_videoPixelBuffer
                                                       withPresentationTime:CMTimeMake(currentTime, 1000)];
                CVPixelBufferUnlockBaseAddress(_videoPixelBuffer, 0);
            }
            
            if (!success) {
                NSString *errorStr = _videoWriter.status == AVAssetWriterStatusFailed ? [_videoWriter.error localizedDescription] : @"unknown error";
                NSLog(@"[Recording] record frame failed: %@", errorStr);
                [self stopVideoRecordingWithError:kVROViewErrorUnknown];
            }
        } else {
            NSLog(@"[Recording] videoWriterInput is not ready for data. It's either finished or we are writing data too fast!");
            // Note: this is not an error, because we'll just skip this frame.
        }
    }
}

- (void)stopVideoRecordingInternal:(VROViewWriteMediaFinishBlock)completionHandler {
    // stop the loop that grabs each frame.
    [_videoLoopTimer invalidate];
    
    // Turn off RTT in the choreographer
    _renderer->getChoreographer()->setRenderToTextureDelegate(nullptr);
    
    [_videoWriterInput markAsFinished];
    if (_videoWriter.status == AVAssetWriterStatusWriting) {
        // CMTime is set up to be value / timescale = seconds, so since we're in millis, timescape is 1000.
        [_videoWriter endSessionAtSourceTime:CMTimeMake(VROTimeCurrentMillis() - _startTimeMillis, 1000)];
        [_videoWriter finishWritingWithCompletionHandler:^(void) {
            if (_videoWriter.status == AVAssetWriterStatusCompleted) {
                completionHandler(YES, _tempVideoFilePath, kVROViewErrorNone);
            } else {
                NSLog(@"[Recording] Failed writing to file: %@", _videoWriter.error ? [_videoWriter.error localizedDescription] : @"Unknown error");
                completionHandler(NO, _tempVideoFilePath, kVROViewErrorWriteToFile);
            }
        }];
    } else {
        completionHandler(NO, nil, kVROViewErrorAlreadyStopped);
    }
}

+ (CVPixelBufferRef)pixelBufferFromCGImage:(CGImageRef)image {
    CGSize frameSize = CGSizeMake(CGImageGetWidth(image), CGImageGetHeight(image));
    NSDictionary *options = @{
                              (__bridge NSString *)kCVPixelBufferCGImageCompatibilityKey: @(YES),
                              (__bridge NSString *)kCVPixelBufferCGBitmapContextCompatibilityKey: @(YES)
                              };
    CVPixelBufferRef pixelBuffer;
    CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, frameSize.width,
                                          frameSize.height,  kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef) options,
                                          &pixelBuffer);

    if (status != kCVReturnSuccess) {
        return NULL;
    }
    
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    void *data = CVPixelBufferGetBaseAddress(pixelBuffer);
    CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data, frameSize.width, frameSize.height,
                                                 8, CVPixelBufferGetBytesPerRow(pixelBuffer), rgbColorSpace,
                                                 (CGBitmapInfo) kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
    CGContextDrawImage(context, CGRectMake(0, 0, CGImageGetWidth(image),
                                           CGImageGetHeight(image)), image);
    CGColorSpaceRelease(rgbColorSpace);
    CGContextRelease(context);
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    
    return pixelBuffer;
}

- (void)lockPixelBuffer {
    CVPixelBufferLockBaseAddress(_videoPixelBuffer, 0);
}

@end

VROViewRecorderRTTDelegate::VROViewRecorderRTTDelegate(VROViewRecorder *recorder,
                                                       std::shared_ptr<VROTexture> texture,
                                                       std::shared_ptr<VRORenderer> renderer,
                                                       std::shared_ptr<VRODriver> driver) :
    _texture(texture),
    _renderer(renderer) {
    _recorder = recorder;
    _renderToTextureTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture, 1, 1, false, false);
    _renderToTextureTarget->attachTexture(_texture, 0);
};

void VROViewRecorderRTTDelegate::didRenderFrame(std::shared_ptr<VRORenderTarget> target,
                                                std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VRORenderer> renderer = _renderer.lock();
    if (!renderer) {
        return;
    }
    VROViewRecorder *recorder = _recorder;
    if (!recorder) {
        return;
    }
    
    VROViewport viewport = renderer->getChoreographer()->getViewport();
    VROViewport rtViewport = VROViewport(0, 0, viewport.getWidth(), viewport.getHeight());
    
    // If the viewport changed, re-attach the video texture
    if (_renderToTextureTarget->setViewport(rtViewport)) {
        _renderToTextureTarget->hydrate();
        _renderToTextureTarget->attachTexture(_texture, 0);
    }

    // Flip/render the image to the RTT target
    driver->bindRenderTarget(_renderToTextureTarget, VRORenderTargetUnbindOp::Invalidate);
    target->blitColor(_renderToTextureTarget, true, driver);
    [recorder lockPixelBuffer];
}
