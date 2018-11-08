//
//  VROIrradianceRenderPass.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROIrradianceRenderPass.h"
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

VROIrradianceRenderPass::VROIrradianceRenderPass() {
    
}

VROIrradianceRenderPass::~VROIrradianceRenderPass() {
    if (_cubeVBO != 0) {
        GL( glDeleteBuffers(1, &_cubeVBO) );
    }
    if (_cubeVAO != 0) {
        GL( glDeleteVertexArrays(1, &_cubeVAO) );
    }
}

void VROIrradianceRenderPass::init(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers = { "environment_map" };
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    std::vector<VROGeometrySourceSemantic> attributes;
    _shader = std::make_shared<VROShaderProgram>("irradiance_convolution_vsh", "irradiance_convolution_fsh", samplers, modifiers, 0,
                                                 std::dynamic_pointer_cast<VRODriverOpenGL>(driver));
    
    _irradianceRenderTarget = driver->newRenderTarget(VRORenderTargetType::CubeTextureHDR16, 1, 6, false, false);
    _irradianceRenderTarget->setViewport( { 0, 0, 32, 32 });
    _irradianceRenderTarget->hydrate();
}

void VROIrradianceRenderPass::render(std::shared_ptr<VROScene> scene,
                                     std::shared_ptr<VROScene> outgoingScene,
                                     VRORenderPassInputOutput &inputs,
                                     VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    if (!_shader) {
        init(driver);
    }
    pglpush("Irradiance");
    
    // Bind the HDR texture to texture unit 0
    VRORenderUtil::bindTexture(0, inputs.textures[kIrradianceLightingEnvironmentInput], driver);
    
    // Bind the destination render target
    driver->bindRenderTarget(_irradianceRenderTarget, VRORenderTargetUnbindOp::Invalidate);
    
    // Setup for rendering the cube
    VRORenderUtil::prepareForBlit(driver, true, false);

    // Compile and bind the shader and its corresponding uniforms
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);
    
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
    
    for (int i = 0; i < 6; ++i) {
        _shader->getUniform("view_matrix")->setMat4(captureViews[i]);
        _irradianceRenderTarget->setTextureCubeFace(i, 0, 0);
        GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
        VRORenderUtil::renderUnitCube(&_cubeVAO, &_cubeVBO);
    }
    driver->unbindShader();
    pglpop();
    inputs.outputTarget = _irradianceRenderTarget;
}



