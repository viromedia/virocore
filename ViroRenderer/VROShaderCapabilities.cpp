//
//  VROShaderCapabilities.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/24/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROShaderCapabilities.h"
#include "VROMaterial.h"
#include "VROMaterialVisual.h"
#include "VROTexture.h"
#include "VROLight.h"
#include "VROShaderModifier.h"
#include "VRORenderContext.h"

#pragma mark - Shader Capability Extraction and Construction

VROMaterialShaderCapabilities VROShaderCapabilities::deriveMaterialCapabilitiesKey(const VROMaterial &material) {
    VROMaterialShaderCapabilities cap;
    cap.diffuseEGLModifier = false;
    cap.specularTexture = false;
    cap.normalTexture = false;
    cap.reflectiveTexture = false;
    cap.roughnessMap = false;
    cap.metalnessMap = false;
    cap.aoMap = false;
    cap.diffuseTextureStereoMode = VROStereoMode::None;
    cap.bloom = false;
    cap.receivesShadows = true;
    
    cap.additionalModifierKeys = VROShaderModifier::getShaderModifierKey(material.getShaderModifiers());
    
    VROLightingModel lightingModel = material.getLightingModel();
    std::string vertexShader = "standard_vsh";
    std::string fragmentShader = "standard_fsh";
    
    VROMaterialVisual &diffuse    = material.getDiffuse();
    VROMaterialVisual &specular   = material.getSpecular();
    VROMaterialVisual &normal     = material.getNormal();
    VROMaterialVisual &reflective = material.getReflective();
    VROMaterialVisual &roughness = material.getRoughness();
    VROMaterialVisual &metalness = material.getMetalness();
    VROMaterialVisual &ao = material.getAmbientOcclusion();
    
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
    
    // PBR Maps
    if (roughness.getTextureType() == VROTextureType::Texture2D) {
        cap.roughnessMap = true;
    }
    if (metalness.getTextureType() == VROTextureType::Texture2D) {
        cap.metalnessMap = true;
    }
    if (ao.getTextureType() == VROTextureType::Texture2D) {
        cap.aoMap = true;
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
    else if (lightingModel == VROLightingModel::PhysicallyBased) {
        cap.lightingModel = VROLightingModel::PhysicallyBased;
    }
    
    // Shadows
    cap.receivesShadows = material.getReceivesShadows();
    
    // Bloom
    cap.bloom = material.isBloomSupported();

    // Chroma key filtering
    if (material.isChromaKeyFilteringEnabled()) {
        cap.chromaKeyFiltering = material.getChromaKeyFilteringColor().hash();
        cap.chromaKeyFilteringColor = material.getChromaKeyFilteringColor();
    } else {
        cap.chromaKeyFiltering = 0;
    }

    return cap;
}

VROLightingShaderCapabilities VROShaderCapabilities::deriveLightingCapabilitiesKey(const std::vector<std::shared_ptr<VROLight>> &lights,
                                                                                   const VRORenderContext &context) {
    VROLightingShaderCapabilities cap;
    cap.shadows = false;
    cap.hdr = context.isHDREnabled();
    cap.pbr = context.isPBREnabled();
    cap.diffuseIrradiance = false;
    cap.specularIrradiance = false;
    
    if (context.getShadowMap() != nullptr) {
        for (const std::shared_ptr<VROLight> &light : lights) {
            if (light->getCastsShadow()) {
                cap.shadows = true;
            }
        }
    }

    if (context.getIrradianceMap() != nullptr) {
        cap.diffuseIrradiance = true;
    }
    if (context.getBRDFMap() != nullptr && context.getPrefilteredMap() != nullptr) {
        cap.specularIrradiance = true;
    }

    return cap;
}
