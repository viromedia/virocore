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
     Retrieve a shader that is capable of rendering the given material with
     the given light configuration.
     
     This will first check if such a shader is cached.
     */
    std::shared_ptr<VROShaderProgram> getShader(const VROMaterial &material,
                                                const std::vector<std::shared_ptr<VROLight>> &lights,
                                                std::shared_ptr<VRODriverOpenGL> driver);
    
private:
    
    /*
     Shader programs cached by their capabilities.
     */
    std::map<VROShaderCapabilities, std::shared_ptr<VROShaderProgram>> _cachedPrograms;
    
    /*
     Derive a key that uniquely identifies the *capabilities* that the shader
     rendering these lights need. For example, a set of lights that require shadow
     map support will differ from a set of lights that do not.
     */
    VROLightingShaderCapabilities deriveLightingCapabilitiesKey(const std::vector<std::shared_ptr<VROLight>> &lights);
    
    /*
     Derive a key that uniquely identifies the *capabilities* that the shader
     rendering this material would need. For example, a material that requires
     stereo rendering, or a material that requires textures, will have a key that
     differs from materials that do not.
     */
    VROMaterialShaderCapabilities deriveMaterialCapabilitiesKey(const VROMaterial &material);
    
    /*
     Build and return a shader with the given capabilities and additional modifiers.
     */
    std::shared_ptr<VROShaderProgram> buildShader(VROShaderCapabilities capabilities,
                                                  const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                  std::shared_ptr<VRODriverOpenGL> driver);
    
    std::shared_ptr<VROShaderModifier> createDiffuseTextureModifier();
    std::shared_ptr<VROShaderModifier> createSpecularTextureModifier();
    std::shared_ptr<VROShaderModifier> createNormalMapTextureModifier();
    std::shared_ptr<VROShaderModifier> createReflectiveTextureModifier();
    std::shared_ptr<VROShaderModifier> createLambertLightingModifier();
    std::shared_ptr<VROShaderModifier> createPhongLightingModifier();
    std::shared_ptr<VROShaderModifier> createBlinnLightingModifier();
    std::shared_ptr<VROShaderModifier> createYCbCrTextureModifier();
    std::shared_ptr<VROShaderModifier> createEGLImageModifier();
    std::shared_ptr<VROShaderModifier> createShadowMapGeometryModifier();
    std::shared_ptr<VROShaderModifier> createShadowMapLightModifier();
    std::shared_ptr<VROShaderModifier> createStereoTextureModifier(VROStereoMode currentStereoMode);

};

#endif /* VROShaderFactory_h */
