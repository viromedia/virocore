//
//  VROARFrameInertial.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARFrameInertial_h
#define VROARFrameInertial_h

#include "VROARFrame.h"

class VROARFrameInertial : public VROARFrame {
public:
    
    VROARFrameInertial(const std::shared_ptr<VROARCamera> &camera);
    virtual ~VROARFrameInertial();
    
    double getTimestamp() const;
    
    const std::shared_ptr<VROARCamera> &getCamera() const;
    std::vector<std::shared_ptr<VROARHitTestResult>> hitTest(int x, int y, std::set<VROARHitTestResultType> types);
    VROMatrix4f getViewportToCameraImageTransform();
    const std::vector<std::shared_ptr<VROARAnchor>> &getAnchors() const;
    
    float getAmbientLightIntensity() const;
    VROVector3f getAmbientLightColor() const;

    std::shared_ptr<VROARPointCloud> getPointCloud();
    
private:
    
    const std::shared_ptr<VROARCamera> _camera;
    double _timestamp;
    std::vector<std::shared_ptr<VROARAnchor>> _anchors;
    
};

#endif /* VROARFrameInertial_h */
