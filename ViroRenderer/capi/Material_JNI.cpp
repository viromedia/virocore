//
//  Material_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "Material_JNI.h"
#include "VROStringUtil.h"
#include "VROLog.h"
#include "VROARShadow.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Material_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type Material_##method_name
#endif

VROVector4f parseColor(VRO_LONG color) {
    float a = ((color >> 24) & 0xFF) / 255.0;
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;
    return {r, g, b, a};
}

extern "C" {

VROLightingModel parseLightingModel(std::string strName) {
    if (VROStringUtil::strcmpinsensitive(strName, "Blinn")) {
        return VROLightingModel::Blinn;
    }
    else if (VROStringUtil::strcmpinsensitive(strName, "Lambert")) {
        return VROLightingModel::Lambert;
    }
    else if (VROStringUtil::strcmpinsensitive(strName, "Phong")) {
        return VROLightingModel::Phong;
    }
    else if (VROStringUtil::strcmpinsensitive(strName, "PBR")) {
        return VROLightingModel::PhysicallyBased;
    }
    else {
        // Default lightingModel is Constant, so no use checking.
        return VROLightingModel::Constant;
    }
}

VROBlendMode parseBlendMode(std::string blendMode) {
    if (VROStringUtil::strcmpinsensitive(blendMode, "None")) {
        return VROBlendMode::None;
    }
    else if (VROStringUtil::strcmpinsensitive(blendMode, "Alpha")) {
        return VROBlendMode::Alpha;
    }
    else if (VROStringUtil::strcmpinsensitive(blendMode, "Add")) {
        return VROBlendMode::Add;
    }
    else if (VROStringUtil::strcmpinsensitive(blendMode, "Subtract")) {
        return VROBlendMode::Subtract;
    }
    else if (VROStringUtil::strcmpinsensitive(blendMode, "Multiply")) {
        return VROBlendMode::Multiply;
    }
    else if (VROStringUtil::strcmpinsensitive(blendMode, "Screen")) {
        return VROBlendMode::Screen;
    }
    else {
        return VROBlendMode::None;
    }
}

VROTransparencyMode parseTransparencyMode(std::string strName) {
    if (VROStringUtil::strcmpinsensitive(strName, "RGBZero")) {
        return VROTransparencyMode::RGBZero;
    }
    else {
        // Default transparencyMode is AOne, so no use checking.
        return VROTransparencyMode::AOne;
    }
}

VROCullMode parseCullMode(std::string strName) {
    if (VROStringUtil::strcmpinsensitive(strName, "None")) {
        return VROCullMode::None;
    }
    else if (VROStringUtil::strcmpinsensitive(strName, "Front")) {
        return VROCullMode::Front;
    } else {
        // Default cullMode is Back, so no use checking.
        return VROCullMode::Back;
    }
}

VRO_METHOD(VRO_REF(VROMaterial), nativeCreateMaterial)(VRO_NO_ARGS) {
    std::shared_ptr<VROMaterial> materialPtr = std::make_shared<VROMaterial>();
    return VRO_REF_NEW(VROMaterial, materialPtr);
}

VRO_METHOD(VRO_REF(VROMaterial), nativeCreateImmutableMaterial)(VRO_ARGS
                                                   VRO_STRING lightingModel, VRO_LONG diffuseColor, VRO_REF(VROTexture) diffuseTexture,
                                                   VRO_FLOAT diffuseIntensity, VRO_REF(VROTexture) specularTexture,
                                                   VRO_FLOAT shininess, VRO_FLOAT fresnelExponent, VRO_REF(VROTexture) normalMap, VRO_STRING cullMode,
                                                   VRO_STRING transparencyMode, VRO_STRING blendMode, VRO_FLOAT bloomThreshold,
                                                   VRO_BOOL writesToDepthBuffer, VRO_BOOL readsFromDepthBuffer) {
    VRO_METHOD_PREAMBLE;

    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setLightingModel(parseLightingModel(VRO_STRING_STL(lightingModel)));
    material->getDiffuse().setColor(parseColor(diffuseColor));
    if (diffuseTexture != 0) { material->getDiffuse().setTexture(VRO_REF_GET(VROTexture, diffuseTexture)); }
    material->getDiffuse().setIntensity(diffuseIntensity);
    if (specularTexture != 0) { material->getSpecular().setTexture(VRO_REF_GET(VROTexture, specularTexture)); }
    material->setShininess(shininess);
    material->setFresnelExponent(fresnelExponent);
    if (normalMap != 0) { material->getNormal().setTexture(VRO_REF_GET(VROTexture, normalMap)); }
    material->setCullMode(parseCullMode(VRO_STRING_STL(cullMode)));
    material->setTransparencyMode(parseTransparencyMode(VRO_STRING_STL(transparencyMode)));
    material->setBlendMode(parseBlendMode(VRO_STRING_STL(blendMode)));
    material->setBloomThreshold(bloomThreshold);
    material->setWritesToDepthBuffer(writesToDepthBuffer);
    material->setReadsFromDepthBuffer(readsFromDepthBuffer);

    return VRO_REF_NEW(VROMaterial, material);
}

VRO_METHOD(void, nativeSetWritesToDepthBuffer)(VRO_ARGS
                                               VRO_REF(VROMaterial) material_j,
                                               VRO_BOOL writesToDepthBuffer) {
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([material_w, writesToDepthBuffer] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setWritesToDepthBuffer(writesToDepthBuffer);
        }
    });
}

VRO_METHOD(void, nativeSetReadsFromDepthBuffer)(VRO_ARGS
                                                VRO_REF(VROMaterial) material_j,
                                                VRO_BOOL readsFromDepthBuffer) {
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([material_w, readsFromDepthBuffer] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setReadsFromDepthBuffer(readsFromDepthBuffer);
        }
    });
}

VRO_METHOD(void, nativeSetTexture)(VRO_ARGS
                                   VRO_REF(VROMaterial) material_j,
                                   VRO_REF(VROTexture) textureRef,
                                   VRO_STRING materialPropertyName) {
    VRO_METHOD_PREAMBLE;
    std::string strName = VRO_STRING_STL(materialPropertyName);

    std::shared_ptr<VROTexture> texture;
    if (textureRef != -1) {
        texture = VRO_REF_GET(VROTexture, textureRef);
    }
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);

    VROPlatformDispatchAsyncRenderer([texture, material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            // Depending on the name, set the texture
            if (VROStringUtil::strcmpinsensitive(strName, "diffuseTexture")) {
                material->getDiffuse().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "specularTexture")) {
                material->getSpecular().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "normalTexture")) {
                material->getNormal().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "reflectiveTexture")) {
                material->getReflective().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "emissionTexture")) {
                material->getEmission().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "roughnessTexture")) {
                material->getRoughness().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "metalnessTexture")) {
                material->getMetalness().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "multiplyTexture")) {
                material->getMultiply().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "ambientOcclusionTexture")) {
                material->getAmbientOcclusion().setTexture(texture);
            } else if (VROStringUtil::strcmpinsensitive(strName, "selfIlluminationTexture")) {
                material->getSelfIllumination().setTexture(texture);
            }
        }
    });
}

VRO_METHOD(void, nativeSetColor)(VRO_ARGS
                                 VRO_REF(VROMaterial) material_j,
                                 VRO_LONG color,
                                 VRO_STRING materialPropertyName) {
    VRO_METHOD_PREAMBLE;
    std::string strName = VRO_STRING_STL(materialPropertyName);
    VROVector4f vecColor = parseColor(color);

    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([vecColor, material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            // Depending on the name, set the correct color
            if (VROStringUtil::strcmpinsensitive(strName, "diffuseColor")) {
                material->getDiffuse().setColor(vecColor);
            } else if (VROStringUtil::strcmpinsensitive(strName, "specularColor")) {
                material->getSpecular().setColor(vecColor);
            } else if (VROStringUtil::strcmpinsensitive(strName, "normalColor")) {
                material->getNormal().setColor(vecColor);
            } else if (VROStringUtil::strcmpinsensitive(strName, "reflectiveColor")) {
                material->getReflective().setColor(vecColor);
            } else if (VROStringUtil::strcmpinsensitive(strName, "emissionColor")) {
                material->getEmission().setColor(vecColor);
            } else if (VROStringUtil::strcmpinsensitive(strName, "multiplyColor")) {
                material->getMultiply().setColor(vecColor);
            } else if (VROStringUtil::strcmpinsensitive(strName, "ambientOcclusionColor")) {
                material->getAmbientOcclusion().setColor(vecColor);
            } else if (VROStringUtil::strcmpinsensitive(strName, "selfIlluminationColor")) {
                material->getSelfIllumination().setColor(vecColor);
            }
        }
    });
}

VRO_METHOD(void, nativeSetFloat)(VRO_ARGS
                                 VRO_REF(VROMaterial) material_j,
                                 VRO_FLOAT value,
                                 VRO_STRING name_j) {
    VRO_METHOD_PREAMBLE;
    std::string name_s = VRO_STRING_STL(name_j);

    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([value, material_w, name_s] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            if (VROStringUtil::strcmpinsensitive(name_s, "metalness")) {
                material->getMetalness().setColor({ value, value, value, 1.0 });
            } else if (VROStringUtil::strcmpinsensitive(name_s, "roughness")) {
                material->getRoughness().setColor({ value, value, value, 1.0 });
            }
        }
    });
}

VRO_METHOD(void, nativeSetShininess)(VRO_ARGS
                                     VRO_REF(VROMaterial) material_j,
                                     VRO_DOUBLE shininess) {
    VRO_METHOD_PREAMBLE;

    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([material_w, shininess] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setShininess(shininess);
        }
    });
}

VRO_METHOD(void, nativeSetFresnelExponent)(VRO_ARGS
                                           VRO_REF(VROMaterial) material_j,
                                           VRO_DOUBLE fresnelExponent) {
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([material_w, fresnelExponent] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setFresnelExponent(fresnelExponent);
        }
    });
}

VRO_METHOD(void, nativeSetLightingModel)(VRO_ARGS
                                         VRO_REF(VROMaterial) material_j,
                                         VRO_STRING lightingModelName) {
    VRO_METHOD_PREAMBLE;

    std::string strName = VRO_STRING_STL(lightingModelName);
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);

    VROPlatformDispatchAsyncRenderer([material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setLightingModel(parseLightingModel(strName));
        }
    });
}

VRO_METHOD(void, nativeSetBlendMode)(VRO_ARGS
                                     VRO_REF(VROMaterial) material_j,
                                     VRO_STRING blendMode_s) {
    VRO_METHOD_PREAMBLE;

    std::string blendMode = VRO_STRING_STL(blendMode_s);
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);

    VROPlatformDispatchAsyncRenderer([material_w, blendMode] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setBlendMode(parseBlendMode(blendMode));
        }
    });
}

VRO_METHOD(void, nativeSetTransparencyMode)(VRO_ARGS
                                            VRO_REF(VROMaterial) material_j,
                                            VRO_STRING transparencyModeName) {
    VRO_METHOD_PREAMBLE;

    std::string strName = VRO_STRING_STL(transparencyModeName);
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);

    VROPlatformDispatchAsyncRenderer([material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setTransparencyMode(parseTransparencyMode(strName));
        }
    });
}

VRO_METHOD(void, nativeSetCullMode)(VRO_ARGS
                                    VRO_REF(VROMaterial) material_j,
                                    VRO_STRING cullModeName) {
    VRO_METHOD_PREAMBLE;

    std::string strName = VRO_STRING_STL(cullModeName);
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);

    VROPlatformDispatchAsyncRenderer([material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setCullMode(parseCullMode(strName));
        }
    });
}

VRO_METHOD(void, nativeSetDiffuseIntensity)(VRO_ARGS
                                            VRO_REF(VROMaterial) material_j, VRO_FLOAT diffuseIntensity) {
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([material_w, diffuseIntensity] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->getDiffuse().setIntensity(diffuseIntensity);
        }
    });
}

VRO_METHOD(void, nativeSetBloomThreshold)(VRO_ARGS
                                          VRO_REF(VROMaterial) material_j, VRO_FLOAT bloomThreshold) {
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROPlatformDispatchAsyncRenderer([material_w, bloomThreshold] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setBloomThreshold(bloomThreshold);
        }
    });
}

VRO_METHOD(void, nativeDestroyMaterial)(VRO_ARGS
                                        VRO_REF(VROMaterial) nativeRef) {
    VRO_REF_DELETE(VROMaterial, nativeRef);
}

VRO_METHOD(void, nativeSetShadowMode(VRO_ARGS
                                     VRO_REF(VROMaterial) material_j, VRO_STRING shadow_j)) {
    VRO_METHOD_PREAMBLE;

    std::string shadow_s = VRO_STRING_STL(shadow_j);
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);

    VROPlatformDispatchAsyncRenderer([material_w, shadow_s] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (!material) {
            return;
        }

        if (VROStringUtil::strcmpinsensitive(shadow_s, "Disabled")) {
            VROARShadow::remove(material);
            material->setReceivesShadows(false);
        }
        else if (VROStringUtil::strcmpinsensitive(shadow_s, "Transparent")) {
            VROARShadow::apply(material);
            material->setReceivesShadows(true);
        }
        else { // Normal
            VROARShadow::remove(material);
            material->setReceivesShadows(true);
        }
    });
}

VRO_METHOD(void, nativeSetName(VRO_ARGS
                               VRO_REF(VROMaterial) jMaterial, VRO_STRING jName)) {
    VRO_METHOD_PREAMBLE;

    std::string strName = VRO_STRING_STL(jName);
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, jMaterial);

    VROPlatformDispatchAsyncRenderer([material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (!material) {
            return;
        }

        material->setName(strName);
    });
}

VRO_METHOD(void, nativeSetChromaKeyFilteringEnabled)(VRO_ARGS
                                                     VRO_REF(VROMaterial) material_j, VRO_BOOL enabled) {
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);

    VROPlatformDispatchAsyncRenderer([material_w, enabled] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (!material) {
            return;
        }
        material->setChromaKeyFilteringEnabled(enabled);
    });
}

VRO_METHOD(void, nativeSetChromaKeyFilteringColor)(VRO_ARGS
                                                   VRO_REF(VROMaterial) material_j, VRO_LONG color_j) {
    std::weak_ptr<VROMaterial> material_w = VRO_REF_GET(VROMaterial, material_j);
    VROVector4f color = parseColor(color_j);

    VROPlatformDispatchAsyncRenderer([material_w, color] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (!material) {
            return;
        }
        material->setChromaKeyFilteringColor({ color.x, color.y, color.z });
    });
}


}  // extern "C"
