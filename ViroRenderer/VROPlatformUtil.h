//
//  VROPlatformUtil.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROPlatformUtil_h
#define VROPlatformUtil_h

#include "VRODefines.h"
#include <string>

std::string VROPlatformGetPathForResource(std::string resource, std::string type);
std::string VROPlatformLoadResourceAsString(std::string resource, std::string type);
std::string VROPlatformLoadFileAsString(std::string path);

#if VRO_PLATFORM_ANDROID

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>

void VROPlatformSetEnv(JNIEnv *env, jobject activity, jobject assetManager);
void VROPlatformReleaseEnv();

// Note the returned buffer *must* be freed by the caller!
void *VROPlatformLoadBinaryAsset(std::string resource, std::string type, size_t *length);

// Note the returned buffer *must* be freed by the caller!
void *VROPlatformLoadImageAssetRGBA8888(std::string resource, int *bitmapLength, int *width, int *height);

// Create a video sink on the Java side. Returns the Surface.
jobject VROPlatformCreateVideoSink(int textureId);

// Get audio properties for this device.
int VROPlatformGetAudioSampleRate();
int VROPlatformGetAudioBufferSize();

JNIEnv *VROPlatformGetJNIEnv();
AAssetManager *VROPlatformGetAssetManager();

#endif

#endif /* VROPlatformUtil_h */
