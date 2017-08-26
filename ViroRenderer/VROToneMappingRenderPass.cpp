//
//  VROToneMappingRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/21/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROToneMappingRenderPass.h"
#include "VRODriver.h"
#include "VROImagePostProcess.h"
#include "VROImageShaderProgram.h"
#include "VRORenderContext.h"
#include "VROShaderModifier.h"
#include "VROAnimationFloat.h"
#include "VROOpenGL.h"
#include "VRORenderTarget.h"

VROToneMappingRenderPass::VROToneMappingRenderPass(VROToneMappingType type, std::shared_ptr<VRODriver> driver) :
    _type(type),
    _exposure(1.0),
    _gammaCorrectionEnabled(true) {
   
}

VROToneMappingRenderPass::~VROToneMappingRenderPass() {
    
}

std::shared_ptr<VROImagePostProcess> VROToneMappingRenderPass::createPostProcess(std::shared_ptr<VRODriver> driver,
                                                                                 bool gammaCorrect) {
    std::vector<std::string> samplers = { "hdr_texture" };
    std::vector<std::string> code = {
        "uniform sampler2D hdr_texture;",
        "highp vec3 hdr_color = texture(hdr_texture, v_texcoord).rgb;",
    };
    
    /*
     Perform tone-mapping.
     
     TODO VIRO-1521: When iOS Cardboard starts supporting sRGB backbuffers, we will no longer
                     need the gamma correction step in this shader, as the native backbuffer can
                     perform the gamma correction for free.

     TODO VIRO-1555 On Android GVR, we may have to eliminate the manual gamma correction here
                    in favor of using an Android generated SRGB buffer in ViroGVRLayout. We'll do
                    the same for iOS once VIRO-1521 is completed
     */
    std::vector<std::string> toneMappingCode;
    if (_type == VROToneMappingType::Reinhard) {
        toneMappingCode = {
            "uniform lowp float exposure;",
            "highp vec3 mapped = hdr_color / (hdr_color + vec3(1.0));",
        };
    }
    else {
        toneMappingCode = {
            "uniform lowp float exposure;",
            "highp vec3 mapped = vec3(1.0) - exp(-hdr_color * exposure);",
        };
    }
    code.insert(code.end(), toneMappingCode.begin(), toneMappingCode.end());
    
    /*
     Finally, gamma correct.
     */
    if (_gammaCorrectionEnabled) {
        code.push_back("const highp float gamma = 2.2;");
        code.push_back("mapped = pow(mapped, vec3(1.0 / gamma));");
        code.push_back("frag_color = vec4(mapped, 1.0);");
    }
    else {
        code.push_back("frag_color = vec4(mapped, 1.0);");
    }
    
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
    
    if (_type == VROToneMappingType::Exposure) {
        std::weak_ptr<VROToneMappingRenderPass> weakSelf = std::dynamic_pointer_cast<VROToneMappingRenderPass>(shared_from_this());
        modifier->setUniformBinder("exposure", [weakSelf] (VROUniform *uniform, GLuint location,
                                                           const VROGeometry *geometry, const VROMaterial *material) {
            std::shared_ptr<VROToneMappingRenderPass> strongSelf = weakSelf.lock();
            if (strongSelf) {
                uniform->setFloat(strongSelf->_exposure);
            }
        });
    }
    
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
    std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
    
    return driver->newImagePostProcess(shader);
}

VRORenderPassInputOutput VROToneMappingRenderPass::render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                                          VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    if (!_postProcessHDR) {
        _postProcessHDR = createPostProcess(driver, false);
        _postProcessHDRAndGamma = createPostProcess(driver, true);
    }
    
    std::shared_ptr<VRORenderTarget> hdrInput = inputs[kToneMappingHDRInput];
    std::shared_ptr<VRORenderTarget> target = inputs[kToneMappingOutput];
    
    pglpush("Tone Mapping");
    if (_gammaCorrectionEnabled) {
        _postProcessHDRAndGamma->blit(hdrInput, 0, target, {}, driver);
    }
    else {
        _postProcessHDR->blit(hdrInput, 0, target, {}, driver);
    }
    pglpop();
    
    VRORenderPassInputOutput output;
    output[kToneMappingOutput] = target;
    
    return output;
}

void VROToneMappingRenderPass::setGammaCorrectionEnabled(bool enabled) {
    _gammaCorrectionEnabled = enabled;
}

void VROToneMappingRenderPass::setExposure(float exposure) {
    if (_type != VROToneMappingType::Exposure) {
        return;
    }
    
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float e) {
        ((VROToneMappingRenderPass *)animatable)->_exposure = e;
    }, _exposure, exposure));
}

