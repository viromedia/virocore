//
//  VROImagePostProcessOpenGL.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROImagePostProcessOpenGL.h"
#include "VROShaderProgram.h"
#include "VRORenderTarget.h"
#include "VRODriverOpenGL.h"
#include "VROOpenGL.h"
#include "VROGeometrySource.h"
#include "VROGeometryUtil.h"
#include "VROTexture.h"
#include "VRORenderUtil.h"
#include "VROLog.h"
#include "VROShaderModifier.h"
#include "VROTextureSubstrateOpenGL.h"

VROImagePostProcessOpenGL::VROImagePostProcessOpenGL(std::shared_ptr<VROShaderProgram> shader) :
    _shader(shader),
    _quadVAO(0),
    _quadVBO(0) {
    buildQuadFSVAR(false);
        
    /*
     Collect all the shader modifiers so we can bind them whenever this
     post-process is run.
     */
    for (const std::shared_ptr<VROShaderModifier> &modifier : _shader->getModifiers()) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniform *uniform = _shader->getUniform(uniformName);
            passert_msg (uniform != nullptr, "Failed to find shader modifier uniform '%s' in program!",
                         uniformName.c_str());
            _shaderModifierUniforms.push_back(uniform);
        }
    }
}

VROImagePostProcessOpenGL::~VROImagePostProcessOpenGL() {
    
}

void VROImagePostProcessOpenGL::setVerticalFlip(bool flip) {
    buildQuadFSVAR(flip);
}

void VROImagePostProcessOpenGL::blit(std::vector<std::shared_ptr<VROTexture>> textures,
                                     std::shared_ptr<VRODriver> &driver) {
    
    // Bind the source textures
    if (!bind(textures, driver)) {
        return;
    }
    
    driver->setDepthWritingEnabled(false);
    driver->setDepthReadingEnabled(false);
    driver->setStencilTestEnabled(true);
    
    // Compile and bind the shader and its corresponding uniforms
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);
    for (VROUniform *uniform : _shaderModifierUniforms) {
        uniform->set(nullptr, nullptr, nullptr);
    }
    
    drawScreenSpaceVAR();
    driver->unbindShader();
}

void VROImagePostProcessOpenGL::begin(std::shared_ptr<VRODriver> &driver) {
    driver->setDepthWritingEnabled(false);
    driver->setDepthReadingEnabled(false);
    driver->setStencilTestEnabled(true);
    
    // Compile and bind the shader
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);
}

void VROImagePostProcessOpenGL::blitOpt(std::vector<std::shared_ptr<VROTexture>> textures,
                                        std::shared_ptr<VRODriver> &driver) {
    
    // Bind the source textures
    if (!bind(textures, driver)) {
        return;
    }
    for (VROUniform *uniform : _shaderModifierUniforms) {
        uniform->set(nullptr, nullptr, nullptr);
    }
    drawScreenSpaceVAR();
}

void VROImagePostProcessOpenGL::end(std::shared_ptr<VRODriver> &driver) {
    driver->unbindShader();
}

bool VROImagePostProcessOpenGL::bind(std::vector<std::shared_ptr<VROTexture>> textures,
                                     std::shared_ptr<VRODriver> &driver) {
    
    // Bind textures to their units
    int unit = 0;
    for (std::shared_ptr<VROTexture> &texture : textures) {
        VRORenderUtil::bindTexture(unit, texture, driver);
        ++unit;
    }
    return true;
}

void VROImagePostProcessOpenGL::drawScreenSpaceVAR() {
    if (_quadVAO == 0) {
        GL( glGenBuffers(1, &_quadVBO));
        GL( glBindBuffer(GL_ARRAY_BUFFER, _quadVBO) );
        GL( glBufferData(GL_ARRAY_BUFFER, sizeof(_quadFSVAR), _quadFSVAR, GL_STATIC_DRAW) );
        
        GL( glGenVertexArrays(1, &_quadVAO) );
        GL( glBindVertexArray(_quadVAO) );
        
        int verticesIndex = VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Vertex);
        GL( glEnableVertexAttribArray(verticesIndex) );
        GL( glVertexAttribPointer(verticesIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0) );
        
        int texcoordIndex = VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Texcoord);
        GL( glEnableVertexAttribArray(texcoordIndex) );
        GL( glVertexAttribPointer(texcoordIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float))) );
    }
    
    GL( glBindVertexArray(_quadVAO) );
    GL( glDrawArrays(GL_TRIANGLE_STRIP, 0, 4) );
    GL( glBindVertexArray(0) );
}

void VROImagePostProcessOpenGL::buildQuadFSVAR(bool flipped) {
    float qstartV = flipped ? 1.0 : 0.0f;
    float qendV = flipped ? 0.0 : 1.0f;
    float qendU = 1.0f;
    
    float qleft = -1;
    float qright = 1;
    float qbottom = -1;
    float qtop = 1;
    
    //BL
    _quadFSVAR[0] = qleft;
    _quadFSVAR[1] = qbottom;
    _quadFSVAR[2] = 0;
    _quadFSVAR[3] = qstartV;
    
    //BR
    _quadFSVAR[4] = qright;
    _quadFSVAR[5] = qbottom;
    _quadFSVAR[6] = qendU;
    _quadFSVAR[7] = qstartV;
    
    //TL
    _quadFSVAR[8] = qleft;
    _quadFSVAR[9] = qtop;
    _quadFSVAR[10] = 0;
    _quadFSVAR[11] = qendV;
    
    //TR
    _quadFSVAR[12] = qright;
    _quadFSVAR[13] = qtop;
    _quadFSVAR[14] = qendU;
    _quadFSVAR[15] = qendV;
    
    //TL
    _quadFSVAR[16] = qleft;
    _quadFSVAR[17] = qtop;
    _quadFSVAR[18] = 0;
    _quadFSVAR[19] = qendV;
    
    //BR
    _quadFSVAR[20] = qright;
    _quadFSVAR[21] = qbottom;
    _quadFSVAR[22] = qendU;
    _quadFSVAR[23] = qstartV;
}
