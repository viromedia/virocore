//
//  VROEquirectangularToCubeRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROEquirectangularToCubeRenderPass.h"
#include "VRODriver.h"
#include "VROShaderProgram.h"
#include "VRORenderContext.h"
#include "VROOpenGL.h"
#include "VRORenderTarget.h"
#include "VROTexture.h"
#include "VRODriverOpenGL.h"
#include "VRORenderUtil.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

VROEquirectangularToCubeRenderPass::VROEquirectangularToCubeRenderPass() {
    
}

VROEquirectangularToCubeRenderPass::~VROEquirectangularToCubeRenderPass() {
    if (_cubeVBO != 0) {
        glDeleteBuffers(1, &_cubeVBO);
    }
    if (_cubeVAO != 0) {
        glDeleteVertexArrays(1, &_cubeVAO);
    }
}

void VROEquirectangularToCubeRenderPass::init(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "equirectangular_map" };
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    std::vector<VROGeometrySourceSemantic> attributes;
    _shader = std::make_shared<VROShaderProgram>("equirect_to_cube_vsh", "equirect_to_cube_fsh", samplers, modifiers, 0,
                                                 std::dynamic_pointer_cast<VRODriverOpenGL>(driver));
    
    _cubeRenderTarget = driver->newRenderTarget(VRORenderTargetType::CubeTextureHDR16, 1, 6, false);
    _cubeRenderTarget->setViewport( { 0, 0, 512, 512 });
}

void VROEquirectangularToCubeRenderPass::render(std::shared_ptr<VROScene> scene,
                                                std::shared_ptr<VROScene> outgoingScene,
                                                VRORenderPassInputOutput &inputs,
                                                VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    if (!_shader) {
        init(driver);
    }
    pglpush("EquirectToCube");
    
    // Bind the HDR texture to texture unit 0
    VRORenderUtil::bindTexture(0, inputs.textures[kEquirectangularToCubeHDRTextureInput], driver);
    
    // Bind the destination render target
    driver->bindRenderTarget(_cubeRenderTarget, VRORenderTargetUnbindOp::Invalidate);
    
    // Setup for rendering the cube
    driver->setDepthWritingEnabled(true);
    driver->setDepthReadingEnabled(true);
    driver->setStencilTestEnabled(false);
    
    // Compile and bind the shader and its corresponding uniforms
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);
    
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f,  1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f,  1.0f,  0.0f))
    };
    
    _shader->getUniform("projection_matrix")->setMat4(captureProjection);
    
    for (int i = 0; i < 6; ++i) {
        _shader->getUniform("view_matrix")->setMat4(captureViews[i]);
        _cubeRenderTarget->setTextureCubeFace(i, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        VRORenderUtil::renderUnitCube(&_cubeVAO, &_cubeVBO);
    }
    driver->unbindShader();
    pglpop();
    inputs.outputTarget = _cubeRenderTarget;
}

