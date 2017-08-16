//
//  VROMaterialSubstrateOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROMaterialSubstrateOpenGL.h"
#include "VRODriverOpenGL.h"
#include "VROMaterial.h"
#include "VROShaderProgram.h"
#include "VROAllocationTracker.h"
#include "VROEye.h"
#include "VROLight.h"
#include "VROSortKey.h"
#include "VROBoneUBO.h"
#include "VROInstancedUBO.h"
#include <sstream>

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
static std::map<VROStereoMode ,std::shared_ptr<VROShaderModifier>> sStereoscopicTextureModifiers;

#pragma mark - Loading Materials

void VROMaterialSubstrateOpenGL::hydrateProgram(VRODriverOpenGL &driver) {
    _program->hydrate();
}

VROMaterialSubstrateOpenGL::VROMaterialSubstrateOpenGL(VROMaterial &material, VRODriverOpenGL &driver) :
    _material(material),
    _lightingModel(material.getLightingModel()),
    _program(nullptr),
    _diffuseSurfaceColorUniform(nullptr),
    _diffuseIntensityUniform(nullptr),
    _alphaUniform(nullptr),
    _shininessUniform(nullptr),
    _normalMatrixUniform(nullptr),
    _modelMatrixUniform(nullptr),
    _viewMatrixUniform(nullptr),
    _projectionMatrixUniform(nullptr),
    _cameraPositionUniform(nullptr),
    _eyeTypeUniform(nullptr),
    _shadowViewMatrixUniform(nullptr),
    _shadowProjectionMatrixUniform(nullptr) {

    switch (material.getLightingModel()) {
        case VROLightingModel::Constant:
            loadConstantLighting(material, driver);
            break;
                
        case VROLightingModel::Lambert:
        case VROLightingModel::Blinn:
        case VROLightingModel::Phong:
            configureStandardShader(material, driver);
            break;
                
        default:
            break;
    }
        
    ALLOCATION_TRACKER_ADD(MaterialSubstrates, 1);
}
    
VROMaterialSubstrateOpenGL::~VROMaterialSubstrateOpenGL() {
    ALLOCATION_TRACKER_SUB(MaterialSubstrates, 1);
}

void VROMaterialSubstrateOpenGL::loadConstantLighting(const VROMaterial &material, VRODriverOpenGL &driver) {
    VROMaterialVisual &diffuse = material.getDiffuse();
    
    std::string vertexShader = "standard_vsh";
    std::string fragmentShader;
    
    std::vector<std::string> samplers;
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = material.getShaderModifiers();

    if (diffuse.getTextureType() == VROTextureType::None) {
        fragmentShader = "constant_fsh";
    }
    else if (diffuse.getTextureType() == VROTextureType::Texture2D) {
        if (diffuse.getTexture()->getStereoMode() != VROStereoMode::None){
            modifiers.push_back(createStereoTextureModifier(diffuse.getTexture()->getStereoMode()));
        }

        _textures.push_back(diffuse.getTexture());
        if (diffuse.getTexture()->getInternalFormat() == VROTextureInternalFormat::YCBCR) {
            samplers.push_back("diffuse_texture_y");
            samplers.push_back("diffuse_texture_cbcr");
            modifiers.push_back(createYCbCrTextureModifier());
        }
        else {
            samplers.push_back("diffuse_texture");
            modifiers.push_back(createDiffuseTextureModifier());
        }
        fragmentShader = "constant_fsh";
    }
    else if (diffuse.getTextureType() == VROTextureType::TextureEGLImage) {
        if (diffuse.getTexture()->getStereoMode() != VROStereoMode::None){
            modifiers.push_back(createStereoTextureModifier(diffuse.getTexture()->getStereoMode()));
        }

        _textures.push_back(diffuse.getTexture());
        samplers.push_back("diffuse_texture");
        modifiers.push_back(createDiffuseTextureModifier());

        fragmentShader = "constant_fsh";
        modifiers.push_back(createEGLImageModifier());
    }
    else { // TextureCube
        _textures.push_back(diffuse.getTexture());
        samplers.push_back("diffuse_texture");

        fragmentShader = "constant_q_fsh";
    }
    
    _program = driver.getPooledShader(vertexShader, fragmentShader, samplers, modifiers);
    if (!_program->isHydrated()) {
        hydrateProgram(driver);
    }
    loadUniforms();
}

void VROMaterialSubstrateOpenGL::configureStandardShader(const VROMaterial &material, VRODriverOpenGL &driver) {
    VROLightingModel lightingModel = material.getLightingModel();
    std::string vertexShader = "standard_vsh";
    std::string fragmentShader = "standard_fsh";
    
    std::vector<std::string> samplers;
    std::vector<std::shared_ptr<VROShaderModifier>> modifiers = material.getShaderModifiers();

    VROMaterialVisual &diffuse    = material.getDiffuse();
    VROMaterialVisual &specular   = material.getSpecular();
    VROMaterialVisual &normal     = material.getNormal();
    VROMaterialVisual &reflective = material.getReflective();

    // Diffuse Map
    if (diffuse.getTextureType() != VROTextureType::None) {
        if (diffuse.getTexture()->getStereoMode() != VROStereoMode::None){
            modifiers.push_back(createStereoTextureModifier(diffuse.getTexture()->getStereoMode()));
        }

        _textures.push_back(diffuse.getTexture());
        if (diffuse.getTexture()->getInternalFormat() == VROTextureInternalFormat::YCBCR) {
            samplers.push_back("diffuse_texture_y");
            samplers.push_back("diffuse_texture_cbcr");
            modifiers.push_back(createYCbCrTextureModifier());
        }
        else {
            samplers.push_back("diffuse_texture");
            modifiers.push_back(createDiffuseTextureModifier());
        }
        
        // For Android video
        if (diffuse.getTextureType() == VROTextureType::TextureEGLImage) {
            modifiers.push_back(createEGLImageModifier());
        }
    }
    
    // Specular Map
    if (specular.getTextureType() == VROTextureType::Texture2D &&
        (lightingModel == VROLightingModel::Blinn || lightingModel == VROLightingModel::Phong)) {
        _textures.push_back(specular.getTexture());
        samplers.push_back("specular_texture");
        modifiers.push_back(createSpecularTextureModifier());
    }
    
    // Normal Map
    if (normal.getTextureType() == VROTextureType::Texture2D) {
        _textures.push_back(normal.getTexture());
        samplers.push_back("normal_texture");
        modifiers.push_back(createNormalMapTextureModifier());
    }

    // Reflective Map
    if (reflective.getTextureType() == VROTextureType::TextureCube) {
        _textures.push_back(reflective.getTexture());
        samplers.push_back("reflect_texture");
        modifiers.push_back(createReflectiveTextureModifier());
    }
    
    // Lighting Model. If we don't have a specular texture then we fall back to Lambert
    if (lightingModel == VROLightingModel::Lambert) {
        modifiers.push_back(createLambertLightingModifier());
    }
    else if (lightingModel == VROLightingModel::Blinn) {
        if (specular.getTextureType() != VROTextureType::Texture2D) {
            modifiers.push_back(createLambertLightingModifier());
        }
        else {
            modifiers.push_back(createBlinnLightingModifier());
        }
    }
    else if (lightingModel == VROLightingModel::Phong) {
        if (specular.getTextureType() != VROTextureType::Texture2D) {
            modifiers.push_back(createLambertLightingModifier());
        }
        else {
            modifiers.push_back(createPhongLightingModifier());
        }
    }
    
    // Shadow maps
    modifiers.push_back(createShadowMapGeometryModifier());
    modifiers.push_back(createShadowMapLightModifier());
    samplers.push_back("shadow_map");
    
    _program = driver.getPooledShader(vertexShader, fragmentShader, samplers, modifiers);
    if (!_program->isHydrated()) {
        _program->addUniform(VROShaderProperty::Float, 1, "material_shininess");
        hydrateProgram(driver);
    }
    
    _shininessUniform = _program->getUniform("material_shininess");
    loadUniforms();
}

void VROMaterialSubstrateOpenGL::loadUniforms() {
    _diffuseSurfaceColorUniform = _program->getUniform("material_diffuse_surface_color");
    _diffuseIntensityUniform = _program->getUniform("material_diffuse_intensity");
    _alphaUniform = _program->getUniform("material_alpha");
    
    _normalMatrixUniform = _program->getUniform("normal_matrix");
    _modelMatrixUniform = _program->getUniform("model_matrix");
    _projectionMatrixUniform = _program->getUniform("projection_matrix");
    _viewMatrixUniform = _program->getUniform("view_matrix");
    _cameraPositionUniform = _program->getUniform("camera_position");
    _eyeTypeUniform = _program->getUniform("eye_type");
    _shadowViewMatrixUniform = _program->getUniform("shadow_view_matrix");
    _shadowProjectionMatrixUniform = _program->getUniform("shadow_projection_matrix");
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : _program->getModifiers()) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniform *uniform = _program->getUniform(uniformName);
            passert_msg (uniform != nullptr, "Failed to find shader modifier uniform '%s' in program!",
                         uniformName.c_str());
            
            _shaderModifierUniforms.push_back(uniform);
        }
    }
}

#pragma mark - Binding Materials

void VROMaterialSubstrateOpenGL::bindProperties() {
    bindMaterialUniforms();
}

void VROMaterialSubstrateOpenGL::bindGeometry(float opacity, const VROGeometry &geometry){
    bindGeometryUniforms(opacity, geometry);
}

void VROMaterialSubstrateOpenGL::bindShader() {
    _program->bind();
}

void VROMaterialSubstrateOpenGL::bindLights(int lightsHash,
                                            const std::vector<std::shared_ptr<VROLight>> &lights,
                                            const VRORenderContext &context,
                                            std::shared_ptr<VRODriver> &driver) {
    
    if (lights.empty()) {
        VROLightingUBO::unbind(_program);
        _lightingUBO.reset();
        return;
    }
    
    VRODriverOpenGL &glDriver = (VRODriverOpenGL &)(*driver.get());
    for (const std::shared_ptr<VROLight> &light : lights) {
        light->propagateUpdates();
    }

    if (!_lightingUBO || _lightingUBO->getHash() != lightsHash) {
        _lightingUBO = glDriver.getLightingUBO(lightsHash);
        if (!_lightingUBO) {
            _lightingUBO = glDriver.createLightingUBO(lightsHash, lights);
        }
    }
    
    _lightingUBO->bind(_program);
}

void VROMaterialSubstrateOpenGL::bindView(VROMatrix4f modelMatrix, VROMatrix4f viewMatrix,
                                          VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                                          VROVector3f cameraPosition, VROEyeType eyeType,
                                          VROMatrix4f shadowViewMatrix, VROMatrix4f shadowProjectionMatrix) {
    if (_normalMatrixUniform != nullptr) {
        _normalMatrixUniform->setMat4(normalMatrix);
    }
    if (_modelMatrixUniform != nullptr) {
        _modelMatrixUniform->setMat4(modelMatrix);
    }
    if (_projectionMatrixUniform != nullptr) {
        _projectionMatrixUniform->setMat4(projectionMatrix);
    }
    if (_viewMatrixUniform != nullptr) {
        _viewMatrixUniform->setMat4(viewMatrix);
    }
    if (_cameraPositionUniform != nullptr) {
        _cameraPositionUniform->setVec3(cameraPosition);
    }
    if (_eyeTypeUniform != nullptr){
        _eyeTypeUniform->setInt(static_cast<int>(eyeType));
    }
    if (_shadowViewMatrixUniform != nullptr) {
        _shadowViewMatrixUniform->setMat4(shadowViewMatrix);
    }
    if (_shadowProjectionMatrixUniform != nullptr) {
        _shadowProjectionMatrixUniform->setMat4(shadowProjectionMatrix);
    }
}

void VROMaterialSubstrateOpenGL::bindMaterialUniforms() {
    if (_diffuseSurfaceColorUniform != nullptr) {
        _diffuseSurfaceColorUniform->setVec4(_material.getDiffuse().getColor());
    }
    if (_diffuseIntensityUniform != nullptr) {
        _diffuseIntensityUniform->setFloat(_material.getDiffuse().getIntensity());
    }
    if (_shininessUniform != nullptr) {
        _shininessUniform->setFloat(_material.getShininess());
    }
}

void VROMaterialSubstrateOpenGL::bindGeometryUniforms(float opacity, const VROGeometry &geometry) {
    if (_alphaUniform != nullptr) {
        _alphaUniform->setFloat(_material.getTransparency() * opacity);
    }
    for (VROUniform *uniform : _shaderModifierUniforms) {
        uniform->set(nullptr, &geometry);
    }
}

void VROMaterialSubstrateOpenGL::bindBoneUBO(const std::unique_ptr<VROBoneUBO> &boneUBO) {
    boneUBO->bind(_program);
}

void VROMaterialSubstrateOpenGL::bindInstanceUBO(const std::shared_ptr<VROInstancedUBO> &instanceUBO) {
    instanceUBO->bind(_program);
}

void VROMaterialSubstrateOpenGL::updateSortKey(VROSortKey &key) const {
    key.shader = _program->getShaderId();
    key.textures = hashTextures(_textures);
}

#pragma mark - Shader Modifiers

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createDiffuseTextureModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createSpecularTextureModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createNormalMapTextureModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createShadowMapGeometryModifier() {
    /*
     Modifier that outputs shadow map texture coordinates for the fragment shader.
     */
    if (!sShadowMapGeometryModifier) {
        std::vector<std::string> modifierCode = {
            "uniform mat4 shadow_view_matrix;",
            "uniform mat4 shadow_projection_matrix;",
            "out lowp vec4 shadow_coord;",
            "shadow_coord = shadow_projection_matrix * shadow_view_matrix * _transforms.model_matrix * vec4(_geometry.position.xyz, 1.0);",
            "shadow_coord.x = shadow_coord.x * 0.5 + 0.5;",
            "shadow_coord.y = shadow_coord.y * 0.5 + 0.5;",
            "shadow_coord.z = shadow_coord.z * 0.5 + 0.5;",
        };
        
        sShadowMapGeometryModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                         modifierCode);
    }
    return sShadowMapGeometryModifier;
}

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createShadowMapLightModifier() {
    /*
     Modifier that samples a shadow map to determine if the fragment is in light.
     */
    if (!sShadowMapLightModifier) {
        /*
        std::vector<std::string> modifierCode = {
            "in lowp vec4 shadow_coord;",
            "_output_color = vec4(0.0, shadow_coord.y, 0.0, 1.0);",
        };
        
        sShadowMapLightModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Fragment,
                                                                      modifierCode);
        */
        
        std::vector<std::string> modifierCode = {
            "uniform highp sampler2DShadow shadow_map;",
            "in lowp vec4 shadow_coord;",
            "lowp vec3 comparison = vec3(shadow_coord.xy, shadow_coord.z - 0.005);",
            "if (shadow_coord.x < 0.0 || shadow_coord.y < 0.0 || shadow_coord.x > 1.0 || shadow_coord.y > 1.0) {",
            "    _lightingContribution.visibility = 1.0;",
            "} else {",
            "    _lightingContribution.visibility = texture(shadow_map, comparison);",
            "}",
        };
        
        sShadowMapLightModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::LightingModel,
                                                                      modifierCode);
    }
    return sShadowMapLightModifier;
}

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createReflectiveTextureModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createLambertLightingModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createPhongLightingModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createBlinnLightingModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createStereoTextureModifier(VROStereoMode currentStereoMode) {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createYCbCrTextureModifier() {
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

std::shared_ptr<VROShaderModifier> VROMaterialSubstrateOpenGL::createEGLImageModifier() {
    std::vector<std::string> input;
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface, input);
    modifier->addReplacement("uniform sampler2D diffuse_texture;", "uniform samplerExternalOES diffuse_texture;");
    
    return modifier;
}

uint32_t VROMaterialSubstrateOpenGL::hashTextures(const std::vector<std::shared_ptr<VROTexture>> &textures) const {
    uint32_t h = 0;
    for (const std::shared_ptr<VROTexture> &texture : textures) {
        h = 31 * h + texture->getTextureId();
    }
    return h;
}
