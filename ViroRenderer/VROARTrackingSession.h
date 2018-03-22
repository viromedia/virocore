//
//  VROARTrackingSession.cpp
//  ViroKit
//
// Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROARTRACKINGSESSION_H
#define ANDROID_VROARTRACKINGSESSION_H

#include <vector>
#include <map>
#include "VROARImageTarget.h"
#include "VROARImageAnchor.h"
#include "VROARImageTracker.h"
#include <VROTextureReader.h>
#include <arcore/VROARFrameARCore.h>

/*
 Listener interface for the VROARTrackingSession
 */
class VROARTrackingListener {
public:
    virtual ~VROARTrackingListener() {}

    virtual void onTrackedAnchorFound(std::shared_ptr<VROARAnchor> anchor) = 0;
    virtual void onTrackedAnchorUpdated(std::shared_ptr<VROARAnchor> anchor) = 0;
    virtual void onTrackedAnchorRemoved(std::shared_ptr<VROARAnchor> anchor) = 0;
};

/*
 This class contains the logic and manages the lifecycle for tracking images, objects, etc.
 */
class VROARTrackingSession {
public:
    
    VROARTrackingSession();
    virtual ~VROARTrackingSession();

    void createTextureReader(VROARFrameARCore *frame, GLuint cameraTextureId,
                             int width, int height);

    void startTextureReader(std::shared_ptr<VROFrameSynchronizer> synchronizer);

    /*
     Notifies the tracking session that a new frame has been found.
     */
    void updateFrame(VROARFrame *frame);

    /*
     Sets the listener
     */
    void setListener(std::shared_ptr<VROARTrackingListener> listener);

    /*
     Adds an image target that should be tracked by the session.
     */
    void addARImageTarget(std::shared_ptr<VROARImageTarget> target);

    /*
     Removes an image target from the list of those that should be tracked.
     */
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> target);

private:
    /*
     The listener that should be notified when a tracking target is
     found, updated or removed.
     */
    std::weak_ptr<VROARTrackingListener> _weakListener;

    /*
     Contains the currently active image targets to look for
     */
    std::vector<std::shared_ptr<VROARImageTarget>> _imageTargets;

    /*
     Maps each VROARImageTarget to the VROARAnchor that it will update if the image is found
     TODO: anchor has a shared_ptr to target, but target has a weak_ptr to anchor, if we reversed
     that we may not need this map, because we have targets and we want the anchor they're attached
     to, but we'll need to test ARKit targets/anchors to make sure nothing blows up in our faces
     */
    std::map<std::shared_ptr<VROARImageTarget>, std::shared_ptr<VROARImageAnchor>> _targetAnchorMap;

#if ENABLE_OPENCV
    /*
     This is the component that contains all the logic for tracking. TODO: we should probably move
     that logic into this class instead (but all the old testing code uses VROARImageTracker).
     */
    std::shared_ptr<VROARImageTracker> _tracker;
#endif

    std::atomic_bool _readyForTracking;
    std::atomic_bool _haveActiveTargets;

    std::shared_ptr<VROTextureReader> _cameraTextureReader;

    std::shared_ptr<VROARCamera> _lastCamera;

    /*
     TODO: remove this when we actually get tracking working/integrated
     The below fields/functions are just used for mocking
     */
    long _count = 0;
    std::shared_ptr<VROARImageAnchor> _imageAnchor;
    void mockTracking();

};


#endif //ANDROID_VROARTRACKINGSESSION_H
