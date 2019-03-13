//
//  VROARFrameARCore.h
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARFrameARCore_h
#define VROARFrameARCore_h

#include "VROCameraTexture.h"
#include "VROARFrame.h"
#include "VROViewport.h"
#include "ARCore_API.h"

class VROARSessionARCore;

class VROARFrameARCore : public VROARFrame {
public:
    
    VROARFrameARCore(arcore::Frame *frame, VROViewport viewport, std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARFrameARCore();
    
    double getTimestamp() const;
    
    const std::shared_ptr<VROARCamera> &getCamera() const;
    VROCameraOrientation getOrientation() const {
        return VROCameraOrientation::Portrait;
    }
    std::vector<std::shared_ptr<VROARHitTestResult>> hitTest(int x, int y, std::set<VROARHitTestResultType> types);
    std::vector<std::shared_ptr<VROARHitTestResult>> hitTestRay(VROVector3f *origin, VROVector3f *destination , std::set<VROARHitTestResultType> types);
    VROMatrix4f getViewportToCameraImageTransform() const;
    const std::vector<std::shared_ptr<VROARAnchor>> &getAnchors() const;
    
    float getAmbientLightIntensity() const;
    VROVector3f getAmbientLightColor() const;

    bool hasDisplayGeometryChanged();
    void getBackgroundTexcoords(VROVector3f *BL, VROVector3f *BR, VROVector3f *TL, VROVector3f *TR);

    arcore::Frame *getFrameInternal() {
        return _frame;
    }

    std::shared_ptr<VROARPointCloud> getPointCloud();

private:

    arcore::Frame *_frame;
    std::weak_ptr<VROARSessionARCore> _session;
    std::shared_ptr<VROARCamera> _camera;
    VROViewport _viewport;
    std::vector<std::shared_ptr<VROARAnchor>> _anchors; // Unused in ARCore
    std::shared_ptr<VROARPointCloud> _pointCloud;

};

#endif /* VROARFrameARCore_h */
