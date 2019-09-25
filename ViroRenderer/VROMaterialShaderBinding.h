//
//  VROMaterialShaderBinding.h
//  ViroKit
//
//  Created by Raj Advani on 1/24/18.
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

#ifndef VROMaterialShaderBinding_h
#define VROMaterialShaderBinding_h

#include <memory>
#include <vector>
#include "VROShaderCapabilities.h"

class VROVector3f;
class VROMatrix4f;
class VROGeometry;
class VRODriver;
class VROTextureReference;
class VROUniform;
class VROUniformBinder;
class VROShaderProgram;
enum class VROEyeType;

/*
 The association between a material and a given shader program. Contains all the uniforms
 that we need to set each time we bind the shader with the properties of this material.
 
 The shader used, and therefore the uniforms that need to be bound, is a function of the
 material and the lighting configuration. These bindings are created by
 VROMaterial::getShaderBindingForLights(), which is invoked every frame in case the lighting
 environment changed (e.g. if a light changed from casting a shadow to not casting a shadow,
 or if a lighting environment was installed for the scene).
 */
class VROMaterialShaderBinding {
public:
    
    /*
     Construct a new binding between the given shader and the given material. The shader has
     the given lighting capabilities.
     */
    VROMaterialShaderBinding(std::shared_ptr<VROShaderProgram> program, VROLightingShaderCapabilities capabilities,
                             const VROMaterial &material);
    virtual ~VROMaterialShaderBinding();
    
    /*
     The binding's lighting capabilities: indicates what lights are compatible with this
     shader and material combination.
     */
    VROLightingShaderCapabilities lightingShaderCapabilities;
    
    void bindViewUniforms(VROMatrix4f &modelMatrix, VROMatrix4f &viewMatrix,
                          VROMatrix4f &projectionMatrix, VROMatrix4f &normalMatrix,
                          VROVector3f &cameraPosition, VROEyeType &eyeType);
    void bindMaterialUniforms(const VROMaterial &material,
                              std::shared_ptr<VRODriver> &driver);
    void bindGeometryUniforms(float opacity, const VROGeometry &geometry, const VROMaterial &material);
    
    std::shared_ptr<VROShaderProgram> &getProgram() {
        return _program;
    }
    const std::vector<VROTextureReference> &getTextures() const {
        return _textures;
    }
    
    /*
     Load the textures used by the material into this binding. Uses the samplers
     from the shader to determine the texture order. This should be invoked whenever
     the material's textures change.
     */
    void loadTextures();
    
private:
    
    /*
     The program and material associated with this binding.
     */
    std::shared_ptr<VROShaderProgram> _program;
    const VROMaterial &_material;
    
    /*
     The various uniforms are owned by the active VROShaderProgram.
     */
    VROUniform *_diffuseSurfaceColorUniform;
    VROUniform *_diffuseIntensityUniform;
    VROUniform *_alphaUniform;
    VROUniform *_shininessUniform;
    VROUniform *_roughnessUniform;
    VROUniform *_metalnessUniform;
    VROUniform *_roughnessIntensityUniform;
    VROUniform *_metalnessIntensityUniform;
    VROUniform *_aoUniform;
    
    VROUniform *_normalMatrixUniform;
    VROUniform *_modelMatrixUniform;
    VROUniform *_modelViewMatrixUniform;
    VROUniform *_viewMatrixUniform;
    VROUniform *_projectionMatrixUniform;
    
    VROUniform *_cameraPositionUniform;
    VROUniform *_eyeTypeUniform;
    std::vector<std::pair<VROUniformBinder *, VROUniform *>> _modifierUniformBinders;
    
    /*
     The textures of the material, in order of the samplers in the shader program.
     These consist of local textures (textures held by the material directly) and
     global textures (textures stored in the VRORenderContext).
     */
    std::vector<VROTextureReference> _textures;
    
    void loadUniforms();
    
};

#endif /* VROMaterialShaderBinding_h */
