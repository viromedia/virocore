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
#include "VRORenderer.h" // for kZNear and kZFar
#include "VROPencil.h"

VROLight::VROLight(VROLightType type) :
    _lightId(++sLightId),
    _type(type),
    _color({ 1.0, 1.0, 1.0 }),
    _intensity(1000.0),
    _updatedFragmentData(true),
    _updatedVertexData(true),
    _attenuationStartDistance(2.0),
    _attenuationEndDistance(std::numeric_limits<float>::max()),
    _attenuationFalloffExponent(2.0),
    _direction( { 0, 0, -1.0} ),
    _spotInnerAngle(0),
    _spotOuterAngle(45),
    _castsShadow(false),
    _shadowOpacity(1.0),
    _shadowMapSize(1024),
    _shadowBias(0.005),
    _shadowOrthographicScale(10),
    _shadowNearZ(0.1),
    _shadowFarZ(20),
    _shadowMapIndex(-1) {
    
}

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
                                                       ((VROLight *)animatable)->_updatedFragmentData = true;
                                                   }, _color, color));
}

void VROLight::setIntensity(float intensity) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float i) {
        ((VROLight *)animatable)->_intensity = i;
        ((VROLight *)animatable)->_updatedFragmentData = true;
    }, _intensity, intensity));
}

void VROLight::setPosition(VROVector3f position) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f p) {
                                                       ((VROLight *)animatable)->_position = p;
                                                       ((VROLight *)animatable)->_updatedFragmentData = true;
                                                   }, _position, position));
}

void VROLight::setDirection(VROVector3f direction) {
    direction = direction.normalize();
    
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f d) {
                                                       ((VROLight *)animatable)->_direction = d;
                                                       ((VROLight *)animatable)->_updatedFragmentData = true;
                                                   }, _direction, direction));
}

void VROLight::setAttenuationStartDistance(float attenuationStartDistance) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_attenuationStartDistance = value;
                                                    ((VROLight *)animatable)->_updatedFragmentData = true;
                                                }, _attenuationStartDistance, attenuationStartDistance));
}

void VROLight::setAttenuationEndDistance(float attenuationEndDistance) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_attenuationEndDistance = value;
                                                    ((VROLight *)animatable)->_updatedFragmentData = true;
                                                }, _attenuationEndDistance, attenuationEndDistance));
}

void VROLight::setAttenuationFalloffExponent(float attenuationFalloffExponent) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_attenuationFalloffExponent = value;
                                                    ((VROLight *)animatable)->_updatedFragmentData = true;
                                                }, _attenuationFalloffExponent, attenuationFalloffExponent));
}

void VROLight::setSpotInnerAngle(float spotInnerAngle) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_spotInnerAngle = value;
                                                    ((VROLight *)animatable)->_updatedFragmentData = true;
                                                }, _spotInnerAngle, spotInnerAngle));
}

void VROLight::setSpotOuterAngle(float spotOuterAngle) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_spotOuterAngle = value;
                                                    ((VROLight *)animatable)->_updatedFragmentData = true;
                                                }, _spotOuterAngle, spotOuterAngle));
}

void VROLight::setCastsShadow(bool castsShadow) {
    if (!castsShadow) {
        _shadowMapIndex = -1;
    }
    _castsShadow = castsShadow;
}

void VROLight::setShadowOpacity(float shadowOpacity) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float value) {
                                                    ((VROLight *)animatable)->_shadowOpacity = value;
                                                    ((VROLight *)animatable)->_updatedFragmentData = true;
    }, _shadowOpacity, shadowOpacity));
}

void VROLight::propagateFragmentUpdates() {
    if (!_updatedFragmentData) {
        return;
    }
    for (std::weak_ptr<VROLightingUBO> ubo_weak : _ubos) {
        std::shared_ptr<VROLightingUBO> ubo = ubo_weak.lock();
        if (ubo) {
            ubo->setNeedsFragmentUpdate();
        }
    }
    _updatedFragmentData = false;
}

void VROLight::propagateVertexUpdates() {
    if (!_updatedVertexData) {
        return;
    }
    for (std::weak_ptr<VROLightingUBO> ubo_weak : _ubos) {
        std::shared_ptr<VROLightingUBO> ubo = ubo_weak.lock();
        if (ubo) {
            ubo->setNeedsVertexUpdate();
        }
    }
    _updatedVertexData = false;
}

void VROLight::drawLightFrustum(std::shared_ptr<VROPencil> pencil) {
    // The coordinates of the frustum in NDC space
    VROVector4f nearBL(-1, -1, -1,  1);
    VROVector4f farBL (-1, -1,  1,  1);
    VROVector4f nearBR( 1, -1, -1,  1);
    VROVector4f farBR ( 1, -1,  1,  1);
    VROVector4f nearTL(-1,  1, -1,  1);
    VROVector4f farTL (-1,  1,  1,  1);
    VROVector4f nearTR( 1,  1, -1,  1);
    VROVector4f farTR ( 1,  1,  1,  1);
    
    VROVector4f frustumNDC[8] = { nearBL, farBL, nearBR, farBR, nearTL, farTL, nearTR, farTR };
    
    /*
    VROMatrix4f inverseProjection = _shadowProjectionMatrix.invert();
    VROVector4f frustumEye[8];
    for (int i = 0; i < 8; i++) {
        frustumEye[i] = inverseProjection.multiply(frustumNDC[i]);
        frustumEye[i].x /= frustumEye[i].w;
        frustumEye[i].y /= frustumEye[i].w;
        frustumEye[i].z /= frustumEye[i].w;
        frustumEye[i].w /= frustumEye[i].w;
        
    }
    
    VROMatrix4f inverseView = _shadowViewMatrix.invert();
    for (int i = 0; i < 8; i++) {
        frustum[i] = inverseView.multiply(frustumEye[i]);
    }
    */
    
    VROVector4f frustum[8];
    VROMatrix4f inverseLightMVP = (getShadowProjectionMatrix().multiply(getShadowViewMatrix())).invert();
    for (int i = 0; i < 8; i++) {
        frustum[i] = inverseLightMVP.multiply(frustumNDC[i]);
        frustum[i].x = frustum[i].x / frustum[i].w;
        frustum[i].y = frustum[i].y / frustum[i].w;
        frustum[i].z = frustum[i].z / frustum[i].w;
        frustum[i].w = 1.0;
    }
    
    VROVector3f nBL(frustum[0].x, frustum[0].y, frustum[0].z);
    VROVector3f fBL(frustum[1].x, frustum[1].y, frustum[1].z);
    VROVector3f nBR(frustum[2].x, frustum[2].y, frustum[2].z);
    VROVector3f fBR(frustum[3].x, frustum[3].y, frustum[3].z);
    VROVector3f nTL(frustum[4].x, frustum[4].y, frustum[4].z);
    VROVector3f fTL(frustum[5].x, frustum[5].y, frustum[5].z);
    VROVector3f nTR(frustum[6].x, frustum[6].y, frustum[6].z);
    VROVector3f fTR(frustum[7].x, frustum[7].y, frustum[7].z);
    
    pencil->draw(nBL, fBL);
    pencil->draw(nBR, fBR);
    pencil->draw(nTL, fTL);
    pencil->draw(nTR, fTR);
    
    pencil->draw(fTL, fTR);
    pencil->draw(fTR, fBR);
    pencil->draw(fBR, fBL);
    pencil->draw(fBL, fTL);
    
    pencil->draw(nTL, nTR);
    pencil->draw(nTR, nBR);
    pencil->draw(nBR, nBL);
    pencil->draw(nBL, nTL);
}
