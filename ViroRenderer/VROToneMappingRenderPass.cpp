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

VROToneMappingRenderPass::VROToneMappingRenderPass(VROToneMappingMethod method, bool gammaCorrectSoftware,
                                                   std::shared_ptr<VRODriver> driver) :
    _method(method),
    _exposure(1.5),
    _whitePoint(11.2),
    _gammaCorrectionEnabled(gammaCorrectSoftware) {
  
}

VROToneMappingRenderPass::~VROToneMappingRenderPass() {
    
}

std::shared_ptr<VROImagePostProcess> VROToneMappingRenderPass::createPostProcess(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "hdr_texture" };
    std::vector<std::string> code = {
        "uniform sampler2D hdr_texture;",
        "highp vec4 hdr_color = texture(hdr_texture, v_texcoord).rgba;",
    };
    
    /*
     Perform tone-mapping.
     */
    std::vector<std::string> toneMappingCode;
    if (_method == VROToneMappingMethod::Disabled) {
        toneMappingCode = {
            "highp vec3 mapped = hdr_color.rgb;",
        };
    }
    else if (_method == VROToneMappingMethod::Reinhard) {
        toneMappingCode = {
            "uniform lowp float exposure;",
            "highp vec3 mapped = hdr_color.rgb / (hdr_color.rgb + vec3(1.0));",
        };
    }
    else if (_method == VROToneMappingMethod::Hable) {
        toneMappingCode = {
            "uniform highp float white_point;",
            "uniform highp float exposure;",
            "highp float A = 0.15;",
            "highp float B = 0.50;",
            "highp float C = 0.10;",
            "highp float D = 0.20;",
            "highp float E = 0.02;",
            "highp float F = 0.30;",
            "highp vec3 H = hdr_color.rgb * pow(2.0, exposure);",
            "highp vec3 W = vec3(white_point);",
            "highp vec3 hdr_mapped   = ((H * (A * H + C * B) + D * E) / (H * (A * H + B) + D * F)) - E / F;",
            "highp vec3 white_mapped = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;",
            "highp vec3 mapped = hdr_mapped / white_mapped;",
        };
    }
    else if (_method == VROToneMappingMethod::HableLuminanceOnly) {
        toneMappingCode = {
            "uniform highp float white_point;",
            "uniform highp float exposure;",
            "highp float A = 0.15;",
            "highp float B = 0.50;",
            "highp float C = 0.10;",
            "highp float D = 0.20;",
            "highp float E = 0.02;",
            "highp float F = 0.30;",
            "highp vec3 H = hdr_color.rgb * pow(2.0, exposure);",
            "highp vec3 W = vec3(white_point);",
            "highp float luminance_H = dot(H, vec3(0.2126, 0.7152, 0.0722));",
            "highp float luminance_W = dot(W, vec3(0.2126, 0.7152, 0.0722));",
            "highp float hdr_mapped_L   = ((luminance_H * (A * luminance_H + C * B) + D * E) / (luminance_H * (A * luminance_H + B) + D * F)) - E / F;",
            "highp float white_mapped_L = ((luminance_W * (A * luminance_W + C * B) + D * E) / (luminance_W * (A * luminance_W + B) + D * F)) - E / F;",
            "highp vec3 hdr_mapped   = (hdr_mapped_L / luminance_H) * H;",
            "highp vec3 white_mapped = (white_mapped_L / luminance_W) * W;",
            "highp vec3 mapped = hdr_mapped / white_mapped;",
        };
    }
    else {
        toneMappingCode = {
            "uniform lowp float exposure;",
            "highp vec3 mapped = vec3(1.0) - exp(-hdr_color.rgb * exposure);",
        };
    }
    code.insert(code.end(), toneMappingCode.begin(), toneMappingCode.end());
    
    /*
     Gamma correct in the shader if software gamma correction was requested.
     */
    if (_gammaCorrectionEnabled) {
        code.push_back("const highp float gamma = 2.2;");
        code.push_back("mapped = pow(mapped, vec3(1.0 / gamma));");
        code.push_back("frag_color = vec4(mapped, hdr_color.a);");
    
        pinfo("Software gamma correction enabled in tone-mapper");
    }
    else {
        code.push_back("frag_color = vec4(mapped, hdr_color.a);");

        pinfo("No gamma correction enabled in tone-mapper");
    }
    
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
    
    if (_method == VROToneMappingMethod::Exposure || _method == VROToneMappingMethod::Hable || _method == VROToneMappingMethod::HableLuminanceOnly) {
        std::weak_ptr<VROToneMappingRenderPass> weakSelf = std::dynamic_pointer_cast<VROToneMappingRenderPass>(shared_from_this());
        modifier->setUniformBinder("exposure", [weakSelf] (VROUniform *uniform, GLuint location,
                                                           const VROGeometry *geometry, const VROMaterial *material) {
            std::shared_ptr<VROToneMappingRenderPass> strongSelf = weakSelf.lock();
            if (strongSelf) {
                uniform->setFloat(strongSelf->_exposure);
            }
        });
    }
    if (_method == VROToneMappingMethod::Hable || _method == VROToneMappingMethod::HableLuminanceOnly) {
        std::weak_ptr<VROToneMappingRenderPass> weakSelf = std::dynamic_pointer_cast<VROToneMappingRenderPass>(shared_from_this());
        modifier->setUniformBinder("white_point", [weakSelf] (VROUniform *uniform, GLuint location,
                                                              const VROGeometry *geometry, const VROMaterial *material) {
            std::shared_ptr<VROToneMappingRenderPass> strongSelf = weakSelf.lock();
            if (strongSelf) {
                uniform->setFloat(strongSelf->_whitePoint);
            }
        });
    }
    
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
    std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
    
    return driver->newImagePostProcess(shader);
}

VRORenderPassInputOutput VROToneMappingRenderPass::render(std::shared_ptr<VROScene> scene,
                                                          std::shared_ptr<VROScene> outgoingScene,
                                                          VRORenderPassInputOutput &inputs,
                                                          VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    if (!_postProcess) {
        _postProcess = createPostProcess(driver);
    }
    
    std::shared_ptr<VRORenderTarget> hdrInput = inputs[kToneMappingHDRInput];
    std::shared_ptr<VRORenderTarget> target = inputs[kToneMappingOutput];
    
    pglpush("Tone Mapping");
    _postProcess->blit({ hdrInput->getTexture(0) }, target, driver);
    pglpop();
    
    VRORenderPassInputOutput output;
    output[kToneMappingOutput] = target;
    
    return output;
}

void VROToneMappingRenderPass::setMethod(VROToneMappingMethod method) {
    if (_method != method) {
        _method = method;
        _postProcess = nullptr;
    }
}

void VROToneMappingRenderPass::setExposure(float exposure) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float e) {
        ((VROToneMappingRenderPass *)animatable)->_exposure = e;
    }, _exposure, exposure));
}

void VROToneMappingRenderPass::setWhitePoint(float whitePoint) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float w) {
        ((VROToneMappingRenderPass *)animatable)->_whitePoint = w;
    }, _whitePoint, whitePoint));
}

