//
//  VROTrackingHelper.m
//  ViroRenderer
//
//  Created by Andy Chu on 6/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#if ENABLE_OPENCV

#import "VROTrackingHelper.h"
#import "VROARImageTracker.h"
#import "opencv2/imgcodecs/ios.h"
#import "opencv2/imgproc/imgproc.hpp"

@interface VROTrackingHelperOutput() {
    VROARImageTrackerOutput _output;
}

@property (nonatomic, strong) UIImage *outputImage;

@end

@implementation VROTrackingHelperOutput

- (instancetype) initWithTrackerOutput:(VROARImageTrackerOutput)output withImage:(UIImage *)outputImage {
    self = [super init];
    if (self) {
        _output = output;
        _outputImage = outputImage;
    }
    return self;
}

- (VROARImageTrackerOutput)getImageTrackerOutput {
    return _output;
}

- (UIImage *)getOutputImage {
    return _outputImage;
}

@end

@interface VROTrackingHelper () {
    std::shared_ptr<VROARImageTracker> _tracker;
}

@property (nonatomic, assign) BOOL ready;
@property (nonatomic, assign) NSInteger count;
@property (nonatomic, assign) float *intrinsics;
@property (nonatomic, assign) BOOL shouldTrack;

/*
 Turning this on saves the image with the corners drawn to the camera roll and only runs
 the algorithm once (was useful back then before we added the preview UIImageView).
 */
@property (nonatomic, assign) BOOL writeResultToCameraRoll;

@end

@implementation VROTrackingHelper

- (instancetype)init {
    self = [super init];
    if (self) {
        _ready = YES;
        _count = 0;
        _intrinsics = NULL;
        _shouldTrack = YES; // if this is initialized to NO, then tracking won't turn on

        _writeResultToCameraRoll = NO; // should prob be NO for now.
    }
    return self;
}

- (void)findInScreenshot {
    UIImage *screenshot = [UIImage imageNamed:@"screenshot.png"];
    cv::Mat screnshotMat = cv::Mat();
    UIImageToMat(screenshot, screnshotMat);

    std::shared_ptr<VROARImageTracker> tracker = VROARImageTracker::createARImageTracker([self getBenTarget]);
    
    std::vector<VROARImageTrackerOutput> outputs = tracker->findTarget(screnshotMat, NULL);
    if (outputs.size() > 0) {
        VROARImageTrackerOutput output = outputs[0];
        std::vector<cv::Point2f> corners = output.corners;
        for (int i = 0; i < corners.size(); i++) {
            cv::Point2f point = corners[i];
            pinfo("findInScreenshot output corner %f", point.x);
            pinfo("findInScreenshot output corner %f", point.y);
        }
        pinfo("findInScreenshot output position %f, %f, %f", output.translation.at<double>(0,0), - output.translation.at<double>(1,0), - output.translation.at<double>(2,0));
        pinfo("findInScreenshot output rotation %f, %f, %f", output.rotation.at<double>(0,0), - output.rotation.at<double>(1,0), - output.rotation.at<double>(2,0));
    } else {
        pinfo("findInScreenshot output not found");
    }
}

- (std::shared_ptr<VROARImageTarget>)getBenTarget {
    UIImage *targetImage = [UIImage imageNamed:@"ben.jpg"];
    cv::Mat targetMat = cv::Mat();
    UIImageToMat(targetImage, targetMat); // should give us a RGB Mat.
    
    // width of a US bill is 6.14 inches ~= .156 meters
    std::shared_ptr<VROARImageTarget> arImageTarget = std::make_shared<VROARImageTarget>(VROImageOrientation::Up, .156);
    arImageTarget->setTargetMat(targetMat);

    return arImageTarget;
}

- (std::shared_ptr<VROARImageTarget>)getVarietyTarget {
    UIImage *targetImage = [UIImage imageNamed:@"variety_magazine.jpg"];
    cv::Mat targetMat = cv::Mat();
    UIImageToMat(targetImage, targetMat); // should give us a RGB Mat.
    
    std::shared_ptr<VROARImageTarget> arImageTarget = std::make_shared<VROARImageTarget>(VROImageOrientation::Up, .19);
    arImageTarget->setTargetMat(targetMat);
    
    return arImageTarget;
}

- (std::shared_ptr<VROARImageTarget>)getQRCodeTarget {
    UIImage *targetImage = [UIImage imageNamed:@"wikipedia_qr.jpg"];
    cv::Mat targetMat = cv::Mat();
    UIImageToMat(targetImage, targetMat); // should give us a RGB Mat.
    
    // the printed QR code is 0.06096 meters wide
    std::shared_ptr<VROARImageTarget> arImageTarget = std::make_shared<VROARImageTarget>(VROImageOrientation::Up, 0.06096);
    arImageTarget->setTargetMat(targetMat);
    
    return arImageTarget;
}

- (void)toggleTracking:(BOOL)tracking {
    NSLog(@"[Viro] setting tracking %@", tracking ? @"on" : @"off");
    _shouldTrack = tracking;
}

- (BOOL)toggleTracking {
    _shouldTrack = !_shouldTrack;
    NSLog(@"[Viro] setting tracking %@", _shouldTrack ? @"on" : @"off");
    return _shouldTrack;
}

- (void)setIntrinsics:(float *)intrinsics {
    _intrinsics = intrinsics;
}

- (VROARImageTrackerOutput)runTracking:(cv::Mat)cameraInput
                                camera:(std::shared_ptr<VROARCamera>)camera {

    if (!_tracker) {
        // initialize the tracker
        _tracker = VROARImageTracker::createARImageTracker([self getVarietyTarget]);
        //_tracker = VROARImageTracker::createARImageTracker([self getQRCodeTarget]);
    }

    // find the target in the given cameraInput (should already be in RGB format).
    VROARImageTrackerOutput output = {false, };
    std::vector<VROARImageTrackerOutput> outputs = _tracker->findTarget(cameraInput, _intrinsics, camera);
    
    if (outputs.size() > 0) {
        output = outputs[0]; // grab the first one because we only have 1 target here
    }
    
    if (!output.found) {
        NSLog(@"VROTrackingHelper, couldn't find target in given image");
    } else {
        // check if we should write to photo album...
        if (_writeResultToCameraRoll) {
            UIImage *outputImage = MatToUIImage(output.outputImage);
            UIImageWriteToSavedPhotosAlbum(outputImage, nil, nil, nil);
        }
    }
    
    @synchronized (self) {
        // if we're not writing to the camera roll, then we're ready once we're done processing a frame
        // otherwise, we'll only run until the first time we "found" the target, then leave _ready = NO
        // so we don't keep running and creating more and more images in the camera roll. Normally this
        // should always set _ready = YES to process the next frame.
        if (!_writeResultToCameraRoll || !output.found) {
            _ready = YES;
        }
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

    [self processPixelBufferRef:pixelBuffer forceRun:YES camera:nullptr completion:nil];
}

// this function is called by forceRun = true if the input is from camera, or false if from AR.
- (void)processPixelBufferRef:(CVPixelBufferRef)pixelBuffer
                     forceRun:(BOOL)forceRun
                       camera:(std::shared_ptr<VROARCamera>)camera
                   completion:(void (^)(VROTrackingHelperOutput *output))completionHandler {
    if (!_shouldTrack) {
        return;
    }

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
    // the low level camera notification API's notify on the background thread). - actually it seems to be okay on "high"
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
        VROARImageTrackerOutput output = [self runTracking:rgbImage camera:camera];
        if (_shouldTrack && completionHandler) {
            completionHandler([[VROTrackingHelperOutput alloc] initWithTrackerOutput:output withImage:MatToUIImage(output.outputImage)]);
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

#endif /* ENABLE_OPENCV */
