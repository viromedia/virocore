//
//  VROShaderFactory.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROShaderFactory.h"
#include "VROTexture.h"
#include "VROFrameTimer.h"
#include "VROShaderProgram.h"
#include "VROShaderModifier.h"
#include "VROLight.h"
#include "VROGeometrySource.h"
#include "VROMaterial.h"
#include "VROGeometry.h"
#include "VROEye.h"
#include "VROStringUtil.h"
#include "VROShadowMapRenderPass.h"
#include "VROShaderCapabilities.h"
#include "VRODriverOpenGL.h"
#include <tuple>

static std::shared_ptr<VROShaderModifier> sDiffuseTextureModifier;
static std::shared_ptr<VROShaderModifier> sSpecularTextureModifier;
static std::shared_ptr<VROShaderModifier> sNormalMapTextureModifier;
static std::shared_ptr<VROShaderModifier> sReflectiveTextureModifier;
static std::shared_ptr<VROShaderModifier> sRoughnessTextureModifier;
static std::shared_ptr<VROShaderModifier> sMetalnessTextureModifier;
static std::shared_ptr<VROShaderModifier> sAOTextureModifier;

static std::shared_ptr<VROShaderModifier> sLambertLightingModifier;
static std::shared_ptr<VROShaderModifier> sPhongLightingModifier;
static std::shared_ptr<VROShaderModifier> sBlinnLightingModifier;
static std::shared_ptr<VROShaderModifier> sPBRSurfaceModifier;
static std::shared_ptr<VROShaderModifier> sPBRDirectLightingModifier;
static std::shared_ptr<VROShaderModifier> sPBRConstantAmbientFragmentModifier;
static std::shared_ptr<VROShaderModifier> sPBRDiffuseIrradianceFragmentModifier;
static std::shared_ptr<VROShaderModifier> sPBRDiffuseAndSpecularIrradianceFragmentModifier;
static std::shared_ptr<VROShaderModifier> sYCbCrTextureModifier;
static std::shared_ptr<VROShaderModifier> sShadowMapGeometryModifier;
static std::shared_ptr<VROShaderModifier> sShadowMapLightModifier;
static std::shared_ptr<VROShaderModifier> sBloomModifier;
static std::map<VROStereoMode, std::shared_ptr<VROShaderModifier>> sStereoscopicTextureModifiers;

// Debugging
static std::shared_ptr<VROShaderModifier> sShadowMapFragmentModifier;

VROShaderFactory::VROShaderFactory() {
    
}

VROShaderFactory::~VROShaderFactory() {
    
}

#pragma mark - Shader Caching

std::shared_ptr<VROShaderProgram> VROShaderFactory::getShader(VROMaterialShaderCapabilities materialCapabilities,
                                                              VROLightingShaderCapabilities lightingCapabilities,
                                                              const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                              std::shared_ptr<VRODriverOpenGL> &driver) {
    
    VROShaderCapabilities capabilities;
    capabilities.materialCapabilities = materialCapabilities;
    capabilities.lightingCapabilities = lightingCapabilities;
    
    auto it = _cachedPrograms.find(capabilities);
    if (it == _cachedPrograms.end()) {
        std::shared_ptr<VROShaderProgram> program = buildShader(capabilities, modifiers, driver);
        _cachedPrograms[capabilities] = program;
        
        return program;
    }
    else {
        return it->second;
    }
}

void VROShaderFactory::purgeUnusedShaders(const VROFrameTimer &timer, bool force) {
    std::map<VROShaderCapabilities, std::shared_ptr<VROShaderProgram>>::iterator it = _cachedPrograms.begin();
    while (it != _cachedPrograms.end()) {
        if (!force && timer.isTimeRemainingInFrame()) {
            return;
        }
        
        if (it->second.unique()) {
            it = _cachedPrograms.erase(it);
        }
        else {
            ++it;
        }
    }
}

std::shared_ptr<VROShaderProgram> VROShaderFactory::buildShader(VROShaderCapabilities capabilities,
                                                                const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers_in,
                                                                std::shared_ptr<VRODriverOpenGL> &driver) {
    
    VROMaterialShaderCapabilities &materialCapabilities = capabilities.materialCapabilities;
    VROLightingShaderCapabilities &lightingCapabilities = capabilities.lightingCapabilities;
    
    // Degrade capabilities if using an antiquated device
    VROLightingModel lightingModel = materialCapabilities.lightingModel;
    bool shadows = lightingCapabilities.shadows;
    
    if (driver->getGPUType() == VROGPUType::Adreno330OrOlder) {
        lightingModel = VROLightingModel::Constant;
        shadows = false;
    }
    
    // Derive the base shader from the required capabilities
    std::string vertexShader = "standard_vsh";
    std::string fragmentShader;
    if (lightingModel == VROLightingModel::Constant) {
        if (materialCapabilities.diffuseTexture == VRODiffuseTextureType::Cube) {
            fragmentShader = "constant_q_fsh";
        }
        else {
            fragmentShader = "constant_fsh";
        }
    }
    else {
        fragmentShader = "standard_fsh";
    }
    if (driver->getGPUType() == VROGPUType::Adreno330OrOlder) {
        vertexShader   += "_adreno";
        fragmentShader += "_adreno";
    }
    
    std::vector<std::string> samplers;
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers;
    
    // Stereo mode must be placed prior to diffuse texture (because it modifies
    // the texture coordinates used when sampling the diffuse texture)
    if (materialCapabilities.diffuseTextureStereoMode != VROStereoMode::None) {
        modifiers.push_back(createStereoTextureModifier(materialCapabilities.diffuseTextureStereoMode));
    }
    
    // Diffuse Map
    if (materialCapabilities.diffuseTexture == VRODiffuseTextureType::Normal) {
        samplers.push_back("diffuse_texture");
        modifiers.push_back(createDiffuseTextureModifier());
    }
    else if (materialCapabilities.diffuseTexture == VRODiffuseTextureType::YCbCr) {
        samplers.push_back("diffuse_texture_y");
        samplers.push_back("diffuse_texture_cbcr");
        modifiers.push_back(createYCbCrTextureModifier(driver->getColorRenderingMode() != VROColorRenderingMode::NonLinear));
    }
    else if (materialCapabilities.diffuseTexture == VRODiffuseTextureType::Cube) {
        samplers.push_back("diffuse_texture");
        // No modifier here since constant_q has this built in
    }
    else {
        // Do nothing
    }
    
    if (materialCapabilities.diffuseEGLModifier) {
        modifiers.push_back(createEGLImageModifier(driver->getColorRenderingMode() != VROColorRenderingMode::NonLinear));
    }
    
    // Normal Map (note this must be placed before the PBR surface modifier, which uses the normal)
    if (materialCapabilities.normalTexture) {
        samplers.push_back("normal_texture");
        modifiers.push_back(createNormalMapTextureModifier());
    }
    
    // PBR lighting model
    if (lightingModel == VROLightingModel::PhysicallyBased &&
        lightingCapabilities.pbr) {
        if (materialCapabilities.roughnessMap) {
            samplers.push_back("roughness_map");
            modifiers.push_back(createRoughnessTextureModifier());
        }
        if (materialCapabilities.metalnessMap) {
            samplers.push_back("metalness_map");
            modifiers.push_back(createMetalnessTextureModifier());
        }
        if (materialCapabilities.aoMap) {
            samplers.push_back("ao_map");
            modifiers.push_back(createAOTextureModifier());
        }
        modifiers.push_back(createPBRSurfaceModifier());
        modifiers.push_back(createPBRDirectLightingModifier());
        
        if (lightingCapabilities.diffuseIrradiance && !lightingCapabilities.specularIrradiance) {
            samplers.push_back("irradiance_map");
            modifiers.push_back(createPBRDiffuseIrradianceFragmentModifier());
        }
        else if (lightingCapabilities.diffuseIrradiance && lightingCapabilities.specularIrradiance) {
            samplers.push_back("irradiance_map");
            samplers.push_back("prefiltered_map");
            samplers.push_back("brdf_map");
            modifiers.push_back(createPBRDiffuseAndSpecularIrradianceFragmentModifier());
        }
        else {
            modifiers.push_back(createPBRConstantAmbientFragmentModifier());
        }
    }
    
    // All other lighting models
    else {
        // Specular Map
        if (materialCapabilities.specularTexture) {
            samplers.push_back("specular_texture");
            modifiers.push_back(createSpecularTextureModifier());
        }
        
        // Lighting Model modifiers
        if (lightingModel == VROLightingModel::Lambert) {
            modifiers.push_back(createLambertLightingModifier());
        }
        // Blinn is used also as a fallback if PBR is disabled
        else if (lightingModel == VROLightingModel::Blinn ||
                 lightingModel == VROLightingModel::PhysicallyBased) {
            modifiers.push_back(createBlinnLightingModifier());
        }
        else if (lightingModel == VROLightingModel::Phong) {
            modifiers.push_back(createPhongLightingModifier());
        }
    }
    
    // Reflective Map
    if (materialCapabilities.reflectiveTexture) {
        samplers.push_back("reflect_texture");
        modifiers.push_back(createReflectiveTextureModifier());
    }
    
    // Shadow modifiers
    if (shadows && materialCapabilities.receivesShadows) {
        modifiers.push_back(createShadowMapGeometryModifier());
        modifiers.push_back(createShadowMapLightModifier());
        if (kDebugShadowMaps) {
            modifiers.push_back(createShadowMapFragmentModifier());
        }
        samplers.push_back("shadow_map");
    }
    
    // Bloom
    if (materialCapabilities.bloom && driver->isBloomSupported()) {
        modifiers.push_back(createBloomModifier());
    }
    
    // Custom material modifiers. These are added to the back of the modifiers list
    // so that they can build off the standard modifiers.
    modifiers.insert(modifiers.end(), modifiers_in.begin(), modifiers_in.end());
    
    // All shaders use these three base attributes. Add additional attributes from the
    // modifiers.
    int attributes = (int)(VROShaderMask::Tex) | (int)(VROShaderMask::Norm) | (int)(VROShaderMask::Tangent);
    for (std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
        attributes |= modifier->getAttributes();
    }
    
    return std::make_shared<VROShaderProgram>(vertexShader, fragmentShader, samplers, modifiers, attributes,
                                              driver);
}

#pragma mark - Texture Modifiers

std::shared_ptr<VROShaderModifier> VROShaderFactory::createDiffuseTextureModifier() {
    /*
     Modifier that multiplies the material's surface color by a diffuse texture.
     */
    if (!sDiffuseTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform sampler2D diffuse_texture;",
            "_surface.diffuse_color *= texture(diffuse_texture, _surface.diffuse_texcoord);"
        };
        sDiffuseTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                      modifierCode);
        sDiffuseTextureModifier->setName("diffuse");
    }
    return sDiffuseTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createSpecularTextureModifier() {
    /*
     Modifier that multiplies the material's specular color by a specular texture.
     */
    if (!sSpecularTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform sampler2D specular_texture;",
            "_surface.specular_color = texture(specular_texture, _surface.specular_texcoord).xyz;"
        };
        sSpecularTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                       modifierCode);
        sSpecularTextureModifier->setName("spec");
    }
    return sSpecularTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createNormalMapTextureModifier() {
    /*
     Modifier that samples a normal map to determine the direction of the normal to use at each
     fragment.
     */
    if (!sNormalMapTextureModifier) {
        std::vector<std::string> modifierCode =  {
            // Note that both normalize() calls are necessary: we normalize the sample, then normalize the
            // result after multiplyign by TBN. If we remove one, the 'glinting' effect in PBR disappears as
            // we lose unit length
            "uniform sampler2D normal_texture;",
            "_surface.normal = normalize(v_tbn * normalize( texture(normal_texture, _surface.diffuse_texcoord).xyz * 2.0 - 1.0 ));"
        };
        sNormalMapTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                        modifierCode);
        sNormalMapTextureModifier->setName("normal");
    }
    return sNormalMapTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createRoughnessTextureModifier() {
    if (!sRoughnessTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform sampler2D roughness_map;",
            "_surface.roughness = max(0.04, texture(roughness_map, _surface.diffuse_texcoord).r);"
        };
        sRoughnessTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                        modifierCode);
        sRoughnessTextureModifier->setName("roughness");
    }
    return sRoughnessTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createMetalnessTextureModifier() {
    if (!sMetalnessTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform sampler2D metalness_map;",
            "_surface.metalness = texture(metalness_map, _surface.diffuse_texcoord).r;"
        };
        sMetalnessTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                        modifierCode);
        sMetalnessTextureModifier->setName("metalness");
    }
    return sMetalnessTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createAOTextureModifier() {
    if (!sAOTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform sampler2D ao_map;",
            "_surface.ao = texture(ao_map, _surface.diffuse_texcoord).r;"
        };
        sAOTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                 modifierCode);
        sAOTextureModifier->setName("diffuse");
    }
    return sAOTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createReflectiveTextureModifier() {
    /*
     Modifier that adds reflective color to the final light computation.
     */
    if (!sReflectiveTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform samplerCube reflect_texture;",
            "lowp vec4 reflective_color = compute_reflection(_surface.position, camera_position, _surface.normal, reflect_texture);",
            "_output_color.xyz += reflective_color.xyz;"
        };
        sReflectiveTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                         modifierCode);
        sReflectiveTextureModifier->setName("reflect");
    }
    
    return sReflectiveTextureModifier;
}

#pragma mark - Shadow Modifiers

std::shared_ptr<VROShaderModifier> VROShaderFactory::createShadowMapGeometryModifier() {
    /*
     Modifier that outputs shadow map texture coordinates for the fragment shader.
     
     Note it's VERY important to ensure the w coordinate gets influenced by the
     bias matrix (which shifts us from [-1,1] to [0,1]). We have to add 0.5w because
     we're shifting to [-1, 1] *before* the perspective divide.
     */
    if (!sShadowMapGeometryModifier) {
        std::vector<std::string> modifierCode = {
            "out highp vec4 shadow_coords[8];",
            "for (int i = 0; i < lv_num_lights; i++) {",
            "   shadow_coords[i] = shadow_projection_matrices[i] * shadow_view_matrices[i] * _transforms.model_matrix * vec4(_geometry.position.xyz, 1.0);",
            "   shadow_coords[i].x = shadow_coords[i].x * 0.5 + shadow_coords[i].w * 0.5;",
            "   shadow_coords[i].y = shadow_coords[i].y * 0.5 + shadow_coords[i].w * 0.5;",
            "   shadow_coords[i].z = shadow_coords[i].z * 0.5 + shadow_coords[i].w * 0.5;",
            "}",
        };
        
        sShadowMapGeometryModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                         modifierCode);
        sShadowMapGeometryModifier->setName("shadow");
    }
    return sShadowMapGeometryModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createShadowMapLightModifier() {
    /*
     Modifier that samples a shadow map to determine if the fragment is in light.
     */
    if (!sShadowMapLightModifier) {
        std::vector<std::string> modifierCode;
        
        // This block is run when we're in Shadow Maps debugging mode. It operates
        // on a single shadow map.
        if (kDebugShadowMaps) {
            modifierCode = {
                "uniform highp sampler2DShadow shadow_map;",
                "in highp vec4 shadow_coords[8];",
        
                // Build the comparison vector. The x and y coordinates are the texture lookup, and require
                // perspective divide. The z coordinate is the depth value of the current fragment; it also
                // needs the perspective divide and must be adjusted by bias to prevent z-fighting (acne).
                "highp vec3 comparison = vec3(shadow_coords[i].xy / shadow_coords[i].w, (shadow_coords[i].z - lights[i].shadow_bias) / shadow_coords[i].w);",
                
                // Boundary condition to keep the area outside the texture map white.
                "if (lights[i].shadow_map_index < 0 || comparison.x < 0.0 || comparison.y < 0.0 || comparison.x > 1.0 || comparison.y > 1.0) {",
                "    _lightingContribution.visibility = 1.0;",
                
                // Perform the shadow test: the texture() command compares the occluder depth (the depth in
                // the map) to the current fragment depth with PCF. We modify this by our shadow opacity param.
                "} else {",
                "    lowp float shadow_intensity = lights[i].shadow_opacity * (1.0 - texture(shadow_map, comparison));",
                "    _lightingContribution.visibility = 1.0 - shadow_intensity;",
                "}"
            };
        }
        
        else {
            modifierCode = {
                "uniform highp sampler2DArrayShadow shadow_map;",
                "in highp vec4 shadow_coords[8];",
                
                // Build the comparison vector. The x and y coordinates are the texture lookup, and require
                // perspective divide. The w coordinate is the depth value of the current fragment; it also
                // needs the perspective divide and must be adjusted by bias to prevent z-fighting (acne).
                // Finally, the z coordinate is the index into the texture array that we are checking.
                "highp vec4 comparison = vec4(shadow_coords[i].xy / shadow_coords[i].w, lights[i].shadow_map_index, (shadow_coords[i].z - lights[i].shadow_bias) / shadow_coords[i].w);",

                // Boundary condition to keep the area outside the texture map white.
                "if (lights[i].shadow_map_index < 0 || comparison.x < 0.0 || comparison.y < 0.0 || comparison.x > 1.0 || comparison.y > 1.0) {",
                "    _lightingContribution.visibility = 1.0;",

                // Perform the shadow test: the texture() command compares the occluder depth (the depth in
                // the map) to the current fragment depth with PCF. We modify this by our shadow opacity param.
                "} else {",
                "    lowp float shadow_intensity = lights[i].shadow_opacity * (1.0 - texture(shadow_map, comparison));",
                "    _lightingContribution.visibility = 1.0 - shadow_intensity;",
                "}",
            };
        }
        
        sShadowMapLightModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                      modifierCode);
        // No name added to the modifier because the vertex modifier has one
    }
    return sShadowMapLightModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createShadowMapFragmentModifier() {
    /*
     Modifier that can change the _output_color. For shadow map debugging. Left
     checked-in because may be useful when working on Cascaded Shadow Maps.
     */
    if (!sShadowMapFragmentModifier) {
        std::vector<std::string> modifierCode=  {
            //"_output_color = vec4(?, ?, ?, ?);
        };
        sShadowMapFragmentModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                         modifierCode);
        sShadowMapFragmentModifier->setName("shadowdebug");
    }
    return sShadowMapFragmentModifier;
}

#pragma mark - Lighting Model Modifiers

std::shared_ptr<VROShaderModifier> VROShaderFactory::createLambertLightingModifier() {
    /*
     Modifier that implements the Lambert lighting model.
     */
    if (!sLambertLightingModifier) {
        std::vector<std::string> modifierCode = {
            "highp vec3 L;",
            "highp float attenuation = compute_attenuation(_light, _surface.position, L);",
            "highp vec3 luminance = _light.color * _light.intensity / 1000.0;",
            "highp float diffuse_coeff = max(0.0, dot(_surface.normal, L));",
            "_lightingContribution.diffuse += (attenuation * diffuse_coeff * luminance);",
        };
        
        sLambertLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                       modifierCode);
        sLambertLightingModifier->setName("lambert");
    }
    return sLambertLightingModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createPhongLightingModifier() {
    /*
     Modifier that implements the Phong lighting model.
     */
    if (!sPhongLightingModifier) {
        std::vector<std::string> modifierCode = {
            "highp vec3 L;",
            "highp float attenuation = compute_attenuation(_light, _surface.position, L);",
            "highp vec3 luminance = _light.color * _light.intensity / 1000.0;",
            "highp float diffuse_coeff = max(0.0, dot(_surface.normal, L));",
            "_lightingContribution.diffuse += (attenuation * diffuse_coeff * luminance);",
            "lowp float specular_coeff = 0.0;",
            "if (diffuse_coeff > 0.0) {",
            "    specular_coeff = pow(max(0.0, dot(_surface.view,",
            "                                      reflect(-L, _surface.normal))),",
            "                         _surface.shininess);",
            "}",
            "_lightingContribution.specular += (attenuation * specular_coeff * luminance);",
        };
        
        sPhongLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                     modifierCode);
        sPhongLightingModifier->setName("phong");
    }
    return sPhongLightingModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createBlinnLightingModifier() {
    /*
     Modifier that implements the Blinn lighting model.
     */
    if (!sBlinnLightingModifier) {
        std::vector<std::string> modifierCode = {
            "highp vec3 L;",
            "highp float attenuation = compute_attenuation(_light, _surface.position, L);",
            "highp float diffuse_coeff = max(0.0, dot(_surface.normal, L));",
            "highp vec3 luminance = _light.color * _light.intensity / 1000.0;",
            "_lightingContribution.diffuse += (attenuation * diffuse_coeff * luminance);",
            "lowp float specular_coeff = 0.0;",
            "if (diffuse_coeff > 0.0) {",
            "    specular_coeff = pow(max(0.0, dot(normalize(_surface.view + L),",
            "                                      _surface.normal)),",
            "                         _surface.shininess);",
            "}",
            "_lightingContribution.specular += (attenuation * specular_coeff * luminance);",
        };
        
        sBlinnLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                     modifierCode);
        sBlinnLightingModifier->setName("blinn");
    }
    return sBlinnLightingModifier;
}

#pragma mark - PBR Modifiers

std::shared_ptr<VROShaderModifier> VROShaderFactory::createPBRSurfaceModifier() {
    /*
     Computes PBR values that apply to all lights.
     */
    if (!sPBRSurfaceModifier) {
        std::vector<std::string> modifierCode =  {
            "highp vec3 albedo = _surface.diffuse_color.xyz;",
            "highp vec3 F0 = vec3(0.04);",
            "F0 = mix(F0, albedo, _surface.metalness);",
            "highp vec3 V = _surface.view;",
            "highp vec3 N = _surface.normal;",
        };
        sPBRSurfaceModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                  modifierCode);
        sPBRSurfaceModifier->setName("pbr_surface");
    }
    return sPBRSurfaceModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createPBRDirectLightingModifier() {
    /*
     Modifier that implements physically based lighting from direct light sources.
     */
    if (!sPBRDirectLightingModifier) {
        std::vector<std::string> modifierCode = {
            // Compute the attenuation (which factors in the luminous flux to intensity conversion)
            "highp vec3 L;",
            "highp float attenuation = compute_attenuation_pbr(_light, _surface.position, L);",
            "highp vec3 H = normalize(V + L);",
            "highp float NdotL = max(dot(N, L), 0.0);",
            
            // Compute the luminance of the light (the intensity per unit area is baked into the
            // attenuation)
            "highp vec3 luminance = _light.color * attenuation;",
            
            // Cook-Torrance BRDF
            
            // Specular component (NDF: normal distribution function, G: geometry function, F: reflectance function)
            // Note the divide by PI here is in the distribution_GGX function
            "highp float NDF = distribution_ggx(N, H, _surface.roughness);",
            "highp float G   = geometry_smith(N, V, L, _surface.roughness);",
            "highp vec3  F   = fresnel_schlick(clamp(dot(H, V), 0.0, 1.0), F0);",
            "highp vec3  nominator = NDF * G * F;",
            "highp float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;",
            "highp vec3  specular_brdf = nominator / denominator;",
            
            // Diffuse (Lambertian) component
            "highp vec3  diffuse_brdf = albedo / PI;",
            
            // Compute the ratios of refracted (diffuse, kD) to reflected (specular, kS) light
            // The specular component is equal to fresnel, and the diffuse component is derived
            // from energy conservation
            "highp vec3 kS = F;",
            "highp vec3 kD = vec3(1.0) - kS;",
            // Only non-metals have diffuse lighting
            "kD *= (1.0 - _surface.metalness);",
            
            // Add the outgoing radiance to the diffuse lighting contribution term. Note that
            // kS is assumed to be 1.0 here, since we already multiplied by F in the specular BRDF
            // (that's confusing)
            "highp vec3 illumination = (kD * diffuse_brdf + specular_brdf) * luminance * NdotL;",
            
            // Finally, for punctual lights we multiply by PI (effectively this cancels out the PI
            // in diffuse_brdf and specular_brdf)
            "_lightingContribution.diffuse += (illumination * PI);",
        };
        
        sPBRDirectLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                         modifierCode);
        sPBRDirectLightingModifier->setName("pbr_direct");
    }
    return sPBRDirectLightingModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createPBRConstantAmbientFragmentModifier() {
    if (!sPBRConstantAmbientFragmentModifier) {
        std::vector<std::string> modifierCode = {
            "highp vec3 pbr_ambient = _ambient * albedo * _surface.ao;",
            "highp vec3 rgb_color = pbr_ambient + _diffuse;",
            "_output_color = vec4(rgb_color, _output_color.a);",
        };
        sPBRConstantAmbientFragmentModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                                  modifierCode);
        sPBRConstantAmbientFragmentModifier->setName("pbr_const_amb");
    }
    return sPBRConstantAmbientFragmentModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createPBRDiffuseIrradianceFragmentModifier() {
    if (!sPBRDiffuseIrradianceFragmentModifier) {
        std::vector<std::string> modifierCode = {
                "uniform samplerCube irradiance_map;",
                "highp vec3 ambient_kS = fresnel_schlick_roughness(max(dot(N, V), 0.0), F0, _surface.roughness);",
                "highp vec3 ambient_kD = 1.0 - ambient_kS;",
                "ambient_kD *= 1.0 - _surface.metalness;",

                "highp vec3 irradiance = texture(irradiance_map, N).rgb;",
                "highp vec3 ambient_diffuse = irradiance * albedo;",

                "highp vec3 pbr_ambient = (_ambient + ambient_kD * ambient_diffuse) * _surface.ao;",
                "highp vec3 rgb_color = pbr_ambient + _diffuse;",
                "_output_color = vec4(rgb_color, _output_color.a);",
        };
        sPBRDiffuseIrradianceFragmentModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                                    modifierCode);
        sPBRDiffuseIrradianceFragmentModifier->setName("pbr_ibl");
    }
    return sPBRDiffuseIrradianceFragmentModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createPBRDiffuseAndSpecularIrradianceFragmentModifier() {
    if (!sPBRDiffuseAndSpecularIrradianceFragmentModifier) {
        std::vector<std::string> modifierCode = {
                // Initialize our input uniforms to sample from
                "const highp float MAX_REFLECTION_LOD = 4.0;",
                "uniform samplerCube irradiance_map;",
                "uniform samplerCube prefiltered_map;",
                "uniform sampler2D brdf_map;",
                "highp vec3 irradiance = texture(irradiance_map, N).rgb;",

                // Calculate both specular and diffuse ratios
                "highp vec3 ambient_kS = fresnel_schlick_roughness(max(dot(N, V), 0.0), F0, _surface.roughness);",
                "highp vec3 ambient_kD = 1.0 - ambient_kS;",
                "ambient_kD *= 1.0 - _surface.metalness;",

                // Compute ambient specular lighting.
                // Note the flipped Z axis for N and V to account for the cube map's flipped front facing Z.
                "highp vec3 n_cube = vec3(N.x, N.y, -N.z);",
                "highp vec3 v_cube = vec3(vec3(V.x, V.y, -V.z));",
                "highp vec3 R = reflect(-v_cube, n_cube); ",
                "highp vec3 prefilteredColor = textureLod(prefiltered_map, R, _surface.roughness * MAX_REFLECTION_LOD).rgb;",
                "highp vec2 brdf = texture(brdf_map, vec2(max(dot(N, V), 0.0), _surface.roughness)).xy;",
                "highp vec3 ambient_specular = prefilteredColor * (ambient_kS * brdf.x + brdf.y);",

                // Compute ambient diffuse lighting.
                "highp vec3 ambient_diffuse = irradiance * albedo;",

                // Combine both specular and diffuse computations into _output_color
                "highp vec3 pbr_ambient = (ambient_kD * ambient_diffuse + _ambient + ambient_specular) * _surface.ao;",
                "highp vec3 rgb_color = pbr_ambient + _diffuse;",
                "_output_color = vec4(rgb_color, _output_color.a);",
        };
        sPBRDiffuseAndSpecularIrradianceFragmentModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                                    modifierCode);
        sPBRDiffuseAndSpecularIrradianceFragmentModifier->setName("pbr_ibl");
    }
    return sPBRDiffuseAndSpecularIrradianceFragmentModifier;
}

#pragma mark - Other Modifiers

std::shared_ptr<VROShaderModifier> VROShaderFactory::createStereoTextureModifier(VROStereoMode currentStereoMode) {
    /*
     Modifier that renders half of the diffuse image for each eye for stereoscopic behavior.
     */
    std::shared_ptr<VROShaderModifier> modifier;
    if (sStereoscopicTextureModifiers[currentStereoMode]){
        modifier = sStereoscopicTextureModifiers[currentStereoMode];
    }
    else {
        // If stereoscopic image is vertical, change stereoAxis to y
        std::string stereoAxis = "x";
        if (currentStereoMode == VROStereoMode::TopBottom || currentStereoMode == VROStereoMode::BottomTop) {
            stereoAxis = "y";
        }

        std::vector<std::string> surfaceModifierCode;
        if (currentStereoMode == VROStereoMode::LeftRight || currentStereoMode == VROStereoMode::TopBottom) {
            surfaceModifierCode = {
                "uniform highp float eye_type;",
                "_surface.diffuse_texcoord." + stereoAxis + " = _surface.diffuse_texcoord." + stereoAxis + " * 0.5 + eye_type * 0.5;",
            };
        }
        // For stereo modes where the eyes are switched, we flip them.
        else {
            surfaceModifierCode = {
                "uniform highp float eye_type;",
                "_surface.diffuse_texcoord." + stereoAxis + " = _surface.diffuse_texcoord." + stereoAxis + " * 0.5 + (1.0 - eye_type) * 0.5;",
            };
        }
        
        modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface, surfaceModifierCode);
        modifier->setName("stereo");
        sStereoscopicTextureModifiers[currentStereoMode] = modifier;
    }
    
    return modifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createYCbCrTextureModifier(bool linearizeColor) {
    /*
     Modifier that converts a YCbCr image (encoded in two textures) into an RGB color.
     Note the cbcr texture luminance_alpha, which is why we access the B and A coordinates
     (in luminance_alpha R, G, and B are all equal).
     */
    if (!sYCbCrTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform sampler2D diffuse_texture_y;",
            "uniform sampler2D diffuse_texture_cbcr;",
            "const highp mat4x4 ycbcrToRGBTransform = mat4x4(",
            
               // There are a number of YCbCr conversion matrices to choose
               // from. Apple doesn't seem to recommend either (one of their
               // examples uses full range, the other doesn't). For now we're
               // using full-range BT.601. Default is left for comparison.
            
               // ITU-R BT.601 Full Range Conversion
               "   vec4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),",
               "   vec4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),",
               "   vec4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),",
               "   vec4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)",
            
               // ITU-R BT.601 Conversion (standard for SDTV)
               //"     vec4(+1.164380f, +1.164380f, +1.164380f, +0.000000f),",
               //"     vec4(+0.000000f, -0.391762f, +2.017230f, +0.000000f),",
               //"     vec4(+1.596030f, -0.812968f, +0.000000f, +0.000000f),",
               //"     vec4(-0.874202f, +0.531668f, -1.085630f, +1.000000f)",
            ");",
            "highp vec4 ycbcr = vec4(texture(diffuse_texture_y, _surface.diffuse_texcoord).r,",
            "                        texture(diffuse_texture_cbcr, _surface.diffuse_texcoord).ba, 1.0);",
            "_surface.diffuse_color *= (ycbcrToRGBTransform * ycbcr);"
        };
        
        /*
         Manually linearize the color if requested. We typically do this if gamma correction is
         enabled.
         */
        if (linearizeColor) {
            std::vector<std::string> gamma = createColorLinearizationCode();
            modifierCode.insert(modifierCode.end(), gamma.begin(), gamma.end());
        }
        sYCbCrTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                    modifierCode);
        sYCbCrTextureModifier->setName("ycbcr");
    }
    
    return sYCbCrTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createEGLImageModifier(bool linearizeColor) {
    std::vector<std::string> input;
    if (linearizeColor) {
        input = createColorLinearizationCode();
    }
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface, input);
    modifier->addReplacement("uniform sampler2D diffuse_texture;", "uniform samplerExternalOES diffuse_texture;");

    return modifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createBloomModifier() {
    /*
     Modifier that writes bloom regions to an output variable _bright_color.
     */
    if (!sBloomModifier) {
        std::vector<std::string> modifierCode =  {
            "layout (location = 1) out highp vec4 _bright_color;",
            "uniform highp float bloom_threshold;",
            
            "highp float brightness = dot(_output_color.rgb, vec3(0.2126, 0.7152, 0.0722));",
            "if (brightness > bloom_threshold) {",
            "   _bright_color = vec4(_output_color.rgb, _output_color.a);",
            "}",
        };
        sBloomModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment, modifierCode);
        sBloomModifier->setUniformBinder("bloom_threshold", [](VROUniform *uniform, GLuint location,
                                                               const VROGeometry *geometry, const VROMaterial *material) {
            uniform->setFloat(material->getBloomThreshold());
        });
        sBloomModifier->setName("bloom");
    }
    return sBloomModifier;
}

std::vector<std::string> VROShaderFactory::createColorLinearizationCode() {
    std::vector<std::string> code = {
        /*
         The way we linearize from gamma-corrected space depends on our values:
         If they're below the cutoff (low-light), we use the latter (lower) operation;
         if they're above the cutoff we use the higher operation (pow).
         The mix with a bvec trick is a technique to avoid branching in the shader.

         The values here are for gamma 2.2.
         */
        "bvec3 cutoff = lessThan(_surface.diffuse_color.rgb, vec3(0.082));",
        "highp vec3 higher = pow((_surface.diffuse_color.rgb + vec3(0.099))/vec3(1.099), vec3(2.2));",
        "highp vec3 lower = _surface.diffuse_color.rgb / vec3(4.5);",
        "_surface.diffuse_color.rgb = mix(higher, lower, cutoff);",

        /*
         The following values are for gamma 2.4. Left here in case we find it's better
         for certain devices.

        "bvec3 cutoff = lessThan(_surface.diffuse_color.rgb, vec3(0.04045));",
        "highp vec3 higher = pow((_surface.diffuse_color.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));",
        "highp vec3 lower = _surface.diffuse_color.rgb/vec3(12.92);",
        "_surface.diffuse_color.rgb = mix(higher, lower, cutoff);",
        */
    };
    return code;
}
