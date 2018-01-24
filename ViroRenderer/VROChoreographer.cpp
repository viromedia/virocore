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
#include "VROEye.h"
#include "VROToneMappingRenderPass.h"
#include "VROGaussianBlurRenderPass.h"
#include "VROPostProcessEffectFactory.h"
#include "VRORenderMetadata.h"
#include "VRORenderToTextureDelegate.h"
#include "VROPreprocess.h"
#include "VROShadowPreprocess.h"
#include "VROIBLPreprocess.h"
#include <vector>

#pragma mark - Initialization

VROChoreographer::VROChoreographer(std::shared_ptr<VRODriver> driver) :
    _driver(driver),
    _mrtEnabled(driver->getGPUType() != VROGPUType::Adreno330OrOlder),
    _renderToTexture(false),
    _renderShadows(driver->getGPUType() != VROGPUType::Adreno330OrOlder),
    _blurScaling(0.25) {

    if (_mrtEnabled) {
        initTargets(driver);
        // We use HDR only if linear rendering is enabled
        _renderHDR = driver->getColorRenderingMode() != VROColorRenderingMode::NonLinear;
        _renderBloom = driver->isBloomEnabled();
        initHDR(driver);
    }
    else {
        _renderHDR = false;
        _renderBloom = false;
    }

    _postProcessEffectFactory = std::make_shared<VROPostProcessEffectFactory>();
    _renderToTextureDelegate = nullptr;
}

VROChoreographer::~VROChoreographer() {
    
}

void VROChoreographer::initTargets(std::shared_ptr<VRODriver> driver) {
    VRORenderTargetType colorType = _renderHDR ? VRORenderTargetType::ColorTextureHDR16 : VRORenderTargetType::ColorTexture;
    
    std::vector<std::string> blitSamplers = { "source_texture" };
    std::vector<std::string> blitCode = {
        "uniform sampler2D source_texture;",
        "frag_color = texture(source_texture, v_texcoord);"
    };
    std::shared_ptr<VROShaderProgram> blitShader = VROImageShaderProgram::create(blitSamplers, blitCode, driver);
    _blitPostProcess = driver->newImagePostProcess(blitShader);
    _blitTarget = driver->newRenderTarget(colorType, 1, 1, false);
    
    _renderToTextureTarget = driver->newRenderTarget(colorType, 1, 1, false);
    _postProcessTarget = driver->newRenderTarget(colorType, 1, 1, false);

    if (_renderShadows) {
        _preprocesses.push_back(std::make_shared<VROShadowPreprocess>(driver));
    }
    _preprocesses.push_back(std::make_shared<VROIBLPreprocess>());
}

void VROChoreographer::initHDR(std::shared_ptr<VRODriver> driver) {
    if (!_renderHDR) {
        return;
    }
    
    if (_renderBloom) {
        // The HDR target includes an additional attachment to which we render bloom
        _hdrTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 2, 1, false);
        _blurTargetA = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false);
        _blurTargetB = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false);
        _gaussianBlurPass = std::make_shared<VROGaussianBlurRenderPass>();
        
        std::vector<std::string> samplers = { "hdr_texture", "bloom_texture" };
        std::vector<std::string> code = {
            "uniform sampler2D hdr_texture;",
            "uniform sampler2D bloom_texture;",
            "highp vec4 hdr_rgba = texture(hdr_texture, v_texcoord).rgba;",
            "highp vec4 bloom_rbga = texture(bloom_texture, v_texcoord).rgba;",
            "frag_color = vec4(hdr_rgba + bloom_rbga);",
        };
        _additiveBlendPostProcess = driver->newImagePostProcess(VROImageShaderProgram::create(samplers, code, driver));
    }
    else {
        _hdrTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false);
    }
    _toneMappingPass = std::make_shared<VROToneMappingRenderPass>(VROToneMappingMethod::HableLuminanceOnly,
                                                                  driver->getColorRenderingMode() == VROColorRenderingMode::LinearSoftware,
                                                                  driver);
}
        
void VROChoreographer::setViewport(VROViewport viewport, std::shared_ptr<VRODriver> &driver) {
    /*
     The display needs the full viewport, in case it's rendering to a translated
     half of a larger screen (e.g. as in VR).
     */
    driver->getDisplay()->setViewport(viewport);

    /*
     The render targets use an un-translated viewport. We simply blit over the final
     render target to the display, which will translate it to the correct location
     on the display because we gave the display the fully specified viewport.
     */
    VROViewport rtViewport = VROViewport(0, 0, viewport.getWidth(), viewport.getHeight());

    if (_blitTarget) {
        _blitTarget->setViewport(rtViewport);
    }
    if (_postProcessTarget) {
        _postProcessTarget->setViewport(rtViewport);
    }
    if (_renderToTextureTarget) {
        _renderToTextureTarget->setViewport(rtViewport);
    }
    if (_hdrTarget) {
        _hdrTarget->setViewport(rtViewport);
    }
    if (_blurTargetA) {
        _blurTargetA->setViewport({ rtViewport.getX(), rtViewport.getY(), (int)(rtViewport.getWidth()  * _blurScaling),
                                                                          (int)(rtViewport.getHeight() * _blurScaling) });
    }
    if (_blurTargetB) {
        _blurTargetB->setViewport({ rtViewport.getX(), rtViewport.getY(), (int)(rtViewport.getWidth()  * _blurScaling),
                                                                          (int)(rtViewport.getHeight() * _blurScaling) });
    }
}

#pragma mark - Main Render Cycle

void VROChoreographer::render(VROEyeType eye,
                              std::shared_ptr<VROScene> scene,
                              std::shared_ptr<VROScene> outgoingScene,
                              const std::shared_ptr<VRORenderMetadata> &metadata,
                              VRORenderContext *context,
                              std::shared_ptr<VRODriver> &driver) {
    
    if (eye == VROEyeType::Left || eye == VROEyeType::Monocular) {
        for (std::shared_ptr<VROPreprocess> &preprocess : _preprocesses) {
            preprocess->execute(scene, context, driver);
        }
    }
    renderScene(scene, outgoingScene, metadata, context, driver);
}

void VROChoreographer::renderScene(std::shared_ptr<VROScene> scene,
                                   std::shared_ptr<VROScene> outgoingScene,
                                   const std::shared_ptr<VRORenderMetadata> &metadata,
                                   VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    VRORenderPassInputOutput inputs;
    if (_renderHDR) {
        if (_renderBloom && metadata->requiresBloomPass()) {
            // Render the scene + bloom to the floating point HDR MRT target
            inputs[kRenderTargetSingleOutput] = _hdrTarget;
            _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
            
            // Blur the image. The finished result will reside in _blurTargetB.
            inputs[kGaussianInput] = _hdrTarget;
            inputs[kGaussianPingPongA] = _blurTargetA;
            inputs[kGaussianPingPongB] = _blurTargetB;
            _gaussianBlurPass->render(scene, outgoingScene, inputs, context, driver);
            
            // Additively blend the bloom back into the image, store in _postProcessTarget
            _additiveBlendPostProcess->blit({ _hdrTarget->getTexture(0), _blurTargetB->getTexture(0) }, _postProcessTarget, driver);
            
            // Run additional post-processing on the normal HDR image
            bool postProcessed = _postProcessEffectFactory->handlePostProcessing(_postProcessTarget, _hdrTarget, driver);
            
            // Blend, tone map, and gamma correct
            if (postProcessed) {
                inputs[kToneMappingHDRInput] = _hdrTarget;
            }
            else {
                inputs[kToneMappingHDRInput] = _postProcessTarget;
            }
            if (_renderToTexture) {
                inputs[kToneMappingOutput] = _blitTarget;
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
                renderToTextureAndDisplay(_blitTarget, driver);
            }
            else {
                inputs[kToneMappingOutput] = driver->getDisplay();
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
            }
        }
        else {
            // Render the scene to the floating point HDR target
            inputs[kRenderTargetSingleOutput] = _hdrTarget;
            _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
            
            // Run additional post-processing on the HDR image
            bool postProcessed = _postProcessEffectFactory->handlePostProcessing(_hdrTarget, _postProcessTarget, driver);
            
            // Perform tone-mapping with gamma correction
            if (postProcessed) {
                inputs[kToneMappingHDRInput] = _postProcessTarget;
            }
            else {
                inputs[kToneMappingHDRInput] = _hdrTarget;
            }
            if (_renderToTexture) {
                inputs[kToneMappingOutput] = _blitTarget;
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
                renderToTextureAndDisplay(_blitTarget, driver);
            }
            else {
                inputs[kToneMappingOutput] = driver->getDisplay();
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
            }
        }
    }
    else if (_mrtEnabled && _renderToTexture) {
        inputs[kRenderTargetSingleOutput] = _blitTarget;
        _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
        renderToTextureAndDisplay(_blitTarget, driver);
    }
    else {
        // Render to the display directly
        inputs[kRenderTargetSingleOutput] = driver->getDisplay();
        _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
    }
}

void VROChoreographer::setClearColor(VROVector4f color, std::shared_ptr<VRODriver> driver) {
    // Set the default clear color for the following targets
    driver->getDisplay()->setClearColor(color);
    _blitTarget->setClearColor(color);
    _hdrTarget->setClearColor(color);
    _blurTargetA->setClearColor(color);
    _blurTargetB->setClearColor(color);
    _postProcessTarget->setClearColor(color);
}

#pragma mark - Render to Texture

void VROChoreographer::renderToTextureAndDisplay(std::shared_ptr<VRORenderTarget> input,
                                                 std::shared_ptr<VRODriver> driver) {
    // Flip/render the image to the RTT target
    input->blitColor(_renderToTextureTarget, true, driver);

    if (_renderToTextureDelegate) {
        _renderToTextureDelegate->renderedFrameTexture(input, driver);
    }

    // Blit direct to the display. We can't use the blitColor method here
    // because the display is multisampled (blitting to a multisampled buffer
    // is not supported).
    _blitPostProcess->blit({ input->getTexture(0) }, driver->getDisplay(), driver);
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

std::shared_ptr<VROPostProcessEffectFactory> VROChoreographer::getPostProcessEffectFactory(){
    return _postProcessEffectFactory;
}

void VROChoreographer::setRenderToTextureDelegate(std::shared_ptr<VRORenderToTextureDelegate> delegate) {
    _renderToTextureDelegate = delegate;
}
