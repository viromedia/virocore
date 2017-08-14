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
#include "VROLog.h"
#include "VROShaderModifier.h"
#include "VROTextureSubstrateOpenGL.h"

VROImagePostProcessOpenGL::VROImagePostProcessOpenGL(std::shared_ptr<VROShaderProgram> shader) :
    _shader(shader) {
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

bool VROImagePostProcessOpenGL::bindTexture(int unit, const std::shared_ptr<VROTexture> &texture,
                                            std::shared_ptr<VRODriver> &driver) {
    VROTextureSubstrateOpenGL *substrate = (VROTextureSubstrateOpenGL *) texture->getSubstrate(0, driver, nullptr);
    if (!substrate) {
        return false;
    }
    std::pair<GLenum, GLuint> targetAndTexture = substrate->getTexture();
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(targetAndTexture.first, targetAndTexture.second);
    return true;
}

void VROImagePostProcessOpenGL::blit(std::shared_ptr<VRORenderTarget> source,
                                     std::shared_ptr<VRORenderTarget> destination,
                                     std::shared_ptr<VRODriver> &driver) {
    if (!bind(source, destination, driver)) {
        return;
    }
    drawScreenSpaceVAR();
}

void VROImagePostProcessOpenGL::accumulate(std::shared_ptr<VRORenderTarget> source,
                                           std::shared_ptr<VRORenderTarget> destination,
                                           std::shared_ptr<VRODriver> &driver) {
    if (!bind(source, destination, driver)) {
        return;
    }
    glBlendFunc(GL_ONE, GL_ONE);
    drawScreenSpaceVAR();
    driver->setBlendingMode(VROBlendMode::Alpha);
}

bool VROImagePostProcessOpenGL::bind(std::shared_ptr<VRORenderTarget> source,
                                     std::shared_ptr<VRORenderTarget> destination,
                                     std::shared_ptr<VRODriver> &driver) {
    
    // Bind the source target to texture unit 0
    const std::shared_ptr<VROTexture> &texture = source->getTexture();
    passert_msg(texture != nullptr, "Render target had no texture: was a viewport set?");
    
    if (!bindTexture(0, texture, driver)) {
        return false;
    }
    
    // Bind the destination render target and disable depth testing
    destination->bind();
    driver->setDepthWritingEnabled(false);
    driver->setDepthReadingEnabled(false);
    
    // Compile and bind the shader and its corresponding uniforms
    if (!_shader->isHydrated()) {
        _shader->hydrate();
    }
    driver->bindShader(_shader);
    for (VROUniform *uniform : _shaderModifierUniforms) {
        uniform->set(nullptr, nullptr);
    }
    
    return true;
}

void VROImagePostProcessOpenGL::drawScreenSpaceVAR() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    int verticesIndex = VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Vertex);
    glEnableVertexAttribArray(verticesIndex);
    glVertexAttribPointer(verticesIndex, 2, GL_FLOAT, 0, 16, _quadFSVAR);

    int texcoordIndex = VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Texcoord);
    glEnableVertexAttribArray(texcoordIndex);
    glVertexAttribPointer(texcoordIndex, 2, GL_FLOAT, 0, 16, ((char *) _quadFSVAR + 2 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
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
