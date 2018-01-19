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
#include <map>
#include <memory>
#include "VROOpenGL.h"
#include "VROShaderCapabilities.h"

class VROShaderProgram;
class VRODriverOpenGL;
class VROLight;
class VROMatrix4f;
class VROVector3f;
class VROUniform;
class VROGeometry;
class VROLightingUBO;
class VROInstancedUBO;
class VROBoneUBO;
enum class VROEyeType;

/*
 The association between a material and a given shader program. Contains
 all the uniforms that we need to set each time we bind the shader with
 the properties of this material.
 
 The shader used, and therefore the uniforms that need to be bound, is a function
 of the material and the lighting configuration. These bindings are created
 by VROMaterial::getShaderBindingForLights().
 */
class VROMaterialShaderBinding {
public:
    
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
    void bindMaterialUniforms(const VROMaterial &material);
    void bindGeometryUniforms(float opacity, const VROGeometry &geometry, const VROMaterial &material);
    
    std::shared_ptr<VROShaderProgram> &getProgram() {
        return _program;
    }
    const std::vector<std::shared_ptr<VROTexture>> &getTextures() const {
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
    VROUniform *_aoUniform;
    
    VROUniform *_normalMatrixUniform;
    VROUniform *_modelMatrixUniform;
    VROUniform *_modelViewMatrixUniform;
    VROUniform *_viewMatrixUniform;
    VROUniform *_projectionMatrixUniform;
    
    VROUniform *_cameraPositionUniform;
    VROUniform *_eyeTypeUniform;
    std::vector<VROUniform *> _shaderModifierUniforms;
    
    /*
     The textures of the material, in order of the samplers in the
     shader program.
     */
    std::vector<std::shared_ptr<VROTexture>> _textures;
    
    void loadUniforms();

};

class VROMaterialSubstrateOpenGL : public VROMaterialSubstrate {
    
public:
    
    VROMaterialSubstrateOpenGL(VROMaterial &material, std::shared_ptr<VRODriverOpenGL> &driver);
    virtual ~VROMaterialSubstrateOpenGL();
    
    /*
     Bind the shader used in this material to the active rendering context.
     This is kept independent of the bindProperties() function because shader changes
     are expensive, so we want to manage them independent of materials in the
     render loop.
     
     The shader used is a function both of the underlying material properties
     and of the desired lighting configuration.
     */
    void bindShader(int lightsHash,
                    const std::vector<std::shared_ptr<VROLight>> &lights,
                    std::shared_ptr<VRODriver> &driver);
    
    /*
     Bind the properties of this material to the active rendering context.
     These properties should be node and geometry independent. The shader
     should always be bound first (via bindShader()).
     */
    void bindProperties();
    
    /*
     Bind the properties of the given geometry to the active rendering context.
     These are material properties (e.g. shader uniforms) that are dependent
     on properties of the geometry.
     */
    void bindGeometry(float opacity, const VROGeometry &geometry);
    
    /*
     Bind the properties of the view and projection to the active rendering
     context.
     */
    void bindView(VROMatrix4f modelMatrix, VROMatrix4f viewMatrix,
                  VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                  VROVector3f cameraPosition, VROEyeType eyeType);
    
    const std::vector<std::shared_ptr<VROTexture>> &getTextures() const {
        passert (_activeBinding != nullptr);
        return _activeBinding->getTextures();
    }
    
    void updateTextures();
    void updateSortKey(VROSortKey &key, const std::vector<std::shared_ptr<VROLight>> &lights,
                       std::shared_ptr<VRODriver> driver);

private:

    const VROMaterial &_material;
    VROMaterialShaderCapabilities _materialShaderCapabilities;
    
    /*
     The last used program's binding and its lighting capabilities. This is cached
     to reduce lookups into the _programs map.
     */
    VROMaterialShaderBinding *_activeBinding;
    std::shared_ptr<VROLightingUBO> _lightingUBO;
    
    /*
     The shadow map bound with the lights.
     */
    std::shared_ptr<VROTexture> _shadowMap;
    
    /*
     The shader programs that have been used by this material, each represented as a
     VROMaterialShaderBinding, a binding between this material and a program.
     
     Each program here is capable of rendering materials with _materialShaderCapabilities;
     what makes them unique is they render different lighting configurations (e.g. a
     material can have one shader program it uses when not rendering shadows, and
     another when it is rendering shadows).
     */
    std::map<VROLightingShaderCapabilities, std::unique_ptr<VROMaterialShaderBinding>> _shaderBindings;
    
    /*
     Get the shader program that should be used for the given light configuration.
     Returned as a material-shader binding. If no such binding exists, it is created
     and cached here.
     */
    VROMaterialShaderBinding *getShaderBindingForLights(const std::vector<std::shared_ptr<VROLight>> &lights,
                                                        std::shared_ptr<VRODriver> driver);

    uint32_t hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const;
    
};

#endif /* VROMaterialSubstrateOpenGL_h */
