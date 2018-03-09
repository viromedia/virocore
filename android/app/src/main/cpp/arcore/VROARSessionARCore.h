//
//  VROARSessionARCore.h
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARSessioniOS_h
#define VROARSessioniOS_h

#include "VROARSession.h"
#include "VROARFrameARCore.h"
#include "VROViewport.h"
#include "VROOpenGL.h"
#include "ARCore_API.h"
#include <map>
#include <vector>
#include <VROCameraTexture.h>
#include <VROARPlaneAnchor.h>
#include <VROARTrackingSession.h>


enum class VROARDisplayRotation {
    R0,
    R90,
    R180,
    R270
};

class VRODriverOpenGL;

class VROARSessionARCore : public VROARSession, public VROARTrackingListener, public std::enable_shared_from_this<VROARSessionARCore> {
public:
    
    VROARSessionARCore(std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROARSessionARCore();

    void run();
    void pause();
    bool isReady() const;
    void resetSession(bool resetTracking, bool removeAnchors);

    /*
     Configure this ARCore session with the given modes. Returns true if supported.
     */
    bool configure(arcore::LightingMode lightingMode, arcore::PlaneFindingMode planeFindingMode,
                   arcore::UpdateMode updateMode);
    
    void setScene(std::shared_ptr<VROScene> scene);
    void setDelegate(std::shared_ptr<VROARSessionDelegate> delegate);
    bool setAnchorDetection(std::set<VROAnchorDetection> types);
    void addARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void addAnchor(std::shared_ptr<VROARAnchor> anchor);
    void removeAnchor(std::shared_ptr<VROARAnchor> anchor);
    void updateAnchor(std::shared_ptr<VROARAnchor> anchor);

    std::unique_ptr<VROARFrame> &updateFrame();
    std::unique_ptr<VROARFrame> &getLastFrame();
    std::shared_ptr<VROTexture> getCameraBackgroundTexture();
    
    void setViewport(VROViewport viewport);
    void setOrientation(VROCameraOrientation orientation);
    void setWorldOrigin(VROMatrix4f relativeTransform);

    void setAutofocus(bool enabled) {
        // no-op on Android
    };

    void setVideoQuality(VROVideoQuality quality) {
        // no-op on Android
    };

    GLuint getCameraTextureId() const;


    // Internal methods

    /*
     Invoked when ARCore is installed on the device: sets the ARCore session implementation.
     This object will own the session.
     */
    void setARCoreSession(arcore::Session *session);

    /*
     Initialize the camera background texture and install it on the ARCore session.
     */
    void initCameraTexture(std::shared_ptr<VRODriverOpenGL> driver);
    std::shared_ptr<VROARAnchor> getAnchorForNative(arcore::Anchor *anchor);

    arcore::Session *getSessionInternal() {
        return _session;
    }

    void setDisplayGeometry(VROARDisplayRotation rotation, int width, int height);
    VROARDisplayRotation getDisplayRotation() const {  return _displayRotation; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

    // VROARTrackingListener Implementation
    virtual void onTrackedAnchorFound(std::shared_ptr<VROARAnchor> anchor);
    virtual void onTrackedAnchorUpdated(std::shared_ptr<VROARAnchor> anchor);
    virtual void onTrackedAnchorRemoved(std::shared_ptr<VROARAnchor> anchor);

private:

    /*
     The ARCore session.
     */
    arcore::Session *_session;

    /*
     Reusable ARCore frame object.
     */
    arcore::Frame *_frame;

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

    arcore::LightingMode _lightingMode;
    arcore::PlaneFindingMode _planeFindingMode;
    arcore::UpdateMode  _updateMode;

    /*
     Map of ARCore anchors ("native" anchors) to their Viro representation.
     Required so we can update VROARAnchors when their ARCore counterparts are
     updated.
     */
    std::map<std::string, std::shared_ptr<VROARAnchor>> _nativeAnchorMap;

    /*
     Background to be assigned to the VROScene.
     */
    std::shared_ptr<VROTexture> _background;

    /*
     The tracking session that handles all the tracking for us
     */
    std::shared_ptr<VROARTrackingSession> _arTrackingSession;

    /*
     The GL_TEXTURE_EXTERNAL_OES texture used for the camera background.
     */
    GLuint _cameraTextureId;

    /*
     The display rotation used by ARCore.
     */
    VROARDisplayRotation _displayRotation;

    /*
     The width and height used by the viewport (and corresponding camera texture).
     */
    int _width;
    int _height;

    bool updateARCoreConfig();
    void processUpdatedAnchors(VROARFrameARCore *frame);
    void updateAnchorFromARCore(std::shared_ptr<VROARAnchor> anchor, arcore::Anchor *anchorAR);
    void updatePlaneFromARCore(std::shared_ptr<VROARPlaneAnchor> plane, arcore::Plane *planeAR);
};

#endif /* VROARSessionARCore_h */
