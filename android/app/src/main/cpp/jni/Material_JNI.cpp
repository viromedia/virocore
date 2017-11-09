//
//  Material_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "Material_JNI.h"
#include "VROStringUtil.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_Material_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateMaterial)(JNIEnv *env, jobject obj) {
    std::shared_ptr<VROMaterial> materialPtr = std::make_shared<VROMaterial>();
    return Material::jptr(materialPtr);
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

    // Get the color
    float a = ((color >> 24) & 0xFF) / 255.0;
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;
    VROVector4f vecColor(r, g, b, a);

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
            if (VROStringUtil::strcmpinsensitive(strName, "Blinn")) {
                material->setLightingModel(VROLightingModel::Blinn);
            } else if (VROStringUtil::strcmpinsensitive(strName, "Lambert")) {
                material->setLightingModel(VROLightingModel::Lambert);
            } else if (VROStringUtil::strcmpinsensitive(strName, "Phong")) {
                material->setLightingModel(VROLightingModel::Phong);
            } else {
                // Default lightingModel is Constant, so no use checking.
                material->setLightingModel(VROLightingModel::Constant);
            }
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
            if (VROStringUtil::strcmpinsensitive(blendMode, "Alpha")) {
                material->setBlendMode(VROBlendMode::Alpha);
            }
            else if (VROStringUtil::strcmpinsensitive(blendMode, "None")) {
                material->setBlendMode(VROBlendMode::None);
            }
            else {
                // Default transparencyMode is AOne, so no use checking.
                material->setBlendMode(VROBlendMode::Add);
            }
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
            if (VROStringUtil::strcmpinsensitive(strName, "RGBZero")) {
                material->setTransparencyMode(VROTransparencyMode::RGBZero);
            }
            else {
                // Default transparencyMode is AOne, so no use checking.
                material->setTransparencyMode(VROTransparencyMode::AOne);
            }
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
            if (VROStringUtil::strcmpinsensitive(strName, "None")) {
                material->setCullMode(VROCullMode::None);
            }
            else if (VROStringUtil::strcmpinsensitive(strName, "Front")) {
                material->setCullMode(VROCullMode::Front);
            } else {
                // Default cullMode is Back, so no use checking.
                material->setCullMode(VROCullMode::Back);
            }
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

}  // extern "C"
