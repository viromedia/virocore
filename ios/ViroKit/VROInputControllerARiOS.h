//
//  VROInputControllerARiOS.h
//  ViroKit
//
//  Created by Andy Chu on 6/21/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROInputControllerARiOS_h
#define VROInputControllerARiOS_h

#include "VROInputControllerBase.h"
#include "VROInputPresenterARiOS.h"
#include "VROLog.h"
#include "VRORenderer.h"
#include "VROARSession.h"
#include "VROARHitTestResult.h"

const double kARProcessDragInterval = 75; // milliseconds
const double kARDragAnimationDuration = .075; // seconds
const double kARMinDragDistance = .33; // meters
const double kARMaxDragDistance = 5; // meters


class VROInputControllerARiOS : public VROInputControllerBase {
public:
    VROInputControllerARiOS(float viewportWidth, float viewportHeight);
    virtual ~VROInputControllerARiOS() {}
    
    void setRenderer(std::shared_ptr<VRORenderer> renderer) {
        _weakRenderer = renderer;
    }

    void setSession(std::shared_ptr<VROARSession> session) {
        _weakSession = session;
    }

    virtual VROVector3f getDragForwardOffset();

    /*
     This function is called every frame by the renderer to process any controller
     things
     */
    void onProcess(const VROCamera &camera);
    
    /*
     Call this function when the screen is touched down w/ the given ray
     */
    void onScreenTouchDown(VROVector3f tapRay);
    
    /*
     Call this function if the screen is still being touched w/ the given ray
     */
    void onScreenTouchMove(VROVector3f tapRay);
    
    /*
     Call this function when the screen is touched up w/ the given ray
     */
    void onScreenTouchUp(VROVector3f tapRay);
  
    /*
     Call this function when a pinch gesture has begun.
     */
    void onPinchStart(VROVector3f pinchCenter);
  
    /*
     Call this function when a pinch gesture has ended.
     */
    void onPinchEnd();
  
    /*
     Call this function when a pinch gesture is scaling(after start, before end).
     */
    void onPinchScale(float scale);
    
    std::string getHeadset();
    std::string getController();
    
protected:
    std::shared_ptr<VROInputPresenter> createPresenter() {
        return std::make_shared<VROInputPresenterARiOS>();
    }

    /*
     Override parent logic for handling drag logic. In this case, we fire an AR hit test
     to determine where the object should go based on the real world.
     */
    virtual void processDragging(int source);
    
private:
    float _viewportWidth;
    float _viewportHeight;
    float _latestScale;
    bool _isTouchOngoing;
    bool _isPinchOngoing;
    double _lastProcessDragTimeMillis;

    std::weak_ptr<VRORenderer> _weakRenderer;
    std::weak_ptr<VROARSession> _weakSession;
    VROCamera _latestCamera;
    VROVector3f _latestTouchPos;

    VROVector3f calculateCameraRay(VROVector3f touchPos);

    void processTouchMovement();
    
    /*
     Helper function to the overridden processDragging that adds a parameter to determine
     if we should always run it (vs optimizing).
     */
    void processDragging(int source, bool alwaysRun);
    
    /*
     Given a vector a VROARHitTestResults, returns the next position we should move the object to.
     */
    VROVector3f getNextDragPosition(std::vector<VROARHitTestResult> results);

    /*
     True/false if distance between the two points are > kARMinDragDistance and < kARMaxDragDistance
     */
    bool isDistanceWithinBounds(VROVector3f point1, VROVector3f point2);

};

#endif /* VROInputControllerARiOS_h */
