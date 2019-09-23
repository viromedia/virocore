//
//  VROToneMappingRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/21/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROToneMappingRenderPass.h"
#include "VRODriver.h"
#include "VROImagePostProcess.h"
#include "VROImageShaderProgram.h"
#include "VRORenderContext.h"
#include "VROShaderModifier.h"
#include "VROAnimationFloat.h"
#include "VROOpenGL.h"
#include "VROMaterial.h"
#include "VRORenderTarget.h"

VROToneMappingRenderPass::VROToneMappingRenderPass(VROToneMappingMethod method, bool gammaCorrectSoftware,
                                                   std::shared_ptr<VRODriver> driver) :
    _method(method),
    _exposure(kToneMappingDefaultExposure),
    _whitePoint(kToneMappingDefaultWhitePoint),
    _gammaCorrectionEnabled(gammaCorrectSoftware) {
  
}

VROToneMappingRenderPass::~VROToneMappingRenderPass() {
    
}

std::shared_ptr<VROImagePostProcess> VROToneMappingRenderPass::createPostProcess(std::shared_ptr<VRODriver> driver,
                                                                                 VROToneMappingMethod method) {
    std::vector<std::string> samplers = { "hdr_texture", "tone_mapping_mask" };
    std::vector<std::string> code = {
        "uniform sampler2D hdr_texture;",
        "uniform sampler2D tone_mapping_mask;",
        "highp vec4 hdr_color = texture(hdr_texture, v_texcoord).rgba;",
        "lowp float tone_mapped = texture(tone_mapping_mask, v_texcoord).r;",
        "highp vec3 mapped;",
    };
    
    /*
     Perform tone-mapping.
     */
    std::vector<std::string> toneMappingCode;
    if (method == VROToneMappingMethod::Disabled) {
        toneMappingCode = {
            "mapped = hdr_color.rgb;",
        };
    }
    else if (method == VROToneMappingMethod::Reinhard) {
        toneMappingCode = {
            "uniform highp float exposure;",
            "highp vec3 H = hdr_color.rgb * pow(2.0, exposure);",
            "mapped = clamp(H / (H + vec3(1.0)), 0.0, 1.0);",
        };
    }
    else if (method == VROToneMappingMethod::Hable) {
        toneMappingCode = {
            "uniform highp float white_point;",
            "uniform highp float exposure;",
            "const highp float A = 0.15;",
            "const highp float B = 0.50;",
            "const highp float C = 0.10;",
            "const highp float D = 0.20;",
            "const highp float E = 0.02;",
            "const highp float F = 0.30;",
            
            "highp vec3 H = hdr_color.rgb * pow(2.0, exposure);",
            "highp vec3 W = vec3(white_point);",
            "highp vec3 hdr_mapped   = max(((H * (A * H + C * B) + D * E) / (H * (A * H + B) + D * F)) - E / F, vec3(0.0));",
            "highp vec3 white_mapped = max(((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F, vec3(0.0));",
            "mapped = clamp(hdr_mapped / white_mapped, 0.0, 1.0);",
        };
    }
    else if (method == VROToneMappingMethod::HableLuminanceOnly) {
        toneMappingCode = {
            "uniform highp float white_point;",
            "uniform highp float exposure;",
            "const highp float A = 0.15;",
            "const highp float B = 0.50;",
            "const highp float C = 0.10;",
            "const highp float D = 0.20;",
            "const highp float E = 0.02;",
            "const highp float F = 0.30;",
            
            "highp vec3 H = hdr_color.rgb * pow(2.0, exposure);",
            "highp vec3 W = vec3(white_point);",
            "highp float luminance_H = dot(H, vec3(0.2126, 0.7152, 0.0722));",
            "highp float luminance_W = dot(W, vec3(0.2126, 0.7152, 0.0722));",
            "highp float hdr_mapped_L   = max(((luminance_H * (A * luminance_H + C * B) + D * E) / (luminance_H * (A * luminance_H + B) + D * F)) - E / F, 0.0);",
            "highp float white_mapped_L = max(((luminance_W * (A * luminance_W + C * B) + D * E) / (luminance_W * (A * luminance_W + B) + D * F)) - E / F, 0.0);",
            "highp vec3 hdr_mapped   = (hdr_mapped_L / luminance_H) * H;",
            "highp vec3 white_mapped = (white_mapped_L / luminance_W) * W;",
            "mapped = clamp(hdr_mapped / white_mapped, 0.0, 1.0);",
        };
    }
    else {
        toneMappingCode = {
            "uniform lowp float exposure;",
            "mapped = vec3(1.0) - exp(-hdr_color.rgb * exposure);",
        };
    }
    code.insert(code.end(), toneMappingCode.begin(), toneMappingCode.end());
    
    // The tone_mapped value stored in the tone_mapping_mask texture is, for each fragment,
    // the material's tone mapping setting (1.0 to tone-map, 0.0 to not). Multiple fragments
    // are blended together [source * alpha + destination * (1 - alpha)] to get the final
    // tone_mapped value that is received here.
    //
    // This final tone_mapped value will range from 0.0 (no tone-mapping) to 1.0 (full
    // tone-mapping). We blend by this value to determine the amount of tone-mapping to apply.
    // This ensures that tone-mapped alpha surfaces displayed over not-tone-mapped backgrounds
    // blend smoothly from tone-mapped to not-tone-mapped.
    //
    // See VROShaderFactory::createToneMappingMaskModifier for more details on the tone-mapped
    // mask.
    code.push_back("mapped = mix(hdr_color.rgb, mapped, clamp(tone_mapped, 0.0, 1.0));");
    
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
    
    if (method == VROToneMappingMethod::Exposure || method == VROToneMappingMethod::Hable || method == VROToneMappingMethod::HableLuminanceOnly) {
        std::weak_ptr<VROToneMappingRenderPass> weakSelf = std::dynamic_pointer_cast<VROToneMappingRenderPass>(shared_from_this());
        modifier->setUniformBinder("exposure", VROShaderProperty::Float,
                                   [weakSelf] (VROUniform *uniform,
                                               const VROGeometry *geometry, const VROMaterial *material) {
            std::shared_ptr<VROToneMappingRenderPass> strongSelf = weakSelf.lock();
            if (strongSelf) {
                uniform->setFloat(strongSelf->_exposure);
            }
        });
    }
    if (method == VROToneMappingMethod::Hable || method == VROToneMappingMethod::HableLuminanceOnly) {
        std::weak_ptr<VROToneMappingRenderPass> weakSelf = std::dynamic_pointer_cast<VROToneMappingRenderPass>(shared_from_this());
        modifier->setUniformBinder("white_point", VROShaderProperty::Float,
                                   [weakSelf] (VROUniform *uniform,
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

void VROToneMappingRenderPass::render(std::shared_ptr<VROScene> scene,
                                      std::shared_ptr<VROScene> outgoingScene,
                                      VRORenderPassInputOutput &inputs,
                                      VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    if (!_postProcess) {
        _postProcess = createPostProcess(driver, _method);
    }
    
    std::shared_ptr<VROTexture> hdrInput = inputs.textures[kToneMappingHDRInput];
    std::shared_ptr<VROTexture> toneMappingMask = inputs.textures[kToneMappingMaskInput];
    std::shared_ptr<VRORenderTarget> target = inputs.outputTarget;

    pglpush("Tone Mapping");
    driver->bindRenderTarget(target, VRORenderTargetUnbindOp::Invalidate);
    _postProcess->blit({ hdrInput, toneMappingMask }, driver);
    pglpop();
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

