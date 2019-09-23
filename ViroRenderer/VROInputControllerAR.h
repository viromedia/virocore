//
//  VROInputControllerAR.h
//  ViroKit
//
//  Created by Andy Chu on 6/21/17.
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

#ifndef VROInputControllerAR_h
#define VROInputControllerAR_h

#include "VROInputControllerBase.h"
#include "VROInputPresenterAR.h"
#include "VROLog.h"
#include "VRORenderer.h"
#include "VROARSession.h"
#include "VROARHitTestResult.h"

const double kARProcessDragInterval = 75; // milliseconds
const double kARDragAnimationDuration = .075; // seconds
const double kARMinDragDistance = .33; // meters
const double kARMaxDragDistance = 5; // meters

class VROInputControllerAR : public VROInputControllerBase {
public:
    VROInputControllerAR(float viewportWidth, float viewportHeight, std::shared_ptr<VRODriver> driver);
    virtual ~VROInputControllerAR() {}
    
    void setViewportSize(float width, float height) {
        _viewportWidth = width;
        _viewportHeight = height;
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
     Call this function when the screen is touched down w/ the given position
     */
    void onScreenTouchDown(VROVector3f touchPos);

    /*
     Call this function if the screen is still being touched w/ the given position
     */
    void onScreenTouchMove(VROVector3f touchPos);

    /*
     Call this function when the screen is touched up w/ the given screen position
     */
    void onScreenTouchUp(VROVector3f touchPos);
  
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
    
    
    /*
     Call this function when a rotate gesture has begun.
     */
    void onRotateStart(VROVector3f center);
    
    /*
     Call this function when a rotate gesture has ended.
     */
    void onRotateEnd();
    
    /*
     Call this function when a rotate gesture is updating (after start, before end).
     The rotation value should be in radians
     */
    void onRotate(float rotationRadians);
    
    std::string getHeadset();
    std::string getController();
    
protected:
    std::shared_ptr<VROInputPresenter> createPresenter(std::shared_ptr<VRODriver> driver) {
        return std::make_shared<VROInputPresenterAR>();
    }

    /*
     Override parent logic for handling drag logic. In this case, we fire an AR hit test
     to determine where the object should go based on the real world.
     */
    virtual void processDragging(int source);
    virtual void processGazeEvent(int source);

private:
    float _viewportWidth;
    float _viewportHeight;
    float _latestScale;
    float _latestRotation; // degrees
    bool _isTouchOngoing;
    bool _isPinchOngoing;
    bool _isRotateOngoing;
    double _lastProcessDragTimeMillis;

    std::weak_ptr<VROARSession> _weakSession;
    VROCamera _latestCamera;
    VROVector3f _latestTouchPos;
    VROQuaternion _cameraLastQuaternion;
    VROVector3f _cameraLastPosition;
    int _lastPointCloudSize;

    VROVector3f calculateCameraRay(VROVector3f touchPos);

    void processTouchMovement();

    
    /*
     Helper function to the overridden processDragging that adds a parameter to determine
     if we should always run it (vs optimizing).
     */
    void processDragging(int source, bool alwaysRun);

    /*
     Function that performs an ARHitTest from the camera position w/ the camera forward vector
     and returns it to any registered delegates.
     */
    void processCenterCameraHitTest();

    /*
     Function that retrieves the current AR PointCloud and passes it to any registered
     delegates.
     */
    void notifyARPointCloud();

    /*
     Given a vector a VROARHitTestResults, returns the next position we should move the object to.
     */
    VROVector3f getNextDragPosition(std::vector<std::shared_ptr<VROARHitTestResult>> results);

    /*
     True/false if distance between the two points are > kARMinDragDistance and < kARMaxDragDistance
     */
    bool isDistanceWithinBounds(VROVector3f point1, VROVector3f point2);

};

#endif /* VROInputControllerAR_h */
