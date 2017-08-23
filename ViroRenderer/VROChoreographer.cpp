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
#include "VROToneMappingRenderPass.h"
#include "VROShadowMapRenderPass.h"
#include <vector>

VROChoreographer::VROChoreographer(std::shared_ptr<VRODriver> driver) :
    _driver(driver),
    _renderToTexture(false),
    _renderShadows(true) {
        
    initTargets(driver);
        
    // We use HDR if gamma correction is enabled; this way we can
    // bottle up the gamma correction with the HDR shader
    setRenderHDR(driver->isGammaCorrectionEnabled());
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

void VROChoreographer::initHDR(std::shared_ptr<VRODriver> driver) {
    _hdrTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1);
    _toneMappingPass = std::make_shared<VROToneMappingRenderPass>(VROToneMappingType::Reinhard, driver);
}
        
void VROChoreographer::setViewport(VROViewport viewport, std::shared_ptr<VRODriver> &driver) {
    _blitTarget->setViewport(viewport);
    _renderToTextureTarget->setViewport(viewport);
    if (_hdrTarget) {
        _hdrTarget->setViewport(viewport);
    }
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
        if (!light->getCastsShadow()) {
            continue;
        }
        maxSize = std::max(maxSize, light->getShadowMapSize());
    }

    if (maxSize == 0) {
        // No lights are casting a shadow
        return;
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
        
        pglpush("Shadow Pass");
        if (!kDebugShadowMaps) {
            _shadowTarget->setTextureImageIndex(i);
        }
        light->setShadowMapIndex(i);
        
        VRORenderPassInputOutput inputs;
        inputs[kRenderTargetSingleOutput] = _shadowTarget;
        shadowPass->render(scene, inputs, context, driver);
        
        driver->bindShader(nullptr);
        pglpop();
        
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
    if (_renderHDR) {
        // Render the scene to the floating point HDR target
        inputs[kRenderTargetSingleOutput] = _hdrTarget;
        _baseRenderPass->render(scene, inputs, context, driver);
        driver->bindShader(nullptr);
        
        if (_renderToTexture) {
            // Perform tone-mapping with gamma correction, store in _blitTarget
            inputs[kRenderTargetSingleInput]  = _hdrTarget;
            inputs[kRenderTargetSingleOutput] = _blitTarget;
            _toneMappingPass->render(scene, inputs, context, driver);
            
            // Flip/render the image to the RTT target
            _renderToTexturePostProcess->blit(_blitTarget, _renderToTextureTarget, driver);
            
            // Blit direct to the display
            _blitPostProcess->blit(_blitTarget, driver->getDisplay(), driver);
            if (_renderToTextureCallback) {
                _renderToTextureCallback();
            }
        }
        else {
            // Perform tone-mapping and gamma-correction
            inputs[kRenderTargetSingleInput] = _hdrTarget;
            inputs[kRenderTargetSingleOutput] = driver->getDisplay();
            _toneMappingPass->render(scene, inputs, context, driver);
        }
    }
    else if (_renderToTexture) {
        inputs[kRenderTargetSingleOutput] = _blitTarget;
        _baseRenderPass->render(scene, inputs, context, driver);
        driver->bindShader(nullptr);
        
        // The rendered image is upside-down and gamma-corrected in the
        // blitTarget. The back-buffer actually wants it upside-down, but for RTT
        // we want it flipped right side up.
        _renderToTexturePostProcess->blit(_blitTarget, _renderToTextureTarget, driver);
        
        // Finally, blit it over to the display
        _blitPostProcess->blit(_blitTarget, driver->getDisplay(), driver);
        
        if (_renderToTextureCallback) {
            _renderToTextureCallback();
        }
    }
    else {
        // Render to the display directly
        inputs[kRenderTargetSingleOutput] = driver->getDisplay();
        _baseRenderPass->render(scene, inputs, context, driver);
        driver->bindShader(nullptr);
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

void VROChoreographer::setRenderHDR(bool renderHDR) {
    _renderHDR = renderHDR;
    if (_renderHDR) {
        std::shared_ptr<VRODriver> driver = _driver.lock();
        if (driver) {
            initHDR(driver);
        }
    }
}

std::shared_ptr<VROToneMappingRenderPass> VROChoreographer::getToneMapping() {
    return _toneMappingPass;
}


