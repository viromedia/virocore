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
#include "arcore/ARCore_Native.h"

class VROARSessionARCore;

class VROARFrameARCore : public VROARFrame {
public:
    
    VROARFrameARCore(ArFrame *_frame, VROViewport viewport, std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARFrameARCore();
    
    double getTimestamp() const;
    
    const std::shared_ptr<VROARCamera> &getCamera() const;
    std::vector<VROARHitTestResult> hitTest(int x, int y, std::set<VROARHitTestResultType> types);
    VROMatrix4f getViewportToCameraImageTransform();
    const std::vector<std::shared_ptr<VROARAnchor>> &getAnchors() const;
    
    float getAmbientLightIntensity() const;
    float getAmbientLightColorTemperature() const;

    bool hasDisplayGeometryChanged();
    void getBackgroundTexcoords(VROVector3f *BL, VROVector3f *BR, VROVector3f *TL, VROVector3f *TR);

    ArFrame *getFrameInternal() {
        return _frame;
    }

    std::shared_ptr<VROARPointCloud> getPointCloud();

private:

    ArFrame *_frame;
    std::weak_ptr<VROARSessionARCore> _session;
    std::shared_ptr<VROARCamera> _camera;
    VROViewport _viewport;
    std::vector<std::shared_ptr<VROARAnchor>> _anchors;
    std::shared_ptr<VROARPointCloud> _pointCloud;

};

#endif /* VROARFrameARCore_h */
