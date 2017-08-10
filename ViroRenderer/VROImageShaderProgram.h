//
//  VROImageShaderProgram.h
//  ViroKit
//
//  Created by Raj Advani on 8/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

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
