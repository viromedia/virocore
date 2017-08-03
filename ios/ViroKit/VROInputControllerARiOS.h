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
#include "VROARSessioniOS.h"
#include "VROARHitTestResult.h"

class VROInputControllerARiOS : public VROInputControllerBase {
public:
    VROInputControllerARiOS(float viewportWidth, float viewportHeight);
    virtual ~VROInputControllerARiOS() {}
    
    void setRenderer(std::shared_ptr<VRORenderer> renderer) {
        _weakRenderer = renderer;
    }

    void setSession(std::shared_ptr<VROARSessioniOS> session) {
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
     If the current dragged object is a VROARDraggableNode, then we'll want to "drag" it along
     the real-world surfaces by performing AR hit tests.
     */
    virtual void didUpdateDraggedObject();
    
private:
    float _viewportWidth;
    float _viewportHeight;
    float _latestScale;
    bool _isTouchOngoing;
    bool _isPinchOngoing;
    
    std::weak_ptr<VRORenderer> _weakRenderer;
    std::weak_ptr<VROARSessioniOS> _weakSession;
    VROCamera _latestCamera;
    VROVector3f _latestTouchPos;
    
    VROVector3f calculateCameraRay(VROVector3f touchPos);
    void processTouchMovement();

    VROARHitTestResult findBestHitTestResult(std::vector<VROARHitTestResult> results);
    int valueForHitTestResultType(VROARHitTestResultType type);
};

#endif /* VROInputControllerARiOS_h */
