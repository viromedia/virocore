//
//  VROARFrameiOS.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARFrameiOS_h
#define VROARFrameiOS_h

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARFrame.h"
#include "VROViewport.h"
#include <ARKit/ARKit.h>

class VROARFrameiOS : public VROARFrame {
public:
    
    VROARFrameiOS(ARFrame *frame, VROViewport viewport, VROCameraOrientation orientation);
    virtual ~VROARFrameiOS();
    
    CVPixelBufferRef getImage() const;
    double getTimestamp() const;
    
    const std::shared_ptr<VROARCamera> &getCamera() const;
    VROMatrix4f getBackgroundTexcoordTransform();
    const std::vector<std::shared_ptr<VROARAnchor>> &getAnchors() const;
    
    float getAmbientLightIntensity() const;
    
private:
    
    ARFrame *_frame;
    VROViewport _viewport;
    VROCameraOrientation _orientation;
    std::shared_ptr<VROARCamera> _camera;
    std::vector<std::shared_ptr<VROARAnchor>> _anchors;
    
};

#endif
#endif /* VROARFrameiOS_h */