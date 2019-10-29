//
//  VROGaussianBlurRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/24/17.
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

#include "VROGaussianBlurRenderPass.h"
#include "VRODriver.h"
#include "VROImagePostProcess.h"
#include "VROImageShaderProgram.h"
#include "VRORenderContext.h"
#include "VROShaderModifier.h"
#include "VROOpenGL.h"
#include "VRORenderTarget.h"
#include "VROMaterial.h"
#include "VROStringUtil.h"
#include "VROMath.h"
#include "VROViewport.h"

VROGaussianBlurRenderPass::VROGaussianBlurRenderPass() :
    _blurScaling(0.5),
    _numBlurIterations(4),
    _horizontal(false),
    _bilinearTextureLookup(true),
    _sigma(1),
    _kernelSize(5),
    _reinforcedIntensity(1),
    _normalizedKernel(true),
    _considerTransparentPixels(false),
    _preBlurPass(nullptr),
    _gaussianBlur(nullptr),
    _blurTargetA(nullptr),
    _blurTargetB(nullptr) {
}

VROGaussianBlurRenderPass::~VROGaussianBlurRenderPass() {
}

void VROGaussianBlurRenderPass::createRenderTargets(std::shared_ptr<VRODriver> &driver) {
    _blurTargetA = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false, false);
    _blurTargetB = driver->newRenderTarget(VRORenderTargetType::ColorTextureHDR16, 1, 1, false, false);
}

void VROGaussianBlurRenderPass::resetRenderTargets() {
    _blurTargetA.reset();
    _blurTargetB.reset();
}

void VROGaussianBlurRenderPass::setViewPort(VROViewport rtViewport,
                                            std::shared_ptr<VRODriver> &driver) {
    if (_blurTargetA) {
        _blurTargetA->setViewport({ rtViewport.getX(), rtViewport.getY(),
                                    (int) (rtViewport.getWidth()  * _blurScaling),
                                    (int) (rtViewport.getHeight() * _blurScaling) });
    }
    if (_blurTargetB) {
        _blurTargetB->setViewport({ rtViewport.getX(), rtViewport.getY(),
                                    (int) (rtViewport.getWidth()  * _blurScaling),
                                    (int) (rtViewport.getHeight() * _blurScaling) });
    }
}

void VROGaussianBlurRenderPass::setNumBlurIterations(int numIterations) {
    _numBlurIterations = numIterations;
    if (_numBlurIterations % 2 != 0) {
        ++_numBlurIterations;
    }
    resetShaders();
}

void VROGaussianBlurRenderPass::setBlurKernel(int kernelSize, float sigma,  bool normalized) {
    _kernelSize = kernelSize;
    _sigma = sigma;
    _normalizedKernel = normalized;
    resetShaders();
}

void VROGaussianBlurRenderPass::setBilinearTextureLookup(bool enabled) {
    _bilinearTextureLookup = enabled;
    resetShaders();
}

void VROGaussianBlurRenderPass::setClearColor(VROVector4f color) {
    if (_blurTargetA) {
        _blurTargetA->setClearColor(color);
    }
    if (_blurTargetB) {
        _blurTargetB->setClearColor(color);
    }

    _considerTransparentPixels = color.w != 1.0;
    resetShaders();
}

void VROGaussianBlurRenderPass::initPostProcess(std::shared_ptr<VRODriver> driver) {
    initBlurPass(driver);
    initPreBlurPass(driver);
}

void VROGaussianBlurRenderPass::initBlurPass(std::shared_ptr<VRODriver> driver) {
    // Generate kernel weights with the given kernel size and sigma.
    std::vector<float> kernelOffsets = {};
    std::vector<float> kernelWeight = calculateKernel(_sigma, _kernelSize, 1000);
    float count = 0;
    for (int i = 0; i < (int) kernelWeight.size(); i++) {
        kernelOffsets.push_back(count);
        count = count + 1.0;
    }

    // Linearlize the offsets if needed.
    if (_bilinearTextureLookup) {
        convertToLinearKernelSample(kernelWeight, kernelOffsets);
    }

    // Convert the kernel weights into shader code.
    std::string kernelWeightShader = VROStringUtil::toString(kernelWeight[0], 10);
    for (int i = 1; i < kernelWeight.size(); i++) {
        kernelWeightShader += ", " + VROStringUtil::toString(kernelWeight[i], 10);
    }
    std::string kernelOffsetShader = VROStringUtil::toString(kernelOffsets[0], 10);
    for (int i = 1; i < kernelOffsets.size(); i++) {
        kernelOffsetShader += ", " + VROStringUtil::toString(kernelOffsets[i], 10);
    }

    // Create our blur shader with the newly set kernel window and weights.
    int size = (int) kernelWeight.size();
    std::string sizeStr = VROStringUtil::toString(size);
    std::vector<std::string> samplers = { "image" };
    std::vector<std::string> code = {
        "uniform sampler2D image;",
        "uniform bool horizontal;",
        "const highp float offset["+sizeStr+"] = float[] ("+ kernelOffsetShader +");",
        "const highp float weight["+sizeStr+"] = float[]  ("+ kernelWeightShader +");",
        
        "ivec2 tex_size = textureSize(image, 0);",
        "highp vec2 tex_offset = vec2(1.0 / float(tex_size.x), 1.0 / float(tex_size.y));",
        "highp vec4 result = texture(image, v_texcoord);",
        "result *= weight[0];",
    };

    code.push_back("if (horizontal) {");
    for (int i = 1; i < size; i++) {
        std::string iStr = VROStringUtil::toString(i);
        code.push_back("highp vec4 p"+iStr+" = texture(image, v_texcoord + vec2(tex_offset.x * offset["+iStr+"], 0.0));");
        code.push_back("result += (p"+iStr+" * weight["+iStr+"]);");
        code.push_back("highp vec4 pNeg"+iStr+" = texture(image, v_texcoord - vec2(tex_offset.x * offset["+iStr+"], 0.0));");
        code.push_back("result += (pNeg"+iStr+" * weight["+iStr+"]);");
    }
    code.push_back("} else {");
    for (int y = 1; y < size; y ++) {
        std::string yStr = VROStringUtil::toString(y);
        code.push_back("highp vec4 p"+yStr+" = texture(image, v_texcoord + vec2(0.0, tex_offset.y * offset["+yStr+"]));");
        code.push_back("result += (p"+yStr+" * weight["+yStr+"]);");
        code.push_back("highp vec4 pNeg"+yStr+" = texture(image, v_texcoord - vec2(0.0, tex_offset.y * offset["+yStr+"]));");
        code.push_back("result += (pNeg"+yStr+" * weight["+yStr+"]);");
    }
    code.push_back("}");
    code.push_back("frag_color = result;");

    // Add modifiers for blurring the image horizontally and vertically.
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
    std::weak_ptr<VROGaussianBlurRenderPass> weakSelf = std::dynamic_pointer_cast<VROGaussianBlurRenderPass>(shared_from_this());
    modifier->setUniformBinder("horizontal", VROShaderProperty::Float,
                               [weakSelf] (VROUniform *uniform,
                                           const VROGeometry *geometry, const VROMaterial *material) {
        std::shared_ptr<VROGaussianBlurRenderPass> strongSelf = weakSelf.lock();
        if (strongSelf) {
            uniform->setFloat(strongSelf->_horizontal);
        }
    });

    // Finally create our ImagePostProcess program and cache it.
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
    std::shared_ptr<VROImageShaderProgram> shader = std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
    _gaussianBlur =  driver->newImagePostProcess(shader);
}

void VROGaussianBlurRenderPass::initPreBlurPass(std::shared_ptr<VRODriver> driver) {
    // When compositing bloom effects on to a semi-transparent renderer, we'll need consider
    // 'mixing' the alpha channels in the hdr output as well onto the resulting bloom. Thus,
    // to blend correctly with Gaussian blur, we have to use pre-multiplied alpha (on texture
    // lookup multiply the pixel's rgb by its alpha). This is only usually the case for
    // ViroViewScenes with semi-transparent backgrounds. Note that because we are mixing alpha
    // the resulting bloom will be weaker.
    if (_considerTransparentPixels) {
        std::vector<std::string> inputSampler = {"sampled_texture"};
        std::vector<std::string> preMultiplyShader = {
                "uniform sampler2D sampled_texture;",
                "highp vec4 base = texture(sampled_texture, v_texcoord);",
                "base.rgb *= base.a;",
                "frag_color = base;"
        };
        _preBlurPass = driver->newImagePostProcess(
                VROImageShaderProgram::create(inputSampler, preMultiplyShader, driver));
        return;
    }

    // Else, when compositing bloom effects onto a fully opaque renderer, do not mix the alpha
    // channel. This is to prevent the gaussian blur from 'eating into' and weakening the bloom.
    std::vector<std::string> inputSampler = {"sampled_texture"};
    std::vector<std::string> opaqueShader = {
            "uniform sampler2D sampled_texture;",
            "highp vec4 base = texture(sampled_texture, v_texcoord);",
            "frag_color = vec4(base.rgb * "+ VROStringUtil::toString(_reinforcedIntensity, 3) +", 1.0);"
    };
    _preBlurPass = driver->newImagePostProcess(
            VROImageShaderProgram::create(inputSampler, opaqueShader, driver));
}

void VROGaussianBlurRenderPass::render(std::shared_ptr<VROScene> scene,
                                       std::shared_ptr<VROScene> outgoingScene,
                                       VRORenderPassInputOutput &inputs,
                                       VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    passert (_numBlurIterations % 2 == 0);
    passert (_blurTargetA != nullptr && _blurTargetB != nullptr);
    _blurTargetA->hydrate();
    _blurTargetB->hydrate();

    inputs.targets[kGaussianPingPong] = _blurTargetA;
    inputs.outputTarget = _blurTargetB;

    // Setup blur shaders if we haven't already.
    if (_gaussianBlur == nullptr) {
        initPostProcess(driver);
    }

    // Reference our render targets on which to ping-pong.
    std::shared_ptr<VROTexture> input  =  inputs.textures[kGaussianInput];
    std::shared_ptr<VRORenderTarget> bufferA = inputs.targets[kGaussianPingPong];
    std::shared_ptr<VRORenderTarget> bufferB = inputs.outputTarget;

    // Apply any pre-blur passes with the given inputTexture onto Buffer B.
    _preBlurPass->begin(driver);
    driver->setBlendingMode(VROBlendMode::None);
    driver->bindRenderTarget(bufferB, VRORenderTargetUnbindOp::Invalidate);
    _preBlurPass->blitOpt({ input }, driver);
    _preBlurPass->end(driver);

    // Finally perform the blur starting on Buffer B.
    pglpush("Bloom");
    _gaussianBlur->begin(driver);
    driver->setBlendingMode(VROBlendMode::None);
    for (int i = 0; i < _numBlurIterations; i++) {
        if (i == 0) {
            driver->bindRenderTarget(bufferA, VRORenderTargetUnbindOp::Invalidate);
            _gaussianBlur->blitOpt({ bufferB->getTexture(0) }, driver);
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

std::vector<float> VROGaussianBlurRenderPass::calculateKernel(float sigma, float kernelSize, float sampleCount) {
    // Need an even number of intervals for simpson integration => odd number of samples
    int samplesPerBin = ceil(sampleCount / kernelSize);
    if(samplesPerBin % 2 == 0) {
        ++samplesPerBin;
    }

    // Get samples left and right of kernel support first
    float weightSum = 0;
    float kernelLeft = -floor(kernelSize/2);
    std::vector<std::vector<float>> outsideSamplesLeft
            = calculateSamplesForRange(-5 * sigma, kernelLeft - 0.5, samplesPerBin, sigma);
    std::vector<std::vector<float>> outsideSamplesRight
            = calculateSamplesForRange(-kernelLeft+0.5, 5 * sigma, samplesPerBin, sigma);
    std::vector<std::pair<std::vector<std::vector<float>>, float>> allSamples = {{outsideSamplesLeft, 0}};

    // Now sample kernel taps and calculate tap weights
    for(int tap = 0; tap < kernelSize; ++tap) {
        float left = kernelLeft - 0.5 + tap;
        std::vector<std::vector<float>> tapSamples
                = calculateSamplesForRange(left, left+1, samplesPerBin, sigma);
        float tapWeight = integrateSimphson(tapSamples);
        std::pair<std::vector<std::vector<float>>, float> kernelWeightPair
                = std::make_pair(tapSamples, tapWeight);
        allSamples.push_back(kernelWeightPair);
        weightSum += tapWeight;
    }
    allSamples.push_back({outsideSamplesRight, 0});

    // Re-normalize kernel and round to 6 decimals
    if (_normalizedKernel) {
        float sixDecimal = pow(10, 6);
        for(int i=0; i < allSamples.size(); ++i) {
            float f = allSamples[i].second / weightSum;
            float ff = (f * sixDecimal) + 0.5f;
            int roundedfInt = (int)(ff);
            float roundedfFloat = (float) roundedfInt / sixDecimal;
            allSamples[i].second = roundedfFloat;
        }
    }

    std::vector<float> finalKernel;
    for(int i = (int) allSamples.size() / 2; i < allSamples.size() - 1; ++i) {
        float k = allSamples[i].second;
        finalKernel.push_back(k);
    }
    return finalKernel;
}

float VROGaussianBlurRenderPass::integrateSimphson(std::vector<std::vector<float>> samples) {
    float result = samples[0][1] + samples[samples.size()-1][1];

    for(int s = 1; s < samples.size() - 1; ++s) {
        float sampleWeight = (s % 2 == 0) ? 2.0 : 4.0;
        result += sampleWeight * samples[s][1];
    }

    float h = (samples[samples.size() - 1][0] - samples[0][0]) / (samples.size() - 1);
    return result * h / 3.0;
}

std::vector<std::vector<float>> VROGaussianBlurRenderPass::calculateSamplesForRange(float minInclusive,
                                                                                    float maxInclusive,
                                                                                    int sampleCount,
                                                                                    float sigma) {
    std::vector<std::vector<float>> result;
    float stepSize = (maxInclusive - minInclusive) / (sampleCount-1);

    for(int s=0; s<sampleCount; ++s) {
        float x = minInclusive + s * stepSize;
        float y = gaussianDistribution(x, 0, sigma);
        result.push_back({x, y});
    }

    return result;
}

float VROGaussianBlurRenderPass::gaussianDistribution(float x, float mu, float sigma) {
    float d = x - mu;
    float n = 1.0 / (sqrt(2 * M_PI) * sigma);
    return exp(-d*d/(2 * sigma * sigma)) * n;
}

void VROGaussianBlurRenderPass::convertToLinearKernelSample(std::vector<float> &kernelWeights,
                                                            std::vector<float> &kernelOffsets) {
    std::vector<float> linearWeights = {};
    std::vector<float> linearOffsets = {};

    // Calculate the new offset window for linear sampling.
    linearWeights.push_back(kernelWeights[0]);
    for (int i = 1; i < kernelWeights.size(); i = i + 2) {
        float nextWeight = i + 1 < kernelWeights.size() ? kernelWeights[i + 1] : 0;
        float currentWeight = kernelWeights[i] + nextWeight;
        if (isnan(currentWeight) || currentWeight < 0.0 || isinf(currentWeight)){
            currentWeight = 0.0;
        }

        linearWeights.push_back(currentWeight);
    }

    // Calculate the new weighted window for linear sampling.
    linearOffsets.push_back(0.0);
    for (int i = 1; i < kernelOffsets.size(); i = i + 2) {
        float nextOffset = i + 1 < kernelOffsets.size() ? kernelOffsets[i + 1] : 0;
        float weightedOffset1 = (kernelOffsets[i] * kernelWeights[i]);
        float weightedOffset2 = (nextOffset * kernelWeights[i + 1]);
        float newOffset = (weightedOffset1 + weightedOffset2) / (kernelWeights[i] + kernelWeights[i + 1]);
        if (isnan(newOffset) || newOffset < 0.0 || isinf(newOffset)){
            newOffset = 0.0;
        }

        linearOffsets.push_back(newOffset);
    }

    kernelOffsets = linearOffsets;
    kernelWeights = linearWeights;
}

void VROGaussianBlurRenderPass::resetShaders() {
    _gaussianBlur = nullptr;
}
