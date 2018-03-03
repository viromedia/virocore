//
//  VROTrackingHelper.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>
#import "opencv2/imgcodecs/ios.h"
#import "VROARImageTrackerOutput.h"

@interface VROTrackingHelperOutput : NSObject

- (instancetype) initWithTrackerOutput:(std::shared_ptr<VROARImageTrackerOutput>)output withImage:(UIImage *)outputImage;

- (std::shared_ptr<VROARImageTrackerOutput>)getImageTrackerOutput;
- (UIImage *)getOutputImage;

@end

/*
 This class is currently meant to show as a proof of concept that we can fetch images from
 the AVCapture session and run them through image detection w/ OpenCV
 
 This function
 */
@interface VROTrackingHelper : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property (nonatomic, strong) UIImageView *overlayImageView;

- (instancetype)init;

/*
 This function runs the same test as ViroActivity's testFindInScreenshot on Android. Useful
 for ensuring similar performance on both platforms.
 */
- (void)findInScreenshot;

- (void)setIntrinsics:(float *)intrinsics;

- (void)processBuffer:(CMSampleBufferRef)newBuffer;

- (void)processPixelBufferRef:(CVPixelBufferRef)pixelBuffer
                     forceRun:(BOOL)forceRun
                   completion:(void (^)(VROTrackingHelperOutput *output))completionHandler;

- (cv::Mat)matFromYCbCrBuffer:(CVImageBufferRef)buffer;

/*
 Sets tracking on/off depending on the given bool.
 */
- (void)toggleTracking:(BOOL)tracking;

/*
 Toggles tracking on/off depending on existing state.
 */
- (BOOL)toggleTracking;


@end
