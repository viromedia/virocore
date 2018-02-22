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
#include "arcore/ARCore_Native.h"
#include <map>
#include <vector>
#include <VROCameraTexture.h>
#include <VROARPlaneAnchor.h>

class VRODriverOpenGL;

class VROARSessionARCore : public VROARSession, public std::enable_shared_from_this<VROARSessionARCore> {
public:
    
    VROARSessionARCore(void *context, std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROARSessionARCore();

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
    
    /*
     Internal methods.
     */
    void initGL(std::shared_ptr<VRODriverOpenGL> driver);
    std::shared_ptr<VROARAnchor> getAnchorForNative(ArAnchor *anchor);

    ArSession *getSessionInternal() {
        return _session;
    }

    void setDisplayGeometry(int rotation, int width, int height);

private:

    /*
     The ARCore session.
     */
    ArSession *_session;

    /*
     Reusable ARCore frame object.
     */
    ArFrame *_frame;

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

    arcore::config::LightingMode _lightingMode;
    arcore::config::PlaneFindingMode _planeFindingMode;
    arcore::config::UpdateMode  _updateMode;

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
     The GL_TEXTURE_EXTERNAL_OES texture used for the camera background.
     */
    GLuint _cameraTextureId;

    void updateARCoreConfig();
    void processUpdatedAnchors(VROARFrameARCore *frame);
    void updateAnchorFromARCore(std::shared_ptr<VROARAnchor> anchor, ArAnchor *anchorAR);
    void updatePlaneFromARCore(std::shared_ptr<VROARPlaneAnchor> plane, ArPlane *planeAR);
};

#endif /* VROARSessionARCore_h */
