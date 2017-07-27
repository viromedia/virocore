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
    bool _isRecording;
    bool _saveToCameraRoll;
    NSURL *_videoFilePath;
    AVAssetWriter *_videoWriter;
    AVAssetWriterInput *_videoWriterInput;
    AVAssetWriterInputPixelBufferAdaptor *_videoWriterPixelBufferAdaptor;
    double _startTimeMillis;
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
}

- (void)initRenderer {
    if (!self.context) {
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context = context;
    }
    
    /*
     Setup the animation loop for the GLKView.
     */
    VROWeakProxy *proxy = [VROWeakProxy weakProxyForObject:self];
    _displayLink = [CADisplayLink displayLinkWithTarget:proxy selector:@selector(display)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    
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
    _inputController = std::make_shared<VROInputControllerARiOS>(self.bounds.size.width * self.contentScaleFactor,
                                                                 self.bounds.size.height * self.contentScaleFactor);
    _renderer = std::make_shared<VRORenderer>(_inputController);
    _inputController->setRenderer(_renderer);
    _hasTrackingInitialized = false;
    
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

#pragma mark - Recording and Screen Capture

- (void)startVideoRecording:(NSString *)fileName saveToCameraRoll:(BOOL)saveToCamera {
    if (_isRecording) {
        return;
    }
    _saveToCameraRoll = saveToCamera;
    _videoFilePath = [self getTempFileURL:fileName];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    // if the given fileName exists, then just remove it because we'll overwrite it.
    if ([fileManager fileExistsAtPath:[_videoFilePath path]]) {
        [fileManager removeItemAtURL:_videoFilePath error:nil];
    }
    
    [self startRecordingV1:_videoFilePath];
}

- (void)stopVideoRecordingWithHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    // this block will be called once the video writer in stopRecordingV1 finishes writing the vid
    VROViewWriteMediaFinishBlock wrappedCompleteHandler = ^(NSURL *filepath) {
        if (_saveToCameraRoll) {
            [self writeMediaToCameraRoll:filepath isPhoto:NO withCompletionHandler:completionHandler];
        } else {
            completionHandler(filepath);
        }
        _saveToCameraRoll = NO;
        _isRecording = NO;
    };
    [self stopRecordingV1:wrappedCompleteHandler];
}

- (void)takeScreenshot:(NSString *)fileName
      saveToCameraRoll:(BOOL)saveToCamera
 withCompletionHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    
    NSURL *filePath = [self getTempFileURL:fileName];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    // if the given fileName exists, then just remove it because we'll overwrite it.
    if ([fileManager fileExistsAtPath:[filePath path]]) {
        [fileManager removeItemAtURL:filePath error:nil];
    }
    
    [UIImagePNGRepresentation(self.snapshot) writeToFile:[filePath path] atomically:YES];
    
    if (saveToCamera) {
        [self writeMediaToCameraRoll:filePath isPhoto:YES withCompletionHandler:completionHandler];
    } else {
        completionHandler(filePath);
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
            completionHandler(filePath);
        } else {
            NSLog(@"[Recording] saving media failed w/ error: %@", error.localizedDescription);
            completionHandler(NULL);
        }
    }];
}

// TODO: use kVROViewTempMediaDirectory. We can't write to a file if the directory it lives in doesn't exist.
- (NSURL *)getTempFileURL:(NSString *)filename {
    NSURL *filepath = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingString:filename]];
    return filepath;
}

#pragma mark - Video Recording V1
/*
 The below code is the "V1" version of video recording which fetches each video frame by extracting a
 UIImage from this GLKView which is slow and uses recursion w/ GCD to loop.
 */

- (void)startRecordingV1:(NSURL *)filePath {
    if (_isRecording) {
        return; // do nothing if we're already recording...
    }
    
    NSError *error = nil;
    _videoWriter = [[AVAssetWriter alloc] initWithURL:filePath fileType:AVFileTypeMPEG4 error:&error];
    
    NSDictionary *videoSetting = @{
                                   AVVideoCodecKey : AVVideoCodecH264,
                                   AVVideoWidthKey : @(self.frame.size.width * self.contentScaleFactor / 2.0),
                                   AVVideoHeightKey : @(self.frame.size.height * self.contentScaleFactor / 2.0)
                                   };
    
    _videoWriterInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:videoSetting];
    
    _videoWriterPixelBufferAdaptor = [AVAssetWriterInputPixelBufferAdaptor
                                                     assetWriterInputPixelBufferAdaptorWithAssetWriterInput:_videoWriterInput
                                                     sourcePixelBufferAttributes:nil];
    
    [_videoWriter addInput:_videoWriterInput];
    
    [_videoWriter startWriting];
    [_videoWriter startSessionAtSourceTime:kCMTimeZero];
    _startTimeMillis = VROTimeCurrentMillis();
    _isRecording = YES;
    
    // we have to run this on the main thread because self.snapshot seems to access some UIView properties that require it.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0), dispatch_get_main_queue(), ^{
        [self recordFrameV1];
    });
}

/*
 This function should initially be run on the UI thread as it requires calling self.snapshot which
 accesses some UIView properties.
 */
- (void)recordFrameV1 {
    double currentTime = VROTimeCurrentMillis() - _startTimeMillis;
    NSLog(@"[Recording] record frame at currentTime %f", currentTime);
    if (_isRecording) {
        CGImageRef image = self.snapshot.CGImage;
        CVPixelBufferRef ref = [VROViewAR pixelBufferFromCGImage:image];
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            
            BOOL success = [_videoWriterPixelBufferAdaptor appendPixelBuffer:ref withPresentationTime:CMTimeMake(currentTime, 1000)];
            CVPixelBufferRelease(ref);
            
            if (!success) {
                NSString *errorStr = _videoWriter.status == AVAssetWriterStatusFailed ? [_videoWriter.error localizedDescription] : @"unknown issue";
                NSLog(@"[Recording] record frame failed: %@", errorStr);
                // stop recording/looping
                // TODO: clean up? or rather, assume that the "record loop" actually stops recording. But this is V1 (throw away) code.
                return;
            }
            
            // loop again, but on the main thread! There's no delay because getting snapshot takes too long right now.
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0*NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                [self recordFrameV1];
            });
        });
    }
}

- (void)stopRecordingV1:(VROViewWriteMediaFinishBlock)completionHandler {
    [_videoWriterInput markAsFinished];
    // CMTime is set up to be value / timescale = seconds, so since we're in millis, timescape is 1000.
    [_videoWriter endSessionAtSourceTime:CMTimeMake(VROTimeCurrentMillis() - _startTimeMillis, 1000)];
    [_videoWriter finishWritingWithCompletionHandler:^(void) {
        NSLog(@"[Recording] finished writing video");
        completionHandler(_videoFilePath);
    }];
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
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
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
        
        _pointOfView->setBackground(_cameraBackground);

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
    material->setReadsFromDepthBuffer(false);
    
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
