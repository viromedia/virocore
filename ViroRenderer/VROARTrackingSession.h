//
//  VROARTrackingSession.cpp
//  ViroKit
//
// Created by Andy Chu on 3/7/18.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
class VROARTrackingSession : public VROARImageTrackerListener,
                             public std::enable_shared_from_this<VROARTrackingSession> {
public:
    
    VROARTrackingSession();
    virtual ~VROARTrackingSession();

    /*
     This function initializes the VROARTrackingSession and creates the underlying texture reader
     and should be called whenever any of its configuration changes.
     */
    void init(VROARFrameARCore *frame, std::shared_ptr<VROFrameSynchronizer> synchronizer,
              GLuint cameraTextureId, int width, int height);

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

    /*
     Whether or not to track at all (used for debugging)
     */
    void enableTracking(bool shouldTrack);


#if ENABLE_OPENCV
    // -- VROARImageTrackerListener Functions --
    void onImageFound(VROARImageTrackerOutput output);
    void onFindTargetFinished();
#endif /* ENABLE_OPENCV */

private:

    /*
    Starts the underlying texture reader (which begins tracking). Calling this is idempotent.
    */
    void startTextureReader();

    /*
     Stops the underlying texture reader (which stops tracking). Calling this is idempotent.
     */
    void stopTextureReader();

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
     Contains a set of targets that we need to remove once we're not tracking
     since tracking happens on a separate thread, we cant remove whenever we want.
     */
    std::set<std::shared_ptr<VROARImageTarget>> _targetsToRemove;

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

    /*
     Keeps tracking of whether or not the underlying camera texture reader is running.
     */
    bool _isTextureReaderStarted;

    /*
     Whether or not we're ready to start the next tracking task (there isn't already a tracking task running).
     */
    std::atomic_bool _readyForTracking;

    /*
     Whether or not we have active targets.
     */
    std::atomic_bool _haveActiveTargets;

    /*
     Master toggle that determines whether or not to perform tracking.
     */
    bool _shouldTrack;

    /*
     The camera texture reader.
     */
    std::shared_ptr<VROTextureReader> _cameraTextureReader;

    /*_frameSynchronizer
     The frame synchronizer that the camera texture reader is attached to.
     */
    std::shared_ptr<VROFrameSynchronizer> _frameSynchronizer;

    /*
     The last camera that we were given.
     */
    std::shared_ptr<VROARCamera> _lastCamera;

    /*
     This functions clears out the _targetsToRemove vector and actually removes them
     from the tracker. This should be called on the renderer thread!
     */
    void flushTargetsToRemove();

    /*
     The below fields/functions are just used for mocking
     TODO: remove this when we actually get tracking working/integrated
     */
    long _count = 0;
    std::shared_ptr<VROARImageAnchor> _imageAnchor;
    void mockTracking();

};


#endif //ANDROID_VROARTRACKINGSESSION_H
