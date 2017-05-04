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
#include "VROTransaction.h"

static const float kTriggerAnimationDuration = 0.4;
static const float kTriggerAnimationInnerCircleThicknessMultiple = 3;
static const float kTriggerAnimationWhiteCircleMultiple = 4;
static const float kCircleSegments = 64;
static const float kFuseRadiusMultiplier = 3;

VROReticle::VROReticle(std::shared_ptr<VROTexture> reticleTexture) :
    _isPointerFixed(false),
    _enabled(true),
    _isFusing(false),
    _size(0.01),
    _thickness(0.005),
    _endThickness(_thickness * kTriggerAnimationInnerCircleThicknessMultiple),
    _reticleIcon(nullptr) {

    // All nodes containing the reticle's UI
    _reticleBaseNode = std::make_shared<VRONode>();
    _fuseNode = std::make_shared<VRONode>();
    _fuseBackgroundNode = std::make_shared<VRONode>();
    _fuseTriggeredNode = std::make_shared<VRONode>();

    // Polyline Reticle
    if (!reticleTexture) {
        std::vector<VROVector3f> path = createArc(_size, kCircleSegments);
        _reticleLine = VROPolyline::createPolyline(path, _thickness);
        _reticleLine->setName("Reticle");
        _reticleLine->getMaterials().front()->setWritesToDepthBuffer(false);
        _reticleLine->getMaterials().front()->setReadsFromDepthBuffer(false);
        _reticleLine->getMaterials().front()->getDiffuse().setColor({0.33, 0.976, 0.968, 1.0});
        _reticleBaseNode->setGeometry(_reticleLine);
        _reticleBaseNode->setPosition({0, 0, -2});
    }

    // Image Reticle
    else {
        _reticleIcon = VROSurface::createSurface(0.02, 0.02);
        const std::shared_ptr<VROMaterial> &material = _reticleIcon->getMaterials().front();
        material->getDiffuse().setTexture(reticleTexture);
        material->setWritesToDepthBuffer(false);
        material->setReadsFromDepthBuffer(false);
        _reticleBaseNode->setGeometry(_reticleIcon);
    }

    // Create the semi-transparent fuse line Background
    _cachedCirclePoints = createArc(_size, kCircleSegments);
    _fuseBackgroundLine = VROPolyline::createPolyline(_cachedCirclePoints, _thickness);
    _fuseBackgroundLine->setName("Reticle_FuseBackground");
    _fuseBackgroundLine->getMaterials().front()->setTransparencyMode(VROTransparencyMode::AOne);
    _fuseBackgroundLine->getMaterials().front()->setTransparency(0.1);
    _fuseBackgroundLine->getMaterials().front()->setWritesToDepthBuffer(false);
    _fuseBackgroundLine->getMaterials().front()->setReadsFromDepthBuffer(false);
    _fuseBackgroundLine->getMaterials().front()->getDiffuse().setColor({0.33, 0.976, 0.968, 1.0});
    _fuseBackgroundNode->setGeometry(_fuseBackgroundLine);

    // Create UI lines needed for the fuseTriggered animation.
    _fuseTriggeredLine = VROPolyline::createPolyline(_cachedCirclePoints, _thickness * 3);
    _fuseTriggeredLine->setName("Reticle_FuseTriggered");
    _fuseTriggeredLine->getMaterials().front()->setWritesToDepthBuffer(false);
    _fuseTriggeredLine->getMaterials().front()->setReadsFromDepthBuffer(false);
    _fuseTriggeredLine->getMaterials().front()->getDiffuse().setColor({1.0, 1.0, 1.0, 0.5});
    _fuseTriggeredNode->setGeometry(_fuseTriggeredLine);

    // Set default fuse node positions
    _fuseBackgroundNode->setPosition({0, 0, -2});
    _fuseNode->setPosition({0, 0, -2});
    _fuseTriggeredNode->setPosition({0, 0, -2});

    // Set visibility flags
    _reticleBaseNode->setHidden(!_enabled);
    _fuseNode->setHidden(!_enabled);
    _fuseBackgroundNode->setHidden(!_enabled);
    _fuseTriggeredNode->setHidden(!_enabled);
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
        _reticleLine->setWidth(thickness);
        _fuseBackgroundLine->setWidth(thickness);
        if (_fuseLine){
            _fuseLine->setWidth(thickness);
        }
    }, VROTimingFunctionType::Linear, kTriggerAnimationDuration);

    _reticleBaseNode->runAction(action);
    _fuseNode->runAction(action);
    _fuseBackgroundNode->runAction(action);
    _endThickness = _thickness * kTriggerAnimationInnerCircleThicknessMultiple;
}

void VROReticle::setEnabled(bool enabled) {
    /**
     * Note: As the reticle doesn't currently support hierarchal rendering, We have
     * to manually set the property of each node.
     */
    _reticleBaseNode->setHidden(!enabled);
    _fuseNode->setHidden(!enabled);
    _fuseBackgroundNode->setHidden(!enabled);
    _fuseTriggeredNode->setHidden(!enabled);
}

void VROReticle::setPosition(VROVector3f position){
    /**
     * Note: As the reticle doesn't currently support hierarchal rendering, We have
     * to manually set the property of each node.
     */
    _reticleBaseNode->setPosition(position);
    _fuseNode->setPosition(position);
    _fuseBackgroundNode->setPosition(position);
    _fuseTriggeredNode->setPosition(position);
}

void VROReticle::setRadius(float radius) {
    _radius = radius;
    float scale = radius / _size;
    _reticleBaseNode->setScale({scale, scale, scale});
}

void VROReticle::setPointerMode(bool pointerMode){
    _isPointerFixed = pointerMode;

    // Add billboard constraints in pointer mode so that reticle always faces the
    // user even if it's pointed at a sharp angle.
    if (_isPointerFixed){
        _reticleBaseNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        _fuseNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        _fuseBackgroundNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        _fuseTriggeredNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
    } else {
        _reticleBaseNode->removeConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        _fuseNode->removeConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        _fuseBackgroundNode->removeConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
        _fuseTriggeredNode->removeConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
    }
}

bool VROReticle::getPointerMode(){
    return _isPointerFixed;
}

void VROReticle::renderEye(VROEyeType eye, const VRORenderContext &renderContext, std::shared_ptr<VRODriver> &driver) {
    if (kDebugSortOrder) {
        pinfo("Updating reticle key");
    }
    
    VROMatrix4f parentTransform;
    if (!_isPointerFixed) {
        parentTransform = renderContext.getHUDViewMatrix();
    }

    if (_isFusing){
        renderNode(_fuseBackgroundNode, parentTransform, renderContext, driver);
        renderNode(_fuseNode, parentTransform, renderContext, driver);
        renderNode(_fuseTriggeredNode, parentTransform, renderContext, driver);
    }
    else {
        renderNode(_reticleBaseNode, parentTransform, renderContext, driver);
    }
}

void VROReticle::renderNode(std::shared_ptr<VRONode> node, VROMatrix4f parentTransform,
                            const VRORenderContext &renderContext, std::shared_ptr<VRODriver> &driver){
    VRORenderParameters renderParams;
    node->computeTransforms(renderContext, parentTransform, {});
    node->applyConstraints(renderContext, parentTransform, false);
    node->updateSortKeys(0, renderParams, renderContext, driver);
    const std::shared_ptr<VROGeometry> &geometry = node->getGeometry();
    if (!geometry){
        return;
    }

    std::shared_ptr<VROMaterial> material = geometry->getMaterials().front();
    material->bindShader(driver);
    node->render(0, material, renderContext, driver);
};

std::vector<VROVector3f> VROReticle::createArc(float radius, int numSegments) {
    // Start drawing the Arc from a 12 o'clock position
    float x = 0;
    float y = radius;

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

    // Reverse the paths to start drawing in clockwise direction
    std::reverse(path.begin(),path.end());
    return path;
}

void VROReticle::animateFuse(float ratio){
    // Start fuse scaling animation if we haven't yet already
    if (!_isFusing){
        float scale = _radius / _size * kFuseRadiusMultiplier;
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(0.25);
        VROTransaction::setTimingFunction(VROTimingFunctionType::PowerDecel);
        _fuseNode->setScale({scale,scale,scale});
        _fuseBackgroundNode->setScale({scale,scale,scale});
        VROTransaction::commit();

        _isFusing = true;
    }

    // Animate trigger animation if we have finished fusing.
    if (ratio == 1){
        animateFuseTriggered();
    }
    
    if (_fuseLine != nullptr){
        _fuseLine = nullptr;
    }

    // Normalize the fuseRatio against the number of segments in a circle
    // and update the fuse line circle with the new segments. This
    // results in a partially drawn circle that animates to completion
    // (as the fuseRatio reaches 1)
    int circleRatio = ceil(kCircleSegments * ratio);
    std::vector<VROVector3f> points(_cachedCirclePoints.begin(),
                                    _cachedCirclePoints.begin() + circleRatio);
    _fuseLine = VROPolyline::createPolyline(points, _thickness);
    _fuseLine->setName("Reticle_Fuse");
    _fuseLine->getMaterials().front()->setWritesToDepthBuffer(false);
    _fuseLine->getMaterials().front()->setReadsFromDepthBuffer(false);
    _fuseLine->getMaterials().front()->getDiffuse().setColor({1.0, 1.0, 0.968, 1.0});
    
    _fuseNode->setGeometry(_fuseLine);
}

void VROReticle::stopFuseAnimation() {
    float scale = _radius / _size;
    _fuseNode->setScale({scale,scale,scale});
    _fuseBackgroundNode->setScale({scale,scale,scale});
    _fuseTriggeredNode->setScale({scale,scale,scale});
    _fuseTriggeredNode->setOpacity(0);
    _isFusing = false;
    _fuseTriggered = false;
}

void VROReticle::animateFuseTriggered() {
    if (_fuseTriggered){
        return;
    }

    _fuseTriggered = true;
    _fuseTriggeredNode->setOpacity(0.5);
    float targetedScale = _radius / _size * (kFuseRadiusMultiplier + .5);
    _fuseTriggeredNode->setScale({targetedScale/2,targetedScale/2,targetedScale/2});

    VROTransaction::begin();
    VROTransaction::setAnimationDuration(0.4);
    VROTransaction::setTimingFunction(VROTimingFunctionType::EaseIn);
    _fuseTriggeredNode->setScale({targetedScale,targetedScale,targetedScale});
    _fuseTriggeredNode->setOpacity(0);
    VROTransaction::commit();
    trigger();
}
