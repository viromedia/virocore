//
//  VROImagePostProcessOpenGL.h
//  ViroKit
//
//  Created by Raj Advani on 8/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROImagePostProcessOpenGL_h
#define VROImagePostProcessOpenGL_h

#include "VROImagePostProcess.h"
#include <vector>

class VROShaderProgram;
class VROUniformBinder;
class VROUniform;

class VROImagePostProcessOpenGL : public VROImagePostProcess {
public:
    
    VROImagePostProcessOpenGL(std::shared_ptr<VROShaderProgram> shader);
    virtual ~VROImagePostProcessOpenGL();
    
    void setVerticalFlip(bool flip);

    void blit(std::vector<std::shared_ptr<VROTexture>> textures,
              std::shared_ptr<VRODriver> &driver);
    
    void begin(std::shared_ptr<VRODriver> &driver);
    void blitOpt(std::vector<std::shared_ptr<VROTexture>> textures,
                 std::shared_ptr<VRODriver> &driver);
    void end(std::shared_ptr<VRODriver> &driver);
    
private:
    
    /*
     Full screen quad for compositing operations.
     */
    float _quadFSVAR[24];
    unsigned int _quadVBO;
    unsigned int _quadVAO;
    
    /*
     The shader that will run on the input texture to produce the
     output texture. The vertex shader is generally fixed; the fragment
     shader is customizable through an Image shader modifier.
     */
    std::shared_ptr<VROShaderProgram> _shader;
    
    /*
     The uniforms (with binders) attached to the shaders. The callback
     associated with each of these is run, and the uniform bound, before
     rendering the post-process.
     */
    std::vector<std::pair<VROUniformBinder *, VROUniform *>> _uniformBinders;
    
    /*
     Perform common operations before running the post process. Returns true
     if the bind was successful.
     */
    bool bind(std::vector<std::shared_ptr<VROTexture>> textures,
              std::shared_ptr<VRODriver> &driver);
    
    /*
     Build the full-screen quad VAR.
     */
    void buildQuadFSVAR(bool flipped);
    
    /*
     Draw the vertex array.
     */
    void drawScreenSpaceVAR();
    
};

#endif /* VROImagePostProcessOpenGL_h */
