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
#include "VROLog.h"
#include "VROLightingUBO.h"

uint32_t VROLight::hashLights(const std::vector<std::shared_ptr<VROLight>> &lights) {
    uint32_t h = 0;
    for (const std::shared_ptr<VROLight> &light : lights) {
        h = 31 * h + light->getLightId();
    }
    return h;
}

void VROLight::setColor(VROVector3f color) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f c) {
                                                       ((VROLight *)animatable)->_color = c;
                                                       ((VROLight *)animatable)->_updated = true;
                                                   }, _color, color));
}

void VROLight::setPosition(VROVector3f position) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f p) {
                                                       ((VROLight *)animatable)->_position = p;
                                                       ((VROLight *)animatable)->_updated = true;
                                                   }, _position, position));
}

void VROLight::setDirection(VROVector3f direction) {
    direction = direction.normalize();
    
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f d) {
                                                       ((VROLight *)animatable)->_direction = d;
                                                       ((VROLight *)animatable)->_updated = true;
                                                   }, _direction, direction));
}

void VROLight::setAttenuationStartDistance(float attenuationStartDistance) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_attenuationStartDistance = value;
                                                    ((VROLight *)animatable)->_updated = true;
                                                }, _attenuationStartDistance, attenuationStartDistance));
}

void VROLight::setAttenuationEndDistance(float attenuationEndDistance) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_attenuationEndDistance = value;
                                                    ((VROLight *)animatable)->_updated = true;
                                                }, _attenuationEndDistance, attenuationEndDistance));
}

void VROLight::setAttenuationFalloffExponent(float attenuationFalloffExponent) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_attenuationFalloffExponent = value;
                                                    ((VROLight *)animatable)->_updated = true;
                                                }, _attenuationFalloffExponent, attenuationFalloffExponent));
}

void VROLight::setSpotInnerAngle(float spotInnerAngle) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_spotInnerAngle = value;
                                                    ((VROLight *)animatable)->_updated = true;
                                                }, _spotInnerAngle, spotInnerAngle));
}

void VROLight::setSpotOuterAngle(float spotOuterAngle) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_spotOuterAngle = value;
                                                    ((VROLight *)animatable)->_updated = true;
                                                }, _spotOuterAngle, spotOuterAngle));
}

void VROLight::propagateUpdates() {
    if (!_updated) {
        return;
    }
    for (std::weak_ptr<VROLightingUBO> ubo_weak : _ubos) {
        std::shared_ptr<VROLightingUBO> ubo = ubo_weak.lock();
        if (ubo) {
            ubo->setNeedsUpdate();
        }
    }
    _updated = false;
}
