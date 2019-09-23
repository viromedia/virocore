//
//  VROBRDFRenderPass.cpp
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
        GL( glDeleteBuffers(1, &_quadVBO) );
    }
    if (_quadVAO != 0) {
        GL( glDeleteVertexArrays(1, &_quadVAO) );
    }
}

void VROBRDFRenderPass::init(std::shared_ptr<VRODriver> driver) {
    std::vector<std::string> samplers;
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    _shader = std::make_shared<VROShaderProgram>("brdf_vsh",
                                                 "brdf_fsh",
                                                 samplers, modifiers, 0,
                                                 std::dynamic_pointer_cast<VRODriverOpenGL>(driver));
    _BRDFRenderTarget = driver->newRenderTarget(VRORenderTargetType::ColorTextureRG16, 1, 1, false, false);
    _BRDFRenderTarget->setViewport({ 0, 0, 512, 512 });
    _BRDFRenderTarget->hydrate();
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
    driver->bindRenderTarget(_BRDFRenderTarget, VRORenderTargetUnbindOp::Invalidate);

    // Setup for rendering the quad
    VRORenderUtil::prepareForBlit(driver, true, false);

    // Compile and bind the shader and its corresponding uniforms
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);

    // Render our brdf convolution
    GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    VRORenderUtil::renderQuad(&_quadVAO, &_quadVBO);
    driver->unbindShader();
    pglpop();
    output.outputTarget = _BRDFRenderTarget;
}
