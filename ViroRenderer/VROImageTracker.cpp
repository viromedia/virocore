//
//  VROImageTracking.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include "VROImageTracker.h"

#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"

std::shared_ptr<VROImageTracker> VROImageTracker::createImageTracker(cv::Mat image) {
    return std::shared_ptr<VROImageTracker>(new VROImageTracker(image));
}

VROImageTracker::VROImageTracker(cv::Mat targetImage) :
    _targetImage(targetImage) {
    _targetKeyPoints = std::vector<cv::KeyPoint>();
    _targetDescriptors = cv::Mat();

    detectKeypointsAndDescriptors(_targetImage, _targetKeyPoints, _targetDescriptors);
}

std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTarget(cv::Mat inputImage) {

    if (_targetKeyPoints.size() == 0 || _targetDescriptors.cols == 0) {
        pwarn("Could not find target keypoints and/or descriptors.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    std::vector<cv::Point2f> inputCorners;
    cv::Mat inputTranslation;
    cv::Mat inputRotation;

    std::vector<cv::KeyPoint> inputKeyPoints;
    cv::Mat inputDescriptor;

    detectKeypointsAndDescriptors(inputImage, inputKeyPoints, inputDescriptor);

    if (inputKeyPoints.size() == 0 || inputDescriptor.cols == 0) {
        pinfo("Could not find keypoints and/or descriptors for the input image.");
        return VROImageTrackerOutput::createFalseOutput();
    }
    cv::BFMatcher matcher = cv::BFMatcher();
    std::vector<cv::DMatch> matches;
    matcher.match(_targetDescriptors, inputDescriptor, matches);

    int minGoodMatches = 10; // we need at least 10 good matches before we continue.
    double maxDist = 0;
    double minDist = 100;

    for (int i = 0; i < _targetDescriptors.rows; i++) {
        double dist = matches[i].distance;
        if (dist < minDist) {
            minDist = dist;
        }
        if (dist > maxDist) {
            maxDist = dist;
        }
    }

    std::vector<cv::DMatch> goodMatches;

    for (int i = 0; i < _targetDescriptors.rows; i++) {
        if (matches[i].distance < 3 * minDist) {
            goodMatches.push_back(matches[i]);
        }
    }
    if (goodMatches.size() < minGoodMatches) {
        pinfo("Could not find enough good matching points.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    std::vector<cv::Point2f> objectPoints;
    std::vector<cv::Point2f> inputPoints;

    for( int i = 0; i < goodMatches.size(); i++ ) {
        objectPoints.push_back(_targetKeyPoints[goodMatches[i].queryIdx].pt);
        inputPoints.push_back(inputKeyPoints[goodMatches[i].trainIdx].pt);
    }

    cv::Mat homographyMat = findHomography(cv::Mat(objectPoints), cv::Mat(inputPoints), CV_RANSAC, 5.0);

    if (homographyMat.cols == 0) {
        pinfo("Could not find a homography matrix.");
        return VROImageTrackerOutput::createFalseOutput();
    }
    std::vector<cv::Point2f> objectCorners(4);
    objectCorners[0] = cvPoint(0,0);
    objectCorners[1] = cvPoint(_targetImage.cols, 0);
    objectCorners[2] = cvPoint(_targetImage.cols, _targetImage.rows);
    objectCorners[3] = cvPoint(0, _targetImage.rows);

    perspectiveTransform(objectCorners, inputCorners, homographyMat);

    if (inputCorners.size() < objectCorners.size()) {
        pinfo("Could not find corners of target in input image.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    // TODO: get the target image's 3D translation and rotation in the input image.
    return std::make_shared<VROImageTrackerOutput>(true, inputCorners, inputTranslation, inputRotation);
}

void VROImageTracker::detectKeypointsAndDescriptors(cv::Mat inputImage,
                                                    std::vector<cv::KeyPoint> &keypoints,
                                                    cv::Mat &descriptors) {
    cv::Ptr<cv::BRISK> feature = cv::BRISK::create();
    feature->detect(inputImage, keypoints);
    feature->compute(inputImage, keypoints, descriptors);
}




