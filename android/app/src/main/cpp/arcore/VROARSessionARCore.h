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
#include "arcore/ARCore_JNI.h"
#include <map>
#include <vector>
#include <VROCameraTexture.h>
#include <VROARPlaneAnchor.h>

class VRODriverOpenGL;

class VROARSessionARCore : public VROARSession, public std::enable_shared_from_this<VROARSessionARCore> {
public:
    
    VROARSessionARCore(jni::Object<arcore::Session> sessionJNI, std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROARSessionARCore();

    void run();
    void pause();
    bool isReady() const;
    
    void setScene(std::shared_ptr<VROScene> scene);
    void setAnchorDetection(std::set<VROAnchorDetection> types);
    void addAnchor(std::shared_ptr<VROARAnchor> anchor);
    void removeAnchor(std::shared_ptr<VROARAnchor> anchor);
    void updateAnchor(std::shared_ptr<VROARAnchor> anchor);

    std::unique_ptr<VROARFrame> &updateFrame();
    std::unique_ptr<VROARFrame> &getLastFrame();
    std::shared_ptr<VROTexture> getCameraBackgroundTexture();
    
    void setViewport(VROViewport viewport);
    void setOrientation(VROCameraOrientation orientation);
    
    /*
     Internal methods.
     */
    void initGL(std::shared_ptr<VRODriverOpenGL> driver);
    VROMatrix4f getProjectionMatrix(float near, float far);
    std::shared_ptr<VROARAnchor> getAnchorForNative(jni::Object<arcore::Plane> plane);

private:
    /*
     The ARCore session.
     */
    jni::UniqueObject<arcore::Session> _sessionJNI;

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
     Map of ARCore anchors ("native" anchors) to their Viro representation.
     Required so we can update VROARAnchors when their ARCore counterparts are
     updated.
     */
    std::map<std::string, std::shared_ptr<VROARAnchor>> _nativeAnchorMap;

    /*
     Background to be assigned to the VROScene.
     */
    std::shared_ptr<VROTexture> _background;

    void processUpdatedAnchors(VROARFrameARCore *frame);

    void updateAnchorFromJni(std::shared_ptr<VROARAnchor> anchor, jni::Object<arcore::Anchor> anchorJni);

    void updatePlaneFromJni(std::shared_ptr<VROARPlaneAnchor> plane, jni::Object<arcore::Plane> planeJni);
};

#endif /* VROARSessionARCore_h */
