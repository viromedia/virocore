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

@interface VROTrackingHelperOutput() {
    std::shared_ptr<VROImageTrackerOutput> _output;
}

@property (nonatomic, strong) UIImage *outputImage;

@end

@implementation VROTrackingHelperOutput

- (instancetype) initWithTrackerOutput:(std::shared_ptr<VROImageTrackerOutput>)output withImage:(UIImage *)outputImage {
    self = [super init];
    if (self) {
        _output = output;
        _outputImage = outputImage;
    }
    return self;
}

- (std::shared_ptr<VROImageTrackerOutput>)getImageTrackerOutput {
    return _output;
}

- (UIImage *)getOutputImage {
    return _outputImage;
}

@end

@interface VROTrackingHelper () {
    std::shared_ptr<VROImageTracker> _tracker;
}

@property (nonatomic, assign) BOOL ready;
@property (nonatomic, assign) NSInteger count;
@property (nonatomic, assign) float *intrinsics;

@end

@implementation VROTrackingHelper

- (instancetype)init {
    self = [super init];
    if (self) {
        _ready = YES;
        _count = 0;
        _intrinsics = NULL;
    }
    return self;
}

- (void)setIntrinsics:(float *)intrinsics {
    _intrinsics = intrinsics;
}


- (std::shared_ptr<VROImageTrackerOutput>)runTracking:(cv::Mat)cameraInput {

    if (!_tracker) {
        // fetch the target image to find
        UIImage *targetImage = [UIImage imageNamed:@"ben.jpg"];
        
        cv::Mat targetMat = cv::Mat();
        UIImageToMat(targetImage, targetMat); // should give us a RGB Mat.
        
        // initialize the tracker
        _tracker = VROImageTracker::createImageTracker(targetMat);
    }

    // find the target in the given cameraInput (should already be in RGB format).
    std::shared_ptr<VROImageTrackerOutput> output;
    if (_intrinsics == NULL) {
        output = _tracker->findTarget(cameraInput);
    } else {
        output = _tracker->findTarget(cameraInput, _intrinsics);
    }
    
    if (!output->found) {
        NSLog(@"VROTrackingHelper, couldn't find target in given image");
    } else {
        cv::line(cameraInput, output->corners[0], output->corners[1], cv::Scalar(0, 255, 0), 5);
        cv::line(cameraInput, output->corners[1], output->corners[2], cv::Scalar(0, 255, 0), 5);
        cv::line(cameraInput, output->corners[2], output->corners[3], cv::Scalar(0, 255, 0), 5);
        cv::line(cameraInput, output->corners[3], output->corners[0], cv::Scalar(0, 255, 0), 5);
        
        output->outputImage = cameraInput;
        // write to photo album...
        //UIImage *outputImage = MatToUIImage(cameraInput);
        //UIImageWriteToSavedPhotosAlbum(outputImage, nil, nil, nil);
    }
    
    @synchronized (self) {
        // since we're writing out to the camera roll, do it only once :p
        //if (!output->found) {
            _ready = YES;
        //}
    }

    return output;
}

// this function assumes the buffer comes directly from the Camera.
- (void)processBuffer:(CMSampleBufferRef)newBuffer {
    
    @synchronized(self) {
        if (!_ready) {
            _count++;
            if (_count % 20 == 1) {
                NSLog(@"VROTrackingHelper, not ready for next frame! count %ld", (long)_count);
            }
            return;
        } else {
            NSLog(@"VROTrackingHelper, ready for next frame! skipped %ld frames", (long)_count);
            _count = 0;
            _ready = NO;
        }
    }

    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(newBuffer);

    [self processPixelBufferRef:pixelBuffer forceRun:YES completion:nil];
}

// this function is called by forceRun = true if the input is from camera, or false if from AR.
- (void)processPixelBufferRef:(CVPixelBufferRef)pixelBuffer forceRun:(BOOL)forceRun completion:(void (^)(VROTrackingHelperOutput *output))completionHandler {
    if (!forceRun) {
        @synchronized(self) {
            if (!_ready) {
                _count++;
                if (_count % 20 == 1) {
                    NSLog(@"VROTrackingHelper, not ready for next frame! count %ld", (long)_count);
                }
                return;
            } else {
                NSLog(@"VROTrackingHelper, ready for next frame! skipped %ld frames", (long)_count);
                _count = 0;
                _ready = NO;
            }
        }
    }

    cv::Mat rgbImage;

    if (forceRun) {
        CVPixelBufferLockBaseAddress( pixelBuffer, kCVPixelBufferLock_ReadOnly );
        
        unsigned char *base = (unsigned char *)CVPixelBufferGetBaseAddress( pixelBuffer );
        size_t height = CVPixelBufferGetHeight( pixelBuffer );
        size_t stride = CVPixelBufferGetBytesPerRow( pixelBuffer );
        size_t extendedWidth = stride / sizeof( uint32_t ); // each pixel is 4 bytes/32 bits
        
        cv::Mat bgraImage = cv::Mat((int)height, (int)extendedWidth, CV_8UC4, base);
        cv::cvtColor(bgraImage, rgbImage, cv::COLOR_BGRA2RGB);
        
        CVPixelBufferUnlockBaseAddress( pixelBuffer, 0 );
    } else {
        rgbImage = [self matFromYCbCrBuffer:pixelBuffer];
    }
    
    // anything higher than "low" priority and we start skipping frames (probably because
    // the low level camera notification API's notify on the background thread).
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^{
        std::shared_ptr<VROImageTrackerOutput> output = [self runTracking:rgbImage];
        if (completionHandler) {
            completionHandler([[VROTrackingHelperOutput alloc] initWithTrackerOutput:output withImage:MatToUIImage(output->outputImage)]);
        }
    });
}

- (cv::Mat)matFromYCbCrBuffer:(CVImageBufferRef)buffer {
    
    cv::Mat mat;
    CVPixelBufferLockBaseAddress(buffer,0);
    //Get the data from the first plane (Y)
    void *address = CVPixelBufferGetBaseAddressOfPlane(buffer, 0);
    int bufferWidth = (int)CVPixelBufferGetWidthOfPlane(buffer,0);
    int bufferHeight = (int)CVPixelBufferGetHeightOfPlane(buffer, 0);
    int bytePerRow = (int)CVPixelBufferGetBytesPerRowOfPlane(buffer, 0);
    //Get the pixel format
    OSType pixelFormat = CVPixelBufferGetPixelFormatType(buffer);
    
    cv::Mat converted;
    //NOTE: CV_8UC3 means unsigned (0-255) 8 bits per pixel, with 3 channels!
    //Check to see if this is the correct pixel format
    if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
        //We have an ARKIT buffer
        //Get the yPlane (Luma values)
        cv::Mat yPlane = cv::Mat(bufferHeight, bufferWidth, CV_8UC1, address);
        //Get cbcrPlane (Chroma values)
        int cbcrWidth = (int)CVPixelBufferGetWidthOfPlane(buffer,1);
        int cbcrHeight = (int)CVPixelBufferGetHeightOfPlane(buffer, 1);
        void *cbcrAddress = CVPixelBufferGetBaseAddressOfPlane(buffer, 1);
        //Since the CbCr Values are alternating we have 2 channels: Cb and Cr. Thus we need to use CV_8UC2 here.
        cv::Mat cbcrPlane = cv::Mat(cbcrHeight, cbcrWidth, CV_8UC2, cbcrAddress);
        //Split them apart so we can merge them with the luma values
        std::vector<cv::Mat> cbcrPlanes;
        split(cbcrPlane, cbcrPlanes);
        
        cv::Mat cbPlane;
        cv::Mat crPlane;
        //Since we have a 4:2:0 format, cb and cr values are only present for each 2x2 luma pixels. Thus we need to enlargen them (by a factor of 2).
        resize(cbcrPlanes[0], cbPlane, yPlane.size(), 0, 0, cv::INTER_NEAREST);
        resize(cbcrPlanes[1], crPlane, yPlane.size(), 0, 0, cv::INTER_NEAREST);
        
        cv::Mat ycbcr;
        std::vector<cv::Mat> allPlanes = {yPlane, cbPlane, crPlane};
        merge(allPlanes, ycbcr);
        //ycbcr now contains all three planes. We need to convert it from YCbCr to RGB so OpenCV can work with it
        cvtColor(ycbcr, converted, cv::COLOR_YCrCb2RGB);
    } else {
        //Probably RGB so just use that.
        converted = cv::Mat(bufferHeight, bufferWidth, CV_8UC3, address, bytePerRow).clone();
    }
    
    //Since we clone the cv::Mat no need to keep the Buffer Locked while we work on it.
    CVPixelBufferUnlockBaseAddress(buffer, 0);
    
    cv::Mat rotated;
    transpose(converted, rotated);
    flip(rotated,rotated, 1);
    
    return rotated;
}

#pragma mark AVCaptureVideoDataOutputSampleBufferDelegate

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    
    [self processBuffer:sampleBuffer];
}

@end
