//
//  VROImageShaderProgram.h
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

#ifndef VROImageShaderProgram_h
#define VROImageShaderProgram_h

#include "VROShaderProgram.h"

class VRODriver;
class VRODriverOpenGL;

/*
 Subclass of shader program that simplifies the construction of shaders
 meant for 2D image post-processing.
 */
class VROImageShaderProgram : public VROShaderProgram {
public:
    
    /*
     Create a new post-processing shader program with the given samplers and
     code. The code is wrapped into an Image shader modifier.
     */
    static std::shared_ptr<VROShaderProgram> create(const std::vector<std::string> &samplers,
                                                    const std::vector<std::string> &code,
                                                    std::shared_ptr<VRODriver> driver);
    
    /*
     Use this constructor to attach specific modifiers (with uniform binders) for
     more complex image effects.
     */
    VROImageShaderProgram(const std::vector<std::string> &samplers,
                          const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                          std::shared_ptr<VRODriver> driver);
    virtual ~VROImageShaderProgram();
    
protected:
    
    // Override VROShaderProgram
    void bindAttributes();
    void bindUniformBlocks();
    void addStandardUniforms();
    
private:
    
    
};

#endif /* VROImageShaderProgram_h */
