//
//  VRODevice.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRODevice_h
#define VRODevice_h

#include <string>
#include "VRODistortion.h"
#include "VROFieldOfView.h"

/*
 Encapsulates the parameters that define a given physical VR device. These 
 include the distance between the lenses, the distance from the screen to
 the lens, etc.
 */
class VRODevice {
    
public:
   
    VRODevice() :
        _vendor("com.google"),
        _model("cardboard"),
        _interLensDistance(0.06f),
        _verticalDistanceToLensCenter(0.035f),
        _screenToLensDistance(0.042f) {
            
    }
    
    VRODevice(const VRODevice *device) :
        _vendor(device->_vendor),
        _model(device->_model),
        _interLensDistance(device->_interLensDistance),
        _verticalDistanceToLensCenter(device->_verticalDistanceToLensCenter),
        _screenToLensDistance(device->_screenToLensDistance) {
    }
    
    ~VRODevice() {}
    
    std::string getVendor() const {
        return _vendor;
    }
    std::string getModel() const {
        return _model;
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
    
    bool equals(const VRODevice *other) const {
        if (other == nullptr) {
            return false;
        }
        else if (other == this) {
            return true;
        }
        
        return (getVendor() == other->getVendor()) &&
               (getModel() == other->getModel()) &&
               (getInterLensDistance() == other->getInterLensDistance()) &&
               (getVerticalDistanceToLensCenter() == other->getVerticalDistanceToLensCenter()) &&
               (getScreenToLensDistance() == other->getScreenToLensDistance()) &&
               (getMaximumLeftEyeFOV().equals(&other->getMaximumLeftEyeFOV())) &&
               (getDistortion().equals(&other->getDistortion()));
    }
    
private:
    
    std::string _vendor;
    std::string _model;
    std::string _version;
    
    float _interLensDistance;
    float _verticalDistanceToLensCenter;
    float _screenToLensDistance;
    
    VROFieldOfView _maximumLeftEyeFOV;
    VRODistortion _distortion;
    
};

#endif /* VRODevice_h */
