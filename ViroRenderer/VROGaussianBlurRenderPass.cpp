//
//  VROGaussianBlurRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/24/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROGaussianBlurRenderPass.h"
#include "VRODriver.h"
#include "VROImagePostProcess.h"
#include "VROImageShaderProgram.h"
#include "VRORenderContext.h"
#include "VROShaderModifier.h"
#include "VROOpenGL.h"
#include "VRORenderTarget.h"

VROGaussianBlurRenderPass::VROGaussianBlurRenderPass() :
    _numBlurIterations(4),
    _horizontal(false) {
    
}

VROGaussianBlurRenderPass::~VROGaussianBlurRenderPass() {
    
}

void VROGaussianBlurRenderPass::initPostProcess(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "image" };

    // TODO VIRO-2680: Enable Alpha Channel for Gaussian Blur / Bloom Shaders
    std::vector<std::string> code = {
        "uniform sampler2D image;",
        "uniform bool horizontal;",
        "const highp float offset[3] = float[] (0.0, 1.3846153846, 3.2307692308);",
        "const highp float weight[3] = float[] (0.2270270270, 0.3162162162, 0.0702702703);",
        
        "ivec2 tex_size = textureSize(image, 0);",
        "highp vec2 tex_offset = vec2(1.0 / float(tex_size.x), 1.0 / float(tex_size.y));",
        
        "highp vec4 result = texture(image, v_texcoord).rgba * weight[0];",
        "if (horizontal)",
        "{",
        "   result += texture(image, v_texcoord + vec2(tex_offset.x * offset[1], 0.0)).rgba * weight[1];",
        "   result += texture(image, v_texcoord - vec2(tex_offset.x * offset[1], 0.0)).rgba * weight[1];",
        "   result += texture(image, v_texcoord + vec2(tex_offset.x * offset[2], 0.0)).rgba * weight[2];",
        "   result += texture(image, v_texcoord - vec2(tex_offset.x * offset[2], 0.0)).rgba * weight[2];",
        "}",
        "else",
        "{",
        "   result += texture(image, v_texcoord + vec2(0.0, tex_offset.y * offset[1])).rgba * weight[1];",
        "   result += texture(image, v_texcoord - vec2(0.0, tex_offset.y * offset[1])).rgba * weight[1];",
        "   result += texture(image, v_texcoord + vec2(0.0, tex_offset.y * offset[2])).rgba * weight[2];",
        "   result += texture(image, v_texcoord - vec2(0.0, tex_offset.y * offset[2])).rgba * weight[2];",
        "}",
        "frag_color = result;",
    };
    
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
    
    std::weak_ptr<VROGaussianBlurRenderPass> weakSelf = std::dynamic_pointer_cast<VROGaussianBlurRenderPass>(shared_from_this());
    modifier->setUniformBinder("horizontal", [weakSelf] (VROUniform *uniform, GLuint location,
                                                         const VROGeometry *geometry, const VROMaterial *material) {
        std::shared_ptr<VROGaussianBlurRenderPass> strongSelf = weakSelf.lock();
        if (strongSelf) {
            uniform->setFloat(strongSelf->_horizontal);
        }
    });
    
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
    std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
    
    _gaussianBlur = driver->newImagePostProcess(shader);
}

void VROGaussianBlurRenderPass::setNumBlurIterations(int numIterations) {
    _numBlurIterations = numIterations;
    if (_numBlurIterations % 2 != 0) {
        ++_numBlurIterations;
    }
}

VRORenderPassInputOutput VROGaussianBlurRenderPass::render(std::shared_ptr<VROScene> scene,
                                                           std::shared_ptr<VROScene> outgoingScene,
                                                           VRORenderPassInputOutput &inputs,
                                                           VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    if (!_gaussianBlur) {
        initPostProcess(driver);
    }
    
    std::shared_ptr<VRORenderTarget> input  =  inputs[kGaussianInput];
    std::shared_ptr<VRORenderTarget> bufferA = inputs[kGaussianPingPongA];
    std::shared_ptr<VRORenderTarget> bufferB = inputs[kGaussianPingPongB];
    
    _horizontal = true;
    passert (_numBlurIterations % 2 == 0);
    
    pglpush("Bloom");
    _gaussianBlur->begin(driver);
    for (int i = 0; i < _numBlurIterations; i++) {
        if (i == 0) {
            _gaussianBlur->blitOpt({ input->getTexture(1) }, bufferA, driver);
        }
        else if (i % 2 == 1) {
            _gaussianBlur->blitOpt({ bufferA->getTexture(0) }, bufferB, driver);
        }
        else {
            _gaussianBlur->blitOpt({ bufferB->getTexture(0) }, bufferA, driver);
        }
        _horizontal = !_horizontal;
    }
    _gaussianBlur->end(driver);
    pglpop();
    
    VRORenderPassInputOutput renderPassOutput;
    renderPassOutput[kRenderTargetSingleOutput] = bufferB;
    return renderPassOutput;
}
