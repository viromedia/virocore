//
//  VROTrackingHelper.m
//  ViroRenderer
//
//  Created by Andy Chu on 6/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROTrackingHelper.h"
#import "VROImageTracker.h"
#import "opencv2/imgcodecs/ios.h"
#import "opencv2/imgproc/imgproc.hpp"

@interface VROTrackingHelper () {
    std::shared_ptr<VROImageTracker> _tracker;
}

@property (nonatomic, assign) BOOL ready;
@property (nonatomic, assign) NSInteger count;

@end

@implementation VROTrackingHelper

- (instancetype)init {
    self = [super init];
    if (self) {
        _ready = YES;
        _count = 0;
    }
    return self;
}

- (void)runTracking:(cv::Mat)cameraInput {

    if (!_tracker) {
        // fetch the target image to find
        UIImage *targetImage = [UIImage imageNamed:@"ben.jpg"];
        
        cv::Mat targetMat = cv::Mat();
        UIImageToMat(targetImage, targetMat); // should give us a RGB Mat.
        
        // initialize the tracker
        _tracker = VROImageTracker::createImageTracker(targetMat);
    }

    // find the target in the given cameraInput (should already be in RGB format).
    std::shared_ptr<VROImageTrackerOutput> output = _tracker->findTarget(cameraInput);
    
    if (!output->found) {
        NSLog(@"VROTrackingHelper, couldn't find target in given image");
    }
    
    @synchronized (self) {
        _ready = YES;
    }
}

- (void)processBuffer:(CMSampleBufferRef)newBuffer {
    
    @synchronized(self) {
        if (!_ready) {
            _count++;
            if (_count % 20 == 1) {
                NSLog(@"VROTrackingHelper, not ready for next frame! count %ld", _count);
            }
            return;
        } else {
            NSLog(@"VROTrackingHelper, ready for next frame! skipped %ld frames", _count);
            _count = 0;
            _ready = NO;
        }
    }

    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(newBuffer);
    
    CVPixelBufferLockBaseAddress( pixelBuffer, 0 );
    
    unsigned char *base = (unsigned char *)CVPixelBufferGetBaseAddress( pixelBuffer );
    size_t height = CVPixelBufferGetHeight( pixelBuffer );
    size_t stride = CVPixelBufferGetBytesPerRow( pixelBuffer );
    size_t extendedWidth = stride / sizeof( uint32_t ); // each pixel is 4 bytes/32 bits
    
    cv::Mat bgraImage = cv::Mat((int)height, (int)extendedWidth, CV_8UC4, base);
    cv::Mat rgbImage;
    cv::cvtColor(bgraImage, rgbImage, cv::COLOR_BGRA2RGB);

    CVPixelBufferUnlockBaseAddress( pixelBuffer, 0 );

    // anything higher than "low" priority and we start skipping frames (probably because
    // the low level camera notification API's notify on the background thread).
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^{
        [self runTracking:rgbImage];
    });
}

#pragma mark AVCaptureVideoDataOutputSampleBufferDelegate

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    
    [self processBuffer:sampleBuffer];
}

@end
