//
//  VROLight.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/7/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROLight_h
#define VROLight_h

#include <string>
#include "VROVector3f.h"
#include "VROVector4f.h"
#include "VROSize.h"

enum class VROLightType {
    Ambient,
    Omni,
    Directional,
    Spot
};

enum class VROShadowMode {
    Forward,
    Deferred,
    Modulated
};

class VROLight {
    
public:
    
    VROLight(VROLightType type) :
        _type(type),
        _color({ 1.0, 1.0, 1.0, 1.0 }),
        _attenuationStartDistance(0.0),
        _attenuationEndDistance(0.0),
        _attenuationFalloffExponent(2.0),
        _direction( { 0, 0, -1.0} ),
        _spotInnerAngle(0),
        _spotOuterAngle(45)
    {}
    
    ~VROLight()
    {}
    
    VROLightType getType() const {
        return _type;
    }
    
    void setColor(VROVector4f color) {
        _color = color;
    }
    VROVector4f getColor() const {
        return _color;
    }
    
    void setName(std::string name) {
        this->_name = name;
    }
    std::string getName() const {
        return _name;
    }
    
    void setDirection(VROVector3f direction) {
        _direction = direction;
        _direction.normalize();
    }
    VROVector3f getDirection() const {
        return _direction;
    }
    
    void setAttenuationStartDistance(float attenuationStartDistance) {
        _attenuationStartDistance = attenuationStartDistance;
    }
    float getAttenuationStartDistance() const {
        return _attenuationStartDistance;
    }
    
    void setAttenuationEndDistance(float attenuationEndDistance) {
        _attenuationEndDistance = attenuationEndDistance;
    }
    float getAttenuationEndDistance() const {
        return _attenuationEndDistance;
    }
    
    void setAttenuationFalloffExponent(float attenuationFalloffExponent) {
        _attenuationFalloffExponent = attenuationFalloffExponent;
    }
    float getAttenuationFalloffExponent() const {
        return _attenuationFalloffExponent;
    }
    
    void setSpotInnerAngle(float spotInnerAngle) {
        _spotInnerAngle = spotInnerAngle;
    }
    float getSpotInnerAngle() const {
        return _spotInnerAngle;
    }
    
private:
    
    VROLightType _type;
    VROVector4f _color;
    
    std::string _name;
    
    float _attenuationStartDistance;
    float _attenuationEndDistance;
    float _attenuationFalloffExponent;
    
    /*
     Diffuse parameters.
     */
    VROVector3f _direction;
    
    /*
     Spotlight parameters.
     */
    float _spotInnerAngle;
    float _spotOuterAngle;
    
    /*
     Shadow parameters.
     */
    bool  _castsShadow;
    float _shadowRadius;
    VROVector4f _shadowColor;
    VROSize _shadowMapSize;
    int   _shadowSampleCount;
    float _shadowBias;
    float _orthographicScale;
    float _zFar;
    float _zNear;
    
    int _categoryBitMask;
    
};



#endif /* VROLight_h */
