//
//  VROTrackingHelper.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#if ENABLE_OPENCV

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>
#import "opencv2/imgcodecs/ios.h"
#import "VROARCamera.h"
#import "VROARImageTracker.h"

@interface VROTrackingHelperOutput : NSObject

- (instancetype)initWithTrackerOutput:(VROARImageTrackerOutput)output withImage:(UIImage *)outputImage;

- (VROARImageTrackerOutput)getImageTrackerOutput;
- (UIImage *)getOutputImage;

@end

/*
 This class is a helper class that takes the given PixelBuffer and feeds it into a
 VROARImageTracker that has been preconfigured with a target image
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
                       camera:(std::shared_ptr<VROARCamera>)camera
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

#endif /* ENABLE_OPENCV */
