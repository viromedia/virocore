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
#include <ARKit/ARKit.h>
#include <map>
#include <vector>

// image tracking inports
#include "VROTrackingHelper.h"
#include "VRORenderer.h"

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
    bool setAnchorDetection(std::set<VROAnchorDetection> types);
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
    void setAutofocus(bool enabled);
    void setVideoQuality(VROVideoQuality quality);
    
    /*
     Internal methods.
     */
    void setFrame(ARFrame *frame);
    std::shared_ptr<VROARAnchor> getAnchorForNative(ARAnchor *anchor);
    void updateAnchor(std::shared_ptr<VROARAnchor> anchor);

    void addAnchor(ARAnchor *anchor);
    void updateAnchor(ARAnchor *anchor);
    void removeAnchor(ARAnchor *anchor);

    // -- Image tracking functions --
    void setTrackerOutputView(UIImageView *view) {
        _trackerOutputView = view;
    }

    void setTrackerOutputText(UITextView *outputText) {
        _trackerOutputText = outputText;
    }

    void setTrackerStatusText(UITextView *statusText) {
        _trackerStatusText = statusText;
    }

    void setRenderer(std::shared_ptr<VRORenderer> renderer) {
        _renderer = renderer;
    }

    void setWidth(float width) {
        _screenWidth = width;
    }

    void setHeight(float height) {
        _screenHeight = height;
    }
    
    void outputTextTapped();

private:
    
    /*
     The ARKit session, configuration, and delegate.
     */
    ARSession *_session;
    ARConfiguration *_sessionConfiguration;
    VROARKitSessionDelegate *_delegateAR;

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    NSMutableSet<ARReferenceImage *> *_arKitImageDetectionSet;
#endif

    /*
     Image Tracking Helper.
     */
    VROTrackingHelper *_trackingHelper;

    /*
     The UIImageView used to display the image tracking output.
     */
    UIImageView *_trackerOutputView;
    
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
    
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    /*
     Map of ARReferenceImage ObjC objects to astd::shared_ptr<VROARImageTarget>
     that was used to create the ARReferenceImage.
     */
    std::map<ARReferenceImage *, std::shared_ptr<VROARImageTarget>> _arKitReferenceImageMap;
#endif
    
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
    
    // -- Image tracking-required stuff --
    
    /*
     Image Tracking Helper.
     */
    VROTrackingHelper *_trackingHelper;
    
    /*
     The UIImageView used to display the image tracking output.
     */
    UIImageView *_trackerOutputView;
    
    /*
     The UITextView used to display tracking output text.
     */
    UITextView *_trackerOutputText;
    
    /*
     The UITextView used to display tracking status... ish
     */
    UITextView *_trackerStatusText;
    
    /*
     The node will be moved according to the results of image tracking.
     */
    std::shared_ptr<VRONode> _imageTrackingResultNode;
    
    std::shared_ptr<VRONode> _imageResultsContainer;
    
    float _screenWidth;
    float _screenHeight;

    std::shared_ptr<VRORenderer> _renderer; // this is bad... but temporary for now.
};

/*
 Delegate for ARKit's ARSession.
 */
@interface VROARKitSessionDelegate : NSObject<ARSessionDelegate>

- (id)initWithSession:(std::shared_ptr<VROARSessioniOS>)session;

@end

#endif
#endif /* VROARSessioniOS_h */
