//
//  VROLight.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/7/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROLight.h"
#include "VROAnimationVector3f.h"
#include "VROAnimationFloat.h"

void VROLight::setColor(VROVector3f color) {
    animate(std::make_shared<VROAnimationVector3f>(shared_from_this(),
                                                   [this](VROVector3f c) {
                                                       _color = c;
                                                   }, _color, color));
}

void VROLight::setPosition(VROVector3f position) {
    animate(std::make_shared<VROAnimationVector3f>(shared_from_this(),
                                                   [this](VROVector3f p) {
                                                       _position = p;
                                                   }, _position, position));
}

void VROLight::setDirection(VROVector3f direction) {
    direction.normalize();
    
    animate(std::make_shared<VROAnimationVector3f>(shared_from_this(),
                                                   [this](VROVector3f d) {
                                                       _direction = d;
                                                   }, _direction, direction));
}

void VROLight::setAttenuationStartDistance(float attenuationStartDistance) {
    animate(std::make_shared<VROAnimationFloat>(shared_from_this(),
                                                [this](float value) {
                                                    _attenuationStartDistance = value;
                                                }, _attenuationStartDistance, attenuationStartDistance));
}

void VROLight::setAttenuationEndDistance(float attenuationEndDistance) {
    animate(std::make_shared<VROAnimationFloat>(shared_from_this(),
                                                [this](float value) {
                                                    _attenuationEndDistance = value;
                                                }, _attenuationEndDistance, attenuationEndDistance));
}

void VROLight::setAttenuationFalloffExponent(float attenuationFalloffExponent) {
    animate(std::make_shared<VROAnimationFloat>(shared_from_this(),
                                                [this](float value) {
                                                    _attenuationFalloffExponent = value;
                                                }, _attenuationFalloffExponent, attenuationFalloffExponent));
}

void VROLight::setSpotInnerAngle(float spotInnerAngle) {
    animate(std::make_shared<VROAnimationFloat>(shared_from_this(),
                                                [this](float value) {
                                                    _spotInnerAngle = value;
                                                }, _spotInnerAngle, spotInnerAngle));
}

void VROLight::setSpotOuterAngle(float spotOuterAngle) {
    animate(std::make_shared<VROAnimationFloat>(shared_from_this(),
                                                [this](float value) {
                                                    _spotOuterAngle = value;
                                                }, _spotOuterAngle, spotOuterAngle));
}
