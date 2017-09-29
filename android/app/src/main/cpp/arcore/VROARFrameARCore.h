//
//  VROARFrameARCore.h
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARFrameARCore_h
#define VROARFrameARCore_h

#include "VROARFrame.h"
#include "VROViewport.h"
#include "arcore/ARCore_JNI.h"

class VROARSessionARCore;

class VROARFrameARCore : public VROARFrame {
public:
    
    VROARFrameARCore(jni::Object<arcore::Frame> frameJNI, VROViewport viewport, std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARFrameARCore();
    
    double getTimestamp() const;
    
    const std::shared_ptr<VROARCamera> &getCamera() const;
    std::vector<VROARHitTestResult> hitTest(int x, int y, std::set<VROARHitTestResultType> types);
    VROMatrix4f getViewportToCameraImageTransform();
    const std::vector<std::shared_ptr<VROARAnchor>> &getAnchors() const;
    
    float getAmbientLightIntensity() const;
    float getAmbientLightColorTemperature() const;

    bool isDisplayRotationChanged();
    void getBackgroundTexcoords(VROVector3f *BL, VROVector3f *BR, VROVector3f *TL, VROVector3f *TR);

private:

    jni::UniqueObject<arcore::Frame> _frameJNI;
    std::weak_ptr<VROARSessionARCore> _session;
    std::shared_ptr<VROARCamera> _camera;
    VROViewport _viewport;
    std::vector<std::shared_ptr<VROARAnchor>> _anchors;
    
};

#endif /* VROARFrameARCore_h */
