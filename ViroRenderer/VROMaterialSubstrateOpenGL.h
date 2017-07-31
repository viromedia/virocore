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

class VROShaderProgram;
class VRODriverOpenGL;
class VROLight;
class VROMatrix4f;
class VROVector3f;
class VROUniform;
class VROGeometry;
class VROLightingUBO;
class VROBoneUBO;
enum class VROEyeType;

class VROMaterialSubstrateOpenGL : public VROMaterialSubstrate {
    
public:
    
    VROMaterialSubstrateOpenGL(VROMaterial &material, VRODriverOpenGL &driver);
    virtual ~VROMaterialSubstrateOpenGL();
    
    /*
     Bind the shader used in this material to the active rendering context.
     This is kept independent of the bind() function because shader changes
     are expensive, so we want to manage them independent of materials in the
     render loop.
     */
    void bindShader();
    
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
     Bind the properties of the given lights to the active rendering context.
     */
    void bindLights(int lightsHash,
                    const std::vector<std::shared_ptr<VROLight>> &lights,
                    const VRORenderContext &context,
                    std::shared_ptr<VRODriver> &driver);
    
    /*
     Bind the properties of the view and projection to the active rendering
     context.
     */
    void bindView(VROMatrix4f transform, VROMatrix4f modelview,
                  VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                  VROVector3f cameraPosition, VROEyeType eyeType);
    
    void bindBoneUBO(const std::unique_ptr<VROBoneUBO> &boneUBO);
    
    const std::vector<std::shared_ptr<VROTexture>> &getTextures() const {
        return _textures;
    }
    
    void updateSortKey(VROSortKey &key) const;
    
private:
        
    void loadUniforms();
    void hydrateProgram(VRODriverOpenGL &driver);

    const VROMaterial &_material;
    VROLightingModel _lightingModel;
    
    std::shared_ptr<VROShaderProgram> _program;
    std::vector<std::shared_ptr<VROTexture>> _textures;
    std::shared_ptr<VROLightingUBO> _lightingUBO;
    
    VROUniform *_diffuseSurfaceColorUniform;
    VROUniform *_diffuseIntensityUniform;
    VROUniform *_alphaUniform;
    VROUniform *_shininessUniform;
    
    VROUniform *_normalMatrixUniform;
    VROUniform *_modelMatrixUniform;
    VROUniform *_modelViewMatrixUniform;
    VROUniform *_modelViewProjectionMatrixUniform;
    VROUniform *_cameraPositionUniform;
    VROUniform *_eyeTypeUniform;

    std::vector<VROUniform *> _shaderModifierUniforms;
    
    void bindDepthSettings();
    void bindCullingSettings();
    void bindMaterialUniforms();
    void bindGeometryUniforms(float opacity, const VROGeometry &geometry);
    
    void loadConstantLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    void loadLambertLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    void loadPhongLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    void loadBlinnLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    
    void configureSpecularShader(std::string vertexShader, std::string fragmentShader,
                                 const VROMaterial &material, VRODriverOpenGL &driver);
    
    std::shared_ptr<VROShaderModifier> createDiffuseTextureModifier();
    std::shared_ptr<VROShaderModifier> createNormalMapTextureModifier();
    std::shared_ptr<VROShaderModifier> createReflectiveTextureModifier();
    std::shared_ptr<VROShaderModifier> createYCbCrTextureModifier();
    std::shared_ptr<VROShaderModifier> createEGLImageModifier();
    std::shared_ptr<VROShaderModifier> createStereoTextureModifier(VROStereoMode currentStereoMode);

    uint32_t hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const;
    
};

#endif /* VROMaterialSubstrateOpenGL_h */
