//
//  Material_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "Material_JNI.h"
#include "VROStringUtil.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_Material_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateMaterial)(JNIEnv *env, jobject obj) {
    std::shared_ptr<VROMaterial> materialPtr = std::make_shared<VROMaterial>();
    return Material::jptr(materialPtr);
}

JNI_METHOD(void, nativeSetWritesToDepthBuffer)(JNIEnv *env, jobject obj,
                                               jlong nativeRef,
                                               jboolean writesToDepthBuffer) {
    Material::native(nativeRef).get()->setWritesToDepthBuffer(writesToDepthBuffer);
}

JNI_METHOD(void, nativeSetReadsFromDepthBuffer)(JNIEnv *env, jobject obj,
                                               jlong nativeRef,
                                               jboolean readsFromDepthBuffer) {
    Material::native(nativeRef).get()->setReadsFromDepthBuffer(readsFromDepthBuffer);
}

JNI_METHOD(void, nativeSetTexture)(JNIEnv *env, jobject obj,
                                   jlong nativeRef,
                                   jlong textureRef,
                                   jstring materialPropertyName) {
    // Get the string
    const char *cStrName = env->GetStringUTFChars(materialPropertyName, NULL);
    std::string strName(cStrName);

    // Get the texture
    std::shared_ptr<VROTexture> texture = Texture::native(textureRef);

    // Depending on the name, set the texture
    if (VROStringUtil::strcmpinsensitive(strName, "diffuseTexture")) {
        Material::native(nativeRef).get()->getDiffuse().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "specularTexture")) {
        Material::native(nativeRef).get()->getSpecular().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "normalTexture")) {
        Material::native(nativeRef).get()->getNormal().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "reflectiveTexture")) {
        Material::native(nativeRef).get()->getReflective().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "emissionTexture")) {
        Material::native(nativeRef).get()->getEmission().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "transparentTexture")) {
        Material::native(nativeRef).get()->getTransparent().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "multiplyTexture")) {
        Material::native(nativeRef).get()->getMultiply().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "ambientOcclusionTexture")) {
        Material::native(nativeRef).get()->getAmbientOcclusion().setTexture(texture);
    } else if (VROStringUtil::strcmpinsensitive(strName, "selfIlluminationTexture")) {
        Material::native(nativeRef).get()->getSelfIllumination().setTexture(texture);
    }

    env->ReleaseStringUTFChars(materialPropertyName, cStrName);
}

JNI_METHOD(void, nativeSetColor)(JNIEnv *env, jobject obj,
                                 jlong nativeRef,
                                 jlong color,
                                 jstring materialPropertyName) {
    // Get the string
    const char *cStrName = env->GetStringUTFChars(materialPropertyName, NULL);
    std::string strName(cStrName);

    // Get the color
    float a = ((color >> 24) & 0xFF) / 255.0;
    float r = ((color >> 16) & 0xFF) / 255.0;
    float g = ((color >> 8) & 0xFF) / 255.0;
    float b = (color & 0xFF) / 255.0;

    VROVector4f vecColor(r, g, b, a);

    // Depending on the name, set the color
    if (VROStringUtil::strcmpinsensitive(strName, "diffuseColor")) {
        Material::native(nativeRef).get()->getDiffuse().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "specularColor")) {
        Material::native(nativeRef).get()->getSpecular().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "normalColor")) {
        Material::native(nativeRef).get()->getNormal().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "reflectiveColor")) {
        Material::native(nativeRef).get()->getReflective().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "emissionColor")) {
        Material::native(nativeRef).get()->getEmission().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "transparentColor")) {
        Material::native(nativeRef).get()->getTransparent().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "multiplyColor")) {
        Material::native(nativeRef).get()->getMultiply().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "ambientOcclusionColor")) {
        Material::native(nativeRef).get()->getAmbientOcclusion().setColor(vecColor);
    } else if (VROStringUtil::strcmpinsensitive(strName, "selfIlluminationColor")) {
        Material::native(nativeRef).get()->getSelfIllumination().setColor(vecColor);
    }

    env->ReleaseStringUTFChars(materialPropertyName, cStrName);
}

JNI_METHOD(void, nativeSetShininess)(JNIEnv *env, jobject obj,
                                     jlong nativeRef,
                                     jdouble shininess) {
    Material::native(nativeRef).get()->setShininess(shininess);
}

JNI_METHOD(void, nativeSetFresnelExponent)(JNIEnv *env, jobject obj,
                                     jlong nativeRef,
                                     jdouble fresnelExponent) {
    Material::native(nativeRef).get()->setFresnelExponent(fresnelExponent);
}

JNI_METHOD(void, nativeSetLightingModel)(JNIEnv *env, jobject obj,
                                         jlong nativeRef,
                                         jstring lightingModelName) {
    const char *cStrName = env->GetStringUTFChars(lightingModelName, NULL);
    std::string strName(cStrName);

    if (VROStringUtil::strcmpinsensitive(strName, "Blinn")) {
        Material::native(nativeRef).get()->setLightingModel(VROLightingModel::Blinn);
    } else if (VROStringUtil::strcmpinsensitive(strName, "Lambert")) {
        Material::native(nativeRef).get()->setLightingModel(VROLightingModel::Lambert);
    } else if (VROStringUtil::strcmpinsensitive(strName, "Phong")) {
        Material::native(nativeRef).get()->setLightingModel(VROLightingModel::Phong);
    } else {
        // Default lightingModel is Constant, so no use checking.
        Material::native(nativeRef).get()->setLightingModel(VROLightingModel::Constant);
    }

    env->ReleaseStringUTFChars(lightingModelName, cStrName);
}

JNI_METHOD(void, nativeSetTransparencyMode)(JNIEnv *env, jobject obj,
                                      jlong nativeRef,
                                      jstring transparencyModeName) {
    const char *cStrName = env->GetStringUTFChars(transparencyModeName, NULL);
    std::string strName(cStrName);

    if (VROStringUtil::strcmpinsensitive(strName, "RGBZero")) {
        Material::native(nativeRef).get()->setTransparencyMode(VROTransparencyMode::RGBZero);
    } else {
        // Default transparencyMode is AOne, so no use checking.
        Material::native(nativeRef).get()->setTransparencyMode(VROTransparencyMode::AOne);
    }

    env->ReleaseStringUTFChars(transparencyModeName, cStrName);
}

JNI_METHOD(void, nativeSetCullMode)(JNIEnv *env,
                                    jobject obj,
                                    jlong nativeRef,
                                    jstring cullModeName) {
    const char *cStrName = env->GetStringUTFChars(cullModeName, NULL);
    std::string strName(cStrName);

    if (VROStringUtil::strcmpinsensitive(strName, "None")) {
        Material::native(nativeRef).get()->setCullMode(VROCullMode::None);
    } else if (VROStringUtil::strcmpinsensitive(strName, "Front")) {
        Material::native(nativeRef).get()->setCullMode(VROCullMode::Front);
    } else {
        // Default cullMode is Back, so no use checking.
        Material::native(nativeRef).get()->setCullMode(VROCullMode::Back);
    }

    env->ReleaseStringUTFChars(cullModeName, cStrName);
}

JNI_METHOD(void, nativeSetDiffuseIntensity)(JNIEnv *env, jobject obj, jlong nativeRef, jfloat diffuseIntensity) {
    Material::native(nativeRef)->getDiffuse().setIntensity(diffuseIntensity);
}

JNI_METHOD(void, nativeSetBloomThreshold)(JNIEnv *env, jobject obj, jlong nativeRef, jfloat bloomThreshold) {
    Material::native(nativeRef)->setBloomThreshold(bloomThreshold);
}

JNI_METHOD(void, nativeDestroyMaterial)(JNIEnv *env,
                                        jobject obj,
                                        jlong nativeRef) {
    delete reinterpret_cast<PersistentRef<VROMaterial> *>(nativeRef);
}

}  // extern "C"
