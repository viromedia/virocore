//
//  VROViewAR.m
//  ViroRenderer
//
//  Created by Raj Advani on 5/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <Photos/Photos.h>
#import "VROViewAR.h"
#import "VRORenderer.h"
#import "VROSceneController.h"
#import "VRORenderDelegateiOS.h"
#import "VROTime.h"
#import "VROEye.h"
#import "VRODriverOpenGLiOS.h"
#import "VROApiKeyValidator.h"
#import "VROApiKeyValidatorDynamo.h"
#import "VROARSessioniOS.h"
#import "VROARSessionInertial.h"
#import "VROARCamera.h"
#import "VROARAnchor.h"
#import "VROARFrame.h"
#import "VROConvert.h"
#import "VROARHitTestResult.h"
#import "VRONodeCamera.h"
#import "vr/gvr/capi/include/gvr_audio.h"
#import "VROARScene.h"
#import "VROARComponentManager.h"
#import "VROChoreographer.h"
#import "VROVideoTextureCache.h"
#import "VROInputControllerARiOS.h"
#import "VROProjector.h"
#import "VROWeakProxy.h"

static VROVector3f const kZeroVector = VROVector3f();

@interface VROViewAR () {
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::shared_ptr<VROSurface> _cameraBackground;
    std::shared_ptr<VROARSession> _arSession;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROARComponentManager> _arComponentManager;
    std::shared_ptr<VROInputControllerARiOS> _inputController;
    
    CADisplayLink *_displayLink;
    int _frame;
    
    double _suspendedNotificationTime;
    bool _hasTrackingInitialized;

    /*
     Video Recording/Screenshot variables
     */
    VROViewRecordingErrorBlock _errorBlock;
    bool _isRecording;
    bool _saveToCameraRoll;
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
}

@property (readwrite, nonatomic) id <VROApiKeyValidator> keyValidator;

@end

@implementation VROViewAR

@dynamic renderDelegate;
@dynamic sceneController;

#pragma mark - Initialization

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame context:(EAGLContext *)context {
    self = [super initWithFrame:frame context:context];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    
    // If the frame changes, then update the _camera background to match
    if (_cameraBackground) {
        _cameraBackground->setX(self.frame.size.width * self.contentScaleFactor / 2.0);
        _cameraBackground->setY(self.frame.size.height * self.contentScaleFactor / 2.0);
        _cameraBackground->setWidth(self.frame.size.width * self.contentScaleFactor);
        _cameraBackground->setHeight(self.frame.size.height * self.contentScaleFactor);
    }

    if (_inputController) {
        _inputController->setViewportSize(self.frame.size.width * self.contentScaleFactor,
                                          self.frame.size.height * self.contentScaleFactor);
    }
}

- (void)initRenderer {
    if (!self.context) {
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context = context;
    }
    
    /*
     Setup the GLKView.
     */
    self.enableSetNeedsDisplay = NO;
    self.drawableColorFormat = GLKViewDrawableColorFormatRGB565;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    self.drawableMultisample = GLKViewDrawableMultisample4X;
    
    [EAGLContext setCurrentContext:self.context];
    
    /*
     Setup the animation loop for the GLKView.
     */
    VROWeakProxy *proxy = [VROWeakProxy weakProxyForObject:self];
    _displayLink = [CADisplayLink displayLinkWithTarget:proxy selector:@selector(display)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    /*
     Disable going to sleep, and setup notifications.
     */
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive:)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive:)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    
    /*
     Create Viro renderer objects.
     */
    _frame = 0;
    _gvrAudio = std::make_shared<gvr::AudioApi>();
    _gvrAudio->Init(GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    _driver = std::make_shared<VRODriverOpenGLiOS>(self.context, _gvrAudio);
    _suspendedNotificationTime = VROTimeCurrentSeconds();

    _inputController = std::make_shared<VROInputControllerARiOS>(self.frame.size.width * self.contentScaleFactor,
                                                                 self.frame.size.height * self.contentScaleFactor);
    _renderer = std::make_shared<VRORenderer>(_inputController);
    _inputController->setRenderer(_renderer);
    _hasTrackingInitialized = false;
    
    /*
     Set up the Audio Session properly for recording and playing back audio. We need
     to do this *AFTER* we init _gvrAudio, because it resets some setting, else audio
     recording won't work.
     */
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
    
    /*
     Create AR session.
     */
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
    _arSession = std::make_shared<VROARSessioniOS>(VROTrackingType::DOF6, _driver);
#else
    _arSession = std::make_shared<VROARSessionInertial>(VROTrackingType::DOF3, _driver);
    // in the 3DOF case, tracking doesn't take time to initialize, but the sceneController hasn't yet been set.
    _hasTrackingInitialized = true;
#endif
    _arSession->setOrientation(VROConvert::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]));
    
    /*
     Create AR component manager and set it as the delegate to the AR session.
     */
    _arComponentManager = std::make_shared<VROARComponentManager>();
    _arSession->setDelegate(_arComponentManager);
    
    // set session on input controller!
    _inputController->setSession(std::dynamic_pointer_cast<VROARSession>(_arSession));
    
    /*
     Set the point of view to a special node that will follow the user's
     real position.
     */
    _pointOfView = std::make_shared<VRONode>();
    _pointOfView->setCamera(std::make_shared<VRONodeCamera>());
    _renderer->setPointOfView(_pointOfView);
    
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];
    
    UILongPressGestureRecognizer *longPress = [[UILongPressGestureRecognizer alloc] initWithTarget:self
                                                                                    action:@selector(handleLongPress:)];
    longPress.minimumPressDuration = 0;
    [self addGestureRecognizer:longPress];
  
    UIPinchGestureRecognizer *twoFingerPinch = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];
    [self addGestureRecognizer:twoFingerPinch];
    
    UIRotationGestureRecognizer *rotateRotatePinch = [[UIRotationGestureRecognizer alloc] initWithTarget:self action:@selector(handleRotate:)];
    [self addGestureRecognizer:rotateRotatePinch];
}

- (void)handleRotate:(UIRotationGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
    
    if(recognizer.state == UIGestureRecognizerStateBegan) {
        _inputController->onRotateStart(viewportTouchPos);
    } else if(recognizer.state == UIGestureRecognizerStateChanged) {
        _inputController->onRotate(recognizer.rotation);
    } else if(recognizer.state == UIGestureRecognizerStateEnded) {
        _inputController->onRotateEnd();
    }
}

- (void)handlePinch:(UIPinchGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
  
    if(recognizer.state == UIGestureRecognizerStateBegan) {
      _inputController->onPinchStart(viewportTouchPos);
    } else if(recognizer.state == UIGestureRecognizerStateChanged) {
      _inputController->onPinchScale(recognizer.scale);
    } else if(recognizer.state == UIGestureRecognizerStateEnded) {
      _inputController->onPinchEnd();
    }
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
    
    if (recognizer.state == UIGestureRecognizerStateBegan) {
        _inputController->onScreenTouchDown(viewportTouchPos);
    } else if (recognizer.state == UIGestureRecognizerStateEnded) {
        _inputController->onScreenTouchUp(viewportTouchPos);
    } else {
        _inputController->onScreenTouchMove(viewportTouchPos);
        return;
    }

    if (_arSession && _arSession->isReady()) {
        std::unique_ptr<VROARFrame> &frame = _arSession->getLastFrame();
        std::vector<VROARHitTestResult> results = frame->hitTest(location.x * self.contentScaleFactor,
                                                                 location.y * self.contentScaleFactor,
                       { VROARHitTestResultType::ExistingPlaneUsingExtent,
                         VROARHitTestResultType::ExistingPlane,
                         VROARHitTestResultType::EstimatedHorizontalPlane,
                         VROARHitTestResultType::FeaturePoint });
        
        for (VROARHitTestResult &result : results) {
            if (self.tapHandler) {
                self.tapHandler(result, _arSession, _sceneController->getScene());
            }
        }
    }
}
// End TODO

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if (_displayLink) {
        [_displayLink invalidate];
    }
}

- (void)setPaused:(BOOL)paused {
    [_displayLink setPaused:paused];
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

    // we MUST first check if a recording session is ongoing BEFORE we override state variables.
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

    // if we're saving this file to camera roll, then also check for that permission before we start
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

- (void)takeScreenshot:(NSString *)fileName
      saveToCameraRoll:(BOOL)saveToCamera
 withCompletionHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    
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
    
    [UIImagePNGRepresentation(self.snapshot) writeToFile:[filePath path] atomically:YES];
    
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
    NSError *error = nil;
    _videoWriter = [[AVAssetWriter alloc] initWithURL:filePath fileType:AVFileTypeMPEG4 error:&error];
    
    int width  = self.frame.size.width  * self.contentScaleFactor;
    int height = self.frame.size.height * self.contentScaleFactor;
    
    NSDictionary *videoSetting = @{
                                   AVVideoCodecKey : AVVideoCodecH264,
                                   AVVideoWidthKey : @(width),
                                   AVVideoHeightKey : @(height)
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
    std::unique_ptr<VROTextureSubstrate> substrate = _videoTextureCache->createTextureSubstrate(_videoPixelBuffer);
    std::shared_ptr<VROTexture> texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, std::move(substrate));
    _renderer->getChoreographer()->setRenderToTextureEnabled(true);
    _renderer->getChoreographer()->setRenderTexture(texture);
    
    __weak VROViewAR *weakSelf = self;
    _renderer->getChoreographer()->setRenderToTextureCallback([weakSelf] {
        VROViewAR *strongSelf = weakSelf;
        if (strongSelf) {
            CVPixelBufferLockBaseAddress(strongSelf->_videoPixelBuffer, 0);
        }
    });
    
    _startTimeMillis = VROTimeCurrentMillis();
    _isRecording = YES;
    _videoLoopTimer = [NSTimer timerWithTimeInterval:0.03 target:self selector:@selector(recordFrame) userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:_videoLoopTimer forMode:NSRunLoopCommonModes];
}

- (void)recordFrame {
    if (_isRecording) {
        if ([_videoWriterInput isReadyForMoreMediaData]) {
            double currentTime = VROTimeCurrentMillis() - _startTimeMillis;
            
            BOOL success = [_videoWriterPixelBufferAdaptor appendPixelBuffer:_videoPixelBuffer
                                                        withPresentationTime:CMTimeMake(currentTime, 1000)];
            CVPixelBufferUnlockBaseAddress(_videoPixelBuffer, 0);
            
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
    _renderer->getChoreographer()->setRenderToTextureEnabled(false);

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

#pragma mark - AR Functions
- (std::vector<VROARHitTestResult>)performARHitTest:(VROVector3f)ray {
    // check that the ray is in front of the camera
    VROVector3f cameraForward = _renderer->getRenderContext()->getCamera().getForward();
    if (cameraForward.dot(ray) <= 0) {
        return std::vector<VROARHitTestResult>();
    }

    int viewportArr[4] = {0, 0,
        (int) (self.bounds.size.width  * self.contentScaleFactor),
        (int) (self.bounds.size.height * self.contentScaleFactor)};

    // create the mvp (in this case, the model mat is identity).
    VROMatrix4f projectionMat = _renderer->getRenderContext()->getProjectionMatrix();
    VROMatrix4f viewMat = _renderer->getRenderContext()->getViewMatrix();
    VROMatrix4f vpMat = projectionMat.multiply(viewMat);

    // get the 2D point
    VROVector3f point;
    VROProjector::project(ray, vpMat.getArray(), viewportArr, &point);

    // check the 2D point, perform and return the results from the AR hit test
    std::unique_ptr<VROARFrame> &frame = _arSession->getLastFrame();
    if (frame && point.x >= 0 && point.x <= viewportArr[2] && point.y >= 0 && point.y <= viewportArr[3]) {
        std::vector<VROARHitTestResult> results = frame->hitTest(point.x,
                                                                 point.y,
                                                                 { VROARHitTestResultType::ExistingPlaneUsingExtent,
                                                                   VROARHitTestResultType::ExistingPlane,
                                                                   VROARHitTestResultType::EstimatedHorizontalPlane,
                                                                   VROARHitTestResultType::FeaturePoint });
        return results;
    }
    return std::vector<VROARHitTestResult>();
}

#pragma mark - Settings and Notifications

- (void)orientationDidChange:(NSNotification *)notification {
    // the _cameraBackground will be updated if/when the frame is actually set.
    _arSession->setOrientation(VROConvert::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]));
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    _displayLink.paused = YES;
    _arSession->pause();
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    _displayLink.paused = NO;
    _arSession->run(); 
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    _renderDelegateWrapper = std::make_shared<VRORenderDelegateiOS>(renderDelegate);
    _renderer->setDelegate(_renderDelegateWrapper);
}

- (void)setARSessionDelegate:(std::shared_ptr<VROARSessionDelegate>)delegate {
    _arSession->setDelegate(delegate);
}

- (void)setVrMode:(BOOL)enabled {
    // No-op in AR mode
}

- (void)setDebugHUDEnabled:(BOOL)enabled {
    _renderer->setDebugHUDEnabled(enabled);
}

- (NSString *)getPlatform {
    return @"ar";
}

- (NSString *)getHeadset {
    return @"mobile";
}

- (NSString *)getController {
    return @"screen";
}

#pragma mark - Key Validation

- (void)validateApiKey:(NSString *)apiKey withCompletionBlock:(VROViewValidApiKeyBlock)completionBlock {
    // If the user gives us a key, then let them use the API until we successfully checked the key.
    self.suspended = NO;
    
    __weak typeof(self) weakSelf = self;
    
    VROApiKeyValidatorBlock validatorCompletionBlock = ^(BOOL valid) {
        typeof(weakSelf) strongSelf = weakSelf;
        if (!strongSelf) {
            return;
        }
        
        strongSelf.suspended = !valid;
        completionBlock(valid);
        
        NSLog(@"[ApiKeyValidator] The key is %@!", valid ? @"valid" : @"invalid");
    };
    [self.keyValidator validateApiKey:apiKey withCompletionBlock:validatorCompletionBlock];
}

#pragma mark - Camera

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    pabort("May not set POV in AR mode");
}

- (void)layoutSubviews {
    [super layoutSubviews];
    _renderer->updateRenderViewSize(self.bounds.size.width, self.bounds.size.height);
}

#pragma mark - Scene Loading

- (void)setSceneController:(std::shared_ptr<VROSceneController>) sceneController {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, _driver);
    
    if (_hasTrackingInitialized) {
        std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
        arScene->trackingHasInitialized();
    }
    
    // Reset the camera background for the new scene
    _cameraBackground.reset();
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController
                  duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, _driver);
    
    // Reset the camera background for the new scene
    _cameraBackground.reset();
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

#pragma mark - Rendering

- (void)drawRect:(CGRect)rect {
    @autoreleasepool {
        if (self.suspended) {
            [self renderSuspended];
        }
        else {
            [self renderFrame];
        }
    }
    
    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

- (void)renderFrame {
    /*
     Setup GL state.
     */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    _driver->setCullMode(VROCullMode::Back);
    
    VROViewport viewport(0, 0, self.bounds.size.width  * self.contentScaleFactor,
                               self.bounds.size.height * self.contentScaleFactor);
    
    if (_sceneController) {
        if (!_cameraBackground) {
            [self initARSessionWithViewport:viewport scene:_sceneController->getScene()];
        }
    }
    
    if (_arSession->isReady()) {
        // TODO Only on viewport change
        _arSession->setViewport(viewport);
        if (!_sceneController->getScene()->getRootNode()->getBackground()) {
            _sceneController->getScene()->getRootNode()->setBackground(_cameraBackground);
        }

        /*
         Retrieve transforms from the AR session.
         */
        std::unique_ptr<VROARFrame> &frame = _arSession->updateFrame();
        const std::shared_ptr<VROARCamera> camera = frame->getCamera();
        
        VROFieldOfView fov;
        VROMatrix4f projection = camera->getProjection(viewport, kZNear, _renderer->getFarClippingPlane(), &fov);
        VROMatrix4f rotation = camera->getRotation();
        VROVector3f position = camera->getPosition();
        
        if (_sceneController && !_hasTrackingInitialized) {
            if (position != kZeroVector) {
                _hasTrackingInitialized = true;
                std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
                arScene->trackingHasInitialized();
            }
        }
        
        // TODO Only on orientation change
        VROMatrix4f backgroundTransform = frame->getViewportToCameraImageTransform();
        _cameraBackground->setTexcoordTransform(backgroundTransform);
        
        /*
         Render the 3D scene.
         */
        _pointOfView->getCamera()->setPosition(position);
        _renderer->prepareFrame(_frame, viewport, fov, rotation, projection, _driver);
        
        glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        _renderer->renderEye(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
        _renderer->endFrame(_driver);

        /*
         notify scene of the updated ambient light estimates
         */
        std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(_arSession->getScene());
        scene->updateAmbientLight(frame->getAmbientLightIntensity(), frame->getAmbientLightColorTemperature());
    }
    else {
        /*
         Render black while waiting for the AR session to initialize.
         */
        VROFieldOfView fov = _renderer->computeMonoFOV(viewport.getWidth(), viewport.getHeight());
        VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
        
        _renderer->prepareFrame(_frame, viewport, fov, VROMatrix4f::identity(), projection, _driver);
        glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        _renderer->renderEye(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
        _renderer->endFrame(_driver);
    }
}

- (void)renderSuspended {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    double newTime = VROTimeCurrentSeconds();
    // notify the user about bad keys 5 times a second (every 200ms/.2s)
    if (newTime - _suspendedNotificationTime > .2) {
        perr("Renderer suspended! Do you have a valid key?");
        _suspendedNotificationTime = newTime;
    }
}

- (void)initARSessionWithViewport:(VROViewport)viewport scene:(std::shared_ptr<VROScene>)scene {
    _cameraBackground = VROSurface::createSurface(viewport.getX() + viewport.getWidth()  / 2.0,
                                                  viewport.getY() + viewport.getHeight() / 2.0,
                                                  viewport.getWidth(), viewport.getHeight(),
                                                  0, 0, 1, 1);
    _cameraBackground->setScreenSpace(true);
    _cameraBackground->setName("Camera");
    
    std::shared_ptr<VROMaterial> material = _cameraBackground->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Constant);
    material->getDiffuse().setTexture(_arSession->getCameraBackgroundTexture());
    material->setWritesToDepthBuffer(false);
    
    _arSession->setScene(scene);
    _arSession->setViewport(viewport);
    _arSession->setAnchorDetection({ VROAnchorDetection::PlanesHorizontal });
    _arSession->run();
    
    // TODO: change the scene to VROARScene in this this class?
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(scene);
    arScene->setARComponentManager(_arComponentManager);
    arScene->addNode(_pointOfView);
}

- (void)recenterTracking {
    // TODO Implement this, try to share code with VROSceneRendererCardboardOpenGL; maybe
    //      move the functionality into VRORenderer
}

@end
