//
//  VROARSessioniOS.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARSessioniOS_h
#define VROARSessioniOS_h

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARSession.h"
#include "VROViewport.h"
//#include "VROTrackingHelper.h"
#include <ARKit/ARKit.h>
#include <map>
#include <vector>

class VRODriver;
class VROVideoTextureCacheOpenGL;
@class VROARKitSessionDelegate;

class VROARSessioniOS : public VROARSession, public std::enable_shared_from_this<VROARSessioniOS> {
public:
    
    VROARSessioniOS(VROTrackingType trackingType,
                    VROWorldAlignment worldAlignment,
                    std::shared_ptr<VRODriver> driver);
    virtual ~VROARSessioniOS();
    
    void run();
    void pause();
    bool isReady() const;
    void resetSession(bool resetTracking, bool removeAnchors);
    
    void setScene(std::shared_ptr<VROScene> scene);
    void setDelegate(std::shared_ptr<VROARSessionDelegate> delegate);
    void setAnchorDetection(std::set<VROAnchorDetection> types);
    void addARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void addAnchor(std::shared_ptr<VROARAnchor> anchor);
    void removeAnchor(std::shared_ptr<VROARAnchor> anchor);
    
    std::unique_ptr<VROARFrame> &updateFrame();
    std::unique_ptr<VROARFrame> &getLastFrame();
    std::shared_ptr<VROTexture> getCameraBackgroundTexture();
    
    void setViewport(VROViewport viewport);
    void setOrientation(VROCameraOrientation orientation);
    
    void setWorldOrigin(VROMatrix4f relativeTransform);
    
    /*
     Internal methods.
     */
    void setFrame(ARFrame *frame);
    std::shared_ptr<VROARAnchor> getAnchorForNative(ARAnchor *anchor);
    void updateAnchor(std::shared_ptr<VROARAnchor> anchor);

    void addAnchor(ARAnchor *anchor);
    void updateAnchor(ARAnchor *anchor);
    void removeAnchor(ARAnchor *anchor);
    
private:
    
    /*
     The ARKit session, configuration, and delegate.
     */
    ARSession *_session;
    ARConfiguration *_sessionConfiguration;
    VROARKitSessionDelegate *_delegateAR;
    NSMutableSet<ARReferenceImage *> *_arKitImageDetectionSet;

    /*
     Image Tracking Helper.
     */
    // VROTrackingHelper* _trackingHelper;
    
    /*
     Whether or not the session has been paused.
     */
    bool _sessionPaused;

    /*
     The last computed ARFrame.
     */
    std::unique_ptr<VROARFrame> _currentFrame;
    
    /*
     The current viewport and camera orientation.
     */
    VROViewport _viewport;
    VROCameraOrientation _orientation;
    
    /*
     Vector of all anchors that have been added to this session.
     */
    std::vector<std::shared_ptr<VROARAnchor>> _anchors;
    
    /*
     Map of ARReferenceImage ObjC objects to astd::shared_ptr<VROARImageTarget>
     that was used to create the ARReferenceImage.
     */
    std::map<ARReferenceImage *, std::shared_ptr<VROARImageTarget>> _arKitReferenceImageMap;
    
    /*
     Map of ARKit anchors ("native" anchors) to their Viro representation. 
     Required so we can update VROARAnchors when their ARKit counterparts are
     updated. Note that not all VROARAnchors have an ARKit counterpart (e.g. 
     they may be added and maintained by other tracking software).
     */
    std::map<std::string, std::shared_ptr<VROARAnchor>> _nativeAnchorMap;
    
    /*
     Background to be assigned to the VROScene.
     */
    std::shared_ptr<VROTexture> _background;
    
    /*
     Video texture cache used for transferring camera content to OpenGL.
     */
    std::shared_ptr<VROVideoTextureCacheOpenGL> _videoTextureCache;
    
    /*
     Update the VROARAnchor with the transforms in the given ARAnchor.
     */
    void updateAnchorFromNative(std::shared_ptr<VROARAnchor> vAnchor, ARAnchor *anchor);

};

/*
 Delegate for ARKit's ARSession.
 */
@interface VROARKitSessionDelegate : NSObject<ARSessionDelegate>

- (id)initWithSession:(std::shared_ptr<VROARSessioniOS>)session;

@end

#endif
#endif /* VROARSessioniOS_h */
