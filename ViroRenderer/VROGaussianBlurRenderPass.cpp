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
#include "VROMaterial.h"

VROGaussianBlurRenderPass::VROGaussianBlurRenderPass() :
    _numBlurIterations(4),
    _horizontal(false) {
    
}

VROGaussianBlurRenderPass::~VROGaussianBlurRenderPass() {
    
}

void VROGaussianBlurRenderPass::initPostProcess(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "image" };

    std::vector<std::string> code = {
        "uniform sampler2D image;",
        "uniform bool horizontal;",
        "const highp float offset[3] = float[] (0.0, 1.3846153846, 3.2307692308);",
        "const highp float weight[3] = float[] (0.2270270270, 0.3162162162, 0.0702702703);",
        
        "ivec2 tex_size = textureSize(image, 0);",
        "highp vec2 tex_offset = vec2(1.0 / float(tex_size.x), 1.0 / float(tex_size.y));",
       
        // To blend correctly with Gaussian blur, we have to use premultiplied alpha. Therefore on
        // texture lookup multiply the pixel's rgb by its alpha. In addition, prior to running this
        // code we set our blend mode to VROBlendMode::PremultiplyAlpha, so that OpenGL correctly
        // blends each successive blur to the blur target. Note that the final output of the blur
        // is *also* premultiplied.
        "highp vec4 result = texture(image, v_texcoord);",
        "result.rgb *= result.a;",
        "result *= weight[0];",
        
        "if (horizontal)",
        "{",
        "   highp vec4 p0 = texture(image, v_texcoord + vec2(tex_offset.x * offset[1], 0.0));",
        "   p0.rgb *= p0.a;",
        "   result += (p0 * weight[1]);",
        
        "   highp vec4 p1 = texture(image, v_texcoord - vec2(tex_offset.x * offset[1], 0.0));",
        "   p1.rgb *= p1.a;",
        "   result += (p1 * weight[1]);",
        
        "   highp vec4 p2 = texture(image, v_texcoord + vec2(tex_offset.x * offset[2], 0.0));",
        "   p2.rgb *= p2.a;",
        "   result += (p2 * weight[2]);",
        
        "   highp vec4 p3 = texture(image, v_texcoord - vec2(tex_offset.x * offset[2], 0.0));",
        "   p3.rgb *= p3.a;",
        "   result += (p3 * weight[2]);",
        "}",
        "else",
        "{",
        "   highp vec4 p0 = texture(image, v_texcoord + vec2(0.0, tex_offset.y * offset[1]));",
        "   p0.rgb *= p0.a;",
        "   result += (p0 * weight[1]);",
        
        "   highp vec4 p1 = texture(image, v_texcoord - vec2(0.0, tex_offset.y * offset[1]));",
        "   p1.rgb *= p1.a;",
        "   result += (p1 * weight[1]);",
        
        "   highp vec4 p2 = texture(image, v_texcoord + vec2(0.0, tex_offset.y * offset[2]));",
        "   p2.rgb *= p2.a;",
        "   result += (p2 * weight[2]);",
        
        "   highp vec4 p3 = texture(image, v_texcoord - vec2(0.0, tex_offset.y * offset[2]));",
        "   p3.rgb *= p3.a;",
        "   result += (p3 * weight[2]);",
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

void VROGaussianBlurRenderPass::render(std::shared_ptr<VROScene> scene,
                                       std::shared_ptr<VROScene> outgoingScene,
                                       VRORenderPassInputOutput &inputs,
                                       VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    
    if (!_gaussianBlur) {
        initPostProcess(driver);
    }
    
    std::shared_ptr<VROTexture> input  =  inputs.textures[kGaussianInput];
    std::shared_ptr<VRORenderTarget> bufferA = inputs.targets[kGaussianPingPong];
    std::shared_ptr<VRORenderTarget> bufferB = inputs.outputTarget;
    
    _horizontal = true;
    passert (_numBlurIterations % 2 == 0);
    
    pglpush("Bloom");
    _gaussianBlur->begin(driver);
    driver->setBlendingMode(VROBlendMode::PremultiplyAlpha);
    
    for (int i = 0; i < _numBlurIterations; i++) {
        if (i == 0) {
            driver->bindRenderTarget(bufferA, VRORenderTargetUnbindOp::Invalidate);
            _gaussianBlur->blitOpt({ input }, driver);
        }
        else if (i % 2 == 1) {
            driver->bindRenderTarget(bufferB, VRORenderTargetUnbindOp::Invalidate);
            _gaussianBlur->blitOpt({ bufferA->getTexture(0) }, driver);
        }
        else {
            driver->bindRenderTarget(bufferA, VRORenderTargetUnbindOp::Invalidate);
            _gaussianBlur->blitOpt({ bufferB->getTexture(0) }, driver);
        }
        _horizontal = !_horizontal;
    }
    _gaussianBlur->end(driver);
    pglpop();
}
