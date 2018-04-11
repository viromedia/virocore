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
#include "VRORenderMetadata.h"
#include "VROTypefaceCollection.h"

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
    _text = VROText::createSingleLineText(L"0.0",
                                          "Helvetica", 26, VROFontStyle::Normal, VROFontWeight::Regular,
                                          { 0.6, 1.0, 0.6, 1.0 }, driver);
    for (const std::shared_ptr<VROTypeface> &typeface : _text->getTypefaceCollection()->getTypefaces()) {
        typeface->preloadGlyphs("0123456789.");
    }
    _node->setPosition({-.5, .5, -2.5});
}

void VRODebugHUD::setEnabled(bool enabled) {
    _enabled = enabled;
}

void VRODebugHUD::prepare(const VRORenderContext &context) {
    if (!_enabled) {
        return;
    }
    if (context.getFrame() % kFPSRefreshRate == 0) {
        _text->setText(VROStringUtil::toWString(context.getFPS(), 2));
        _node->setGeometry(_text);
    }
}

void VRODebugHUD::renderEye(VROEyeType eye, const VRORenderContext &context, std::shared_ptr<VRODriver> &driver) {
    if (!_enabled || !_node->getGeometry()) {
        return;
    }
    if (kDebugSortOrder && context.getFrame() % kDebugSortOrderFrameFrequency == 0) {
        pinfo("Updating Debug HUD");
    }
    
    VROMatrix4f identity;
    VRORenderParameters renderParams;
    std::shared_ptr<VRORenderMetadata> metadata = std::make_shared<VRORenderMetadata>();
    _node->computeTransforms(identity, {});
    _node->applyConstraints(context, identity, false);
    _node->updateSortKeys(0, renderParams, metadata, context, driver);
    _node->setAtomicRenderProperties();


    for (int i = 0; i < _node->getGeometry()->getGeometryElements().size(); i++) {
        std::shared_ptr<VROMaterial> &material = _node->getGeometry()->getMaterialForElement(i);
        material->bindShader(0, {}, context, driver);
        material->bindProperties(driver);

        _node->render(i, material, context, driver);
    }
}
