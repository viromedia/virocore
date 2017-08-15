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
#include "VRORenderContext.h"
#include "VROMatrix4f.h"
#include "VROShadowMapRenderPass.h"
#include <vector>

VROChoreographer::VROChoreographer(std::shared_ptr<VRODriver> driver) :
    _renderToTexture(false),
    _width(0),
    _height(0),
    _renderShadows(false) {
    initTargets(driver);
}

VROChoreographer::~VROChoreographer() {
    
}

void VROChoreographer::initTargets(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> blitSamplers = { "source_texture" };
    std::vector<std::string> blitCode = { "uniform sampler2D source_texture;",
        "frag_color = texture(source_texture, v_texcoord);"
    };
    std::shared_ptr<VROShaderProgram> blitShader = VROImageShaderProgram::create(blitSamplers, blitCode, driver);
    
    _blitTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture);
    _blitPostProcess = driver->newImagePostProcess(blitShader);
    
    _renderToTextureTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture);
    _renderToTexturePostProcess = driver->newImagePostProcess(blitShader);
    _renderToTexturePostProcess->setVerticalFlip(true);
    
    // TODO VIRO-1185 This will change so that each individual light has a its own shadow map
    //      render target, and the size (2048 here) can be specified along with other
    //      parameters.
    if (_renderShadows) {
        _shadowTarget = driver->newRenderTarget(VRORenderTargetType::DepthTexture);
        _shadowTarget->setSize(2048, 2048); //TODO VIRO-1185 Make this a function of lights used
        
        _shadowPass = std::make_shared<VROShadowMapRenderPass>();
    }
}
        
void VROChoreographer::setViewportSize(int width, int height) {
    if (_width == width && _height == height) {
        return;
    }
    
    _width = width;
    _height = height;
    _blitTarget->setSize(width, height);
    _blitTarget->attachNewTexture();
    
    _renderToTextureTarget->setSize(width, height);
    _renderToTextureTarget->attachNewTexture();
}

void VROChoreographer::render(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                              std::shared_ptr<VRODriver> &driver) {
    
    if (!_renderShadows) {
        renderBasePass(scene, context, driver);
    }
    else {
        renderShadowPass(scene, context, driver);
        renderBasePass(scene, context, driver);
    }
}

void VROChoreographer::renderShadowPass(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                        std::shared_ptr<VRODriver> &driver) {
    
    VRORenderPassInputOutput inputs;
    inputs[kRenderTargetSingleOutput] = _shadowTarget;
    _shadowPass->render(scene, inputs, context, driver);
    context->setShadowMap(_shadowTarget->getTexture());
}

void VROChoreographer::renderBasePass(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                      std::shared_ptr<VRODriver> &driver) {
    VRORenderPassInputOutput inputs;
    if (_renderToTexture) {
        inputs[kRenderTargetSingleOutput] = _blitTarget;
        _baseRenderPass->render(scene, inputs, context, driver);
        
        // The rendered image is now upside-down in the blitTarget. The back-buffer
        // actually wants it this way, but for RTT we want it flipped right side up.
        _renderToTexturePostProcess->blit(_blitTarget, _renderToTextureTarget, driver);
        
        // Finally, blit it over to the display
        _blitPostProcess->blit(_blitTarget, driver->getDisplay(), driver);
        
        if (_renderToTextureCallback) {
            _renderToTextureCallback();
        }
    }
    else {
        inputs[kRenderTargetSingleOutput] = driver->getDisplay();
        _baseRenderPass->render(scene, inputs, context, driver);
    }
}

void VROChoreographer::setRenderToTextureEnabled(bool enabled) {
    _renderToTexture = enabled;
}

void VROChoreographer::setRenderToTextureCallback(std::function<void()> callback) {
    _renderToTextureCallback = callback;
}

void VROChoreographer::setRenderTexture(std::shared_ptr<VROTexture> texture) {
    _renderToTextureTarget->attachTexture(texture);
}

