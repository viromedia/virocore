//
//  VROChoreographer.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROChoreographer.h"
#include "VRORenderPass.h"
#include "VRODriver.h"
#include "VRORenderTarget.h"
#include "VROImageShaderProgram.h"
#include "VROImagePostProcess.h"
#include <vector>

VROChoreographer::VROChoreographer(std::shared_ptr<VRODriver> driver) :
    _useBlit(false), _width(0), _height(0) {
    _blitTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture);
    
    std::vector<std::string> blitSamplers = { "source_texture" };
    std::vector<std::string> blitCode = { "uniform sampler2D source_texture;",
                                          "frag_color = texture(source_texture, v_texcoord);"
    };
    std::shared_ptr<VROShaderProgram> blitShader = VROImageShaderProgram::create(blitSamplers, blitCode, driver);
    _blitPostProcess = driver->newImagePostProcess(blitShader);
}

VROChoreographer::~VROChoreographer() {
    
}

void VROChoreographer::setViewportSize(int width, int height) {
    if (_width == width && _height == height) {
        return;
    }
    
    _width = width;
    _height = height;
    _blitTarget->setSize(width, height);
    _blitTarget->attachNewTexture();
}

void VROChoreographer::render(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                              std::shared_ptr<VRODriver> &driver) {
    
    VRORenderPassInputOutput inputs;
    if (_useBlit) {
        inputs[kRenderTargetSingleOutput] = _blitTarget;
        _baseRenderPass->render(scene, inputs, context, driver);
        _blitPostProcess->blit(_blitTarget, driver->getDisplay(), driver);
    }
    else {
        inputs[kRenderTargetSingleOutput] = driver->getDisplay();
        _baseRenderPass->render(scene, inputs, context, driver);
    }
}

