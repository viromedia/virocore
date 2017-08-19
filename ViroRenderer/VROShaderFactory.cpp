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
#include "VROEye.h"
#include "VROStringUtil.h"
#include "VROShadowMapRenderPass.h"
#include "VROShaderCapabilities.h"
#include <tuple>

static std::shared_ptr<VROShaderModifier> sDiffuseTextureModifier;
static std::shared_ptr<VROShaderModifier> sSpecularTextureModifier;
static std::shared_ptr<VROShaderModifier> sNormalMapTextureModifier;
static std::shared_ptr<VROShaderModifier> sReflectiveTextureModifier;
static std::shared_ptr<VROShaderModifier> sLambertLightingModifier;
static std::shared_ptr<VROShaderModifier> sPhongLightingModifier;
static std::shared_ptr<VROShaderModifier> sBlinnLightingModifier;
static std::shared_ptr<VROShaderModifier> sYCbCrTextureModifier;
static std::shared_ptr<VROShaderModifier> sShadowMapGeometryModifier;
static std::shared_ptr<VROShaderModifier> sShadowMapLightModifier;
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

#pragma mark - Shader Capability Extraction and Construction

VROMaterialShaderCapabilities VROShaderFactory::deriveMaterialCapabilitiesKey(const VROMaterial &material) {
    VROMaterialShaderCapabilities cap;
    cap.additionalModifierKeys = VROShaderModifier::getShaderModifierKey(material.getShaderModifiers());
    
    VROLightingModel lightingModel = material.getLightingModel();
    std::string vertexShader = "standard_vsh";
    std::string fragmentShader = "standard_fsh";
    
    VROMaterialVisual &diffuse    = material.getDiffuse();
    VROMaterialVisual &specular   = material.getSpecular();
    VROMaterialVisual &normal     = material.getNormal();
    VROMaterialVisual &reflective = material.getReflective();
    
    // Diffuse Map
    if (diffuse.getTextureType() == VROTextureType::Texture2D || diffuse.getTextureType() == VROTextureType::TextureEGLImage) {
        cap.diffuseTextureStereoMode = diffuse.getTexture()->getStereoMode();
        if (diffuse.getTexture()->getInternalFormat() == VROTextureInternalFormat::YCBCR) {
            cap.diffuseTexture = VRODiffuseTextureType::YCbCr;
        }
        else {
            cap.diffuseTexture = VRODiffuseTextureType::Normal;
        }
        
        // For Android video
        cap.diffuseEGLModifier = (diffuse.getTextureType() == VROTextureType::TextureEGLImage);
    }
    else if (diffuse.getTextureType() == VROTextureType::TextureCube) {
        cap.diffuseTexture = VRODiffuseTextureType::Cube;
        cap.diffuseTextureStereoMode = VROStereoMode::None;
        cap.diffuseEGLModifier = false;
    }
    else {
        cap.diffuseTexture = VRODiffuseTextureType::None;
        cap.diffuseTextureStereoMode = VROStereoMode::None;
        cap.diffuseEGLModifier = false;
    }
    
    // Specular Map
    if (specular.getTextureType() == VROTextureType::Texture2D &&
        (lightingModel == VROLightingModel::Blinn || lightingModel == VROLightingModel::Phong)) {
        cap.specularTexture = true;
    }
    
    // Normal Map
    if (normal.getTextureType() == VROTextureType::Texture2D) {
        cap.normalTexture = true;
    }
    
    // Reflective Map
    if (reflective.getTextureType() == VROTextureType::TextureCube) {
        cap.reflectiveTexture = true;
    }
    
    // Lighting Model. If we don't have a specular texture then we fall back to Lambert
    if (lightingModel == VROLightingModel::Constant) {
        cap.lightingModel = VROLightingModel::Constant;
    }
    else if (lightingModel == VROLightingModel::Lambert) {
        cap.lightingModel = VROLightingModel::Lambert;
    }
    else if (lightingModel == VROLightingModel::Blinn) {
        if (specular.getTextureType() != VROTextureType::Texture2D) {
            cap.lightingModel = VROLightingModel::Lambert;
        }
        else {
            cap.lightingModel = VROLightingModel::Blinn;
        }
    }
    else if (lightingModel == VROLightingModel::Phong) {
        if (specular.getTextureType() != VROTextureType::Texture2D) {
            cap.lightingModel = VROLightingModel::Lambert;
        }
        else {
            cap.lightingModel = VROLightingModel::Phong;
        }
    }
    
    return cap;
}

VROLightingShaderCapabilities VROShaderFactory::deriveLightingCapabilitiesKey(const std::vector<std::shared_ptr<VROLight>> &lights) {
    VROLightingShaderCapabilities cap;
    cap.shadows = false;
    
    for (const std::shared_ptr<VROLight> &light : lights) {
        if (light->getCastsShadow()) {
            cap.shadows = true;
        }
    }
    return cap;
}

std::shared_ptr<VROShaderProgram> VROShaderFactory::buildShader(VROShaderCapabilities capabilities,
                                                                const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers_in,
                                                                std::shared_ptr<VRODriverOpenGL> &driver) {
    
    VROMaterialShaderCapabilities &materialCapabilities = capabilities.materialCapabilities;
    VROLightingShaderCapabilities &lightingCapabilities = capabilities.lightingCapabilities;
    
    // Derive the base shader from the required capabilities
    std::string vertexShader = "standard_vsh";
    std::string fragmentShader;
    if (materialCapabilities.lightingModel == VROLightingModel::Constant) {
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
    
    std::vector<std::string> samplers;
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = modifiers_in;
    
    // Diffuse Map
    if (materialCapabilities.diffuseTexture == VRODiffuseTextureType::Normal) {
        samplers.push_back("diffuse_texture");
        modifiers.push_back(createDiffuseTextureModifier());
    }
    else if (materialCapabilities.diffuseTexture == VRODiffuseTextureType::YCbCr) {
        samplers.push_back("diffuse_texture_y");
        samplers.push_back("diffuse_texture_cbcr");
        modifiers.push_back(createYCbCrTextureModifier());
    }
    else if (materialCapabilities.diffuseTexture == VRODiffuseTextureType::Cube) {
        samplers.push_back("diffuse_texture");
        // No modifier here since constant_q has this built in
    }
    else {
        // Do nothing
    }
    
    if (materialCapabilities.diffuseTextureStereoMode != VROStereoMode::None) {
        modifiers.push_back(createStereoTextureModifier(materialCapabilities.diffuseTextureStereoMode));
    }
    if (materialCapabilities.diffuseEGLModifier) {
        modifiers.push_back(createEGLImageModifier());
    }
    
    // Specular Map
    if (materialCapabilities.specularTexture) {
        samplers.push_back("specular_texture");
        modifiers.push_back(createSpecularTextureModifier());
    }
    
    // Normal Map
    if (materialCapabilities.normalTexture) {
        samplers.push_back("normal_texture");
        modifiers.push_back(createNormalMapTextureModifier());
    }
    
    // Reflective Map
    if (materialCapabilities.reflectiveTexture) {
        samplers.push_back("reflect_texture");
        modifiers.push_back(createReflectiveTextureModifier());
    }
    
    // Lighting Model modifiers
    if (materialCapabilities.lightingModel == VROLightingModel::Lambert) {
        modifiers.push_back(createLambertLightingModifier());
    }
    else if (materialCapabilities.lightingModel == VROLightingModel::Blinn) {
        modifiers.push_back(createBlinnLightingModifier());
    }
    else if (materialCapabilities.lightingModel == VROLightingModel::Phong) {
        modifiers.push_back(createPhongLightingModifier());
    }
    
    // Shadow modifiers
    if (lightingCapabilities.shadows) {
        modifiers.push_back(createShadowMapGeometryModifier());
        modifiers.push_back(createShadowMapLightModifier());
        if (kDebugShadowMaps) {
            modifiers.push_back(createShadowMapFragmentModifier());
        }
        samplers.push_back("shadow_map");
    }
    
    const std::vector<VROGeometrySourceSemantic> attributes = { VROGeometrySourceSemantic::Texcoord,
        VROGeometrySourceSemantic::Normal,
        VROGeometrySourceSemantic::Tangent,
        VROGeometrySourceSemantic::BoneIndices,
        VROGeometrySourceSemantic::BoneWeights};
    return std::make_shared<VROShaderProgram>(vertexShader, fragmentShader, samplers, modifiers, attributes,
                                              driver);
}

#pragma mark - Shader Modifiers

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
            "uniform sampler2D normal_texture;",
            "_surface.normal = v_tbn * normalize( texture(normal_texture, _surface.diffuse_texcoord).xyz * 2.0 - 1.0 );"
        };
        sNormalMapTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                        modifierCode);
    }
    
    return sNormalMapTextureModifier;
}

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
            "for (int i = 0; i < num_lights; i++) {",
            "   shadow_coords[i] = shadow_projection_matrices[i] * shadow_view_matrices[i] * _transforms.model_matrix * vec4(_geometry.position.xyz, 1.0);",
            "   shadow_coords[i].x = shadow_coords[i].x * 0.5 + shadow_coords[i].w * 0.5;",
            "   shadow_coords[i].y = shadow_coords[i].y * 0.5 + shadow_coords[i].w * 0.5;",
            "   shadow_coords[i].z = shadow_coords[i].z * 0.5 + shadow_coords[i].w * 0.5;",
            "}",
        };
        
        sShadowMapGeometryModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                         modifierCode);
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
    }
    return sShadowMapFragmentModifier;
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
    }
    
    return sReflectiveTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createLambertLightingModifier() {
    /*
     Modifier that implements the Lambert lighting model.
     */
    if (!sLambertLightingModifier) {
        std::vector<std::string> modifierCode = {
            "highp float diffuse_coeff = max(0.0, dot(-_surface.normal, _light.surface_to_light));",
            "_lightingContribution.diffuse += (_light.attenuation * diffuse_coeff * _light.color);",
        };
        
        sLambertLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                       modifierCode);
    }
    return sLambertLightingModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createPhongLightingModifier() {
    /*
     Modifier that implements the Phong lighting model.
     */
    if (!sPhongLightingModifier) {
        std::vector<std::string> modifierCode = {
            "highp float diffuse_coeff = max(0.0, dot(-_surface.normal, _light.surface_to_light));",
            "_lightingContribution.diffuse += (_light.attenuation * diffuse_coeff * _light.color);",
            "lowp float specular_coeff = 0.0;",
            "if (diffuse_coeff > 0.0) {",
            "    specular_coeff = pow(max(0.0, dot(_surface.view,",
            "                                      reflect(_light.surface_to_light, -_surface.normal))),",
            "                         _surface.shininess);",
            "}",
            "_lightingContribution.specular += (_light.attenuation * specular_coeff * _light.color);",
        };
        
        sPhongLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                     modifierCode);
    }
    return sPhongLightingModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createBlinnLightingModifier() {
    /*
     Modifier that implements the Blinn lighting model.
     */
    if (!sBlinnLightingModifier) {
        std::vector<std::string> modifierCode = {
            "highp float diffuse_coeff = max(0.0, dot(-_surface.normal, _light.surface_to_light));",
            "_lightingContribution.diffuse += (_light.attenuation * diffuse_coeff * _light.color);",
            "lowp float specular_coeff = 0.0;",
            "if (diffuse_coeff > 0.0) {",
            "    specular_coeff = pow(max(0.0, dot(normalize(-_surface.view + _light.surface_to_light),",
            "                                      -_surface.normal)),",
            "                         _surface.shininess);",
            "}",
            "_lightingContribution.specular += (_light.attenuation * specular_coeff * _light.color);",
        };
        
        sBlinnLightingModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                     modifierCode);
    }
    return sBlinnLightingModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createStereoTextureModifier(VROStereoMode currentStereoMode) {
    /*
     Modifier that renders half of the diffuse image for each eye for stereoscopic behavior.
     */
    std::shared_ptr<VROShaderModifier> modifier;
    if (sStereoscopicTextureModifiers[currentStereoMode]){
        modifier = sStereoscopicTextureModifiers[currentStereoMode];
    }
    else {
        // Assume leftRight stereoscopic image by default.
        std::string stereoAxis = "x";
        std::string eye_left = VROStringUtil::toString(static_cast<int>(VROEyeType::Left));
        std::string eye_right = VROStringUtil::toString(static_cast<int>(VROEyeType::Right));
        
        // If stereoscopic image is vertical, change stereoAxis to y
        if (currentStereoMode == VROStereoMode::TopBottom || currentStereoMode == VROStereoMode::BottomTop) {
            stereoAxis = "y";
        }
        
        // For stereo modes where the eyes are switched, we flip them.
        if (currentStereoMode == VROStereoMode::RightLeft || currentStereoMode == VROStereoMode::BottomTop) {
            std::string tmp = eye_left;
            eye_left = eye_right;
            eye_right = tmp;
        }
        
        // Create the shader modifier
        std::vector<std::string> surfaceModifierCode = {
            "uniform int eye_type;",
            "if (eye_type == "+eye_left+") {_surface.diffuse_texcoord."+stereoAxis+" = _surface.diffuse_texcoord."+stereoAxis+" * 0.5;}",
            "else if (eye_type == "+eye_right+") {_surface.diffuse_texcoord."+stereoAxis+" = (_surface.diffuse_texcoord."+stereoAxis+" * 0.5) + 0.5;}"
        };
        
        modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface, surfaceModifierCode);
        sStereoscopicTextureModifiers[currentStereoMode] = modifier;
    }
    
    return modifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createYCbCrTextureModifier() {
    /*
     Modifier that converts a YCbCr image (encoded in two textures) into an RGB color.
     Note the cbcr texture luminance_alpha, which is why we access the B and A coordinates
     (in luminance_alpha R, G, and B are all equal).
     */
    if (!sYCbCrTextureModifier) {
        std::vector<std::string> modifierCode =  {
            "uniform sampler2D diffuse_texture_y;",
            "uniform sampler2D diffuse_texture_cbcr;",
            "const lowp mat4x4 ycbcrToRGBTransform = mat4x4(",
            "   vec4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),",
            "   vec4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),",
            "   vec4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),",
            "   vec4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)",
            ");",
            "lowp vec4 ycbcr = vec4(texture(diffuse_texture_y, _surface.diffuse_texcoord).r,",
            "                       texture(diffuse_texture_cbcr, _surface.diffuse_texcoord).ba, 1.0);",
            "_surface.diffuse_color *= (ycbcrToRGBTransform * ycbcr);"
        };
        sYCbCrTextureModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                    modifierCode);
    }
    
    return sYCbCrTextureModifier;
}

std::shared_ptr<VROShaderModifier> VROShaderFactory::createEGLImageModifier() {
    std::vector<std::string> input;
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface, input);
    modifier->addReplacement("uniform sampler2D diffuse_texture;", "uniform samplerExternalOES diffuse_texture;");
    
    return modifier;
}
