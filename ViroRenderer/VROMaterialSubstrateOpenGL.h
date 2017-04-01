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
class VROLightingUBO;
enum class VROEyeType;

class VROMaterialSubstrateOpenGL : public VROMaterialSubstrate {
    
public:
    
    VROMaterialSubstrateOpenGL(const VROMaterial &material, VRODriverOpenGL &driver);
    virtual ~VROMaterialSubstrateOpenGL();
    
    void bindShader();
    void bindLights(int lightsHash,
                    const std::vector<std::shared_ptr<VROLight>> &lights,
                    const VRORenderContext &context,
                    VRODriver &driver);
    
    void bindDepthSettings();
    void bindCullingSettings();
    void bindViewUniforms(VROMatrix4f transform, VROMatrix4f modelview,
                          VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                          VROVector3f cameraPosition);
    void bindMaterialUniforms(float opacity);
    
    const std::vector<std::shared_ptr<VROTexture>> &getTextures() const {
        return _textures;
    }
    
    void updateSortKey(VROSortKey &key) const;
    
private:
        
    void addUniforms();
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
    
    std::vector<VROUniform *> _shaderModifierUniforms;
    
    void loadConstantLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    void loadLambertLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    void loadPhongLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    void loadBlinnLighting(const VROMaterial &material, VRODriverOpenGL &driver);
    
    std::shared_ptr<VROShaderModifier> createDiffuseTextureModifier();
    std::shared_ptr<VROShaderModifier> createEGLImageModifier();
    uint32_t hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const;
    
};

#endif /* VROMaterialSubstrateOpenGL_h */
