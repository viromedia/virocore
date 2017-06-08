//
//  VROARSession.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARSession_h
#define VROARSession_h

#include <memory>

class VROARAnchor;
class VROARFrame;
class VROTexture;
class VROViewport;
enum class VROCameraOrientation; //defined in VROCameraTexture.h

/*
 Determines if the AR session tracks orientation only, or
 tracks position and orientation.
 */
enum class VROTrackingType {
    DOF3,
    DOF6
};

/*
 Manages the device camera and motion tracking for AR.
 */
class VROARSession {
public:
    
    VROARSession(VROTrackingType trackingType) :
        _trackingType(trackingType) {}
    virtual ~VROARSession() {}
    
    VROTrackingType getTrackingType() const {
        return _trackingType;
    }
    
    /*
     Start the session.
     */
    virtual void run() = 0;
    
    /*
     Pause the session. No new frames will be created.
     */
    virtual void pause() = 0;
    
    /*
     Returns true if at least one frame has been generated.
     */
    virtual bool isReady() const = 0;
    
    /*
     Add or remove anchors from the session.
     */
    virtual void addAnchor(std::shared_ptr<VROARAnchor> anchor) = 0;
    virtual void removeAnchor(std::shared_ptr<VROARAnchor> anchor) = 0;
    
    /*
     Invoke each rendering frame. Updates the AR session with the latest
     AR data, and returns this in a VROARFrame. The camera background is
     updated at this point as well.
     */
    virtual std::unique_ptr<VROARFrame> &updateFrame() = 0;
    
    /*
     Get the background texture for this AR session. The contents of this
     texture are updated after each call to updateFrame().
     */
    virtual std::shared_ptr<VROTexture> getCameraBackgroundTexture() = 0;
    
    /*
     Invoke when the viewport changes. The AR engine may adjust its camera
     background and projection matrices in response to a viewport change.
     */
    virtual void setViewport(VROViewport viewport) = 0;
    
    /*
     Invoke when orientation changes, so the AR engine can make the 
     necessary adjustments.
     */
    virtual void setOrientation(VROCameraOrientation orientation) = 0;
    
private:
    
    VROTrackingType _trackingType;
    
};

#endif /* VROARSession_h */
