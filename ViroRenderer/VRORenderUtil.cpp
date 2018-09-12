//
//  VRORenderUtil.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/23/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VRORenderUtil.h"
#include "VROOpenGL.h"
#include "VROTexture.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VRODriver.h"
#include "VROMaterial.h"

void VRORenderUtil::prepareForBlit(std::shared_ptr<VRODriver> &driver, bool enableDepth,
                                   bool enableStencil) {
    driver->setCullMode(VROCullMode::None);
    driver->setDepthWritingEnabled(enableDepth);
    driver->setDepthReadingEnabled(enableDepth);
    driver->setMaterialColorWritingMask(VROColorMaskAll);
    driver->setStencilTestEnabled(enableStencil);
    driver->setBlendingMode(VROBlendMode::Alpha);
}

void VRORenderUtil::renderUnitCube(unsigned int *vao, unsigned int *vbo) {
    if (*vao == 0) {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        GL( glGenVertexArrays(1, vao) );
        GL( glGenBuffers(1, vbo) );
        GL( glBindBuffer(GL_ARRAY_BUFFER, *vbo) );
        GL( glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW) );
        GL( glBindVertexArray(*vao) );
        GL( glEnableVertexAttribArray(0) );
        GL( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0) );
        GL( glEnableVertexAttribArray(1) );
        GL( glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))) );
        GL( glEnableVertexAttribArray(2) );
        GL( glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))) );
        GL( glBindBuffer(GL_ARRAY_BUFFER, 0) );
        GL( glBindVertexArray(0) );
    }
    
    GL( glBindVertexArray(*vao) );
    GL( glDrawArrays(GL_TRIANGLES, 0, 36) );
    GL( glBindVertexArray(0) );
}

void VRORenderUtil::renderQuad(unsigned int *vao, unsigned int *vbo) {
    if (*vao == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,   1.0f, 0.0f, 1.0f, 1.0f,
            1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };

        // setup quad VAO
        GL( glGenVertexArrays(1, vao) );
        GL( glGenBuffers(1, vbo) );
        GL( glBindVertexArray(*vao) );
        GL( glBindBuffer(GL_ARRAY_BUFFER, *vbo) );
        GL( glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW) );
        GL( glEnableVertexAttribArray(0) );
        GL( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0) );
        GL( glEnableVertexAttribArray(1) );
        GL( glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))) );
    }

    GL( glBindVertexArray(*vao) );
    GL( glDrawArrays(GL_TRIANGLE_STRIP, 0, 4) );
    GL( glBindVertexArray(0) );
}

bool VRORenderUtil::bindTexture(int unit, const std::shared_ptr<VROTexture> &texture,
                                std::shared_ptr<VRODriver> &driver) {
    VROTextureSubstrateOpenGL *substrate = (VROTextureSubstrateOpenGL *) texture->getSubstrate(0, driver, true);
    if (!substrate) {
        return false;
    }
    std::pair<GLenum, GLuint> targetAndTexture = substrate->getTexture();
    
    GL( glActiveTexture(GL_TEXTURE0 + unit) );
    GL( glBindTexture(targetAndTexture.first, targetAndTexture.second) );
    return true;
}
