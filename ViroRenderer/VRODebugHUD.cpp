//
//  VRODebugHUD.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/28/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VRODebugHUD.h"
#include "VRONode.h"
#include "VROLog.h"
#include "VROMaterial.h"
#include "VROGeometry.h"
#include "VROText.h"
#include "VROTypeface.h"
#include "VROStringUtil.h"

// Frames between FPS text refresh
static const int kFPSRefreshRate = 60;

VRODebugHUD::VRODebugHUD() :
    _enabled(false) {
    
}

VRODebugHUD::~VRODebugHUD() {
    
}

void VRODebugHUD::initRenderer(std::shared_ptr<VRODriver> driver) {
    _node = std::make_shared<VRONode>();

    /*
     We have to preload all glyphs that will be used for displaying FPS
     because VROTypeface cannot load new glyphs in the midst of a render-cycle
     (loading glyphs modifies the OpenGL context).
     */
    _typeface = driver->newTypeface("Helvetica", 26);
    _typeface->preloadGlyphs("0123456789.");
    _node->setPosition({-1, 1, -2.5});
}

void VRODebugHUD::setEnabled(bool enabled) {
    _enabled = enabled;
}

void VRODebugHUD::prepare(const VRORenderContext &context) {
    if (context.getFrame() % kFPSRefreshRate == 0 || !_text) {
        _text = VROText::createSingleLineText(VROStringUtil::toString(context.getFPS(), 2),
                                              _typeface,
                                              { 0.6, 1.0, 0.6, 1.0 });
        _node->setGeometry(_text);
    }
}

void VRODebugHUD::renderEye(VROEyeType eye, const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (!_enabled) {
        return;
    }
    if (kDebugSortOrder) {
        pinfo("Updating Debug HUD");
    }
    
    VRORenderParameters renderParams;
    _node->computeTransforms(context.getHUDViewMatrix(), {});
    _node->applyConstraints(context, context.getHUDViewMatrix(), false);
    _node->updateSortKeys(0, renderParams, context, driver);
    
    for (int i = 0; i < _node->getGeometry()->getGeometryElements().size(); i++) {
        std::shared_ptr<VROMaterial> &material = _node->getGeometry()->getMaterialForElement(i);
        material->bindShader(driver);
        material->bindProperties(driver);
        
        _node->render(i, material, context, driver);
    }
}
