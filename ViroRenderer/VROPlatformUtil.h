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
#include <memory>

class VROImage;

std::string VROPlatformGetPathForResource(std::string resource, std::string type);
std::string VROPlatformLoadResourceAsString(std::string resource, std::string type);
std::string VROPlatformLoadFileAsString(std::string path);

/*
 Load the given URL to a file, and return the path to the file. If the file
 is temporary and must be deleted after its processed, temp will be set to true.
 */
std::string VROPlatformLoadURLToFile(std::string url, bool *temp);
void VROPlatformDeleteFile(std::string filename);

std::shared_ptr<VROImage> VROPlatformLoadImageFromFile(std::string filename);

#if VRO_PLATFORM_ANDROID

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>

void VROPlatformSetEnv(JNIEnv *env, jobject activity, jobject assetManager);

// This function was added because VROPlatformConvertBitmap can be called before the renderer
// is created and as a result, activity and assetManager hasn't been set yet. We should think
// about how to do this better.
void VROPlatformSetEnv(JNIEnv *env);

void VROPlatformReleaseEnv();

// Note the returned buffer *must* be freed by the caller!
void *VROPlatformLoadBinaryAsset(std::string resource, std::string type, size_t *length);

// Note the returned buffer *must* be freed by the caller!
void *VROPlatformLoadImageAssetRGBA8888(std::string resource, int *bitmapLength, int *width, int *height);

// Note the returned buffer *must* be freed by the caller!
void *VROPlatformConvertBitmap(jobject jbitmap, int *bitmapLength, int *width, int *height);

// Create a video sink on the Java side. Returns the Surface.
jobject VROPlatformCreateVideoSink(int textureId);
void VROPlatformDestroyVideoSink(int textureId);

// Get audio properties for this device.
int VROPlatformGetAudioSampleRate();
int VROPlatformGetAudioBufferSize();

JNIEnv *VROPlatformGetJNIEnv();
jobject VROPlatformGetJavaAssetManager();
AAssetManager *VROPlatformGetAssetManager();

#endif

#endif /* VROPlatformUtil_h */
