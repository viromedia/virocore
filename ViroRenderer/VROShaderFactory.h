//
//  VROShaderFactory.h
//  ViroKit
//
//  Created by Raj Advani on 8/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROShaderFactory_h
#define VROShaderFactory_h

#include <map>
#include <string>
#include <vector>
#include <memory>

class VROShaderProgram;
class VROFrameTimer;
class VROMaterial;
class VROLight;
class VROShaderModifier;
class VRODriverOpenGL;
class VROVector3f;
enum class VROStereoMode;

struct VROMaterialShaderCapabilities;
struct VROLightingShaderCapabilities;
struct VROShaderCapabilities;

/*
 The VROShaderFactory creates and caches VROShaderPrograms.
 */
class VROShaderFactory {
public:
       
    VROShaderFactory();
    virtual ~VROShaderFactory();
    
    /*
     Purge all shaders that are no longer used. If force is true, then we
     disregard time. If force is false, we only purge as many as we can
     until running out of frame time.
     */
    void purgeUnusedShaders(const VROFrameTimer &timer, bool force);
    
    /*
     Retrieve a shader that has the given material and lighting capabilities.
     If the shader is not cached, it will be created. The modifiers are required
     so that we can build them into the shader if it needs to be constructed.
     */
    std::shared_ptr<VROShaderProgram> getShader(VROMaterialShaderCapabilities materialCapabilities,
                                                VROLightingShaderCapabilities lightingCapabilities,
                                                const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                std::shared_ptr<VRODriverOpenGL> &driver);
    
private:
    
    /*
     Shader programs cached by their capabilities.
     */
    std::map<VROShaderCapabilities, std::shared_ptr<VROShaderProgram>> _cachedPrograms;
    
    /*
     Build and return a shader with the given capabilities and additional modifiers.
     */
    std::shared_ptr<VROShaderProgram> buildShader(VROShaderCapabilities capabilities,
                                                  const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                  std::shared_ptr<VRODriverOpenGL> &driver);
    
    std::shared_ptr<VROShaderModifier> createDiffuseTextureModifier();
    std::shared_ptr<VROShaderModifier> createSpecularTextureModifier();
    std::shared_ptr<VROShaderModifier> createNormalMapTextureModifier();
    std::shared_ptr<VROShaderModifier> createReflectiveTextureModifier();
    std::shared_ptr<VROShaderModifier> createRoughnessTextureModifier();
    std::shared_ptr<VROShaderModifier> createMetalnessTextureModifier();
    std::shared_ptr<VROShaderModifier> createAOTextureModifier();
    
    std::shared_ptr<VROShaderModifier> createLambertLightingModifier();
    std::shared_ptr<VROShaderModifier> createPhongLightingModifier();
    std::shared_ptr<VROShaderModifier> createBlinnLightingModifier();
    
    std::shared_ptr<VROShaderModifier> createShadowMapGeometryModifier();
    std::shared_ptr<VROShaderModifier> createShadowMapLightModifier();
    std::shared_ptr<VROShaderModifier> createShadowMapFragmentModifier();

    std::shared_ptr<VROShaderModifier> createPBRSurfaceModifier();
    std::shared_ptr<VROShaderModifier> createPBRDirectLightingModifier();
    std::shared_ptr<VROShaderModifier> createPBRConstantAmbientFragmentModifier();
    std::shared_ptr<VROShaderModifier> createPBRDiffuseIrradianceFragmentModifier();
    std::shared_ptr<VROShaderModifier> createPBRDiffuseAndSpecularIrradianceFragmentModifier();

    std::shared_ptr<VROShaderModifier> createYCbCrTextureModifier(bool linearizeColor);
    std::shared_ptr<VROShaderModifier> createEGLImageModifier(bool linearizeColor);
    std::shared_ptr<VROShaderModifier> createChromaKeyModifier(VROVector3f chromaKey);
    std::shared_ptr<VROShaderModifier> createStereoTextureModifier(VROStereoMode currentStereoMode);
    std::shared_ptr<VROShaderModifier> createBloomModifier();
    std::shared_ptr<VROShaderModifier> createToneMappingMaskModifier();
    std::vector<std::string> createColorLinearizationCode();
    
};

#endif /* VROShaderFactory_h */
