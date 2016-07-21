//
//  VROCameraMutable.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROCameraMutable_h
#define VROCameraMutable_h

#include "VROAnimatable.h"
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VROView.h"

class VROCameraMutable : public VROAnimatable {
    
public:
    
    VROCameraMutable();
    virtual ~VROCameraMutable();
    
    void setPosition(VROVector3f position);
    void setBaseRotation(VROQuaternion baseRotation);
    void setRotationType(VROCameraRotationType type);
    void setOrbitFocalPoint(VROVector3f focalPt);
    
    VROVector3f getPosition() const {
        return _position;
    }
    VROQuaternion getBaseRotation() const {
        return _baseRotation;
    }
    VROCameraRotationType getRotationType() const {
        return _rotationType;
    }
    VROVector3f getOrbitFocalPoint() const {
        return _orbitFocalPt;
    }
    
private:
    
    VROVector3f _position;
    
    /*
     The base rotation. This is set by the application. Total rotation is head
     rotation plus base rotation.
     */
    VROQuaternion _baseRotation;
    
    /*
     The camera rotation type (orbit around a focal point, or standard rotation).
     */
    VROCameraRotationType _rotationType;
    
    /*
     If in orbit mode, this is the point that the camera focuses on, from its current
     position.
     */
    VROVector3f _orbitFocalPt;
    
};

#endif /* VROCameraMutable_hpp */
