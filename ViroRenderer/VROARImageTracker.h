//
//  VROARImageTracker.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#if ENABLE_OPENCV

#ifndef VROARImageTracker_h
#define VROARImageTracker_h

#include "VROLog.h"
//#include "VROARImageTrackerOutput.h"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "VROARImageTarget.h"
#include <memory>
#include <map>
#include "VROARCamera.h"

enum class VROARImageTrackerType {
    BRISK,
    ORB,
    ORB3,
    ORB4,
};

struct VROARImageTrackerOutput {
    bool found;
    std::vector<cv::Point2f> corners;
    cv::Mat translation;
    cv::Mat rotation;
    
    VROMatrix4f worldTransform;
    std::shared_ptr<VROARImageTarget> target;

    cv::Mat outputImage;
};

struct VROARImageTargetOpenCV {
    std::shared_ptr<VROARImageTarget> arImageTarget;
    std::vector<cv::KeyPoint> keyPoints;
    cv::Mat descriptors;
    cv::Mat rotation; // the most recent found rotation for this target in the last input image
    cv::Mat translation; // the most recent found translation for this target in the last input image
};

// TODO: merge this class into VROARTrackingSession
class VROARImageTracker {
public:
    // helper static function for "legacy"/older tracking test code w/ only 1 target
    static std::shared_ptr<VROARImageTracker> createARImageTracker(std::shared_ptr<VROARImageTarget> arImageTarget);
    
    // helper static function to create a tracker with the best configuration.
    static std::shared_ptr<VROARImageTracker> createDefaultTracker();
    
    // creates an empty, false output
    static VROARImageTrackerOutput createFalseOutput();

    VROARImageTracker(VROARImageTrackerType type);

    // Adds/Removes ARImageTarget from tracking
    void addARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget);
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> arImageTarget);

    // Sets the tracker type we want to use
    void setType(VROARImageTrackerType type);

    // This function takes the input image and detects/computes the keypoints and descriptors (as return arguments)
    void detectKeypointsAndDescriptors(cv::Mat inputImage,
                                       std::vector<cv::KeyPoint> &keypoints,
                                       cv::Mat &descriptors,
                                       bool isTarget);

    /*
     This version accepts a camera used to track and process
     */
    std::vector<VROARImageTrackerOutput> findTarget(cv::Mat inputImage, float* intrinsics, std::shared_ptr<VROARCamera> camera);

    /*
     Finds the _arImageTarget in the inputImage (assumes RGB format). Uses the given intrinsics matrix. Pass in
     NULL for intrinsics ptr if one doesn't exist.
     */
    std::vector<VROARImageTrackerOutput> findTarget(cv::Mat inputImage, float* intrinsics);

private:
    
    void updateType();
    VROARImageTargetOpenCV updateTargetInfo(std::shared_ptr<VROARImageTarget> arImageTarget);

    std::vector<VROARImageTrackerOutput> findTargetInternal(cv::Mat inputImage);
    
    std::vector<VROARImageTrackerOutput> findMultipleTargetsBF(std::vector<cv::KeyPoint> inputKeypoints,
                                                                                cv::Mat inputDescriptors,  cv::Mat inputImage);
    
    bool areCornersValid(std::vector<cv::Point2f> corners);

    /*
     This function converts from OpenCV axes to Viro axes.
     
     OpenCV axes: X to the right, Z into the screen, Y goes down
     Viro axes: X to the right, Z out of the screen (back), Y goes up
     */
    void convertFromCVToViroAxes(cv::Mat inputTranslation, cv::Mat inputRotation, VROVector3f &outTranslation, VROVector3f &outRotation);

    /*
     This function takes a camera and returns the given translation/rotation (in camera coordinates) as a transform in world coordinates.
 
     returns the identity matrix if camera is null, otherwise returns a transformation matrix.
     */
    VROMatrix4f convertToWorldCoordinates(std::shared_ptr<VROARCamera> camera, VROVector3f translation, VROVector3f rotation);
    
    long getCurrentTimeMs();
    long _startTime;

    std::shared_ptr<VROARImageTarget> _arImageTarget;
    
    std::shared_ptr<VROARCamera> _currentCamera;

    int _numberFeaturePoints;
    double _minGoodMatches;

    VROARImageTrackerType _type;
    cv::Ptr<cv::Feature2D> _feature;
    cv::Ptr<cv::Feature2D> _targetFeature;
    int _matcherType;
    cv::Ptr<cv::BFMatcher> _matcher;
    bool _useBfKnnMatcher;
    
    // total time and iteration counts for successful runs
    double _totalTime;
    double _totalIteration;

    // total time and iteration counts for failed runs (unable to find target)
    double _totalFailedTime;
    double _totalFailedIteration;
    
    std::vector<std::shared_ptr<VROARImageTarget>> _arImageTargets;
    std::map<std::shared_ptr<VROARImageTarget>, VROARImageTargetOpenCV> _targetsMap;
    std::map<std::shared_ptr<VROARImageTarget>, std::vector<VROARImageTrackerOutput>> _targetOuputsMap;

    float *_intrinsics;
};

#endif /* VROARImageTracker_h */

#endif /* ENABLE_OPENCV */
