//
//  VROARImageTracker.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#if ENABLE_OPENCV

#include "VROARImageTracker.h"
#include <sys/time.h>
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include "VROMath.h"
#include "VROTriangle.h"
#include "VROLineSegment.h"


#define ENABLE_DETECT_LOGGING 1

#if ENABLE_DETECT_LOGGING && VRO_PLATFORM_IOS
    #define LOG_DETECT_TIME(message) pinfo("[Viro] [%ld ms] %@", getCurrentTimeMs() - _startTime, @#message);
#elif ENABLE_DETECT_LOGGING && VRO_PLATFORM_ANDROID
    #define LOG_DETECT_TIME(message) pinfo("[Viro] [%ld ms] %s", getCurrentTimeMs() - _startTime, #message);
#else
    #define LOG_DETECT_TIME(message) ((void)0);
#endif

std::shared_ptr<VROARImageTracker> VROARImageTracker::createARImageTracker(std::shared_ptr<VROARImageTarget> arImageTarget) {
    std::shared_ptr<VROARImageTracker> tracker = std::make_shared<VROARImageTracker>(VROARImageTrackerType::ORB4);
    tracker->addARImageTarget(arImageTarget);
    return tracker;
}

std::shared_ptr<VROARImageTracker> VROARImageTracker::createDefaultTracker() {
    return std::make_shared<VROARImageTracker>(VROARImageTrackerType::ORB4);
}

VROARImageTrackerOutput VROARImageTracker::createFalseOutput() {
    return {false};
}

VROARImageTracker::VROARImageTracker(VROARImageTrackerType type) :
    _type(type) {
    updateType();
    _totalTime = 0;
    _totalIteration = 0;
    _totalFailedTime = 0;
    _totalFailedIteration = 0;
}

void VROARImageTracker::setType(VROARImageTrackerType type) {
    if (_type == type) {
        return;
    }
    _type = type;
    updateType();
}

void VROARImageTracker::updateType() {
    switch(_type) {
        case VROARImageTrackerType::ORB4:
            
            _numberFeaturePoints = 3500;
            _minGoodMatches = 15;
            pinfo("[Viro] minGoodMatches: %f - average", _minGoodMatches);
            
            // original feature property:
            _feature = cv::ORB::create(500, 1.2f, 8, 31, 0, 4, cv::ORB::HARRIS_SCORE);

            
            // absurdly high, but accurate IIRC:
            //_feature = cv::ORB::create(5000, 1.1f, 20, 0, 0, 4, cv::ORB::HARRIS_SCORE);

            // current testing iPhone SE
            _feature = cv::ORB::create(_numberFeaturePoints, 1.1f, 12, 0, 0, 4, cv::ORB::HARRIS_SCORE);
            _targetFeature = cv::ORB::create(550, 1.2f, 12, 31, 0, 4, cv::ORB::HARRIS_SCORE);

            // current iPad Testing
            //_feature = cv::ORB::create(3000, 1.2f, 8, 31, 0, 4, cv::ORB::HARRIS_SCORE);
            //_targetFeature = cv::ORB::create(1000, 1.2f, 8, 31, 0, 4, cv::ORB::HARRIS_SCORE);
            
            _matcherType = cv::NORM_HAMMING2;
            break;
        case VROARImageTrackerType::ORB3:
            _feature = cv::ORB::create(500, 1.2f, 8, 31, 0, 3, cv::ORB::FAST_SCORE, 31);
            _matcherType = cv::NORM_HAMMING2;
            break;
        case VROARImageTrackerType::ORB:
            _feature = cv::ORB::create();
            _matcherType = cv::NORM_HAMMING;
            break;
        case VROARImageTrackerType::BRISK:
            _feature = cv::BRISK::create();
            _matcherType = cv::NORM_HAMMING;
            break;
    }
    
    _useBfKnnMatcher = _type != VROARImageTrackerType::BRISK;
    if (_useBfKnnMatcher) {
        _matcher = cv::BFMatcher::create();
    } else {
        _matcher = cv::BFMatcher::create(_matcherType, false);
    }

    // Since we may have changed the _feature/_matcherType, we need to recreate all the
    for (int i = 0; i < _arImageTargets.size(); i++) {
        _targetsMap[_arImageTargets[i]] = updateTargetInfo(_arImageTargets[i]);
    }
}

VROARImageTargetOpenCV VROARImageTracker::updateTargetInfo(std::shared_ptr<VROARImageTarget> arImageTarget) {
    
    std::vector<cv::KeyPoint> targetKeyPoints;
    cv::Mat targetDescriptors;
    
    // automatically run keypoint and descriptor extraction on the target image.
    // TODO: we can scale the target image lower to speed up processing, but that'll throw off the translation matrix (by the same factor)
    detectKeypointsAndDescriptors(arImageTarget->getTargetMat(), targetKeyPoints, targetDescriptors, true);

    return {arImageTarget, targetKeyPoints, targetDescriptors, cv::Mat(), cv::Mat()};
}

void VROARImageTracker::addARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget) {
    _arImageTargets.push_back(arImageTarget);
    _targetsMap[arImageTarget] = updateTargetInfo(arImageTarget);
}

void VROARImageTracker::removeARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget) {
    // TODO: fill this in
}

void VROARImageTracker::detectKeypointsAndDescriptors(cv::Mat inputImage,
                                                      std::vector<cv::KeyPoint> &keypoints,
                                                      cv::Mat &descriptors,
                                                      bool isTarget) {
    LOG_DETECT_TIME("start convert image to grayscale");

    // create empty mat for processing output
    cv::Mat processedImage = cv::Mat(inputImage.rows, inputImage.cols, CV_8UC1);

    switch(_type) {
        case VROARImageTrackerType::ORB:
            // convert the image to gray scale
            cv::cvtColor(inputImage, processedImage, cv::COLOR_RGB2GRAY);
            break;
        case VROARImageTrackerType::BRISK:
        default:
            // convert the image to gray scale
            cv::cvtColor(inputImage, processedImage, cv::COLOR_RGB2GRAY);
    }

    if (isTarget) {
        LOG_DETECT_TIME("start detect keypoints");
        _targetFeature->detect(processedImage, keypoints);
        LOG_DETECT_TIME("start compute descriptors");
        _targetFeature->compute(processedImage, keypoints, descriptors);
    } else {
        LOG_DETECT_TIME("start detect keypoints");
        _feature->detect(processedImage, keypoints);
        LOG_DETECT_TIME("start compute descriptors");
        _feature->compute(processedImage, keypoints, descriptors);
    }

    // Compute keypoints and descriptors all together.
//    LOG_DETECT_TIME("start detect and compute descriptors")
//    _feature->detectAndCompute(processedImage, cv::noArray(), keypoints, descriptors);
    LOG_DETECT_TIME("finish detect keypoints & descriptors");
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findTarget(cv::Mat inputImage, float* intrinsics, std::shared_ptr<VROARCamera> camera) {
    _intrinsics = intrinsics;
    _currentCamera = camera;
    std::vector<VROARImageTrackerOutput> outputs = findTargetInternal(inputImage);
    
    return outputs;
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findTarget(cv::Mat inputImage, float* intrinsics) {
    _intrinsics = intrinsics;
    _currentCamera = nullptr;
    std::vector<VROARImageTrackerOutput> outputs = findTargetInternal(inputImage);
    
    return outputs;
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findTargetInternal(cv::Mat inputImage) {

    // start the timer...
    _startTime = getCurrentTimeMs();

    pinfo("[Viro] raw input size is %d x %d", inputImage.rows, inputImage.cols);

    // TODO: scale input image for performance, not sure if we need this now, but the option is there.
    // you *might* need to also account for the scaling in the calculations later to extract position
    // although you may be fine because we overwrite inputImage w/ the scale image...
    bool shouldScaleInput = false;
    float xScale = 1; float yScale = 1;
    
    if (shouldScaleInput) {
        _startTime = getCurrentTimeMs();
        cv::Size newSize(inputImage.cols * xScale, inputImage.rows * yScale);
        cv::Mat resizedInput;
        cv::resize(inputImage, resizedInput, newSize);
        pinfo("[Viro] resized image size is %d x %d", resizedInput.rows, resizedInput.cols);

        inputImage = resizedInput;
    }
    
    // process the input image to extract keypoints and descriptors
    std::vector<cv::KeyPoint> inputKeyPoints;
    cv::Mat inputDescriptors;
    detectKeypointsAndDescriptors(inputImage, inputKeyPoints, inputDescriptors, false);

    // make sure we have some keypoints/descriptors.
    if (inputKeyPoints.size() == 0 || inputDescriptors.cols == 0) {
        pwarn("[Viro] Could not find keypoints and/or descriptors for the input image.");
        return {};
    }

    std::vector<VROARImageTrackerOutput> outputs = findMultipleTargetsBF(inputKeyPoints, inputDescriptors, inputImage);

    for (int i = 0; i < outputs.size(); i++) {
        VROARImageTrackerOutput output = outputs[i];
        // Since we scaled the input image, we need to revert that scale when we return the corners!
        if (output.found && shouldScaleInput) {
            for (int i = 0; i < output.corners.size(); i++) {
                output.corners[i] = cv::Point2f(output.corners[i].x / xScale, output.corners[i].y / yScale);
            }
        }
    }

   return outputs;
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findMultipleTargetsBF(std::vector<cv::KeyPoint> inputKeypoints,
                                                                                               cv::Mat inputDescriptors,  cv::Mat inputImage) {
    LOG_DETECT_TIME("Starting findMultipleTargetsBF.");
    
    std::vector<VROARImageTrackerOutput> outputs;
    
    VROARImageTargetOpenCV currentTarget;
    
    /* Get/calculate the Camera Intrinsic Matrix
     
     The intrinsic matrix looks like the following:
     [ f_x  0  x_0
     0  f_y y_0
     0   0   1 ]
     
     Where f_x and f_y are the focal length whereas x_0 and y_0 is the pixel from the top left representing the "center" of the camera.
     see http://ksimek.github.io/2013/08/13/intrinsic/
     */
    cv::Mat cameraMatrix;
    if (_intrinsics == NULL) {
        // There aren't any intrinsics set, so calculate them here...
        double focalLength = inputImage.cols; // Approximate focal length.
        cv::Point2d center = cv::Point2d(inputImage.cols / 2, inputImage.rows / 2);
        double cameraArr[9] = {focalLength, 0, center.x, 0, focalLength, center.y, 0, 0, 1};
        cameraMatrix = cv::Mat(3, 3, CV_64F, &cameraArr);
    } else {
        // There are intrinsics set, so don't calculate/estimate them!
        // actually, the intrinsics assume the texture/coordinates are in landscape, so we'll need to flip mat[0][2] and mat[1][2] for portrait.
        // TODO: dynamically handle screen rotation.
        cameraMatrix = cv::Mat(3, 3, CV_32F, _intrinsics);
        cameraMatrix = cameraMatrix.t(); // we need to transpose the matrix
        float temp = cameraMatrix.at<float>(0, 2);
        cameraMatrix.at<float>(0, 2) = cameraMatrix.at<float>(1, 2);
        cameraMatrix.at<float>(1, 2) = temp;
    }
    
    std::cout << "[Viro] Camera Intrinsic Matrix: " << cameraMatrix << std::endl;
    
    cv::Mat distCoeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assume no lens distortion
    
    
    // Start the loop over all the targets!
    for (int i = 0; i < _arImageTargets.size(); i++) {
        pinfo("[Viro] processing target #%d", i);
        
        currentTarget = _targetsMap.find(_arImageTargets[i])->second;
        
        LOG_DETECT_TIME("start matching keypoints");
        
        // we need at least 15 good matches before we continue - for 500 feature pts.
        // use the new calculate _minGoodMatches instead now.
        int minGoodMatches = _minGoodMatches; // used to be hard-coded to 15
        std::vector<cv::DMatch> goodMatches;
        
        pinfo("[Viro] processing %d target descriptors and %d input descriptors", currentTarget.descriptors.rows, inputDescriptors.rows);
        
        if (!_useBfKnnMatcher) {
            // BRISK logic
            std::vector<cv::DMatch> matches;
            _matcher->match(currentTarget.descriptors, inputDescriptors, matches);
            LOG_DETECT_TIME("start filtering good matches");
            
            double maxDist = 0;
            double minDist = 100;
            
            for (int i = 0; i < matches.size(); i++) {
                float dist = matches[i].distance;
                if (dist < minDist) {
                    minDist = dist;
                }
                if (dist > maxDist) {
                    maxDist = dist;
                }
            }
            
            double goodMatchThreshold = 3 * minDist; // TODO: make sure this is a legitimate value.
            
            for (int i = 0; i < matches.size(); i++) {
                if (matches[i].distance < goodMatchThreshold) {
                    goodMatches.push_back(matches[i]);
                }
            }
            
        } else {
            // ORB logic
            std::vector<std::vector<cv::DMatch>> matches;
            _matcher->knnMatch(currentTarget.descriptors, inputDescriptors, matches, 2);
            LOG_DETECT_TIME("start filtering good matches");
            
            for (int i = 0; i < matches.size(); i++) {
                if (matches[i][0].distance < (.80 * matches[i][1].distance)) {
                    goodMatches.push_back(matches[i][0]);
                }
            }
        }
        
        if (goodMatches.size() < minGoodMatches) {
            pinfo("[Viro] Could not find enough good matching points. %lu of %d", goodMatches.size(), minGoodMatches);
            
            _totalFailedIteration++;
            _totalFailedTime+=(getCurrentTimeMs() - _startTime);
            pinfo("[Viro] average failed run time %f for %f runs", _totalFailedTime / _totalFailedIteration, _totalFailedIteration);
            
            continue;
        }
        
        std::vector<cv::Point2f> objectPoints;
        std::vector<cv::Point2f> inputPoints;
        
        for( int i = 0; i < goodMatches.size(); i++ ) {
            objectPoints.push_back(currentTarget.keyPoints[goodMatches[i].queryIdx].pt);
            inputPoints.push_back(inputKeypoints[goodMatches[i].trainIdx].pt);
        }
        
        LOG_DETECT_TIME("start finding homography matrix");
        
        cv::Mat homographyMat = findHomography(cv::Mat(objectPoints), cv::Mat(inputPoints), CV_RANSAC, 5.0);
        
        if (homographyMat.cols == 0) {
            pinfo("[Viro] Could not find a homography matrix.");
            continue;
        }
        
        LOG_DETECT_TIME("start finding perspective transform");
        
        std::vector<cv::Point2f> objectCorners(4);
        objectCorners[0] = cvPoint(0,0);
        objectCorners[1] = cvPoint(currentTarget.arImageTarget->getTargetMat().cols, 0);
        objectCorners[2] = cvPoint(currentTarget.arImageTarget->getTargetMat().cols, currentTarget.arImageTarget->getTargetMat().rows);
        objectCorners[3] = cvPoint(0, currentTarget.arImageTarget->getTargetMat().rows);
        
        std::vector<cv::Point2f> inputCorners;
        perspectiveTransform(objectCorners, inputCorners, homographyMat);
        
        if (!areCornersValid(inputCorners)) {
            pinfo("[Viro] Could not find corners of target in input image.");
            
            _totalFailedIteration++;
            _totalFailedTime+=(getCurrentTimeMs() - _startTime);
            pinfo("[Viro] average failed run time %f for %f runs - bad corners", _totalFailedTime / _totalFailedIteration, _totalFailedIteration);
            
            continue;
        }
        
        LOG_DETECT_TIME("start finding object pose");
        
        // Output rotation and translation
        std::vector<cv::Point3d> targetCorners;
        
        // use the below corners to find the position of the "top left" corner of the target
        //    CvPoint3D32f topLeft = cvPoint3D32f(0, 0, 0);
        //    CvPoint3D32f topRight = cvPoint3D32f(currentTarget.arImageTarget.cols, 0, 0);
        //    CvPoint3D32f bottomRight = cvPoint3D32f(currentTarget.arImageTarget.cols, currentTarget.arImageTarget.rows, 0);
        //    CvPoint3D32f bottomLeft = cvPoint3D32f(0, currentTarget.arImageTarget.rows, 0);
        
        // define corners starting from the top left of the image
        CvPoint3D32f topLeft = cvPoint3D32f(- currentTarget.arImageTarget->getTargetMat().cols / 2, - currentTarget.arImageTarget->getTargetMat().rows / 2, 0);
        CvPoint3D32f topRight = cvPoint3D32f(currentTarget.arImageTarget->getTargetMat().cols / 2, - currentTarget.arImageTarget->getTargetMat().rows / 2, 0);
        CvPoint3D32f bottomRight = cvPoint3D32f(currentTarget.arImageTarget->getTargetMat().cols / 2, currentTarget.arImageTarget->getTargetMat().rows / 2, 0);
        CvPoint3D32f bottomLeft = cvPoint3D32f(- currentTarget.arImageTarget->getTargetMat().cols / 2, currentTarget.arImageTarget->getTargetMat().rows / 2, 0);
        
        switch(currentTarget.arImageTarget->getOrientation()) {
            case VROImageOrientation::Up:
                targetCorners = {topLeft, topRight, bottomRight, bottomLeft};
                break;
            case VROImageOrientation::Down:
                targetCorners = {bottomRight, bottomLeft, topLeft, topRight};
                break;
            case VROImageOrientation::Left:
                targetCorners = {bottomLeft, topLeft, topRight, bottomRight};
                break;
            case VROImageOrientation::Right:
                targetCorners = {topRight, bottomRight, bottomLeft, topLeft};
                break;
        }
        
        
        // -- Solve for pose --
        
        // TODO: if we've scaled the input, then inverse the scaling on the corner values before using solvePnP.
        // actually i dont think we need this as long as we overrode the inputImage?
        
        // whether or not to use the (previous) values in _rotation/_translation to help with extracting the next set of them.
        bool useExtrinsicGuess = true;
        
        //cv::solvePnP(targetCorners, inputCorners, cameraMatrix, distCoeffs, _rotation, _translation, useExtrinsicGuess);
        cv::solvePnPRansac(targetCorners, inputCorners, cameraMatrix, distCoeffs, currentTarget.rotation, currentTarget.translation, useExtrinsicGuess);

        LOG_DETECT_TIME("finished detection & pose extraction");
        
        _totalIteration++;
        _totalTime+=(getCurrentTimeMs() - _startTime);
        pinfo("[Viro] average run time %f for %f runs", _totalTime / _totalIteration, _totalIteration);
        
        // Calculate the pixels per meter based on the orientation, size of the target image (in pixels) and the given physical width.
        double pixPerMeter;
        switch (currentTarget.arImageTarget->getOrientation()) {
            case VROImageOrientation::Up:
            case VROImageOrientation::Down:
                pixPerMeter = currentTarget.arImageTarget->getTargetMat().cols / currentTarget.arImageTarget->getPhysicalWidth();
                break;
            case VROImageOrientation::Left:
            case VROImageOrientation::Right:
                pixPerMeter = currentTarget.arImageTarget->getTargetMat().rows / currentTarget.arImageTarget->getPhysicalWidth();
                break;
        }
        
        cv::Mat scaledTranslation(3, 1, cv::DataType<double>::type);
        scaledTranslation.at<double>(0,0) = currentTarget.translation.at<double>(0,0) / pixPerMeter;
        scaledTranslation.at<double>(1,0) = currentTarget.translation.at<double>(1,0) / pixPerMeter;
        scaledTranslation.at<double>(2,0) = currentTarget.translation.at<double>(2,0) / pixPerMeter;

#if ENABLE_DETECT_LOGGING
        for (int i = 0; i < inputCorners.size(); i++) {
            pinfo("[Viro] found corner %d point (before re-scaling): %f, %f", i, inputCorners[i].x, inputCorners[i].y);
        }
        
        pinfo("[Viro] translation: %f, %f, %f", scaledTranslation.at<double>(0,0), scaledTranslation.at<double>(1,0), scaledTranslation.at<double>(2,0));
        pinfo("[Viro] rotation: %f, %f, %f", toDegrees(currentTarget.rotation.at<double>(0,0)), toDegrees(currentTarget.rotation.at<double>(1,0)), toDegrees(currentTarget.rotation.at<double>(2,0)));
#endif
        
        VROVector3f outRotation;
        VROVector3f outTranslation;
        convertFromCVToViroAxes(scaledTranslation, currentTarget.rotation, outTranslation, outRotation);

        VROMatrix4f worldTransform = convertToWorldCoordinates(_currentCamera, outTranslation, outRotation);
        
        outputs.push_back({true, inputCorners, scaledTranslation, currentTarget.rotation, worldTransform, currentTarget.arImageTarget});
    }
    
    return outputs;
}

bool VROARImageTracker::areCornersValid(std::vector<cv::Point2f> corners) {
    
    // if we have less than 4 corners, we definitely did not find a rectangular object!
    if (corners.size() != 4) {
        return false;
    }
    
    /*
     This is a simple distance/sanity check between the corners. If the 4 corners have
     a combined x or y (absolute) distance of less than the minCornerDistance pixels, then
     throw away the result! This is true because in a 1920x1080 or 1280x720 our algorithm
     can't be that accurate!
     */
    int minCornerDistance = 50;
    double sumX = 0;
    double sumY = 0;
    for (int i = 0; i < corners.size() - 1; i++) {
        sumX += std::abs(corners[i].x - corners[i + 1].x);
        sumY += std::abs(corners[i].y - corners[i + 1].y);
    }
    
    if (sumX < minCornerDistance || sumY < minCornerDistance) {
        return false;
    }

    /*
     A rectangular object projected onto a 2d plane can't have any corner contained within
     the triangle formed by the other 3 corners
     */
    std::vector<VROVector3f> vectorCorners = {
        {corners[0].x, corners[0].y},
        {corners[1].x, corners[1].y},
        {corners[2].x, corners[2].y},
        {corners[3].x, corners[3].y}
    };

    for (int i = 0; i < 4; i++) {
        VROTriangle triangle = {vectorCorners[i], vectorCorners[(i + 1) % 4], vectorCorners[(i + 2) % 4]};
        if (triangle.containsPoint(vectorCorners[(i + 3) % 4])) {
            return false;
        }
    }
    
    /*
     Check that the lines forming the boundaries of the rectangle don't cross.
     */
    
    // the line segments are named assuming 1st corner is top left while moving clockwise.
    VROLineSegment lineSegmentTop(vectorCorners[0], vectorCorners[1]);
    VROLineSegment lineSegmentBottom(vectorCorners[2], vectorCorners[3]);
    
    VROLineSegment lineSegmentLeft(vectorCorners[3], vectorCorners[0]);
    VROLineSegment lineSegmentRight(vectorCorners[1], vectorCorners[2]);
    
    if (lineSegmentTop.intersectsSegment2D(lineSegmentBottom) || lineSegmentLeft.intersectsSegment2D(lineSegmentRight)) {
        return false;
    }
    
    // the corners passed all our checks!
    return true;
}

void VROARImageTracker::convertFromCVToViroAxes(cv::Mat inputTranslation, cv::Mat inputRotation, VROVector3f &outTranslation, VROVector3f &outRotation) {
    outTranslation.x = inputTranslation.at<double>(0, 0);
    outTranslation.y = - inputTranslation.at<double>(1, 0);
    outTranslation.z = - inputTranslation.at<double>(2, 0);
    
    outRotation.x = inputRotation.at<double>(0, 0);
    outRotation.y = - inputRotation.at<double>(1, 0);
    outRotation.z = - inputRotation.at<double>(2, 0);
}

VROMatrix4f VROARImageTracker::convertToWorldCoordinates(std::shared_ptr<VROARCamera> camera, VROVector3f translation, VROVector3f rotation) {
    if (!camera) {
        pinfo("[Viro] ARImageTracker - unable to convert to world coordinates with missing camera.");
        return VROMatrix4f();
    }

    VROMatrix4f camMatrix = VROMatrix4f();
    camMatrix.translate(camera->getPosition());
    camMatrix.rotate(camera->getRotation());

    VROMatrix4f inputTransform = VROMatrix4f();
    inputTransform.translate(translation);
    inputTransform.rotate(rotation);

    return camMatrix.multiply(inputTransform);
}

long VROARImageTracker::getCurrentTimeMs() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

#endif /* ENABLE_OPENCV */

