//
//  VROMaterialSubstrateOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROMaterialSubstrateOpenGL_h
#define VROMaterialSubstrateOpenGL_h

#include "VROMaterial.h"
#include "VROMaterialSubstrate.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/glext.h>

class VROShaderProgram;
class VRODriverOpenGL;
class VROLight;
class VROMatrix4f;
class VROVector3f;
class VRORenderParameters;
enum class VROEyeType;

class VROMaterialSubstrateOpenGL : public VROMaterialSubstrate {
    
public:
    
    VROMaterialSubstrateOpenGL(const VROMaterial &material, const VRODriverOpenGL &driver);
    virtual ~VROMaterialSubstrateOpenGL();
    
    void bindShader();
    
    void bindViewUniforms(VROMatrix4f transform, VROMatrix4f modelview,
                          VROMatrix4f projectionMatrix, VROVector3f cameraPosition);
    
    /*
     Set the uniforms required to render this material.
     */
    void bindMaterialUniforms(VRORenderParameters &params, VROEyeType eye, int frame);
    
    /*
     Set the uniforms required to render this given material under the
     given lights.
     */
    void bindLightingUniforms(const std::vector<std::shared_ptr<VROLight>> &lights,
                              VROEyeType eye, int frame);
    
    const std::vector<std::shared_ptr<VROTexture>> &getTextures() const {
        return _textures;
    }
    
private:
    
    const VROMaterial &_material;
    VROLightingModel _lightingModel;
    
    VROShaderProgram *_program;
    std::vector<std::shared_ptr<VROTexture>> _textures;
    
    void loadLightUniforms(VROShaderProgram *program);
    void loadConstantLighting(const VROMaterial &material, const VRODriverOpenGL &driver);
    void loadLambertLighting(const VROMaterial &material, const VRODriverOpenGL &driver);
    void loadBlinnLighting(const VROMaterial &material, const VRODriverOpenGL &driver);
    
};

#endif /* VROMaterialSubstrateOpenGL_h */
