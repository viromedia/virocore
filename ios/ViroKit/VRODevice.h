//
//  VRODevice.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRODevice_h
#define VRODevice_h

#include "VRODefines.h"
#if VRO_METAL

#include <string>
#include "VRODistortion.h"
#include "VROFieldOfView.h"
#include "VROScreen.h"

/*
 Encapsulates the parameters that define a given physical VR device. These 
 include the distance between the lenses, the distance from the screen to
 the lens, etc.
 */
class VRODevice {
    
public:
   
    VRODevice(UIScreen *screen) :
        _vendor("com.google"),
        _model("cardboard"),
        _interLensDistance(0.06f),
        _verticalDistanceToLensCenter(0.035f),
        _screenToLensDistance(0.042f), //.042
        _screen(screen) {
            
    }
    
    ~VRODevice() {}
    
    std::string getVendor() const {
        return _vendor;
    }
    std::string getModel() const {
        return _model;
    }
    
    const VROScreen &getScreen() const {
        return _screen;
    }
    
    float getInterLensDistance() const {
        return _interLensDistance;
    }
    float getVerticalDistanceToLensCenter() const {
        return _verticalDistanceToLensCenter;
    }
    float getScreenToLensDistance() const {
        return _screenToLensDistance;
    }
    
    const VROFieldOfView &getMaximumLeftEyeFOV() const {
        return _maximumLeftEyeFOV;
    }
    const VRODistortion &getDistortion() const {
        return _distortion;
    }
    
private:
    
    std::string _vendor;
    std::string _model;
    std::string _version;
    
    float _interLensDistance;
    float _verticalDistanceToLensCenter;
    float _screenToLensDistance;
    
    VROScreen _screen;
    
    VROFieldOfView _maximumLeftEyeFOV;
    VRODistortion _distortion;
    
};

#endif
#endif /* VRODevice_h */
