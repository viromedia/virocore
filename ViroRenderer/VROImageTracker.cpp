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
#include <iostream>
#include "VROMath.h"


#define ENABLE_DETECT_LOGGING 1

#if ENABLE_DETECT_LOGGING && VRO_PLATFORM_IOS
    #define LOG_DETECT_TIME(message) pinfo("[Viro] [%ld ms] %@", getCurrentTimeMs() - _startTime, @#message);
#elif ENABLE_DETECT_LOGGING && VRO_PLATFORM_ANDROID
    #define LOG_DETECT_TIME(message) pinfo("[Viro] [%ld ms] %s", getCurrentTimeMs() - _startTime, #message);
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
    LOG_DETECT_TIME("start convert image to grayscale");

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

    LOG_DETECT_TIME("start detect keypoints");
    feature->detect(processedImage, keypoints);
    LOG_DETECT_TIME("start compute descriptors");
    feature->compute(processedImage, keypoints, descriptors);
    LOG_DETECT_TIME("finish detect keypoints & descriptors");
}

std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTarget(cv::Mat inputImage, float* intrinsics) {
    _intrinsics = intrinsics;

    // Set this to true, if we just want to test solvePnP
    bool testSolvePnp = false;
    if (testSolvePnp) {
        testSolvePnP();
        return VROImageTrackerOutput::createFalseOutput();
    }

    return findTargetInternal(inputImage);
}

std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTarget(cv::Mat inputImage) {
    _intrinsics = NULL;
    return findTargetInternal(inputImage);
}

std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTargetInternal(cv::Mat inputImage) {

    _startTime = getCurrentTimeMs();

    std::vector<cv::KeyPoint> inputKeyPoints;
    cv::Mat inputDescriptors;

    pinfo("[Viro] raw input size is %d x %d", inputImage.rows, inputImage.cols);
    
    float xScale = 1; float yScale = 1;
    
    _startTime = getCurrentTimeMs();
    cv::Size quarterSize(inputImage.cols * xScale, inputImage.rows * yScale);
    cv::Mat resizedInput;
    cv::resize(inputImage, resizedInput, quarterSize);
    
    pinfo("[Viro] resized image size is %d x %d", resizedInput.rows, resizedInput.cols);

    detectKeypointsAndDescriptors(resizedInput, inputKeyPoints, inputDescriptors);

    std::shared_ptr<VROImageTrackerOutput> output = findTarget(inputKeyPoints, inputDescriptors, inputImage);
    
    // Since we scaled the input image, we need to revert that scale when we return the corners!
    if (output->found) {
        for (int i = 0; i < output->corners.size(); i++) {
            output->corners[i] = cv::Point2f(output->corners[i].x / xScale, output->corners[i].y / yScale);
        }
    }

    return output;
}

std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTarget(std::vector<cv::KeyPoint> inputKeypoints, cv::Mat inputDescriptors, cv::Mat inputImage) {
    if (inputKeypoints.size() == 0 || inputDescriptors.cols == 0) {
        pwarn("[Viro] Could not find keypoints and/or descriptors for the input image.");
        return VROImageTrackerOutput::createFalseOutput();
    }

    // TODO: add other findTarget types
    switch (_type) {
        case VROImageTrackerType::BRISK:
        default:
            return findTargetBF(inputKeypoints, inputDescriptors, inputImage);
    }
}

// TODO: split out pose estimation
std::shared_ptr<VROImageTrackerOutput> VROImageTracker::findTargetBF(std::vector<cv::KeyPoint> inputKeypoints, cv::Mat inputDescriptors, cv::Mat inputImage) {

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
    
    std::vector<cv::Point2f> inputCorners;
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

    /* Get the Camera Intrinsic Matrix
     
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

    // Output rotation and translation
    std::vector<cv::Point3d> targetCorners;

    // use the below corners to find the "top left" corner
//    targetCorners.push_back(cvPoint3D32f(0, 0, 0));
//    targetCorners.push_back(cvPoint3D32f(_targetImage.cols, 0, 0));
//    targetCorners.push_back(cvPoint3D32f(_targetImage.cols, _targetImage.rows, 0));
//    targetCorners.push_back(cvPoint3D32f(0, _targetImage.rows, 0));

    // use the below corners to find the "center" of the image.
    targetCorners.push_back(cvPoint3D32f(- _targetImage.cols / 2, - _targetImage.rows / 2, 0));
    targetCorners.push_back(cvPoint3D32f(_targetImage.cols / 2, - _targetImage.rows / 2, 0));
    targetCorners.push_back(cvPoint3D32f(_targetImage.cols / 2, _targetImage.rows / 2, 0));
    targetCorners.push_back(cvPoint3D32f(- _targetImage.cols / 2, _targetImage.rows / 2, 0));
    
    // TODO: if we've scaled the input, then inverse the scaling on the corner values before using solvePnP.

    // Solve for pose
    bool useExtrinsicGuess = true;
    
    //cv::solvePnP(targetCorners, inputCorners, cameraMatrix, distCoeffs, _rotation, _translation, useExtrinsicGuess);
    cv::solvePnPRansac(targetCorners, inputCorners, cameraMatrix, distCoeffs, _rotation, _translation, useExtrinsicGuess);

    LOG_DETECT_TIME("finished detection & pose extraction");

    // This conversion rate is based on the fact that the target image (100 bill) is 505x1188 pixels
    // vs its real world size of 2.61x6.14 inches. TODO: take this as an argument.
    double pixPerMeter = 7617.58; // ben.jpg
    //double pixPerMeter = 16404.2; // wikipedia_qr.png (2.4 inches represents 1000 px)

    cv::Mat scaledTranslation(3, 1, cv::DataType<double>::type);
    scaledTranslation.at<double>(0,0) = _translation.at<double>(0,0) / pixPerMeter;
    scaledTranslation.at<double>(1,0) = _translation.at<double>(1,0) / pixPerMeter;
    scaledTranslation.at<double>(2,0) = _translation.at<double>(2,0) / pixPerMeter;
    
#if ENABLE_DETECT_LOGGING
    for (int i = 0; i < inputCorners.size(); i++) {
        pinfo("[Viro] found corner %d point (before re-scaling): %f, %f", i, inputCorners[i].x, inputCorners[i].y);
    }
    
    pinfo("[Viro] translation: %f, %f, %f", scaledTranslation.at<double>(0,0), scaledTranslation.at<double>(1,0), scaledTranslation.at<double>(2,0));
    pinfo("[Viro] rotation: %f, %f, %f", toDegrees(_rotation.at<double>(0,0)), toDegrees(_rotation.at<double>(1,0)), toDegrees(_rotation.at<double>(2,0)));
#endif

    return std::make_shared<VROImageTrackerOutput>(true, inputCorners, scaledTranslation, _rotation);
}

void VROImageTracker::testSolvePnP() {
    
    bool runFirstSample = false;
    bool runSecondSample = false;
    bool runThirdSample = true;
    
    cv::Mat cameraMatrix = cv::Mat(3, 3, CV_32F, _intrinsics);

    std::vector<cv::Point3f> targetCorners;
    targetCorners.push_back(cv::Point3f(0,0,0));
    targetCorners.push_back(cv::Point3f(_targetImage.cols, 0, 0));
    targetCorners.push_back(cv::Point3f(_targetImage.cols, _targetImage.rows, 0));
    targetCorners.push_back(cv::Point3f(0, _targetImage.rows, 0));
    
    cv::Mat distCoeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assuming no lens distortion

    cv::Mat inputTranslation(3, 1, cv::DataType<double>::type);
    cv::Mat inputRotation(3, 1, cv::DataType<double>::type);
    
    double pixPerMeter = 7617.58;

    // Updateable values
    
    std::vector<cv::Point2f> inputCorners(4);

    if (runFirstSample) {
        //cv::setIdentity(cameraMatrix);
        
        std::cout << "[Viro] Initial cameraMatrix: " << cameraMatrix << std::endl;

        
        pinfo("[Viro] Run #1 -------------------");
        inputCorners[0] = cvPoint(199,719);
        inputCorners[1] = cvPoint(506, 725);
        inputCorners[2] = cvPoint(509, 860);
        inputCorners[3] = cvPoint(190, 854);

        cv::solvePnP(targetCorners, inputCorners, cameraMatrix, distCoeffs, inputRotation, inputTranslation, false, cv::SOLVEPNP_EPNP);

        inputTranslation.at<double>(0,0) = inputTranslation.at<double>(0,0) / pixPerMeter;
        inputTranslation.at<double>(1,0) = inputTranslation.at<double>(1,0) / pixPerMeter;
        inputTranslation.at<double>(2,0) = inputTranslation.at<double>(2,0) / pixPerMeter;

        pinfo("[Viro] translation: %f, %f, %f", inputTranslation.at<double>(0,0), inputTranslation.at<double>(1,0), inputTranslation.at<double>(2,0));
        pinfo("[Viro] rotation: %f, %f, %f", inputRotation.at<double>(0,0), inputRotation.at<double>(1,0), inputRotation.at<double>(2,0));



        pinfo("[Viro] Run #2 -------------------");
        inputCorners[0] = cvPoint(299,719);
        inputCorners[1] = cvPoint(606, 725);
        inputCorners[2] = cvPoint(609, 860);
        inputCorners[3] = cvPoint(290, 854);

        cv::solvePnP(targetCorners, inputCorners, cameraMatrix, distCoeffs, inputRotation, inputTranslation, false, cv::SOLVEPNP_EPNP);

        inputTranslation.at<double>(0,0) = inputTranslation.at<double>(0,0) / pixPerMeter;
        inputTranslation.at<double>(1,0) = inputTranslation.at<double>(1,0) / pixPerMeter;
        inputTranslation.at<double>(2,0) = inputTranslation.at<double>(2,0) / pixPerMeter;

        pinfo("[Viro] translation: %f, %f, %f", inputTranslation.at<double>(0,0), inputTranslation.at<double>(1,0), inputTranslation.at<double>(2,0));
        pinfo("[Viro] rotation: %f, %f, %f", inputRotation.at<double>(0,0), inputRotation.at<double>(1,0), inputRotation.at<double>(2,0));




        pinfo("[Viro] Run #3 -------------------");
        inputCorners[0] = cvPoint(299,919);
        inputCorners[1] = cvPoint(606, 925);
        inputCorners[2] = cvPoint(609, 1060);
        inputCorners[3] = cvPoint(290, 1054);

        cv::solvePnP(targetCorners, inputCorners, cameraMatrix, distCoeffs, inputRotation, inputTranslation, false, cv::SOLVEPNP_EPNP);

        inputTranslation.at<double>(0,0) = inputTranslation.at<double>(0,0) / pixPerMeter;
        inputTranslation.at<double>(1,0) = inputTranslation.at<double>(1,0) / pixPerMeter;
        inputTranslation.at<double>(2,0) = inputTranslation.at<double>(2,0) / pixPerMeter;

        pinfo("[Viro] translation: %f, %f, %f", inputTranslation.at<double>(0,0), inputTranslation.at<double>(1,0), inputTranslation.at<double>(2,0));
        pinfo("[Viro] rotation: %f, %f, %f", inputRotation.at<double>(0,0), inputRotation.at<double>(1,0), inputRotation.at<double>(2,0));
    }
    
    if (runSecondSample) {
        std::vector<cv::Point3f> points;
        
        float x,y,z;
        x=.5;y=.5;z=-.5;
        points.push_back(cv::Point3f(x,y,z));
        x=.5;y=.5;z=.5;
        points.push_back(cv::Point3f(x,y,z));
        x=-.5;y=.5;z=.5;
        points.push_back(cv::Point3f(x,y,z));
        x=-.5;y=.5;z=-.5;
        points.push_back(cv::Point3f(x,y,z));
        x=.5;y=-.5;z=-.5;
        points.push_back(cv::Point3f(x,y,z));
        x=-.5;y=-.5;z=-.5;
        points.push_back(cv::Point3f(x,y,z));
        x=-.5;y=-.5;z=.5;
        points.push_back(cv::Point3f(x,y,z));

        std::vector<cv::Point2f> points2d;
        x=382;y=274;
        points2d.push_back(cv::Point2f(x,y));
        x=497;y=227;
        points2d.push_back(cv::Point2f(x,y));
        x=677;y=271;
        points2d.push_back(cv::Point2f(x,y));
        x=562;y=318;
        points2d.push_back(cv::Point2f(x,y));
        x=370;y=479;
        points2d.push_back(cv::Point2f(x,y));
        x=550;y=523;
        points2d.push_back(cv::Point2f(x,y));
        x=666;y=475;
        points2d.push_back(cv::Point2f(x,y));
        
        cv::setIdentity(cameraMatrix);

        cv::solvePnP(points, points2d, cameraMatrix, distCoeffs, inputRotation, inputTranslation, false, cv::SOLVEPNP_EPNP);
        
        pinfo("[Viro] translation: %f, %f, %f", inputTranslation.at<double>(0,0), inputTranslation.at<double>(1,0), inputTranslation.at<double>(2,0));
        pinfo("[Viro] rotation: %f, %f, %f", inputRotation.at<double>(0,0), inputRotation.at<double>(1,0), inputRotation.at<double>(2,0));
    }
    
    if (runThirdSample) {
        std::vector<cv::Point2f> points1, points2;
        
        //First point's set
        points1.push_back(cv::Point2f(384.3331f,  162.23618f));
        points1.push_back(cv::Point2f(385.27521f, 135.21503f));
        points1.push_back(cv::Point2f(409.36746f, 139.30315f));
        points1.push_back(cv::Point2f(407.43854f, 165.64435f));
        
        //Second point's set
        points2.push_back(cv::Point2f(427.64938f, 158.77661f));
        points2.push_back(cv::Point2f(428.79471f, 131.60953f));
        points2.push_back(cv::Point2f(454.04532f, 134.97353f));
        points2.push_back(cv::Point2f(452.23096f, 162.13156f));
        
        //Real object point's set
        std::vector<cv::Point3f> object;
        object.push_back(cv::Point3f(-88.0f,88.0f,0));
        object.push_back(cv::Point3f(-88.0f,-88.0f,0));
        object.push_back(cv::Point3f(88.0f,-88.0f,0));
        object.push_back(cv::Point3f(88.0f,88.0f,0));
        
        //Camera matrix
        cv::Mat cam_matrix = cv::Mat(3,3,CV_32FC1,cv::Scalar::all(0));
        cam_matrix.at<float>(0,0) = 519.0f;
        cam_matrix.at<float>(0,2) = 320.0f;
        cam_matrix.at<float>(1,1) = 522.0f;
        cam_matrix.at<float>(1,2) = 240.0f;
        cam_matrix.at<float>(2,2) = 1.0f;
        
        //PnP
        cv::Mat rvec1i,rvec2i,tvec1i,tvec2i;
        cv::Mat rvec1p,rvec2p,tvec1p,tvec2p;
        solvePnP(cv::Mat(object),cv::Mat(points1),cam_matrix,cv::Mat(),rvec1i,tvec1i,false,CV_ITERATIVE);
        solvePnP(cv::Mat(object),cv::Mat(points2),cam_matrix,cv::Mat(),rvec2i,tvec2i,false,CV_ITERATIVE);
        solvePnP(cv::Mat(object),cv::Mat(points1),cam_matrix,cv::Mat(),rvec1p,tvec1p,false,CV_P3P);
        solvePnP(cv::Mat(object),cv::Mat(points2),cam_matrix,cv::Mat(),rvec2p,tvec2p,false,CV_P3P);
        
        //Print result
        std::cout<<"Iterative: "<<std::endl;
        std::cout<<" rvec1 "<<std::endl<<" "<<rvec1i<<std::endl<<std::endl;
        std::cout<<" rvec2 "<<std::endl<<" "<<rvec2i<<std::endl<<std::endl;
        std::cout<<" tvec1 "<<std::endl<<" "<<tvec1i<<std::endl<<std::endl;
        std::cout<<" tvec1 "<<std::endl<<" "<<tvec2i<<std::endl<<std::endl;
        
        std::cout<<"P3P: "<<std::endl;
        std::cout<<" rvec1 "<<std::endl<<" "<<rvec1p<<std::endl<<std::endl;
        std::cout<<" rvec2 "<<std::endl<<" "<<rvec2p<<std::endl<<std::endl;
        std::cout<<" tvec1 "<<std::endl<<" "<<tvec1p<<std::endl<<std::endl;
        std::cout<<" tvec1 "<<std::endl<<" "<<tvec2p<<std::endl<<std::endl;
    }
}

long VROImageTracker::getCurrentTimeMs() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

