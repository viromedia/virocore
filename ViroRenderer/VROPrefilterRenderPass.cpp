//
//  VROPrefilterRenderPass.cpp
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
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

#include "VROPrefilterRenderPass.h"
#include "VRODriver.h"
#include "VROShaderProgram.h"
#include "VRORenderContext.h"
#include "VROOpenGL.h"
#include "VRORenderTarget.h"
#include "VROTexture.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VRODriverOpenGL.h"
#include "VRORenderUtil.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

VROPrefilterRenderPass::VROPrefilterRenderPass() {
}

VROPrefilterRenderPass::~VROPrefilterRenderPass() {
    if (_cubeVBO != 0) {
        GL( glDeleteBuffers(1, &_cubeVBO) );
    }
    if (_cubeVAO != 0) {
        GL( glDeleteVertexArrays(1, &_cubeVAO) );
    }
}

void VROPrefilterRenderPass::init(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "environment_map" };
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    _shader = std::make_shared<VROShaderProgram>("prefilter_convolution_vsh",
                                                 "prefilter_convolution_fsh", samplers, modifiers, 0,
                                                 std::dynamic_pointer_cast<VRODriverOpenGL>(driver));
    _prefilterRenderTarget = driver->newRenderTarget(VRORenderTargetType::CubeTextureHDR16, 1, 6, true, false);
    _prefilterRenderTarget->setViewport( { 0, 0, 128, 128 });
    _prefilterRenderTarget->hydrate();
}

void VROPrefilterRenderPass::render(std::shared_ptr<VROScene> scene,
                                     std::shared_ptr<VROScene> outgoingScene,
                                     VRORenderPassInputOutput &inputs,
                                     VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    if (!_shader) {
        init(driver);
    }
    pglpush("Prefilter");

    // Bind the input lighting environment to texture unit 0
    VRORenderUtil::bindTexture(0, inputs.textures[kPrefilterLightingEnvironmentInput], driver);

    // Bind the destination render target
    driver->bindRenderTarget(_prefilterRenderTarget, VRORenderTargetUnbindOp::Invalidate);

    // Setup for rendering the cube
    VRORenderUtil::prepareForBlit(driver, true, false);

    // Compile and bind the shader and its corresponding uniforms
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);

    // Setup the perspective and view projections for the cube map.
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    _shader->getUniform("projection_matrix")->setMat4(captureProjection);

    // Configure the mip level as a correlation of pbr roughness.
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        _shader->getUniform("material_roughness")->setFloat(roughness);

        // Finally render each face of the convoluted prefilter cubemap.
        for (int i = 0; i < 6; ++i) {
            _shader->getUniform("view_matrix")->setMat4(captureViews[i]);
            _prefilterRenderTarget->setTextureCubeFace(i, mip, 0);
            GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
            VRORenderUtil::renderUnitCube(&_cubeVAO, &_cubeVBO);
        }
    }
    driver->unbindShader();
    pglpop();
    inputs.outputTarget = _prefilterRenderTarget;
}



