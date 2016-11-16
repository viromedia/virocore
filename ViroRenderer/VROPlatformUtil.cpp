//
//  VROPlatformUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROPlatformUtil.h"
#include "VROLog.h"

#if VRO_PLATFORM_IOS

#import <Foundation/Foundation.h>

std::string VROPlatformGetPathForResource(std::string resource, std::string type) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:[NSString stringWithUTF8String:resource.c_str()]
                                      ofType:[NSString stringWithUTF8String:type.c_str()]];
    
    return std::string([path UTF8String]);
}

std::string VROPlatformLoadFileAsString(std::string path) {
    return std::string([[NSString stringWithContentsOfFile:[NSString stringWithUTF8String:path.c_str()]
                                                  encoding:NSUTF8StringEncoding
                                                     error:nil] UTF8String]);
}

std::string VROPlatformLoadResourceAsString(std::string resource, std::string type) {
    return VROPlatformLoadFileAsString(VROPlatformGetPathForResource(resource, type));
}

#elif VRO_PLATFORM_ANDROID

// We can hold a static reference to the JVM and to global references, but not to individual
// JNIEnv objects, as those are thread-local. Access the JNIEnv object via getJNIEnv().
// There is one JavaVM per application on Android (shared across activities).
static JavaVM *sVM = nullptr;
static jobject sActivity = nullptr;
static AAssetManager *sAssetMgr = nullptr;

// Get the JNI Environment for the current thread. If the JavaVM is not yet attached to the
// current thread, attach it
void getJNIEnv(JNIEnv **jenv) {
    if (sVM->GetEnv((void **) jenv, JNI_VERSION_1_6) == JNI_EDETACHED) {
        sVM->AttachCurrentThread(jenv, nullptr);
    }
}

void VROPlatformSetEnv(JNIEnv *env, jobject activity, jobject assetManager) {
    env->GetJavaVM(&sVM);
    sActivity = env->NewGlobalRef(activity);
    sAssetMgr = AAssetManager_fromJava(env, assetManager);
}

JNIEnv *VROPlatformGetJNIEnv() {
    JNIEnv *env;
    getJNIEnv(&env);

    return env;
}

AAssetManager *VROPlatformGetAssetManager() {
    return sAssetMgr;
}

void VROPlatformReleaseEnv() {
    JNIEnv *env;
    getJNIEnv(&env);

    env->DeleteGlobalRef(sActivity);

    sActivity = nullptr;
    sAssetMgr = nullptr;
}

std::string VROPlatformGetPathForResource(std::string resource, std::string type) {
    // Android does not expose paths to resources
    pabort();
    return "";
}

std::string VROPlatformLoadFileAsString(std::string path) {
    pabort();
    return "";
}

std::string VROPlatformLoadResourceAsString(std::string resource, std::string type) {
    std::string assetName = resource + "." + type;

    AAsset *asset = AAssetManager_open(sAssetMgr, assetName.c_str(), AASSET_MODE_BUFFER);
    size_t length = AAsset_getLength(asset);
    
    char *buffer = (char *)malloc(length + 1);
    AAsset_read(asset, buffer, length);
    buffer[length] = 0;
    
    std::string str(buffer);
    AAsset_close(asset);
    free(buffer);

    return str;
}

void *VROPlatformLoadBinaryAsset(std::string resource, std::string type, size_t *length) {
    std::string assetName = resource + "." + type;

    AAsset *asset = AAssetManager_open(sAssetMgr, assetName.c_str(), AASSET_MODE_BUFFER);
    *length = AAsset_getLength(asset);

    char *buffer = (char *)malloc(*length);
    AAsset_read(asset, buffer, *length);
    AAsset_close(asset);

    return buffer;
}

void *VROPlatformLoadImageAssetRGBA8888(std::string resource, int *bitmapLength, int *width, int *height) {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sActivity);
    jmethodID jmethod = env->GetMethodID(cls, "loadBitmap", "(Ljava/lang/String;)Landroid/graphics/Bitmap;");

    jstring string = env->NewStringUTF(resource.c_str());
    jobject jbitmap = env->CallObjectMethod(sActivity, jmethod, string);

    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, jbitmap, &bitmapInfo);

    passert (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888);

    *width = bitmapInfo.width;
    *height = bitmapInfo.height;
    *bitmapLength = bitmapInfo.height * bitmapInfo.stride;

    void *bitmapData;
    AndroidBitmap_lockPixels(env, jbitmap, &bitmapData);

    void *safeData = malloc(*bitmapLength);
    memcpy(safeData, bitmapData, *bitmapLength);

    AndroidBitmap_unlockPixels(env, jbitmap);

    env->DeleteLocalRef(jbitmap);
    env->DeleteLocalRef(string);
    env->DeleteLocalRef(cls);

    return safeData;
}

jobject VROPlatformCreateVideoSink(int textureId) {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sActivity);
    jmethodID jmethod = env->GetMethodID(cls, "createVideoSink", "(I)Landroid/view/Surface;");
    jobject jsurface = env->CallObjectMethod(sActivity, jmethod, textureId);

    env->DeleteLocalRef(cls);
    return jsurface;
}

int VROPlatformGetAudioSampleRate() {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sActivity);
    jmethodID jmethod = env->GetMethodID(cls, "getAudioSampleRate", "()I");
    jint sampleRate = env->CallIntMethod(sActivity, jmethod);

    env->DeleteLocalRef(cls);
    return sampleRate;
}

int VROPlatformGetAudioBufferSize() {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sActivity);
    jmethodID jmethod = env->GetMethodID(cls, "getAudioBufferSize", "()I");
    jint bufferSize = env->CallIntMethod(sActivity, jmethod);

    env->DeleteLocalRef(cls);
    return bufferSize;
}

#endif
