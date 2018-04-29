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
    static VRO_REF jptr(std::shared_ptr<VROMaterial> ptr) {
        PersistentRef<VROMaterial> *persistentRef = new PersistentRef<VROMaterial>(ptr);
        return reinterpret_cast<intptr_t>(persistentRef);
    }

    static std::shared_ptr<VROMaterial> native(VRO_REF ptr) {
        PersistentRef<VROMaterial> *persistentRef = reinterpret_cast<PersistentRef<VROMaterial> *>(ptr);
        return persistentRef->get();
    }

    static VRO_OBJECT createJMaterial(std::shared_ptr<VROMaterial> &mat) {
        JNIEnv *env = VROPlatformGetJNIEnv();
        if (env == nullptr){
            perror("Required JNIEnv to create a jMaterial is null!");
            return NULL;
        }

        // Create a persistent native reference that would represent the jMaterial object.
        PersistentRef<VROMaterial> *persistentRef = new PersistentRef<VROMaterial>(mat);
        jlong matRef = reinterpret_cast<intptr_t>(persistentRef);
        jclass cls = env->FindClass("com/viro/core/Material");
        if (cls == nullptr){
            perror("Required Materia.java class to create a jMaterial is null!");
            return NULL;
        }

        // Create our Material.java object with the native reference.
        jmethodID jmethod = env->GetMethodID(cls, "<init>", "(J)V");
        VRO_OBJECT jMat = env->NewObject(cls, jmethod, matRef);
        VROPlatformSetString(env, cls, jMat, "mName", mat->getName());

        // Set basic visual properties for Material.java
        VROPlatformSetFloat(env, cls, jMat, "mDiffuseIntensity", mat->getDiffuse().getIntensity());
        VROPlatformSetFloat(env, cls, jMat, "mShininess", mat->getShininess());
        VROPlatformSetFloat(env, cls, jMat, "mFresnelExponent", mat->getFresnelExponent());
        VROPlatformSetFloat(env, cls, jMat, "mBloomThreshold", mat->getBloomThreshold());
        VROPlatformSetFloat(env, cls, jMat, "mRoughness", mat->getRoughness().getColor().x);
        VROPlatformSetFloat(env, cls, jMat, "mMetalness", mat->getMetalness().getColor().x);
        VROPlatformSetInt(env, cls, jMat, "mDiffuseColor", parseColor(mat->getDiffuse().getColor()));
        VROPlatformSetBool(env, cls, jMat, "mWritesToDepthBuffer", mat->getWritesToDepthBuffer());
        VROPlatformSetBool(env, cls, jMat, "mReadsFromDepthBuffer", mat->getReadsFromDepthBuffer());

        // Set enum states for Material.java
        setEnumLightModel(env, cls, jMat, "mLightingModel", mat->getLightingModel());
        setCullMode(env, cls, jMat, "mCullMode", mat->getCullMode());
        setTransparencyMode(env, cls, jMat, "mTransparencyMode", mat->getTransparencyMode());
        setBlendMode(env, cls, jMat, "mBlendMode", mat->getBlendMode());
        setShadowMode(env, cls, jMat, "mShadowMode", mat->getReceivesShadows());

        // Set the texture values for Material.java
        setTexture(env, cls, jMat, "mDiffuseTexture", mat->getDiffuse().getTexture());
        setTexture(env, cls, jMat, "mNormalMap", mat->getNormal().getTexture());
        setTexture(env, cls, jMat, "mSpecularTexture", mat->getSpecular().getTexture());
        setTexture(env, cls, jMat, "mMetalnessMap", mat->getMetalness().getTexture());
        setTexture(env, cls, jMat, "mRoughnessMap", mat->getRoughness().getTexture());
        setTexture(env, cls, jMat, "mAmbientOcclusionMap", mat->getAmbientOcclusion().getTexture());

        VRO_DELETE_LOCAL_REF(cls);
        return jMat;
    }

private:
    Material() {}
    static void setEnumLightModel(JNIEnv *env, jclass cls, VRO_OBJECT jMat, const char *jMatFieldName,
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

        VROPlatformSetEnumValue(env, cls, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setCullMode(JNIEnv *env, jclass cls, VRO_OBJECT jMat, const char *jMatFieldName,
                            VROCullMode mode) {
        std::string enumClassPathName = "com/viro/core/Material$CullMode";
        std::string enumValueStr;
        switch(mode) {
            case VROCullMode::Back: enumValueStr = "BACK"; break;
            case VROCullMode::Front: enumValueStr = "FRONT"; break;
            default:
                enumValueStr = "BACK";
        }

        VROPlatformSetEnumValue(env, cls, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setTransparencyMode(JNIEnv *env, jclass cls, VRO_OBJECT jMat, const char *jMatFieldName,
                                    VROTransparencyMode mode) {
        std::string enumClassPathName = "com/viro/core/Material$TransparencyMode";
        std::string enumValueStr;
        switch(mode) {
            case VROTransparencyMode::AOne: enumValueStr = "A_ONE"; break;
            case VROTransparencyMode::RGBZero: enumValueStr = "RGB_ZERO"; break;
            default:
                enumValueStr = "A_ONE";
        }

        VROPlatformSetEnumValue(env, cls, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setBlendMode(JNIEnv *env, jclass cls, VRO_OBJECT jMat, const char *jMatFieldName,
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

        VROPlatformSetEnumValue(env, cls, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setShadowMode(JNIEnv *env, jclass cls, VRO_OBJECT jMat, const char *jMatFieldName,
                              bool shadow) {
        std::string enumClassPathName = "com/viro/core/Material$ShadowMode";
        std::string enumValueStr = shadow ? "NORMAL" : "DISABLED";

        VROPlatformSetEnumValue(env, cls, jMat, jMatFieldName, enumClassPathName, enumValueStr);
    }

    static void setTexture(JNIEnv *env, jclass cls, VRO_OBJECT jObj, const char *fieldName,
                           std::shared_ptr<VROTexture> texture) {
        if (texture == nullptr){
            return;
        }

        VRO_OBJECT jTexture = Texture::createJTexture(texture);
        jfieldID fieldId = env->GetFieldID(cls, fieldName, "Lcom/viro/core/Texture;");
        if (fieldId == NULL){
            pwarn("Attempted to set undefined field: %s", fieldName);
            return;
        }

        env->SetObjectField(jObj, fieldId, jTexture);
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