//
//  VROImageTracking.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include "VROImageTracker.h"
#include <sys/time.h>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define ENABLE_DETECT_LOGGING 1

#if ENABLE_DETECT_LOGGING && VRO_PLATFORM_IOS
    #define LOG_DETECT_TIME(message) pinfo("[Viro] [%ld ms] %@", getCurrentTimeMs() - _startTime, @#message);
#elif ENABLE_DETECT_LOGGING && VRO_PLATFORM_ANDROID
    #define LOG_DETECT_TIME(message) pinfo("[Viro] [%ld ms] %@", getCurrentTimeMs() - _startTime, #message);
#else
    #define LOG_DETECT_TIME(message) ((void)0);
#endif

std::shared_ptr<VROImageTracker> VROImageTracker::createImageTracker(cv::Mat image) {
    return std::shared_ptr<VROImageTracker>(new VROImageTracker(image, VROImageTrackerType::BRISK));
}

VROImageTracker::VROImageTracker(cv::Mat targetImage, VROImageTrackerType type) :
    _targetImage(targetImage),
    _type(type) {
    updateTargetInfo();
}

void VROImageTracker::setType(VROImageTrackerType type) {
    if (_type == type) {
        return;
    }
    _type = type;
    updateTargetInfo();
}

void VROImageTracker::updateTargetInfo() {
    // TODO: we can scale the target image lower to speed up processing, but that'll throw off the translation matrix (by the same factor)
    detectKeypointsAndDescriptors(_targetImage, _targetKeyPoints, _targetDescriptors);

    switch(_type) {
        case VROImageTrackerType::ORB3:
        case VROImageTrackerType::ORB4:
            _matcherType = cv::NORM_HAMMING2;
            break;
        case VROImageTrackerType::BRISK:
        case VROImageTrackerType::ORB:
        default:
            _matcherType = cv::NORM_HAMMING;
            break;
    }
}

void VROImageTracker::detectKeypointsAndDescriptors(cv::Mat inputImage,
                                                    std::vector<cv::KeyPoint> &keypoints,
                                                    cv::Mat &descriptors) {
    // create empty mat for processing output
    cv::Mat processedImage = cv::Mat(inputImage.rows, inputImage.cols, CV_8UC1);
    
    cv::Ptr<cv::Feature2D> feature;

    switch(_type) {
        case VROImageTrackerType::ORB:
            // convert the image to gray scale
            cv::cvtColor(inputImage, processedImage, cv::COLOR_RGB2GRAY);
            feature = cv::ORB::create();
            break;
        case VROImageTrackerType::BRISK:
        default:
            // convert the image to gray scale
            cv::cvtColor(inputImage, processedImage, cv::COLOR_RGB2GRAY);
            feature = cv::BRISK::create();
    }
    _startTime = getCurrentTimeMs();

    LOG_DETECT_TIME("start detect keypoints");
    feature->detect(processedImage, keypoints);
    LOG_DETECT_TIME("start compute descriptors");
    feature->compute(processedImage, keypoints, descriptors);
    LOG_DETECT_TIME("finish detect keypoints & descriptors");
}

std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTarget(cv::Mat inputImage) {

    std::vector<cv::KeyPoint> inputKeyPoints;
    cv::Mat inputDescriptors;

    pinfo("[Viro] raw input size is %d x %d", inputImage.rows, inputImage.cols);
    
    float xScale = .5; float yScale = .5;
    
    _startTime = getCurrentTimeMs();
    cv::Size quarterSize(inputImage.cols * xScale, inputImage.rows * yScale);
    cv::Mat resizedInput;
    cv::resize(inputImage, resizedInput, quarterSize);
    
    pinfo("[Viro] resized image size is %d x %d", resizedInput.rows, resizedInput.cols);

    detectKeypointsAndDescriptors(resizedInput, inputKeyPoints, inputDescriptors);

    std::shared_ptr<VROImageTrackerOutput> output = findTarget(inputKeyPoints, inputDescriptors);
    
    // Since we scaled the input image, we need to revert that scale when we return the corners!
    if (output->found) {
        for (int i = 0; i < output->corners.size(); i++) {
            output->corners[i] = cv::Point2f(output->corners[i].x / xScale, output->corners[i].y / yScale);
        }
    }

    return output;
}

std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTarget(std::vector<cv::KeyPoint> inputKeypoints, cv::Mat inputDescriptors) {
    if (inputKeypoints.size() == 0 || inputDescriptors.cols == 0) {
        pwarn("[Viro] Could not find keypoints and/or descriptors for the input image.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    // TODO: add other findTarget types
    switch (_type) {
        case VROImageTrackerType::BRISK:
        default:
            return findTargetBF(inputKeypoints, inputDescriptors);
    }
}

// TODO: split out pose estimation
std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTargetBF(std::vector<cv::KeyPoint> inputKeypoints, cv::Mat inputDescriptors) {
    std::vector<cv::Point2f> inputCorners;
    cv::Mat inputTranslation;
    cv::Mat inputRotation;
    
    LOG_DETECT_TIME("start matching keypoints");
    cv::Ptr<cv::BFMatcher> matcher = cv::BFMatcher::create(_matcherType, false);
    std::vector<cv::DMatch> matches;
    matcher->match(_targetDescriptors, inputDescriptors, matches);

    LOG_DETECT_TIME("start filtering good matches");
    
    int minGoodMatches = 5; // we need at least 5 good matches before we continue.
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
    std::vector<cv::DMatch> goodMatches;

    for (int i = 0; i < matches.size(); i++) {
        if (matches[i].distance < goodMatchThreshold) {
            goodMatches.push_back(matches[i]);
        }
    }
    
    if (goodMatches.size() < minGoodMatches) {
        pinfo("[Viro] Could not find enough good matching points.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    std::vector<cv::Point2f> objectPoints;
    std::vector<cv::Point2f> inputPoints;
    
    for( int i = 0; i < goodMatches.size(); i++ ) {
        objectPoints.push_back(_targetKeyPoints[goodMatches[i].queryIdx].pt);
        inputPoints.push_back(inputKeypoints[goodMatches[i].trainIdx].pt);
    }

    LOG_DETECT_TIME("start finding homography matrix");

    cv::Mat homographyMat = findHomography(cv::Mat(objectPoints), cv::Mat(inputPoints), CV_RANSAC, 5.0);

    if (homographyMat.cols == 0) {
        pinfo("[Viro] Could not find a homography matrix.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    LOG_DETECT_TIME("start finding perspective transform");

    std::vector<cv::Point2f> objectCorners(4);
    objectCorners[0] = cvPoint(0,0);
    objectCorners[1] = cvPoint(_targetImage.cols, 0);
    objectCorners[2] = cvPoint(_targetImage.cols, _targetImage.rows);
    objectCorners[3] = cvPoint(0, _targetImage.rows);
    
    perspectiveTransform(objectCorners, inputCorners, homographyMat);
    
    // filter out false positives.
    int minCornerDistance = 20;
    double sumX = 0;
    double sumY = 0;
    for (int i = 0; i < inputCorners.size() - 1; i++) {
        sumX += std::abs(inputCorners[i].x - inputCorners[i + 1].x);
        sumY += std::abs(inputCorners[i].y - inputCorners[i + 1].y);
    }

    if (sumX < minCornerDistance || sumY < minCornerDistance || inputCorners.size() < objectCorners.size()) {
        pinfo("[Viro] Could not find corners of target in input image.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    LOG_DETECT_TIME("start finding object pose");

    // TODO: either get actual camera calibration or simply use input image's size like we do below.
    double focalLength = _targetImage.cols; // Approximate focal length.
    cv::Point2d center = cv::Point2d(_targetImage.cols/2,_targetImage.rows/2);
    double cameraArr[3][3] = {{focalLength, 0, center.x},{0 , focalLength, center.y},{0,0,1}};
    cv::Mat cameraMatrix(3, 3, CV_32FC1, &cameraArr);
    cv::Mat distCoeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assuming no lens distortion

    // Output rotation and translation
    cv::Mat rotation_vector; // Rotation in axis-angle form
    cv::Mat translation_vector;

    std::vector<cv::Point3d> targetPoints;
    targetPoints.push_back(cvPoint3D32f(0, 0, 0));
    targetPoints.push_back(cvPoint3D32f(_targetImage.cols, 0, 0));
    targetPoints.push_back(cvPoint3D32f(_targetImage.cols, _targetImage.rows, 0));
    targetPoints.push_back(cvPoint3D32f(0, _targetImage.rows, 0));

    // Solve for pose
    cv::solvePnP(targetPoints, inputCorners, cameraMatrix, distCoeffs, inputRotation, inputTranslation);

    LOG_DETECT_TIME("finished detection & pose extraction");

#if ENABLE_DETECT_LOGGING
    for (int i = 0; i < inputCorners.size(); i++) {
        pinfo("[Viro] found corner point (before re-scaling): %f, %f", inputCorners[i].x, inputCorners[i].y);
    }
    pinfo("[Viro] translation: %f, %f, %f", inputTranslation.at<double>(0,0), inputTranslation.at<double>(1,0), inputTranslation.at<double>(2,0));
    pinfo("[Viro] rotation: %f, %f, %f", inputRotation.at<double>(0,0), inputRotation.at<double>(1,0), inputRotation.at<double>(2,0));
#endif

    // TODO: the inputTranslation is in pixel units, a conversion is necessary to real-world units (based on the target image)
    //       make sure to take into account the scale applied to the input image as that'll scale the translation by same amount

    return std::make_shared<VROImageTrackerOutput>(true, inputCorners, inputTranslation, inputRotation);
}

long VROImageTracker::getCurrentTimeMs() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

