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
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <iostream>
#include "VROMath.h"
#include "VROTriangle.h"
#include "VROLineSegment.h"
#include "VROPlatformUtil.h"
#include "VROStringUtil.h"


#define ENABLE_DETECT_LOGGING 1
// whether or not to draw output corners for debugging
#define DRAW_OUTPUT_CORNERS 0

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

    _distortionCoeffs = getDistortionCoeffs();

    // Calibration properties
    _needsCalibration = false;
    _numCalibrationSamples = 20;
    _calibrationFrameCount = 0;
    _calibrationFoundCount = 0;
}

void VROARImageTracker::setType(VROARImageTrackerType type) {
    if (_type == type) {
        return;
    }
    _type = type;
    updateType();
}

void VROARImageTracker::computeIntrinsicMatrix(int width, int height) {
    _intrinsicMatrix = getIntrinsicMatrix(width, height);
}

void VROARImageTracker::updateType() {
    switch(_type) {
        case VROARImageTrackerType::ORB4:

            // original feature property:
            //_feature = cv::ORB::create(500, 1.2f, 8, 31, 0, 4, cv::ORB::HARRIS_SCORE);

#if VRO_PLATFORM_IOS
            // current values on iPhone
            _numberFeaturePoints = 2500;
            _minGoodMatches = 15;
            _feature = cv::ORB::create(_numberFeaturePoints, 1.1f, 12, 0, 0, 4, cv::ORB::HARRIS_SCORE);
            _targetFeature = cv::ORB::create(700, 1.2f, 8, 31, 0, 4, cv::ORB::HARRIS_SCORE);
            // used for BruteForce knnMatching w/ ORB descriptors (higher means looser)
            _matchRatio = .80;
#else // VRO_PLATFORM_ANDROID
            // current testing on Android
            _numberFeaturePoints = 3000; // 2k is enough for S8+
            _minGoodMatches = 15;
            // TODO: the feature extractor for the input image should adapt to target image size
            // the thing is we limit it to 1920x1080 pixels, but the input could be smaller too...
            _feature = cv::ORB::create(_numberFeaturePoints, 1.2f, 10, 0, 0, 4, cv::ORB::HARRIS_SCORE);
            // TODO: the feature extractor for target image should adapt to target image size.
            _targetFeature = cv::ORB::create(500, 1.2f, 8, 0, 0, 4, cv::ORB::HARRIS_SCORE);
            // used for BruteForce knnMatching w/ ORB descriptors (higher means looser)
            _matchRatio = .80;
#endif
            // current iPad Testing
            //_feature = cv::ORB::create(3000, 1.2f, 8, 31, 0, 4, cv::ORB::HARRIS_SCORE);
            //_targetFeature = cv::ORB::create(1000, 1.2f, 8, 31, 0, 4, cv::ORB::HARRIS_SCORE);
            
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
        _targetToTargetMap[_arImageTargets[i]] = updateTargetInfo(_arImageTargets[i]);
    }
}

VROARImageTargetOpenCV VROARImageTracker::updateTargetInfo(std::shared_ptr<VROARImageTarget> arImageTarget) {
    
    std::vector<cv::KeyPoint> targetKeyPoints;
    cv::Mat targetDescriptors;
    
    // automatically run keypoint and descriptor extraction on the target image.
    detectKeypointsAndDescriptors(arImageTarget->getTargetMat(), targetKeyPoints, targetDescriptors, true);

    return {arImageTarget, targetKeyPoints, targetDescriptors, cv::Mat(), cv::Mat(), {}, createFalseOutput(), false};
}

void VROARImageTracker::addARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget) {
    _arImageTargets.push_back(arImageTarget);
    _targetToTargetMap[arImageTarget] = updateTargetInfo(arImageTarget);
}

void VROARImageTracker::removeARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget) {
    _arImageTargets.erase(std::remove_if(_arImageTargets.begin(), _arImageTargets.end(),
                                         [arImageTarget](std::shared_ptr<VROARImageTarget> candidate) {
                                             return candidate == arImageTarget;
                                         }), _arImageTargets.end());

    auto it = _targetToTargetMap.find(arImageTarget);
    if (it != _targetToTargetMap.end()) {
        _targetToTargetMap.erase(it);
    }
}

void VROARImageTracker::detectKeypointsAndDescriptors(cv::Mat inputImage,
                                                      std::vector<cv::KeyPoint> &keypoints,
                                                      cv::Mat &descriptors,
                                                      bool isTarget) {
    LOG_DETECT_TIME("start convert image to grayscale");

    // create empty mat for processing output
    cv::Mat processedImage = cv::Mat(inputImage.rows, inputImage.cols, CV_8UC1);

    // convert to gray-scale
    cv::cvtColor(inputImage, processedImage, cv::COLOR_RGB2GRAY);

    // Compute keypoints and descriptors all together.
    LOG_DETECT_TIME("start detect and compute descriptors")
    if (isTarget) {
        _targetFeature->detectAndCompute(processedImage, cv::noArray(), keypoints, descriptors);

    } else {
        _feature->detectAndCompute(processedImage, cv::noArray(), keypoints, descriptors);

    }
    LOG_DETECT_TIME("finish detect keypoints & descriptors");
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findTarget(cv::Mat inputImage, float* intrinsics,
                                                                   std::shared_ptr<VROARCamera> camera) {
    _intrinsics = intrinsics;
#if VRO_PLATFORM_IOS
    // on iOS, we need to extract the intrinsic matrix from the array set above, so call it here. We also
    // don't care about width/height because we don't need to compute, just convert from array to cv::Mat
    computeIntrinsicMatrix(0, 0);
#endif
    
    _currentCamera = camera;
    std::vector<VROARImageTrackerOutput> outputs = findTargetInternal(inputImage);

    // Process outputs before determining if our outputs are good
    outputs = processOutputs(outputs);
    
    return outputs;
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findTarget(cv::Mat inputImage, float* intrinsics) {
    return findTarget(inputImage, intrinsics, nullptr);
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findTargetInternal(cv::Mat inputImage) {
    if (_needsCalibration) {
        findChessboardForCalibration(inputImage);
    }

    // start the timer...
    _startTime = getCurrentTimeMs();

    pinfo("[Viro] raw input size is %d x %d", inputImage.rows, inputImage.cols);
    cv::Size size = inputImage.size();
    pinfo("raw input size is %d %d", size.height, size.width);

    // Scale image for performance (esp on devices w/ larger screens)
    float scaleFactor = getScaleFactor(inputImage.rows, inputImage.cols);
    bool shouldScaleInput = scaleFactor != 1.0;


    if (shouldScaleInput) {
        _startTime = getCurrentTimeMs();
        cv::Size newSize(inputImage.cols * scaleFactor, inputImage.rows * scaleFactor);

        cv::Mat resizedInput;
        cv::resize(inputImage, resizedInput, newSize);

        inputImage = resizedInput;
    }

    // TODO: consider sharpening image w/ Gaussian blur (b/c ARCore doesn't have autofocus). Don't
    // forget to replace the imageImage below with sharpImage or sharperImage then.
    // TODO: consider performing basic edge detection to outline edges?
//    cv::Mat sharpImage;
//    cv::GaussianBlur(inputImage, sharpImage, cv::Size(0, 0), 3);
//    cv::addWeighted(inputImage, 1.5, sharpImage, -0.5, 0, sharpImage);
//
//    cv::Mat sharperImage;
//    cv::GaussianBlur(inputImage, sharperImage, cv::Size(0, 0), 6);
//    cv::addWeighted(inputImage, 1.5, sharperImage, -0.5, 0, sharperImage);
    
    // process the input image to extract keypoints and descriptors
    std::vector<cv::KeyPoint> inputKeyPoints;
    cv::Mat inputDescriptors;
    detectKeypointsAndDescriptors(inputImage, inputKeyPoints, inputDescriptors, false);

    // make sure we have some keypoints/descriptors.
    if (inputKeyPoints.size() == 0 || inputDescriptors.cols == 0) {
        pwarn("[Viro] Could not find keypoints and/or descriptors for the input image.");
        return {};
    }

    std::vector<VROARImageTrackerOutput> outputs = findMultipleTargetsBF(inputKeyPoints, inputDescriptors,
                                                                         inputImage, scaleFactor);

    for (int i = 0; i < outputs.size(); i++) {
        VROARImageTrackerOutput output = outputs[i];
        // Since we scaled the input image, we need to revert that scale when we return the corners!
        if (output.found && shouldScaleInput) {
            for (int i = 0; i < output.corners.size(); i++) {
                output.corners[i] = cv::Point2f(output.corners[i].x / scaleFactor, output.corners[i].y / scaleFactor);
            }
        }
    }

#if VRO_PLATFORM_ANDROID
    // Write the file to disk on Android (we'll pass the filepath later).
    if (outputs.size() > 0 && outputs[0].found) {
        std::ostringstream s;
        s << VROPlatformGetCacheDirectory() << "/viro_tracking_output.png";
        std::string filepath(s.str());
        bool success = cv::imwrite(filepath, outputs[0].outputImage);
        VROPlatformSetTrackingImageView(filepath);
    }
#endif

   return outputs;
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::findMultipleTargetsBF(std::vector<cv::KeyPoint> inputKeypoints,
                                                                              cv::Mat inputDescriptors,  cv::Mat inputImage,
                                                                              float scaleFactor) {
    LOG_DETECT_TIME("Starting findMultipleTargetsBF.");
    
    std::vector<VROARImageTrackerOutput> outputs;
    
    VROARImageTargetOpenCV currentTarget;

    // Start the loop over all the targets!
    for (int targetIndex = 0; targetIndex < _arImageTargets.size(); targetIndex++) {
        pinfo("[Viro] processing target #%d", targetIndex);

        currentTarget = _targetToTargetMap.find(_arImageTargets[targetIndex])->second;
        
        if (currentTarget.disableTracking) {
            pinfo("[Viro] target is disabled, skipping this target");
            continue;
        }
        
        LOG_DETECT_TIME("start matching keypoints");
        
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

            // TODO: make sure this is a legitimate value. (this is for BRISK which we dont use)
            double goodMatchThreshold = 3 * minDist;
            
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
                // the higher the constant, the looser the criteria is for a "match"
                if ( (matches[i][0].distance / matches[i][1].distance) < _matchRatio) {
                    goodMatches.push_back(matches[i][0]);
                }
            }
        }
        
        if (goodMatches.size() < _minGoodMatches) {
            pinfo("[Viro] Could not find enough good matching points. %lu of %d", goodMatches.size(),_minGoodMatches);
            
            _totalFailedIteration++;
            _totalFailedTime+=(getCurrentTimeMs() - _startTime);
            pinfo("[Viro] average failed run time %f for %f runs - not enough matching pts",
                  _totalFailedTime / _totalFailedIteration, _totalFailedIteration);
            
            continue;
        }
        
        std::vector<cv::Point2f> objectPoints;
        std::vector<cv::Point2f> inputPoints;
        
        for( int i = 0; i < goodMatches.size(); i++ ) {
            objectPoints.push_back(currentTarget.keyPoints[goodMatches[i].queryIdx].pt);
            inputPoints.push_back(inputKeypoints[goodMatches[i].trainIdx].pt);
        }
        
        LOG_DETECT_TIME("start finding homography matrix");
        
        cv::Mat homographyMat = findHomography(cv::Mat(objectPoints), cv::Mat(inputPoints), CV_RANSAC, 3);
        
        if (homographyMat.cols == 0) {
            pinfo("[Viro] Could not find a homography matrix.");
            continue;
        }
        
        LOG_DETECT_TIME("start finding corners");
        
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

        if (scaleFactor != 1.0) {
            inputCorners[0] /= scaleFactor ;
            inputCorners[1] /= scaleFactor;
            inputCorners[2] /= scaleFactor;
            inputCorners[3] /= scaleFactor;
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

        // whether or not to use the (previous) values in _rotation/_translation to help with extracting the next set of them.
        bool useExtrinsicGuess = currentTarget.rotation.rows != 0; // the first run rotation would be empty

        cv::solvePnP(targetCorners, inputCorners,
                     _intrinsicMatrix, _distortionCoeffs,
                     currentTarget.rotation, currentTarget.translation, useExtrinsicGuess);

        //cv::solvePnPRansac(targetCorners, inputCorners, _intrinsicMatrix, _distortionCoeffs,
        //                   currentTarget.rotation, currentTarget.translation, useExtrinsicGuess);

        LOG_DETECT_TIME("finished detection & pose extraction");

        // TODO: remove this average run time logic
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

        VROVector3f outRotation;
        VROVector3f outTranslation;
        convertFromCVToViroAxes(scaledTranslation, currentTarget.rotation, outTranslation, outRotation);

        VROMatrix4f worldTransform = convertToWorldCoordinates(_currentCamera, outTranslation, outRotation);

        // OpenCV targets are assumed to be in the X-Y plane whereas ARKit returns in the X-Z plane
        // so apply a -90 degree rotation about the X-axis. (we need to apply the transform).
        // TODO: maybe we can account for this by changing how we define the CvPoint3D32f corners
        // points in the lines above.
        VROMatrix4f rotMatrix;
        rotMatrix.rotateX(-M_PI_2);
        worldTransform = worldTransform.multiply(rotMatrix);

#if ENABLE_DETECT_LOGGING
        for (int i = 0; i < inputCorners.size(); i++) {
            pinfo("[Viro] found corner %d point (before re-scaling): %f, %f", i, inputCorners[i].x, inputCorners[i].y);
        }
        
        VROVector3f tempTrans = worldTransform.extractTranslation();
        VROVector3f tempRot = worldTransform.extractRotation(worldTransform.extractScale()).toEuler();
        
        pinfo("[Viro] translation: %f, %f, %f", tempTrans.x, tempTrans.y, tempTrans.z);
        pinfo("[Viro] rotation: %f, %f, %f", toDegrees(tempRot.x), toDegrees(tempRot.y), toDegrees(tempRot.z));
#endif

        VROARImageTrackerOutput output = {true, inputCorners, scaledTranslation, currentTarget.rotation, worldTransform, currentTarget.arImageTarget};
        
// Whether or not we should draw corners on the output image. (for debugging)
#if DRAW_OUTPUT_CORNERS
        output.outputImage = drawCorners(inputImage, inputCorners, scaleFactor);
#endif
        
        // finally add the output to the list of outputs to return
        outputs.push_back(output);
    }
    
    return outputs;
}

std::vector<VROARImageTrackerOutput> VROARImageTracker::processOutputs(std::vector<VROARImageTrackerOutput> rawOutputs) {
    LOG_DETECT_TIME("begin process outputs!");
    std::vector<VROARImageTrackerOutput> newOutputs;
    
    for (int i = 0; i < rawOutputs.size(); i++) {
        VROARImageTrackerOutput output = rawOutputs[i];
        if (output.found) {
            VROARImageTrackerOutput realOutput = determineFoundOrUpdate(output);
            if (realOutput.found) {
                newOutputs.push_back(realOutput);
            }
        }
    }

    return newOutputs;
}

VROARImageTrackerOutput VROARImageTracker::determineFoundOrUpdate(VROARImageTrackerOutput output) {
    return determineFoundOrUpdateV2(output);
}

/*
 This version of determineFoundOrUpdate is the default case where we simply return the rawOutput and
 disable tracking after finding 5 results. This is not that good because while we run pretty fast,
 sometimes the rotations/position are bad.
 */
VROARImageTrackerOutput VROARImageTracker::determineFoundOrUpdateV1(VROARImageTrackerOutput rawOutput) {
    if (rawOutput.found) {
        VROARImageTargetOpenCV *targetOpenCV = &_targetToTargetMap.find(rawOutput.target)->second;

        // if the previous last output was "found" then this is an update.
        rawOutput.isUpdate = targetOpenCV->lastOutput.found;

        targetOpenCV->lastOutput = rawOutput;
        
        targetOpenCV->rawOutputs.push_back(rawOutput);
        // stop tracking the target once we find it 10 times.
        if (targetOpenCV->rawOutputs.size() >= 100) {
            targetOpenCV->disableTracking = true;
        }
        LOG_DETECT_TIME("finish determineFoundOrUpdateV1");
        return rawOutput;
    }
    return createFalseOutput();
}

/*
 This version of determineFoundOrUpdate tries to return as fast as possible while trying to use all the
 information it has to make the best decision by constantly reevaluating which output is the best.
 */
VROARImageTrackerOutput VROARImageTracker::determineFoundOrUpdateV2(VROARImageTrackerOutput latestOutput) {
    // when we have 5 "similar" outputs, then we know we're done looking for the target.
    int quorum = 10;

    // grab the target that this output matched.
    VROARImageTargetOpenCV *targetOpenCV = &_targetToTargetMap.find(latestOutput.target)->second;

    // grab all the previous outputs for this target.
    std::vector<VROARImageTrackerOutput> rawOutputs = targetOpenCV->rawOutputs;
    
    // this will be the new list for this output.
    std::vector<VROARImageTrackerOutput> newSimilarOutputList = {latestOutput};

    long max = 0;
    int maxIndex = 0;
    
    for (int i = 0; i < rawOutputs.size(); i++) {
        // see if the i-th output is similar to the latestOutput
        if (areOutputsSimilar(latestOutput, rawOutputs[i])) {
            // if they're similar, then add each output to each others list
            targetOpenCV->similarOutputsList[i].push_back(latestOutput);
            newSimilarOutputList.push_back(rawOutputs[i]);
        }

        // check if the ith raw output has the longest list so far.
        if (targetOpenCV->similarOutputsList[i].size() >= max) {
            max = targetOpenCV->similarOutputsList[i].size();
            maxIndex = i;
        }
    }

    // add the newSimilarOutputList to our list of similar outputs
    targetOpenCV->similarOutputsList.push_back(newSimilarOutputList);

    // Now check if this list beats all the other ones!
    VROARImageTrackerOutput toReturn;
    if (newSimilarOutputList.size() >= max) {
        max = newSimilarOutputList.size();
        toReturn = newSimilarOutputList[0];
    } else {
        toReturn = targetOpenCV->similarOutputsList[maxIndex][0];
    }

    // if there was a lastOutput, then this is now an update!
    toReturn.isUpdate = targetOpenCV->lastOutput.found;
    targetOpenCV->lastOutput = toReturn;
    // disableTracking once we have a quorum of points that 'agree'
    targetOpenCV->disableTracking = max >= quorum;
    // add the rawOutput to the list of rawOutputs
    targetOpenCV->rawOutputs.push_back(latestOutput);

    LOG_DETECT_TIME("finish determineFoundOrUpdateV2");
    return toReturn;
}

VROARImageTrackerOutput VROARImageTracker::determineFoundOrUpdateV3(VROARImageTrackerOutput rawOutput) {

    LOG_DETECT_TIME("finish determineFoundOrUpdateV3");
}

float VROARImageTracker::getScaleFactor(int rows, int cols) {
    if (rows * cols <= kMaxInputImageSize) {
        return 1.0; // don't scale
    } else {
        // return the sqrt of the ratio b/t max and rows * cols because the rows and columns
        // are scaled independently (thus applying whatever scale we return twice - squaring it).
        return (float) sqrt(((float) kMaxInputImageSize) / (rows * cols));
    }
}

bool VROARImageTracker::areOutputsSimilar(VROARImageTrackerOutput first, VROARImageTrackerOutput second) {
    float similarDistanceThreshold = .015; // 1.5 cm
    
    // if either outputs aren't a "found" output, return false
    if (!first.found || !second.found) {
        return false;
    }

    // Check if the cartesian distance are within a similarDistanceThreshold
    if (first.worldTransform.extractTranslation().distance(second.worldTransform.extractTranslation()) > similarDistanceThreshold) {
        return false;
    }
    
    // Check if the rotations are similar...
    
    // try comparing rotations?
    VROVector3f firstRotation = first.worldTransform.extractRotation(first.worldTransform.extractScale()).toEuler();
    VROVector3f secondRotation = second.worldTransform.extractRotation(second.worldTransform.extractScale()).toEuler();
    
    float xDiff = fabs(firstRotation.x - secondRotation.x);
    float yDiff = fabs(firstRotation.y - secondRotation.y);
    float zDiff = fabs(firstRotation.z - secondRotation.z);

    float maxAxisDiff = 5 * M_PI / 180; // 5 degrees
    float maxTotalDiff = 10 * M_PI / 180; // 10 degrees

    if (xDiff > maxAxisDiff || yDiff > maxAxisDiff || zDiff > maxAxisDiff || (xDiff + yDiff + zDiff) > maxTotalDiff) {
        return false;
    }

    return true;
}

bool VROARImageTracker::areCornersValid(std::vector<cv::Point2f> corners) {
    
    // if we have less than 4 corners, we definitely did not find a rectangular object!
    if (corners.size() != 4) {
        pinfo("[Viro] corner check - fail test 0");
        return false;
    }
    
    /*
     This is a simple distance/sanity check between the corners. If the 4 corners have
     a combined x or y (absolute) distance of less than the minCornerDistance pixels, then
     throw away the result! This is true because in a 1920x1080 or 1280x720 our algorithm
     can't be that accurate!
     */
    int minCornerDistance = 0;
    double sumX = 0;
    double sumY = 0;
    for (int i = 0; i < corners.size() - 1; i++) {
        sumX += std::abs(corners[i].x - corners[i + 1].x);
        sumY += std::abs(corners[i].y - corners[i + 1].y);
    }

    if (sumX < minCornerDistance || sumY < minCornerDistance) {
        pinfo("[Viro] corner check - fail test 1");
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
            pinfo("[Viro] corner check - fail test 2");
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
        pinfo("[Viro] corner check - fail test 3");
        return false;
    }
    
    // the corners passed all our checks!
    return true;
}

VROVector3f VROARImageTracker::eulerFromRodriguesVector(cv::Mat rodrigues) {
    cv::Mat outMat;
    cv::Rodrigues(rodrigues, outMat);


    // The following logic comes from: https://www.learnopencv.com/rotation-matrix-to-euler-angles/
    float sy = sqrt(outMat.at<double>(0,0) * outMat.at<double>(0,0)
                    + outMat.at<double>(1,0) * outMat.at<double>(1,0));

    bool singular = sy < 1e-6; // effectively zero

    float x, y, z;
    if (!singular)
    {
        x = atan2(outMat.at<double>(2,1) , outMat.at<double>(2,2));
        y = atan2(-outMat.at<double>(2,0), sy);
        z = atan2(outMat.at<double>(1,0), outMat.at<double>(0,0));
    }
    else
    {
        x = atan2(-outMat.at<double>(1,2), outMat.at<double>(1,1));
        y = atan2(-outMat.at<double>(2,0), sy);
        z = 0;
    }
    return VROVector3f(x, y, z);
}

void VROARImageTracker::convertFromCVToViroAxes(cv::Mat inputTranslation, cv::Mat inputRotation, VROVector3f &outTranslation, VROVector3f &outRotation) {
    outTranslation.x = inputTranslation.at<double>(0, 0);
    outTranslation.y = - inputTranslation.at<double>(1, 0);
    outTranslation.z = - inputTranslation.at<double>(2, 0);

    outRotation = eulerFromRodriguesVector(inputRotation);

    outRotation.y = - outRotation.y;
    outRotation.z = - outRotation.z;
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

cv::Mat VROARImageTracker::drawCorners(cv::Mat inputImage, std::vector<cv::Point2f> inputCorners,
                                       float scaleFactor) {
    // draw lines between the inputCorners on the input image
    // neon blue - 70, 102, 255, 255
    cv::line(inputImage, inputCorners[0] * scaleFactor, inputCorners[1] * scaleFactor, cv::Scalar(0, 255, 255, 255), 5);
    cv::line(inputImage, inputCorners[1] * scaleFactor, inputCorners[2] * scaleFactor, cv::Scalar(0, 255, 255, 255), 5);
    cv::line(inputImage, inputCorners[2] * scaleFactor, inputCorners[3] * scaleFactor, cv::Scalar(0, 255, 255, 255), 5);
    cv::line(inputImage, inputCorners[3] * scaleFactor, inputCorners[0] * scaleFactor, cv::Scalar(0, 255, 255, 255), 5);

    cv::Mat processedImage = cv::Mat(inputImage.rows, inputImage.cols, CV_32F);
    cv::cvtColor(inputImage, processedImage, cv::COLOR_BGRA2RGBA);

    return processedImage;
}

cv::Mat VROARImageTracker::getIntrinsicMatrix(int inputCols, int inputRows) {
    cv::Mat cameraMatrix;
    if (_intrinsics == NULL) { // this is the Android case (ARCore doesn't provide intrinsics yet)
        std::string model = VROPlatformGetDeviceModel();
        double cols;
        double rows;
        if (VROStringUtil::strcmpinsensitive(model, "Pixel 2")) {
            // the focal distance is fixed regardless of screen resolution, the center X and Y stays
            // relatively fixed (but the actual pixel location depends on the screen resolution).
            double cameraArr[9] = {1512, 0, inputCols * .48,
                                   0, 1512, inputRows * .49,
                                   0, 0, 1};
            cameraMatrix = cv::Mat(3, 3, CV_64F, &cameraArr);
        } else if (VROStringUtil::strcmpinsensitive(model, "SM-G955U")) {
            if (inputCols < inputRows) {
                cols = inputCols * .5318;
                rows = inputRows * .491;
            } else {
                cols = inputCols * .491;
                rows = inputRows * .5318;
            }
            double cameraArr[9] = {2191.487412971218, 0, cols,
                                   0, 2190.428443008615, rows,
                                   0, 0, 1};
            cameraMatrix = cv::Mat(3, 3, CV_64F, &cameraArr);
        } else if (VROStringUtil::strcmpinsensitive(model, "SM-G950U")) {

            if (inputCols < inputRows) {
                cols = inputCols * .5215;
                rows = inputRows * .5022;
            } else {
                cols = inputCols * .5022;
                rows = inputRows * .5215;
            }

            double cameraArr[9] = {2129.987076073671, 0, cols,
                                   0, 2127.653050656804, rows,
                                   0, 0, 1};
            cameraMatrix = cv::Mat(3, 3, CV_64F, &cameraArr);
        } else {
            // Unknown device, so estimate/approx the intrinsic matrix
            // http://ksimek.github.io/2013/08/13/intrinsic/
            double focalLength = inputCols; // Approximate focal length.
            cv::Point2d center = cv::Point2d(inputCols / 2, inputRows / 2);

            double cameraArr[9] = {focalLength, 0, center.x, 0, focalLength, center.y, 0, 0, 1};
            cameraMatrix = cv::Mat(3, 3, CV_64F, &cameraArr);
        }
    } else { // on iOS, the _intrinsics array is set!
        // There are intrinsics set, so don't calculate/estimate them!
        // actually, the intrinsics assume the texture/coordinates are in landscape, so we'll need to flip mat[0][2] and mat[1][2] for portrait.
        // TODO: dynamically handle screen rotation - for iOS
        cameraMatrix = cv::Mat(3, 3, CV_32F, _intrinsics);
        cameraMatrix = cameraMatrix.t(); // we need to transpose the matrix
        float temp = cameraMatrix.at<float>(0, 2);
        cameraMatrix.at<float>(0, 2) = cameraMatrix.at<float>(1, 2);
        cameraMatrix.at<float>(1, 2) = temp;
    }

    // clone before returning to ensure the array-backed matrices aren't dealloc-ed by leaving
    // this function's scope
    return cameraMatrix.clone();
}

cv::Mat VROARImageTracker::getDistortionCoeffs() {
#if VRO_PLATFORM_IOS
    return cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assume no lens distortion
#else
    std::string model = VROPlatformGetDeviceModel();
    if (VROStringUtil::strcmpinsensitive(model, "Pixel 2")) {
        // Pixel 2 distCoeffs
        double distCoeffsArr[5] = {0.3071814282190861, -1.406069924010113, -0.001143236436618327,
                                   -0.003115266690240281, 2.134291153514535};
        cv::Mat distCoeffs(5, 1, CV_64F, &distCoeffsArr);
        // clone before returning because the double array will be dealloced upon leaving the func scope.
        return distCoeffs.clone();
    } else if (VROStringUtil::strcmpinsensitive(model, "SM-G955U")) {
        double distCoeffsArr[5] = {0.2140704096247754, -0.5619697252678999, -0.001414139193934611,
                                   0.002719229177220327, 0.4081823444503178};
        cv::Mat distCoeffs(5, 1, CV_64F, &distCoeffsArr);
        // clone before returning because the double array will be dealloced upon leaving the func scope.
        return distCoeffs.clone();
    } else if (VROStringUtil::strcmpinsensitive(model, "SM-G950U")) {
        double distCoeffsArr[5] = {0.1923965528968363, -0.4941594689145613, 0.004114994401515823,
                                   -0.001190136151737745, 0.03055129101581667};
        cv::Mat distCoeffs(5, 1, CV_64F, &distCoeffsArr);
        return distCoeffs.clone();
    } else {
        // Unknown device, return no distortion
        return cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assume no lens distortion
    }
#endif
}

void VROARImageTracker::findChessboardForCalibration(cv::Mat inputImage) {
    // The instructions come from this page:
    // https://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html?
    // calibration works by finding the corners in multiple images and then comparing the corners
    // and "solving" for the matrix

    // use _calibrationFrameCount to "meter" out how often we attempt to find the corners
    _calibrationFrameCount++;
    if (_calibrationFrameCount % 4 != 0 || _calibrationFoundCount >= _numCalibrationSamples) {
        return;
    }

    _inputSize = inputImage.size();

    pinfo("[Viro] calibration - finding chessboard corners");
    std::vector<cv::Point2f> pointBuf;
    cv::Mat grayInput;
    cv::cvtColor(inputImage, grayInput, CV_RGB2GRAY);

    bool found = cv::findChessboardCorners(grayInput, cv::Size(9,6), pointBuf,
                                           CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

    if (found) {
        _calibrationFoundCount++;
        pinfo("[Viro] calibration - found chessboard corners. Found %d of %d.", _calibrationFoundCount, _numCalibrationSamples);
        cv::cornerSubPix(grayInput, pointBuf, cv::Size(11,11),
                         cv::Size(-1,-1), cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
        _foundPoints.push_back(pointBuf);
    }

    if (_calibrationFoundCount == _numCalibrationSamples) {
        calculateIntrinsicProperties();
    }
}

void VROARImageTracker::calculateIntrinsicProperties() {
    pinfo("[Viro] calibration - calculating intrinsic properties!");
    // we have a 9x6 inner corner chessboard
    std::vector<std::vector<cv::Point3f>> listOfCorners;
    for (int k = 0; k < _numCalibrationSamples; k++) {
        std::vector<cv::Point3f> corners;
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 9; j++) {
                // 50 is what the code they have use for square size... not sure if in pixels or what.
                corners.push_back(cv::Point3f(float(j * 28), float(i * 28), 0));
            }
        }
        listOfCorners.push_back(corners);
    }

    pinfo("[Viro] calibration - calculating intrinsic properties 0!");

    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

    std::vector<cv::Mat> rvecs, tvecs;

    pinfo("[Viro] calibration - calculating intrinsic properties 1! %ld %ld %d %d", listOfCorners.size(), _foundPoints.size(), _inputSize.height, _inputSize.width);
    cv::calibrateCamera(listOfCorners, _foundPoints, _inputSize, cameraMatrix,
                        distCoeffs, rvecs, tvecs, CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    std::stringstream ss;
    ss << "[Viro] calibration, cameraMatrix: " << cameraMatrix << std::endl;
    pinfo("%s", ss.str().c_str());

    std::stringstream ss1;
    ss1 << "[Viro] calibration, distCoeffs: " << distCoeffs << std::endl;
    pinfo("%s", ss1.str().c_str());
}

long VROARImageTracker::getCurrentTimeMs() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return ms;
}

#endif /* ENABLE_OPENCV */

