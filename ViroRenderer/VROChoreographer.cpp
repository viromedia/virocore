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
#include "VROGaussianBlurRenderPass.h"
#include <vector>

const bool kGrayScaleDemo = false;

#pragma mark - Initialization

VROChoreographer::VROChoreographer(std::shared_ptr<VRODriver> driver) :
    _driver(driver),
    _renderToTexture(false),
    _renderShadows(true),
    _blurScaling(0.5) {
        
    initTargets(driver);
        
    // We use HDR if gamma correction is enabled; this way we can
    // bottle up the gamma correction with the HDR shader
    _renderHDR = driver->isGammaCorrectionEnabled();
    _renderBloom = driver->isBloomEnabled();
    initHDR(driver);
        
    if (kGrayScaleDemo) {
        initGrayScalePass(driver);
    }
}

VROChoreographer::~VROChoreographer() {
    
}

void VROChoreographer::initTargets(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> blitSamplers = { "source_texture" };
    std::vector<std::string> blitCode = {
        "uniform sampler2D source_texture;",
        "frag_color = texture(source_texture, v_texcoord);"
    };
    std::shared_ptr<VROShaderProgram> blitShader = VROImageShaderProgram::create(blitSamplers, blitCode, driver);
    _blitPostProcess = driver->newImagePostProcess(blitShader);
    _blitTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture, 1, 1);
    
    _renderToTextureTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture, 1, 1);
    _renderToTexturePostProcess = driver->newImagePostProcess(blitShader);
    _renderToTexturePostProcess->setVerticalFlip(true);
    
    _postProcessTarget = driver->newRenderTarget(VRORenderTargetType::ColorTexture, 1, 1);

    if (_renderShadows) {
        if (kDebugShadowMaps) {
            _shadowTarget = driver->newRenderTarget(VRORenderTargetType::DepthTexture, 1, kMaxLights);
        }
        else {
            _shadowTarget = driver->newRenderTarget(VRORenderTargetType::DepthTextureArray, 1, kMaxLights);
        }
    }
}

void VROChoreographer::initHDR(std::shared_ptr<VRODriver> driver) {
    if (!_renderHDR) {
        return;
    }
    
    if (_renderBloom) {
        _hdrTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 2, 1);
        _blurTargetA = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1);
        _blurTargetB = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1);
        _gaussianBlurPass = std::make_shared<VROGaussianBlurRenderPass>();
        
        std::vector<std::string> samplers = { "hdr_texture", "bloom_texture" };
        std::vector<std::string> code = {
            "uniform sampler2D hdr_texture;",
            "uniform sampler2D bloom_texture;",
            "highp vec3 hdr_color = texture(hdr_texture, v_texcoord).rgb;",
            "highp vec3 bloom_color = texture(bloom_texture, v_texcoord).rgb;",
            "frag_color = vec4(hdr_color + bloom_color, 1.0);",
        };
        _additiveBlendPostProcess = driver->newImagePostProcess(VROImageShaderProgram::create(samplers, code, driver));
    }
    else {
        _hdrTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1);
    }
    _toneMappingPass = std::make_shared<VROToneMappingRenderPass>(VROToneMappingType::Reinhard, driver);
}
        
void VROChoreographer::setViewport(VROViewport viewport, std::shared_ptr<VRODriver> &driver) {
    _blitTarget->setViewport(viewport);
    _postProcessTarget->setViewport(viewport);
    _renderToTextureTarget->setViewport(viewport);
    if (_hdrTarget) {
        _hdrTarget->setViewport(viewport);
    }
    if (_blurTargetA) {
        _blurTargetA->setViewport({ viewport.getX(), viewport.getY(), (int)(viewport.getWidth()  * _blurScaling),
                                                                      (int)(viewport.getHeight() * _blurScaling) });
    }
    if (_blurTargetB) {
        _blurTargetB->setViewport({ viewport.getX(), viewport.getY(), (int)(viewport.getWidth()  * _blurScaling),
                                                                      (int)(viewport.getHeight() * _blurScaling) });
    }
    driver->getDisplay()->setViewport(viewport);
}

#pragma mark - Main Render Cycle

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
            _shadowTarget->setTextureImageIndex(i, 0);
        }
        light->setShadowMapIndex(i);
        
        VRORenderPassInputOutput inputs;
        inputs[kRenderTargetSingleOutput] = _shadowTarget;
        shadowPass->render(scene, inputs, context, driver);
        
        driver->unbindShader();
        pglpop();
        
        ++i;
    }
    
    // If any shadow was rendered, set the shadow map in the context; otherwise
    // make it null
    if (i > 0) {
        context->setShadowMap(_shadowTarget->getTexture(0));
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
        std::shared_ptr<VRORenderTarget> toneMappedTarget;
        
        if (_renderBloom) {
            // Render the scene + bloom to the floating point HDR MRT target
            inputs[kRenderTargetSingleOutput] = _hdrTarget;
            _baseRenderPass->render(scene, inputs, context, driver);
            
            // Blur the image. The finished result will reside in _blurTargetB.
            inputs[kGaussianInput] = _hdrTarget;
            inputs[kGaussianPingPongA] = _blurTargetA;
            inputs[kGaussianPingPongB] = _blurTargetB;
            _gaussianBlurPass->render(scene, inputs, context, driver);
            
            // Additively blend the bloom back into the image, store in _postProcessTarget
            _additiveBlendPostProcess->blit(_hdrTarget, 0, _postProcessTarget, { _blurTargetB->getTexture(0) }, driver);
            
            // Run additional post-processing on the normal HDR image
            bool postProcessed = handlePostProcessing(_postProcessTarget, _hdrTarget, driver);
            
            // Blend, tone map, and gamma correct
            inputs[kToneMappingHDRInput] = postProcessed ? _hdrTarget: _postProcessTarget;
            if (_renderToTexture) {
                inputs[kToneMappingOutput] = _blitTarget;
                _toneMappingPass->render(scene, inputs, context, driver);
                renderToTextureAndDisplay(_blitTarget, driver);
            }
            else {
                inputs[kToneMappingOutput] = driver->getDisplay();
                _toneMappingPass->render(scene, inputs, context, driver);
            }
        }
        else {
            // Render the scene to the floating point HDR target
            inputs[kRenderTargetSingleOutput] = _hdrTarget;
            _baseRenderPass->render(scene, inputs, context, driver);
            
            // Run additional post-processing on the HDR image
            bool postProcessed = handlePostProcessing(_hdrTarget, _postProcessTarget, driver);
            
            // Perform tone-mapping with gamma correction
            inputs[kToneMappingHDRInput]  = postProcessed ? _postProcessTarget : _hdrTarget;
            if (_renderToTexture) {
                inputs[kToneMappingOutput] = _blitTarget;
                _toneMappingPass->render(scene, inputs, context, driver);
                renderToTextureAndDisplay(_blitTarget, driver);
            }
            else {
                inputs[kToneMappingOutput] = driver->getDisplay();
                _toneMappingPass->render(scene, inputs, context, driver);
            }
        }
    }
    else if (_renderToTexture) {
        inputs[kRenderTargetSingleOutput] = _blitTarget;
        _baseRenderPass->render(scene, inputs, context, driver);
        renderToTextureAndDisplay(_blitTarget, driver);
    }
    else {
        // Render to the display directly
        inputs[kRenderTargetSingleOutput] = driver->getDisplay();
        _baseRenderPass->render(scene, inputs, context, driver);
    }
}

#pragma mark - Render to Texture

void VROChoreographer::renderToTextureAndDisplay(std::shared_ptr<VRORenderTarget> input,
                                                 std::shared_ptr<VRODriver> driver) {
    // Flip/render the image to the RTT target
    _renderToTexturePostProcess->blit(input, 0, _renderToTextureTarget, {}, driver);
    
    // Blit direct to the display
    _blitPostProcess->blit(input, 0, driver->getDisplay(), {}, driver);
    if (_renderToTextureCallback) {
        _renderToTextureCallback();
    }
}

void VROChoreographer::setRenderToTextureEnabled(bool enabled) {
    _renderToTexture = enabled;
}

void VROChoreographer::setRenderToTextureCallback(std::function<void()> callback) {
    _renderToTextureCallback = callback;
}

void VROChoreographer::setRenderTexture(std::shared_ptr<VROTexture> texture) {
    _renderToTextureTarget->attachTexture(texture, 0);
}

std::shared_ptr<VROToneMappingRenderPass> VROChoreographer::getToneMapping() {
    return _toneMappingPass;
}

#pragma mark - Additional Post-Process Effects

bool VROChoreographer::handlePostProcessing(std::shared_ptr<VRORenderTarget> source,
                                            std::shared_ptr<VRORenderTarget> destination,
                                            std::shared_ptr<VRODriver> driver) {
    if (kGrayScaleDemo) {
        renderGrayScalePass(source, destination, driver);
        return true;
    }
    else {
        return false;
    }
}

void VROChoreographer::initGrayScalePass(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "source_texture" };
    std::vector<std::string> code = {
        "uniform sampler2D source_texture;",
        "frag_color = texture(source_texture, v_texcoord);",
        "highp float average = 0.2126 * frag_color.r + 0.7152 * frag_color.g + 0.0722 * frag_color.b;",
        "frag_color = vec4(average, average, average, 1.0);",
    };
    std::shared_ptr<VROShaderProgram> shader = VROImageShaderProgram::create(samplers, code, driver);
    
    _grayScalePostProcess = driver->newImagePostProcess(shader);
}

void VROChoreographer::renderGrayScalePass(std::shared_ptr<VRORenderTarget> input,
                                           std::shared_ptr<VRORenderTarget> output,
                                           std::shared_ptr<VRODriver> driver) {
    _grayScalePostProcess->blit(input, 0, output, {}, driver);
}
