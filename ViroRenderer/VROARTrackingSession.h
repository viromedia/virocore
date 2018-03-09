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
     */
    std::map<std::shared_ptr<VROARImageTarget>, std::shared_ptr<VROARAnchor>> _targetAnchorMap;

    /*
     TODO: remove this when we actually get tracking working/integrated
     The below fields are just used for mocking
     */
    long _count = 0;
    std::shared_ptr<VROARImageAnchor> _imageAnchor;
};


#endif //ANDROID_VROARTRACKINGSESSION_H
