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

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Material_##method_name

extern "C" {

VROVector4f parseColor(jlong color) {
    float a = ((color >> 24) & 0xFF) / 255.0;
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;
    return {r, g, b, a};
}

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
    else {
        // Default lightingModel is Constant, so no use checking.
        return VROLightingModel::Constant;
    }
}

VROBlendMode parseBlendMode(std::string blendMode) {
    if (VROStringUtil::strcmpinsensitive(blendMode, "Alpha")) {
        return VROBlendMode::Alpha;
    }
    else if (VROStringUtil::strcmpinsensitive(blendMode, "None")) {
        return VROBlendMode::None;
    }
    else {
        // Default transparencyMode is AOne, so no use checking.
        return VROBlendMode::Add;
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

JNI_METHOD(jlong, nativeCreateMaterial)(JNIEnv *env, jobject obj) {
    std::shared_ptr<VROMaterial> materialPtr = std::make_shared<VROMaterial>();
    return Material::jptr(materialPtr);
}

JNI_METHOD(jlong, nativeCreateImmutableMaterial)(JNIEnv *env, jobject obj,
                                                 jstring lightingModel, jlong diffuseColor, jlong diffuseTexture,
                                                 jfloat diffuseIntensity, jlong specularTexture,
                                                 jfloat shininess, jfloat fresnelExponent, jlong normalMap, jstring cullMode,
                                                 jstring transparencyMode, jstring blendMode, jfloat bloomThreshold,
                                                 jboolean writesToDepthBuffer, jboolean readsFromDepthBuffer) {
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setLightingModel(parseLightingModel(VROPlatformGetString(lightingModel, env)));
    material->getDiffuse().setColor(parseColor(diffuseColor));
    if (diffuseTexture != 0) { material->getDiffuse().setTexture(Texture::native(diffuseTexture)); }
    material->getDiffuse().setIntensity(diffuseIntensity);
    if (specularTexture != 0) { material->getSpecular().setTexture(Texture::native(specularTexture)); }
    material->setShininess(shininess);
    material->setFresnelExponent(fresnelExponent);
    if (normalMap != 0) { material->getNormal().setTexture(Texture::native(normalMap)); }
    material->setCullMode(parseCullMode(VROPlatformGetString(cullMode, env)));
    material->setTransparencyMode(parseTransparencyMode(VROPlatformGetString(transparencyMode, env)));
    material->setBlendMode(parseBlendMode(VROPlatformGetString(blendMode, env)));
    material->setBloomThreshold(bloomThreshold);
    material->setWritesToDepthBuffer(writesToDepthBuffer);
    material->setReadsFromDepthBuffer(readsFromDepthBuffer);
    return Material::jptr(material);
}

JNI_METHOD(void, nativeSetWritesToDepthBuffer)(JNIEnv *env, jobject obj,
                                               jlong material_j,
                                               jboolean writesToDepthBuffer) {
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);
    VROPlatformDispatchAsyncRenderer([material_w, writesToDepthBuffer] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setWritesToDepthBuffer(writesToDepthBuffer);
        }
    });
}

JNI_METHOD(void, nativeSetReadsFromDepthBuffer)(JNIEnv *env, jobject obj,
                                                jlong material_j,
                                                jboolean readsFromDepthBuffer) {
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);
    VROPlatformDispatchAsyncRenderer([material_w, readsFromDepthBuffer] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setReadsFromDepthBuffer(readsFromDepthBuffer);
        }
    });
}

JNI_METHOD(void, nativeSetTexture)(JNIEnv *env, jobject obj,
                                   jlong material_j,
                                   jlong textureRef,
                                   jstring materialPropertyName) {
    std::string strName = VROPlatformGetString(materialPropertyName, env);
    std::shared_ptr<VROTexture> texture = Texture::native(textureRef);
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);

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
            } else if (VROStringUtil::strcmpinsensitive(strName, "transparentTexture")) {
                material->getTransparent().setTexture(texture);
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

JNI_METHOD(void, nativeSetColor)(JNIEnv *env, jobject obj,
                                 jlong material_j,
                                 jlong color,
                                 jstring materialPropertyName) {
    std::string strName = VROPlatformGetString(materialPropertyName, env);
    VROVector4f vecColor = parseColor(color);

    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);
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
            } else if (VROStringUtil::strcmpinsensitive(strName, "transparentColor")) {
                material->getTransparent().setColor(vecColor);
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

JNI_METHOD(void, nativeSetShininess)(JNIEnv *env, jobject obj,
                                     jlong material_j,
                                     jdouble shininess) {
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);
    VROPlatformDispatchAsyncRenderer([material_w, shininess] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setShininess(shininess);
        }
    });
}

JNI_METHOD(void, nativeSetFresnelExponent)(JNIEnv *env, jobject obj,
                                           jlong material_j,
                                           jdouble fresnelExponent) {
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);
    VROPlatformDispatchAsyncRenderer([material_w, fresnelExponent] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setFresnelExponent(fresnelExponent);
        }
    });
}

JNI_METHOD(void, nativeSetLightingModel)(JNIEnv *env, jobject obj,
                                         jlong material_j,
                                         jstring lightingModelName) {
    std::string strName = VROPlatformGetString(lightingModelName, env);
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);

    VROPlatformDispatchAsyncRenderer([material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setLightingModel(parseLightingModel(strName));
        }
    });
}

JNI_METHOD(void, nativeSetBlendMode)(JNIEnv *env, jobject obj,
                                     jlong material_j,
                                     jstring blendMode_s) {
    std::string blendMode = VROPlatformGetString(blendMode_s, env);
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);

    VROPlatformDispatchAsyncRenderer([material_w, blendMode] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setBlendMode(parseBlendMode(blendMode));
        }
    });
}

JNI_METHOD(void, nativeSetTransparencyMode)(JNIEnv *env, jobject obj,
                                            jlong material_j,
                                            jstring transparencyModeName) {
    std::string strName = VROPlatformGetString(transparencyModeName, env);
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);

    VROPlatformDispatchAsyncRenderer([material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setTransparencyMode(parseTransparencyMode(strName));
        }
    });
}

JNI_METHOD(void, nativeSetCullMode)(JNIEnv *env,
                                    jobject obj,
                                    jlong material_j,
                                    jstring cullModeName) {
    std::string strName = VROPlatformGetString(cullModeName, env);
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);

    VROPlatformDispatchAsyncRenderer([material_w, strName] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setCullMode(parseCullMode(strName));
        }
    });
}

JNI_METHOD(void, nativeSetDiffuseIntensity)(JNIEnv *env, jobject obj,
                                            jlong material_j, jfloat diffuseIntensity) {
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);
    VROPlatformDispatchAsyncRenderer([material_w, diffuseIntensity] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->getDiffuse().setIntensity(diffuseIntensity);
        }
    });
}

JNI_METHOD(void, nativeSetBloomThreshold)(JNIEnv *env, jobject obj,
                                          jlong material_j, jfloat bloomThreshold) {
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);
    VROPlatformDispatchAsyncRenderer([material_w, bloomThreshold] {
        std::shared_ptr<VROMaterial> material = material_w.lock();
        if (material) {
            material->setBloomThreshold(bloomThreshold);
        }
    });
}

JNI_METHOD(void, nativeDestroyMaterial)(JNIEnv *env,
                                        jobject obj,
                                        jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROMaterial> *>(nativeRef);
}

JNI_METHOD(void, nativeSetShadowMode(JNIEnv *env, jobject obj,
                                     jlong material_j, jstring shadow_j)) {
    std::string shadow_s = VROPlatformGetString(shadow_j, env);
    std::weak_ptr<VROMaterial> material_w = Material::native(material_j);

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

}  // extern "C"
