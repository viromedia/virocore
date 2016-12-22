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
#include <functional>

#if VRO_PLATFORM_ANDROID

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>

#endif

class VROImage;

#pragma mark - String Loading

std::string VROPlatformLoadResourceAsString(std::string resource, std::string type);
std::string VROPlatformLoadFileAsString(std::string path);

#pragma mark - Network and File Utilities

/*
 Load the given URL to a file, and return the path to the file. If the file
 is temporary and must be deleted after its processed, temp will be set to true.
 */
std::string VROPlatformDownloadURLToFile(std::string url, bool *temp);
void VROPlatformDeleteFile(std::string filename);

#pragma mark - Image Loading

std::shared_ptr<VROImage> VROPlatformLoadImageFromFile(std::string filename);

#if VRO_PLATFORM_ANDROID
std::shared_ptr<VROImage> VROPlatformLoadImageFromAsset(std::string asset);
jobject VROPlatformLoadBitmapFromAsset(std::string resource);
jobject VROPlatformLoadBitmapFromFile(std::string path);

// Note the returned buffer *must* be freed by the caller!
void *VROPlatformConvertBitmap(jobject jbitmap, int *bitmapLength, int *width, int *height);
#endif

#pragma mark - Threading

/*
 Run the given function on the rendering thread, asynchronously (this function
 returns immediately).
 */
void VROPlatformDispatchAsyncRenderer(std::function<void()> fcn);

/*
 Run the given function on a background thread. The thread can be pooled, 
 or spun up fresh. The caller should make no assumptions.
 */
void VROPlatformDispatchAsyncBackground(std::function<void()> fcn);

#pragma mark - Android Setup

#if VRO_PLATFORM_ANDROID

void VROPlatformSetEnv(JNIEnv *env, jobject activity, jobject assetManager, jobject platformUtil);

// This function was added because VROPlatformConvertBitmap can be called before the renderer
// is created and as a result, activity and assetManager hasn't been set yet. We should think
// about how to do this better.
void VROPlatformSetEnv(JNIEnv *env);
void VROPlatformReleaseEnv();

JNIEnv *VROPlatformGetJNIEnv();
jobject VROPlatformGetJavaAssetManager();
AAssetManager *VROPlatformGetAssetManager();

// Copy the given asset into a file with the same name in the cache dir.
// This enables us to load assets through routines that only take file paths,
// for testing purposes only. Not needed in prod because assets are not used
// in prod.
std::string VROPlatformCopyAssetToFile(std::string asset);

// Calls a java function from native through JNI on the given jObject with the given
// classPath, functionName, methodID and desired java function parameters.
//
// Example: VROPlatformCallJavaFunction(jObj,
//                                      "com/viro/renderer/jni/EventDelegateJni",
//                                      "onGaze",
//                                      "(Z)V",
//                                      isGazing);
void VROPlatformCallJavaFunction(jobject javaObject,
                                 std::string classPath,
                                 std::string functionName,
                                 std::string methodID, ...);

#pragma mark - Android A/V

// Create a video sink on the Java side. Returns the Surface.
jobject VROPlatformCreateVideoSink(int textureId);
void VROPlatformDestroyVideoSink(int textureId);

// Get audio properties for this device.
int VROPlatformGetAudioSampleRate();
int VROPlatformGetAudioBufferSize();

extern "C" {

void Java_com_viro_renderer_jni_PlatformUtil_runTask(JNIEnv *env, jclass clazz, jint taskId);

}

#endif

#endif /* VROPlatformUtil_h */
