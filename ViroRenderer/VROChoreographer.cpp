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
#include "VRORenderer.h"
#include <vector>

#pragma mark - Initialization

VROChoreographer::VROChoreographer(VRORendererConfiguration config, std::shared_ptr<VRODriver> driver) :
    _driver(driver),
    _clearColor({ 0, 0, 0, 1 }),
    _renderTargetsChanged(false),
    _blurScaling(0.25) {

    // Derive supported features on this GPU
    _mrtSupported = driver->getGPUType() != VROGPUType::Adreno330OrOlder;
    _hdrSupported = _mrtSupported && driver->getColorRenderingMode() != VROColorRenderingMode::NonLinear;
    _pbrSupported = _hdrSupported;
    _bloomSupported = _mrtSupported && _hdrSupported && driver->isBloomSupported();
        
    // Enable defaults based on input flags and and support
    _shadowsEnabled = _mrtSupported && config.enableShadows;
    _hdrEnabled = _hdrSupported && config.enableHDR;
    _pbrEnabled = _hdrSupported && config.enablePBR;
    _bloomEnabled = _bloomSupported && config.enableBloom;
    createRenderTargets();

    _postProcessEffectFactory = std::make_shared<VROPostProcessEffectFactory>();
    _renderToTextureDelegate = nullptr;
}

VROChoreographer::~VROChoreographer() {
    
}

void VROChoreographer::createRenderTargets() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (!driver) {
        return;
    }
    
    pinfo("Creating render targets with configuration:");
    pinfo("[MRT supported:   %d]", _mrtSupported);
    pinfo("[Shadows enabled: %d]", _shadowsEnabled);
    pinfo("[HDR supported:   %d, HDR enabled:   %d]", _hdrSupported, _hdrEnabled);
    pinfo("[PBR supported:   %d, PBR enabled:   %d]", _pbrSupported, _pbrEnabled);
    pinfo("[Bloom supported: %d, Bloom enabled: %d]", _bloomSupported, _bloomEnabled);
    
    _blitPostProcess.reset();
    _blitTarget.reset();
    _postProcessTargetA.reset();
    _postProcessTargetB.reset();
    _hdrTarget.reset();
    _blurTargetA.reset();
    _blurTargetB.reset();
    _gaussianBlurPass.reset();
    _additiveBlendPostProcess.reset();
    _toneMappingPass.reset();
    _preprocesses.clear();

    VRORenderTargetType colorType = _hdrEnabled ? VRORenderTargetType::ColorTextureHDR16 : VRORenderTargetType::ColorTexture;
    
    if (_mrtSupported) {
        std::vector<std::string> blitSamplers = { "source_texture" };
        std::vector<std::string> blitCode = {
            "uniform sampler2D source_texture;",
            "frag_color = texture(source_texture, v_texcoord);"
        };
        std::shared_ptr<VROShaderProgram> blitShader = VROImageShaderProgram::create(blitSamplers, blitCode, driver);
        _blitPostProcess = driver->newImagePostProcess(blitShader);
        _blitTarget = driver->newRenderTarget(colorType, 1, 1, false);

        _preprocesses.clear();
        if (_shadowsEnabled) {
            _preprocesses.push_back(std::make_shared<VROShadowPreprocess>(driver));
        }
        
        if (_pbrEnabled) {
            _preprocesses.push_back(std::make_shared<VROIBLPreprocess>());
        }
    }
    
    if (_hdrEnabled) {
        _postProcessTargetA = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false);
        _postProcessTargetB = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false);

        if (_bloomEnabled) {
            // The HDR target includes an additional attachment to which we render a tone-mapping mask
            // (indicating what fragments require tone-mapping), and one to which we render bloom
            _hdrTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 3, 1, false);
            _blurTargetA = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false);
            _blurTargetB = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false);
            _gaussianBlurPass = std::make_shared<VROGaussianBlurRenderPass>();
            
            std::vector<std::string> samplers = { "hdr_texture", "bloom_texture" };
            std::vector<std::string> code = {
                "uniform sampler2D hdr_texture;",
                "uniform sampler2D bloom_texture;",
                
                // The HDR input is not premultiplied, so multiply its RGB by its alpha
                "highp vec4 base = texture(hdr_texture, v_texcoord);",
                "base.rgb *= base.a;",
                
                // The bloom input is already premultiplied (see VROGaussianBlurRenderPass)
                "highp vec4 bloom = texture(bloom_texture, v_texcoord);",
                "frag_color = base + bloom;",
            };
            _additiveBlendPostProcess = driver->newImagePostProcess(VROImageShaderProgram::create(samplers, code, driver));
        }
        else {
            // The HDR target includes an additional attachment to which we render a tone-mapping mask
            // (indicating what fragments require tone-mapping)
            _hdrTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 2, 1, false);
        }
        _toneMappingPass = std::make_shared<VROToneMappingRenderPass>(VROToneMappingMethod::HableLuminanceOnly,
                                                                      driver->getColorRenderingMode() == VROColorRenderingMode::LinearSoftware,
                                                                      driver);
    }
    
    /*
     If a viewport has been set, set it on all render targets.
     */
    if (_viewport) {
        setViewport(*_viewport, driver);
    }
    setClearColor(_clearColor, driver);
}
        
void VROChoreographer::setViewport(VROViewport viewport, std::shared_ptr<VRODriver> &driver) {
    _viewport = viewport;
    
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
    if (_postProcessTargetA) {
        _postProcessTargetA->setViewport(rtViewport);
    }
    if (_postProcessTargetB) {
        _postProcessTargetB->setViewport(rtViewport);
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
    if (_renderTargetsChanged) {
        createRenderTargets();
        _renderTargetsChanged = false;
    }
    
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
    if (_hdrEnabled) {
        if (_bloomEnabled && metadata->requiresBloomPass()) {
            // Render the scene + bloom to the floating point HDR MRT target
            inputs.outputTarget = _hdrTarget;
            _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
            
            // Blur the image. The finished result will reside in _blurTargetB.
            inputs.textures[kGaussianInput] = _hdrTarget->getTexture(2);
            inputs.targets[kGaussianPingPong] = _blurTargetA;
            inputs.outputTarget = _blurTargetB;
            _gaussianBlurPass->render(scene, outgoingScene, inputs, context, driver);
            
            // Additively blend the bloom back into the image, store in _blitTarget. Note we
            // have to set the blend mode to PremultiplyAlpha because the input texture (the blur
            // texture) has alpha premultiplied -- so we don't want OpenGL to multiply its colors
            // by alpha *again*.
            driver->bindRenderTarget(_blitTarget, VRORenderTargetUnbindOp::Invalidate);
            driver->setBlendingMode(VROBlendMode::PremultiplyAlpha);
            _additiveBlendPostProcess->blit({ _hdrTarget->getTexture(0), _blurTargetB->getTexture(0) }, driver);
            driver->setBlendingMode(VROBlendMode::Alpha);

            // Run additional post-processing on the normal HDR image
            std::shared_ptr<VRORenderTarget> postProcessTarget = _postProcessEffectFactory->handlePostProcessing(_blitTarget,
                                                                                                                 _postProcessTargetA,
                                                                                                                 _postProcessTargetB,
                                                                                                                 driver);
            passert (postProcessTarget->getTexture(0) != nullptr);
            // Blend, tone map, and gamma correct
            inputs.textures[kToneMappingHDRInput] = postProcessTarget->getTexture(0);
            inputs.textures[kToneMappingMaskInput] = _hdrTarget->getTexture(1);
            
            if (_renderToTextureDelegate) {
                std::shared_ptr<VRORenderTarget> toneMappingTarget = postProcessTarget == _blitTarget ? _postProcessTargetA : _blitTarget;
                
                inputs.outputTarget = toneMappingTarget;
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
                renderToTextureAndDisplay(toneMappingTarget, driver);
            }
            else {
                inputs.outputTarget = driver->getDisplay();
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
            }
        }
        else {
            // Render the scene to the floating point HDR target
            inputs.outputTarget = _hdrTarget;
            _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
            
            // Run additional post-processing on the HDR image
            std::shared_ptr<VRORenderTarget> postProcessTarget = _postProcessEffectFactory->handlePostProcessing(_hdrTarget,
                                                                                                                 _postProcessTargetA,
                                                                                                                 _postProcessTargetB,
                                                                                                                 driver);
            
            // Perform tone-mapping with gamma correction
            inputs.textures[kToneMappingHDRInput] = postProcessTarget->getTexture(0);
            inputs.textures[kToneMappingMaskInput] = _hdrTarget->getTexture(1);

            if (_renderToTextureDelegate) {
                inputs.outputTarget = _blitTarget;
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
                renderToTextureAndDisplay(_blitTarget, driver);
            }
            else {
                inputs.outputTarget = driver->getDisplay();
                _toneMappingPass->render(scene, outgoingScene, inputs, context, driver);
            }
        }
    }
    else if (_mrtSupported && _renderToTextureDelegate) {
        inputs.outputTarget = _blitTarget;
        _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
        renderToTextureAndDisplay(_blitTarget, driver);
    }
    else {
        // Render to the display directly
        inputs.outputTarget = driver->getDisplay();
        _baseRenderPass->render(scene, outgoingScene, inputs, context, driver);
    }
}

void VROChoreographer::setClearColor(VROVector4f color, std::shared_ptr<VRODriver> driver) {
    _clearColor = color;
    // Set the default clear color for the following targets
    driver->getDisplay()->setClearColor(color);
    if (_blitTarget) {
        _blitTarget->setClearColor(color);
    }
    if (_hdrTarget) {
        _hdrTarget->setClearColor(color);
    }
    if (_blurTargetA) {
        _blurTargetA->setClearColor(color);
    }
    if (_blurTargetB) {
        _blurTargetB->setClearColor(color);
    }
    if (_postProcessTargetA) {
        _postProcessTargetA->setClearColor(color);
    }
    if (_postProcessTargetB) {
        _postProcessTargetB->setClearColor(color);
    }
}

#pragma mark - Render to Texture

void VROChoreographer::renderToTextureAndDisplay(std::shared_ptr<VRORenderTarget> input,
                                                 std::shared_ptr<VRODriver> driver) {

    _renderToTextureDelegate->didRenderFrame(input, driver);

    // Blit direct to the display. We can't use the blitColor method here
    // because the display is multisampled (blitting to a multisampled buffer
    // is not supported).
    driver->bindRenderTarget(driver->getDisplay(), VRORenderTargetUnbindOp::Invalidate);
    _blitPostProcess->blit({ input->getTexture(0) }, driver);
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

#pragma mark - Renderer Settings

bool VROChoreographer::setHDREnabled(bool enableHDR) {
    if (!enableHDR) {
        if (_hdrEnabled) {
            _hdrEnabled = false;
            _renderTargetsChanged = true;
        }
        return true;
    }
    else { // enableHDR
        if (!_hdrSupported) {
            return false;
        }
        else if (!_hdrEnabled) {
            _hdrEnabled = true;
            _renderTargetsChanged = true;
        }
        return true;
    }
}

bool VROChoreographer::setPBREnabled(bool enablePBR) {
    if (!enablePBR) {
        if (_pbrEnabled) {
            _pbrEnabled = false;
            _renderTargetsChanged = true;
        }
        return true;
    }
    else { // enablePBR
        if (!_pbrSupported) {
            return false;
        }
        else if (!_pbrEnabled) {
            _pbrEnabled = true;
            _renderTargetsChanged = true;
        }
        return true;
    }
}

bool VROChoreographer::setShadowsEnabled(bool enableShadows) {
    if (!enableShadows) {
        if (_shadowsEnabled) {
            _shadowsEnabled = false;
            _renderTargetsChanged = true;
        }
        return true;
    }
    else { // enableShadows
        if (!_mrtSupported) {
            return false;
        }
        else if (!_shadowsEnabled) {
            _shadowsEnabled = true;
            _renderTargetsChanged = true;
        }
        return true;
    }
}

bool VROChoreographer::setBloomEnabled(bool enableBloom) {
    if (!enableBloom) {
        if (_bloomEnabled) {
            _bloomEnabled = false;
            _renderTargetsChanged = true;
        }
        return true;
    }
    else { // enableBloom
        if (!_bloomSupported) {
            return false;
        }
        else if (!_bloomEnabled) {
            _bloomEnabled = true;
            _renderTargetsChanged = true;
        }
        return true;
    }
}
