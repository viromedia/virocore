//
//  Material_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//
#ifndef Material_JNI_h
#define Material_JNI_h

#include <memory>
#include <VROLog.h>
#include "VROMaterial.h"
#include "Texture_JNI.h"
#include "VROPlatformUtil.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

class Material {
public:

    static VRO_OBJECT createJMaterial(const std::shared_ptr<VROMaterial> &mat) {
        VRO_ENV env = VROPlatformGetJNIEnv();
        if (env == nullptr) {
            perror("Required JNIEnv to create a jMaterial is null!");
            return VRO_OBJECT_NULL;
        }

        // Create a persistent native reference that would represent the jMaterial object.
        VRO_REF(VROMaterial) matRef = VRO_REF_NEW(VROMaterial, mat);

        // Create our Material.java object with the native reference.
        VRO_OBJECT jMat = VROPlatformConstructHostObject("com/viro/core/Material", "(J)V", matRef);
        VROPlatformSetString(env, jMat, "mName", mat->getName());

        // Set basic visual properties for Material.java
        VROPlatformSetFloat(env, jMat, "mDiffuseIntensity", mat->getDiffuse().getIntensity());
        VROPlatformSetFloat(env, jMat, "mShininess", mat->getShininess());
        VROPlatformSetFloat(env, jMat, "mFresnelExponent", mat->getFresnelExponent());
        VROPlatformSetFloat(env, jMat, "mBloomThreshold", mat->getBloomThreshold());
        VROPlatformSetFloat(env, jMat, "mRoughness", mat->getRoughness().getColor().x);
        VROPlatformSetFloat(env, jMat, "mMetalness", mat->getMetalness().getColor().x);
        VROPlatformSetInt(env, jMat, "mDiffuseColor", parseColor(mat->getDiffuse().getColor()));
        VROPlatformSetBool(env, jMat, "mWritesToDepthBuffer", mat->getWritesToDepthBuffer());
        VROPlatformSetBool(env, jMat, "mReadsFromDepthBuffer", mat->getReadsFromDepthBuffer());

        // Set enum states for Material.java
        setEnumLightModel(env, jMat, "mLightingModel", mat->getLightingModel());
        setCullMode(env, jMat, "mCullMode", mat->getCullMode());
        setTransparencyMode(env, jMat, "mTransparencyMode", mat->getTransparencyMode());
        setBlendMode(env, jMat, "mBlendMode", mat->getBlendMode());
        setShadowMode(env, jMat, "mShadowMode", mat->getReceivesShadows());

        // Set the texture values for Material.java
        setTexture(env, jMat, "mDiffuseTexture", mat->getDiffuse().getTexture());
        setTexture(env, jMat, "mNormalMap", mat->getNormal().getTexture());
        setTexture(env, jMat, "mSpecularTexture", mat->getSpecular().getTexture());
        setTexture(env, jMat, "mMetalnessMap", mat->getMetalness().getTexture());
        setTexture(env, jMat, "mRoughnessMap", mat->getRoughness().getTexture());
        setTexture(env, jMat, "mAmbientOcclusionMap", mat->getAmbientOcclusion().getTexture());

        return jMat;
    }

private:
    Material() {}
    static void setEnumLightModel(VRO_ENV env, VRO_OBJECT jMat, const char *jMatFieldName,
                                  VROLightingModel model) {
        std::string enumClassPathName = "com/viro/core/Material$LightingModel";
        std::string enumValueStr;
        switch(model) {
            case VROLightingModel::Phong: enumValueStr = "PHONG"; break;
            case VROLightingModel::Blinn: enumValueStr = "BLINN"; break;
            case VROLightingModel::Lambert: enumValueStr = "LAMBERT"; break;
            case VROLightingModel::PhysicallyBased: enumValueStr = "PHYSICALLY_BASED"; break;
            default:
                enumValueStr = "CONSTANT";
        }

        VROPlatformSetEnumValue(env, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setCullMode(VRO_ENV env, VRO_OBJECT jMat, const char *jMatFieldName,
                            VROCullMode mode) {
        std::string enumClassPathName = "com/viro/core/Material$CullMode";
        std::string enumValueStr;
        switch(mode) {
            case VROCullMode::Back: enumValueStr = "BACK"; break;
            case VROCullMode::Front: enumValueStr = "FRONT"; break;
            default:
                enumValueStr = "BACK";
        }

        VROPlatformSetEnumValue(env, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setTransparencyMode(VRO_ENV env, VRO_OBJECT jMat, const char *jMatFieldName,
                                    VROTransparencyMode mode) {
        std::string enumClassPathName = "com/viro/core/Material$TransparencyMode";
        std::string enumValueStr;
        switch(mode) {
            case VROTransparencyMode::AOne: enumValueStr = "A_ONE"; break;
            case VROTransparencyMode::RGBZero: enumValueStr = "RGB_ZERO"; break;
            default:
                enumValueStr = "A_ONE";
        }

        VROPlatformSetEnumValue(env, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setBlendMode(VRO_ENV env, VRO_OBJECT jMat, const char *jMatFieldName,
                             VROBlendMode mode) {
        std::string enumClassPathName = "com/viro/core/Material$BlendMode";
        std::string enumValueStr;
        switch(mode) {
            case VROBlendMode::None: enumValueStr = "NONE"; break;
            case VROBlendMode::Alpha: enumValueStr = "ALPHA"; break;
            case VROBlendMode::Add: enumValueStr = "ADD"; break;
            default:
                enumValueStr = "ALPHA";
        }

        VROPlatformSetEnumValue(env, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setShadowMode(VRO_ENV env, VRO_OBJECT jMat, const char *jMatFieldName,
                              bool shadow) {
        std::string enumClassPathName = "com/viro/core/Material$ShadowMode";
        std::string enumValueStr = shadow ? "NORMAL" : "DISABLED";

        VROPlatformSetEnumValue(env, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setTexture(VRO_ENV env, VRO_OBJECT jObj, const char *fieldName,
                           std::shared_ptr<VROTexture> texture) {
        if (texture == nullptr){
            return;
        }

        VRO_OBJECT jTexture = Texture::createJTexture(texture);
        VROPlatformSetObject(env, jObj, fieldName, "Lcom/viro/core/Texture;", jTexture);
        VRO_DELETE_LOCAL_REF(jTexture);
    }

    static int parseColor(VROVector4f parseColor) {
        int a = parseColor.w * 255.0;
        int r = parseColor.x * 255.0;
        int g = parseColor.y * 255.0;
        int b = parseColor.z * 255.0;
        int color = (a & 0xFF) << 24 | (r & 0xFF) << 16 | (g & 0xFF) << 8 | (b & 0xFF);
        return color;
    }
};
#endif