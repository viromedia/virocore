//
//  VROReticle.m
//  ViroRenderer
//
//  Created by Raj Advani on 1/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROReticle.h"
#include "VROMath.h"
#include "VROPolyline.h"
#include "VRONode.h"
#include "VROMaterial.h"
#include "VROAction.h"
#include "VROBillboardConstraint.h"
#include "VROPlatformUtil.h"

static const float kTriggerAnimationDuration = 0.4;
static const float kTriggerAnimationInnerCircleThicknessMultiple = 3;
static const float kTriggerAnimationWhiteCircleMultiple = 4;

VROReticle::VROReticle(std::shared_ptr<VROTexture> reticleTexture) :
    _isPointerFixed(false),
    _enabled(true),
    _size(0.01),
    _thickness(0.005),
    _endThickness(_thickness * kTriggerAnimationInnerCircleThicknessMultiple),
    _reticleIcon(nullptr) {

    _node = std::make_shared<VRONode>();
    _node->setHidden(!_enabled);
    
    // Polyline Reticle
    if (!reticleTexture) {
        std::vector<VROVector3f> path = createArc(_size, 32);
        _polyline = VROPolyline::createPolyline(path, _thickness);
        _polyline->setName("Reticle");
        _polyline->getMaterials().front()->setWritesToDepthBuffer(false);
        _polyline->getMaterials().front()->setReadsFromDepthBuffer(false);
        _polyline->getMaterials().front()->getDiffuse().setColor({0.33, 0.976, 0.968, 1.0});
        
        _node->setGeometry(_polyline);
        _node->setPosition({0, 0, -2});
    }
        
    // Image Reticle
    else {
        _reticleIcon = VROSurface::createSurface(0.03,0.03);
        std::shared_ptr<VROMaterial> &material = _reticleIcon->getMaterials().front();
        material->getDiffuse().setTexture(reticleTexture);
        material->setWritesToDepthBuffer(false);
        material->setReadsFromDepthBuffer(false);
        
        _node->setGeometry(_reticleIcon);
    }
}

VROReticle::~VROReticle() {}

void VROReticle::trigger() {
    if (_reticleIcon != nullptr){
        // we don't scale an image reticle during a trigger.
        return;
    }

    std::shared_ptr<VROAction> action = VROAction::timedAction([this](VRONode *const node, float t) {
        float whiteAlpha = 0.0;
        float thickness = _thickness;
        if (t < 0.5) {
            thickness = VROMathInterpolate(t, 0, 0.5, _thickness, _endThickness);
            whiteAlpha = VROMathInterpolate(t, 0, 0.5, 0, 1.0);
        }
        else {
            thickness = VROMathInterpolate(t, 0.5, 1.0, _endThickness, _thickness);
            whiteAlpha = VROMathInterpolate(t, 0.5, 1.0, 1.0, 0);
        }
        // float whiteRadius = VROMathInterpolate(t, 0, 1.0, _size, _size * kTriggerAnimationWhiteCircleMultiple);
        // TODO Draw a filled circle with whiteRadius and whiteAlpha

        _polyline->setWidth(thickness);
    }, VROTimingFunctionType::Linear, kTriggerAnimationDuration);
    
    _node->runAction(action);
    _endThickness = _thickness * kTriggerAnimationInnerCircleThicknessMultiple;
}

void VROReticle::setEnabled(bool enabled) {
    _node->setHidden(!enabled);
}

void VROReticle::setPosition(VROVector3f position){
    _node->setPosition(position);
}

void VROReticle::setRadius(float radius) {
    float scale = radius / _size;
    _node->setScale({scale, scale, scale});
}

void VROReticle::setThickness(float thickness) {
    _polyline->setWidth(thickness);
}

void VROReticle::setPointerMode(bool pointerMode){
    _isPointerFixed = pointerMode;

    // Add billboard constraints in pointer mode so that reticle always faces the
    // user even if it's pointed at a sharp angle.
    if (_isPointerFixed){
        _node->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
    } else {
        _node->removeConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
    }
}

bool VROReticle::getPointerMode(){
    return _isPointerFixed;
}

void VROReticle::renderEye(VROEyeType eye, const VRORenderContext &renderContext, VRODriver &driver) {
    if (kDebugSortOrder) {
        pinfo("Updating reticle key");
    }
    
    VRORenderParameters renderParams;
    if (_isPointerFixed) {
        VROMatrix4f identity;
        renderParams.transforms.push(identity);
    }
    else {
        renderParams.transforms.push(renderContext.getHUDViewMatrix());
    }
    
    renderParams.opacities.push(1.0);
    renderParams.hierarchyDepths.push(-1);
    renderParams.hierarchyId = 0;
    renderParams.distancesFromCamera.push(0);
    
    _node->updateSortKeys(0, renderParams, renderContext, driver);

    std::shared_ptr<VROMaterial> material = _node->getGeometry()->getMaterials().front();
    material->bindShader(driver);

    _node->render(0, material, renderContext, driver);
}

std::vector<VROVector3f> VROReticle::createArc(float radius, int numSegments) {
    float x = radius;
    float y = 0;

    float sincos[2];
    VROMathFastSinCos(2 * M_PI / numSegments, sincos);
    const float angleSin = sincos[0];
    const float angleCos = sincos[1];

    std::vector<VROVector3f> path;
    for (int i = 0; i < numSegments + 1; ++i) {
        path.push_back({ x, y, 0 });

        const float temp = x;
        x = angleCos * x - angleSin * y;
        y = angleSin * temp + angleCos * y;
    }

    return path;
}
