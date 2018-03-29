//
//  VROBRDFRenderPass.cpp
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROBRDFRenderPass.h"
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

VROBRDFRenderPass::VROBRDFRenderPass() {
}

VROBRDFRenderPass::~VROBRDFRenderPass() {
    if (_quadVBO != 0) {
        glDeleteBuffers(1, &_quadVBO);
    }
    if (_quadVAO != 0) {
        glDeleteVertexArrays(1, &_quadVAO);
    }
}

void VROBRDFRenderPass::init(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers;
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    _shader = std::make_shared<VROShaderProgram>("brdf_vsh",
                                                 "brdf_fsh",
                                                 samplers, modifiers, 0,
                                                 std::dynamic_pointer_cast<VRODriverOpenGL>(driver));
    _BRDFRenderTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureRG16, 1, 1, false);
    _BRDFRenderTarget->setViewport({ 0, 0, 512, 512 });
}

void VROBRDFRenderPass::render(std::shared_ptr<VROScene> scene,
                                     std::shared_ptr<VROScene> outgoingScene,
                                     VRORenderPassInputOutput &output,
                                     VRORenderContext *context, std::shared_ptr<VRODriver> &driver) {
    if (!_shader) {
        init(driver);
    }
    pglpush("BRDF");

    // Bind the destination render target
    driver->bindRenderTarget(_BRDFRenderTarget);

    // Setup for rendering the quad
    driver->setDepthWritingEnabled(true);
    driver->setDepthReadingEnabled(true);
    driver->setStencilTestEnabled(false);

    // Compile and bind the shader and its corresponding uniforms
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);

    // Render our brdf convolution
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    VRORenderUtil::renderQuad(&_quadVAO, &_quadVBO);
    driver->unbindShader();
    pglpop();
    output.outputTarget = _BRDFRenderTarget;
}
