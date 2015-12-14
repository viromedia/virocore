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
        _color({ 1.0, 1.0, 1.0 }),
        _attenuationStartDistance(2.0),
        _attenuationEndDistance(10.0),
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
    
    void setColor(VROVector3f color) {
        _color = color;
    }
    VROVector3f getColor() const {
        return _color;
    }
    
    void setName(std::string name) {
        this->_name = name;
    }
    std::string getName() const {
        return _name;
    }
    
    void setPosition(VROVector3f position) {
        _position = position;
    }
    VROVector3f getPosition() const {
        return _position;
    }
    
    void setTransformedPosition(VROVector3f position) {
        _transformedPosition = position;
    }
    VROVector3f getTransformedPosition() const {
        return _transformedPosition;
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
    
    void setSpotOuterAngle(float spotOuterAngle) {
        _spotOuterAngle = spotOuterAngle;
    }
    float getSpotOuterAngle() const {
        return _spotOuterAngle;
    }
    
private:
    
    VROLightType _type;
    VROVector3f _color;
    
    std::string _name;
    
    /*
     Omni and Spot parameters.
     */
    VROVector3f _position;
    float _attenuationStartDistance;
    float _attenuationEndDistance;
    float _attenuationFalloffExponent;
    
    /*
     Diffuse parameters.
     */
    VROVector3f _direction;
    
    /*
     Spot parameters.
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
    
    /*
     Internal.
     */
    VROVector3f _transformedPosition;
    
};



#endif /* VROLight_h */
