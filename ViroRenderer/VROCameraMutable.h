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

class VROCameraMutable : public VROAnimatable {
    
public:
    
    VROCameraMutable();
    virtual ~VROCameraMutable();
    
    void setPosition(VROVector3f position);
    void setBaseRotation(VROQuaternion baseRotation);
    
    VROVector3f getPosition() const {
        return _position;
    }
    VROQuaternion getBaseRotation() const {
        return _baseRotation;
    }
    
private:
    
    VROVector3f _position;
    
    /*
     The base rotation. This is set by the application. Total rotation is head
     rotation plus base rotation.
     */
    VROQuaternion _baseRotation;
    
};

#endif /* VROCameraMutable_hpp */
