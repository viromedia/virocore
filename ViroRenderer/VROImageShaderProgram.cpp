//
//  VROImageShaderProgram.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/10/17.
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
    GL( glBindAttribLocation(getProgram(), (int)VROGeometrySourceSemantic::Vertex, "position") );
    GL( glBindAttribLocation(getProgram(), VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Texcoord), "texcoord") );
}

void VROImageShaderProgram::bindUniformBlocks() {
    // No uniform blocks
}

void VROImageShaderProgram::addStandardUniforms() {
    // No standard uniforms
}
