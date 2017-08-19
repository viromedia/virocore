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
#include "VROScene.h"
#include "VROLightingUBO.h"
#include "VROEye.h"
#include "VROLight.h"
#include "VROShadowMapRenderPass.h"
#include <vector>

VROChoreographer::VROChoreographer(std::shared_ptr<VRODriver> driver) :
    _renderToTexture(false),
    _renderShadows(true) {
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
    
    _blitTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture, 1);
    _blitPostProcess = driver->newImagePostProcess(blitShader);
    
    _renderToTextureTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture, 1);
    _renderToTexturePostProcess = driver->newImagePostProcess(blitShader);
    _renderToTexturePostProcess->setVerticalFlip(true);
    
    if (_renderShadows) {
        if (kDebugShadowMaps) {
            _shadowTarget = driver->newRenderTarget(VRORenderTargetType::DepthTexture, kMaxLights);
        }
        else {
            _shadowTarget = driver->newRenderTarget(VRORenderTargetType::DepthTextureArray, kMaxLights);
        }
    }
}
        
void VROChoreographer::setViewport(VROViewport viewport, std::shared_ptr<VRODriver> &driver) {
    _blitTarget->setViewport(viewport);
    _renderToTextureTarget->setViewport(viewport);
    driver->getDisplay()->setViewport(viewport);
}

void VROChoreographer::render(VROEyeType eye, std::shared_ptr<VROScene> scene, VRORenderContext *context,
                              std::shared_ptr<VRODriver> &driver) {
    
    if (!_renderShadows) {
        renderBasePass(scene, context, driver);
    }
    else {
        if (eye == VROEyeType::Left || eye == VROEyeType::Monocular) {
            renderShadowPasses(scene, context, driver);
        }
        
        renderBasePass(scene, context, driver);
    }
}

void VROChoreographer::renderShadowPasses(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                                        std::shared_ptr<VRODriver> &driver) {
    
    const std::vector<std::shared_ptr<VROLight>> &lights = scene->getLights();
    
    // Get the max requested shadow map size; use that for our render target
    int maxSize = 0;
    for (const std::shared_ptr<VROLight> &light : lights) {
        maxSize = std::max(maxSize, light->getShadowMapSize());
    }
    _shadowTarget->setViewport({ 0, 0, maxSize, maxSize });
    
    std::map<std::shared_ptr<VROLight>, std::shared_ptr<VROShadowMapRenderPass>> activeShadowPasses;
    
    int i = 0;
    for (const std::shared_ptr<VROLight> &light : lights) {
        if (!light->getCastsShadow()) {
            continue;
        }
        
        std::shared_ptr<VROShadowMapRenderPass> shadowPass;
        
        // Get the shadow pass for this light if we already have one from the last frame;
        // otherwise, create a new one
        auto it = _shadowPasses.find(light);
        if (it == _shadowPasses.end()) {
            shadowPass = std::make_shared<VROShadowMapRenderPass>(light, driver);
        }
        else {
            shadowPass = it->second;
        }
        activeShadowPasses[light] = shadowPass;
        
        if (!kDebugShadowMaps) {
            _shadowTarget->setTextureImageIndex(i);
        }
        light->setShadowMapIndex(i);
        
        VRORenderPassInputOutput inputs;
        inputs[kRenderTargetSingleOutput] = _shadowTarget;
        shadowPass->render(scene, inputs, context, driver);
        
        ++i;
    }
    
    // If any shadow was rendered, set the shadow map in the context; otherwise
    // make it null
    if (i > 0) {
        context->setShadowMap(_shadowTarget->getTexture());
    }
    else {
        context->setShadowMap(nullptr);
    }
    
    // Shadow passes that weren't used this frame (e.g. are not in activeShadowPasses),
    // are removed
    _shadowPasses = activeShadowPasses;
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

