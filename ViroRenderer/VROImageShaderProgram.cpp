//
//  VROImageShaderProgram.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROImageShaderProgram.h"
#include "VROGeometrySource.h"
#include "VROGeometryUtil.h"
#include "VROShaderModifier.h"
#include "VROOpenGL.h"
#include "VRODriverOpenGL.h"

std::shared_ptr<VROShaderProgram> VROImageShaderProgram::create(const std::vector<std::string> &samplers,
                                                                const std::vector<std::string> &code,
                                                                std::shared_ptr<VRODriver> driver) {
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Image, code);
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = { modifier };
    return std::make_shared<VROImageShaderProgram>(samplers, modifiers, driver);
}

VROImageShaderProgram::VROImageShaderProgram(const std::vector<std::string> &samplers,
                                             const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                             std::shared_ptr<VRODriver> driver) :
VROShaderProgram("image_vsh", "image_fsh", samplers, modifiers, (int) VROShaderMask::Tex,
                     std::dynamic_pointer_cast<VRODriverOpenGL>(driver)) {
}

VROImageShaderProgram::~VROImageShaderProgram() {
    
}

void VROImageShaderProgram::bindAttributes() {
    glBindAttribLocation(getProgram(), (int)VROGeometrySourceSemantic::Vertex, "position");
    glBindAttribLocation(getProgram(), VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Texcoord), "texcoord");
}

void VROImageShaderProgram::bindUniformBlocks() {
    // No uniform blocks
}

void VROImageShaderProgram::addStandardUniforms() {
    // No standard uniforms
}
