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

VROGaussianBlurRenderPass::VROGaussianBlurRenderPass() :
    _numBlurIterations(10),
    _horizontal(false) {
    
}

VROGaussianBlurRenderPass::~VROGaussianBlurRenderPass() {
    
}

void VROGaussianBlurRenderPass::initPostProcess(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "image" };
    
    std::vector<std::string> code = {
        "uniform sampler2D image;",
        "uniform bool horizontal;",
        "const highp float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);",
        
        "highp vec2 tex_offset;",
        "ivec2 tex_size = textureSize(image, 0);",
        "tex_offset.x = 1.0 / float(tex_size.x);",
        "tex_offset.y = 1.0 / float(tex_size.y);",
        "highp vec3 result = texture(image, v_texcoord).rgb * weight[0];",
        "if (horizontal)",
        "{",
        "   for(int i = 1; i < 5; ++i)",
        "   {",
        "       result += texture(image, v_texcoord + vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];",
        "       result += texture(image, v_texcoord - vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];",
        "   }",
        "}",
        "else",
        "{",
        "   for(int i = 1; i < 5; ++i)",
        "   {",
        "       result += texture(image, v_texcoord + vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];",
        "       result += texture(image, v_texcoord - vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];",
        "   }",
        "}",
        "frag_color = vec4(result, 1.0);",
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

VRORenderPassInputOutput VROGaussianBlurRenderPass::render(std::shared_ptr<VROScene> scene, VRORenderPassInputOutput &inputs,
                                                          VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    if (!_gaussianBlur) {
        initPostProcess(driver);
    }
    
    std::shared_ptr<VRORenderTarget> input  =  inputs[kGaussianInput];
    std::shared_ptr<VRORenderTarget> bufferA = inputs[kGaussianPingPongA];
    std::shared_ptr<VRORenderTarget> bufferB = inputs[kGaussianPingPongB];
    
    _horizontal = true;
    int numIterations = 10;
    passert (numIterations % 2 == 0);
    
    pglpush("Bloom");
    for (int i = 0; i < numIterations; i++) {
        if (i == 0) {
            _gaussianBlur->blit(input, 1, bufferA, {}, driver);
        }
        else if (i % 2 == 1) {
            _gaussianBlur->blit(bufferA, 0, bufferB, {}, driver);
        }
        else {
            _gaussianBlur->blit(bufferB, 0, bufferA, {}, driver);
        }
        _horizontal = !_horizontal;
    }
    pglpop();
    
    VRORenderPassInputOutput renderPassOutput;
    renderPassOutput[kRenderTargetSingleOutput] = bufferB;
    return renderPassOutput;
}
