//
//  VROPlatformUtil.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROPlatformUtil.h"
#include "VROLog.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstring>

static VROPlatformType sPlatformType = VROPlatformType::Unknown;

void VROPlatformSetType(VROPlatformType type) {
    sPlatformType = type;
}
VROPlatformType VROPlatformGetType() {
    return sPlatformType;
}

std::string VROPlatformLastPathComponent(std::string url, std::string fallback) {
    size_t lastIndex = url.find_last_of("/");
    if (lastIndex == std::string::npos || (lastIndex + 1) >= url.size()) {
        return fallback;
    } else {
        return url.substr(lastIndex + 1);
    }
}

std::string VROPlatformLoadFileAsString(std::string path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (input) {
        std::string contents;
        input.seekg(0, std::ios::end);
        contents.resize(input.tellg());
        input.seekg(0, std::ios::beg);
        input.read(&contents[0], contents.size());
        input.close();
        
        return(contents);
    }
    // return an empty string which indicates the file did not load.
    return {};
}

void *VROPlatformLoadFile(std::string filename, int *outLength) {
    FILE *fl = fopen(filename.c_str(), "r");
    if (fl == NULL) {
        pinfo("Failed to open file %s", filename.c_str());
        return NULL;
    }
    
    fseek(fl, 0, SEEK_END);
    *outLength = (int) ftell(fl);
    
    char *ret = (char *)malloc(*outLength);
    fseek(fl, 0, SEEK_SET);
    fread(ret, 1, *outLength, fl);
    fclose(fl);
    
    return ret;
}

#pragma mark - iOS and MacOS
#if VRO_PLATFORM_IOS || VRO_PLATFORM_MACOS

#import <Foundation/Foundation.h>

NSURLSessionDataTask *downloadDataWithURLSynchronous(NSURL *url,
                                                     void (^completionBlock)(NSData *data, NSError *error));

std::string VROPlatformGetPathForResource(std::string resource, std::string type) {
    NSBundle *bundle = [NSBundle bundleWithIdentifier:@"com.viro.ViroKit"];
    NSString *path = [bundle pathForResource:[NSString stringWithUTF8String:resource.c_str()]
                                      ofType:[NSString stringWithUTF8String:type.c_str()]];
    
    return std::string([path UTF8String]);
}

std::string VROPlatformLoadResourceAsString(std::string resource, std::string type) {
    return VROPlatformLoadFileAsString(VROPlatformGetPathForResource(resource, type));
}

std::string VROPlatformDownloadURLToFile(std::string url, bool *temp, bool *success) {
    NSURL *URL = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
    __block NSString *tempFilePath;
    
    downloadDataWithURLSynchronous(URL, ^(NSData *data, NSError *error) {
        if (error.code == NSURLErrorCancelled){
            return;
        }
        
        if (data && !error) {
            NSString *fileName = [NSString stringWithFormat:@"%@_%s",
                                  [[NSProcessInfo processInfo] globallyUniqueString],
                                  VROPlatformLastPathComponent(url, "download.tmp").c_str()];
            NSURL *fileURL = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:fileName]];
            [data writeToURL:fileURL atomically:NO];
            
            *temp = true;
            tempFilePath = [fileURL path];
        }
    });
    
    if (tempFilePath) {
        *success = true;
        return std::string([tempFilePath UTF8String]);
    }
    else {
        *success = false;
        return "";
    }
}

void VROPlatformDownloadURLToFileAsync(std::string url,
                                       std::function<void(std::string, bool)> onSuccess,
                                       std::function<void()> onFailure) {
    NSURL *URL = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
    VROPlatformDownloadDataWithURL(URL, ^(NSData *data, NSError *error) {
        if (error.code == NSURLErrorCancelled) {
            return;
        }
        
        if (data && !error) {
            NSString *fileName = [NSString stringWithFormat:@"%@_%s",
                                  [[NSProcessInfo processInfo] globallyUniqueString],
                                  VROPlatformLastPathComponent(url, "download.tmp").c_str()];
            NSURL *fileURL = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:fileName]];
            [data writeToURL:fileURL atomically:NO];
            
            VROPlatformDispatchAsyncRenderer([fileURL, onSuccess] {
                onSuccess([[fileURL path] UTF8String], true);
            });
        }
        else {
            VROPlatformDispatchAsyncRenderer([onFailure] {
                onFailure();
            });
        }
    });
}

std::string VROPlatformCopyResourceToFile(std::string asset, bool *isTemp) {
    // On iOS, bundled resources have file paths, so we can return the path directly
    // without copying
    *isTemp = false;
    return asset;
}

void VROPlatformDeleteFile(std::string filename) {
    NSError *deleteError = nil;
    [[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithUTF8String:filename.c_str()]
                                               error:&deleteError];
}

void VROPlatformDispatchAsyncRenderer(std::function<void()> fcn) {
    dispatch_async(dispatch_get_main_queue(), ^{
        fcn();
    });
}

void VROPlatformDispatchAsyncBackground(std::function<void()> fcn) {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        fcn();
    });
}

std::string VROPlatformFindValueInResourceMap(std::string key, std::map<std::string, std::string> resourceMap) {
    return "";
}

std::string VROPlatformGetDeviceModel() {
    return ""; // TODO: do this for iOS/MacOS if required
}

std::string VROPlatformGetDeviceBrand() {
    return ""; // TODO: do this for iOS/MacOS if required
}

#pragma mark - iOS

#if VRO_PLATFORM_IOS
#import "VROImageiOS.h"

NSURLSessionDataTask *downloadDataWithURLSynchronous(NSURL *url,
                                                     void (^completionBlock)(NSData *data, NSError *error)) {
    
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    sessionConfig.timeoutIntervalForRequest = 30;
    
    NSURLSession *downloadSession = [NSURLSession sessionWithConfiguration: sessionConfig];
    NSURLSessionDataTask *downloadTask = [downloadSession dataTaskWithURL:url
                                                        completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                                                            NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
                                                            if (httpResponse.statusCode != 200) {
                                                                NSLog(@"HTTP request [%@] unsuccessful [status code %ld]", url, (long)httpResponse.statusCode);
                                                                completionBlock(nil, error);
                                                            }
                                                            else {
                                                                completionBlock(data, error);
                                                            }
                                                            dispatch_semaphore_signal(semaphore);
                                                        }];
    [downloadTask resume];
    [downloadSession finishTasksAndInvalidate];
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    
    return downloadTask;
}

NSURLSessionDataTask *VROPlatformDownloadDataWithURL(NSURL *url, void (^completionBlock)(NSData *data, NSError *error)) {
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    sessionConfig.timeoutIntervalForRequest = 30;
    
    NSURLSession *downloadSession = [NSURLSession sessionWithConfiguration: sessionConfig];
    NSURLSessionDataTask *downloadTask = [downloadSession dataTaskWithURL:url
                                                        completionHandler:^(NSData *data, NSURLResponse *response, NSError *error){
                                                            NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
                                                            if (httpResponse.statusCode != 200) {
                                                                NSLog(@"HTTP request [%@] unsuccessful [status code %ld]", url, (long)httpResponse.statusCode);
                                                                completionBlock(nil, error);
                                                            }
                                                            else {
                                                                completionBlock(data, error);
                                                            }
                                                        }];
    [downloadTask resume];
    [downloadSession finishTasksAndInvalidate];
    return downloadTask;
}

std::shared_ptr<VROImage> VROPlatformLoadImageFromFile(std::string filename,
                                                       VROTextureInternalFormat format) {
    UIImage *image = [UIImage imageNamed:[NSString stringWithUTF8String:filename.c_str()]];
    return std::make_shared<VROImageiOS>(image, format);
}

#pragma mark - MacOS

#elif VRO_PLATFORM_MACOS
#import "VROImageMacOS.h"

NSURLSessionDataTask *downloadDataWithURLSynchronous(NSURL *url,
                                                     void (^completionBlock)(NSData *data, NSError *error)) {
    
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    sessionConfig.timeoutIntervalForRequest = 30;
    
    NSURLSession *downloadSession = [NSURLSession sessionWithConfiguration: sessionConfig];
    NSURLSessionDataTask *downloadTask = [downloadSession dataTaskWithURL:url
                                                        completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                                                            completionBlock(data, error);
                                                            dispatch_semaphore_signal(semaphore);
                                                        }];
    [downloadTask resume];
    [downloadSession finishTasksAndInvalidate];
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    
    return downloadTask;
}

NSURLSessionDataTask *VROPlatformDownloadDataWithURL(NSURL *url, void (^completionBlock)(NSData *data, NSError *error)) {
    NSURLSessionConfiguration *sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    sessionConfig.timeoutIntervalForRequest = 30;
    
    NSURLSession *downloadSession = [NSURLSession sessionWithConfiguration: sessionConfig];
    NSURLSessionDataTask *downloadTask = [downloadSession dataTaskWithURL:url
                                                        completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                                                            completionBlock(data, error);
                                                        }];
    [downloadTask resume];
    [downloadSession finishTasksAndInvalidate];
    return downloadTask;
}

std::shared_ptr<VROImage> VROPlatformLoadImageFromFile(std::string filename,
                                                       VROTextureInternalFormat format) {
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:filename.c_str()]];
    return std::make_shared<VROImageMacOS>(image, format);
}

#endif

#pragma mark - Android
#elif VRO_PLATFORM_ANDROID

#include "VROImageAndroid.h"
#include "VROStringUtil.h"
#include <mutex>
#include <thread>
#include <android/bitmap.h>
#include <algorithm>

// We can hold a static reference to the JVM and to global references, but not to individual
// JNIEnv objects, as those are thread-local. Access the JNIEnv object via getJNIEnv().
// There is one JavaVM per application on Android (shared across activities).
static JavaVM *sVM = nullptr;
static jobject sJavaAppContext = nullptr;
static jobject sJavaAssetMgr = nullptr;
static jobject sPlatformUtil = nullptr;
static AAssetManager *sAssetMgr = nullptr;

// Map (and mutex) holding native tasks waiting to be dispatched. On Android the threading
// functionality is handled on the Java layer, so we need some mechanism for mapping IDs
// to corresponding tasks. Both the ID generator and the map itself are protected by the
// mutex.
static std::mutex sTaskMapMutex;
static int sTaskIdGenerator;
static std::map<int, std::function<void()>> sTaskMap;

// These queues store the ids of tasks to run on their respective threads once VROPlatformUtil
// has properly been setup. This is because running these tasks require the PlatformUtil java
// object to be created and set on sPlatformUtil.
static std::vector<int> sBackgroundQueue;
static std::vector<int> sRendererQueue;
static std::vector<int> sAsyncQueue;

// Get the JNI Environment for the current thread. If the JavaVM is not yet attached to the
// current thread, attach it
void getJNIEnv(JNIEnv **jenv) {
    passert (sVM != nullptr);
    if (sVM->GetEnv((void **) jenv, JNI_VERSION_1_6) == JNI_EDETACHED) {
        sVM->AttachCurrentThread(jenv, nullptr);
    }
}

void VROPlatformSetEnv(JNIEnv *env, jobject appContext, jobject assetManager, jobject platformUtil) {
    env->GetJavaVM(&sVM);
    sJavaAppContext = env->NewGlobalRef(appContext);
    sJavaAssetMgr = env->NewGlobalRef(assetManager);
    sPlatformUtil = env->NewGlobalRef(platformUtil);
    sAssetMgr = AAssetManager_fromJava(env, assetManager);

    // Now that we've properly setup VROPlatformUtil, flush the task queues.
    VROPlatformFlushTaskQueues();
}

void VROPlatformSetEnv(JNIEnv *env) {
    // If the VM was already set, then don't reset it.
    if (!sVM) {
        env->GetJavaVM(&sVM);
    }
}

JNIEnv *VROPlatformGetJNIEnv() {
    JNIEnv *env;
    getJNIEnv(&env);

    return env;
}

jobject VROPlatformGetJavaAppContext() {
    return sJavaAppContext;
}

jobject VROPlatformGetJavaAssetManager() {
    return sJavaAssetMgr;
}

AAssetManager *VROPlatformGetAssetManager() {
    return sAssetMgr;
}

void VROPlatformReleaseEnv() {
    JNIEnv *env;
    getJNIEnv(&env);

    env->DeleteGlobalRef(sJavaAssetMgr);
    env->DeleteGlobalRef(sPlatformUtil);

    sJavaAssetMgr = nullptr;
    sPlatformUtil = nullptr;
    sAssetMgr = nullptr;
}

std::string VROPlatformLoadResourceAsString(std::string resource, std::string type) {
    std::string assetName = resource + "." + type;

    AAsset *asset = AAssetManager_open(sAssetMgr, assetName.c_str(), AASSET_MODE_BUFFER);
    passert_msg(asset != nullptr , "Failed to load resource %s.%s as string", resource.c_str(), type.c_str());
    size_t length = AAsset_getLength(asset);
    
    char *buffer = (char *)malloc(length + 1);
    AAsset_read(asset, buffer, length);
    buffer[length] = 0;
    
    std::string str(buffer);
    AAsset_close(asset);
    free(buffer);

    return str;
}

std::string VROPlatformDownloadURLToFile(std::string url, bool *temp, bool *success) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    VRO_STRING jurl = VRO_NEW_STRING(url.c_str());
    
    jclass cls = env->FindClass("com/viro/core/internal/PlatformUtil");
    jmethodID jmethod = env->GetMethodID(cls, "downloadURLToTempFile", "(Ljava/lang/String;)Ljava/lang/String;");
    VRO_STRING jpath = (VRO_STRING) env->CallObjectMethod(sPlatformUtil, jmethod, jurl);
    
    env->DeleteLocalRef(jurl);
    env->DeleteLocalRef(cls);
    
    if (jpath == nullptr) {
        *success = false;
        return "";
    }
    else {
        *success = true;
    }

    std::string spath = VROPlatformGetString(jpath, env);
    pinfo("Downloaded URL [%s] to file [%s]", url.c_str(), spath.c_str());
    env->DeleteLocalRef(jpath);
    
    *temp = true;
    return spath;
}

void VROPlatformDownloadURLToFileAsync(std::string url,
                                       std::function<void(std::string, bool)> onSuccess,
                                       std::function<void()> onFailure) {
    VROPlatformDispatchAsyncBackground([url, onSuccess, onFailure] {
        bool temp, success;
        std::string file = VROPlatformDownloadURLToFile(url, &temp, &success);
        if (success) {
            VROPlatformDispatchAsyncRenderer([file, onSuccess] {
                onSuccess(file, true);
            });
        }
        else {
            VROPlatformDispatchAsyncRenderer([onFailure] {
                onFailure();
            });
        }
    });
}

void VROPlatformDeleteFile(std::string filename) {
    // As SoundData finalizers in Java can get called after the renderer is destroyed,
    // we perform a null check here. TODO VIRO-2441: Perform Proper sound data cleanup.
    if (sPlatformUtil == NULL) {
        return;
    }

    JNIEnv *env = VROPlatformGetJNIEnv();
    VRO_STRING jfilename = VRO_NEW_STRING(filename.c_str());

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "deleteFile", "(Ljava/lang/String;)V");
    env->CallVoidMethod(sPlatformUtil, jmethod, jfilename);

    env->DeleteLocalRef(jfilename);
    env->DeleteLocalRef(cls);
}

std::string VROPlatformCopyResourceToFile(std::string asset, bool *isTemp) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    VRO_STRING jasset = VRO_NEW_STRING(asset.c_str());

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "copyResourceToFile", "(Ljava/lang/String;)Ljava/lang/String;");
    VRO_STRING jpath = (VRO_STRING) env->CallObjectMethod(sPlatformUtil, jmethod, jasset);

    std::string spath = VROPlatformGetString(jpath, env);
    pinfo("Copied resource %s to [%s]", asset.c_str(), spath.c_str());

    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(jasset);
    env->DeleteLocalRef(cls);

    *isTemp = true;
    return spath;
}

std::map<std::string, std::string> VROPlatformCopyObjResourcesToFile(jobject resourceMap) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "copyResourceMap", "(Ljava/util/Map;)Ljava/util/Map;");
    jobject jresourcemap = (jobject) env->CallObjectMethod(sPlatformUtil, jmethod, resourceMap);

    env->DeleteLocalRef(cls);

    return VROPlatformConvertFromJavaMap(jresourcemap);
}

std::map<std::string, std::string> VROPlatformConvertFromJavaMap(jobject javaMap) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    // get keyset
    jclass mapClass = env->GetObjectClass(javaMap); // should be a Map (hashmap, etc)
    jmethodID keySetMethod = env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;");
    jobject keySet = (jobject) env->CallObjectMethod(javaMap, keySetMethod);
    // get keysetLength
    jclass setClass = env->GetObjectClass(keySet); // should be a Set
    jmethodID sizeMethod = env->GetMethodID(setClass, "size", "()I");
    VRO_INT setSize = (VRO_INT) env->CallIntMethod(keySet, sizeMethod);
    // get keyset array
    jmethodID toArrayMethod = env->GetMethodID(setClass, "toArray", "()[Ljava/lang/Object;");
    jobjectArray keyArray = (jobjectArray) env->CallObjectMethod(keySet, toArrayMethod);

    // create map to return.
    std::map<std::string, std::string> toReturn;

    // for int i, i < keyset length
    for (int i = 0; i < setSize; i++) {
        // get individual value for key
        // get the key
        VRO_STRING key = (VRO_STRING) env->GetObjectArrayElement(keyArray, i);
        // convert to std::string
        std::string strKey = VROPlatformGetString(key, env);
        // get the value from the Map
        jmethodID mapGetMethod = env->GetMethodID(mapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        VRO_STRING value = (VRO_STRING) env->CallObjectMethod(javaMap, mapGetMethod, key);
        // convert to std::string
        std::string strValue = VROPlatformGetString(value, env);
        // add to the map
        toReturn.insert(std::make_pair(strKey.c_str(), strValue.c_str()));
    }
    return toReturn;
}

/**
 * This is a helper function that takes a map of resources (from Android Release builds) that map
 * the resource name (ie. js_res_male02) to their downloaded locations (ie. /data/.../cache/js_res_male02)
 * and a "key" which is matches the suffix of one of the keys in the map minus the extension (ie. maleO2.obj).
 *
 * @param key - ie. male02.mtl
 * @param resourceMap - ie {js_res_male02 : /data/.../cache/js_res_male02}
 */
std::string VROPlatformFindValueInResourceMap(std::string key, std::map<std::string, std::string> resourceMap) {
    // The suffix of a key in the map is the given key minus the extension
    std::string keySuffix = key.substr(0, key.find_last_of('.'));
    // transform the string to lower case.
    VROStringUtil::toLowerCase(keySuffix);

    // remove any hyphens because android resources removes them.
    // TODO: add any other characters that android resources remove...
    keySuffix.erase(std::remove(keySuffix.begin(), keySuffix.end(), '-'), keySuffix.end());

    // go through every key and if one ends with the suffix, then return that value.
    std::map<std::string, std::string>::iterator it;
    for (it = resourceMap.begin(); it != resourceMap.end(); it++) {
        if (VROStringUtil::endsWith(it->first, keySuffix)) {
            return it->second;
        }
    }
    return ""; // if we can't find the given key, return the empty string.
}

std::string VROPlatformCopyAssetToFile(std::string asset) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    VRO_STRING jasset = VRO_NEW_STRING(asset.c_str());

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "copyAssetToFile", "(Ljava/lang/String;)Ljava/lang/String;");
    VRO_STRING jpath = (VRO_STRING) env->CallObjectMethod(sPlatformUtil, jmethod, jasset);

    std::string spath = VROPlatformGetString(jpath, env);
    pinfo("Copied asset %s to [%s]", asset.c_str(), spath.c_str());

    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(jasset);
    env->DeleteLocalRef(cls);

    return spath;
}

std::pair<std::string, int> VROPlatformFindFont(std::string typeface, bool isItalic, int weight) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    VRO_STRING jtypeface = VRO_NEW_STRING(typeface.c_str());
    
    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jfindFontFile = env->GetMethodID(cls, "findFontFile", "(Ljava/lang/String;ZI)Ljava/lang/String;");
    jmethodID jfindFontIndex = env->GetMethodID(cls, "findFontIndex", "(Ljava/lang/String;ZI)I");

    VRO_STRING jpath = (VRO_STRING) env->CallObjectMethod(sPlatformUtil, jfindFontFile, jtypeface, isItalic, weight);

    std::string path;
    int index = -1;

    if (jpath != NULL) {
        path = VROPlatformGetString(jpath, env);
        index = env->CallIntMethod(sPlatformUtil, jfindFontIndex, jtypeface, isItalic, weight);
        env->DeleteLocalRef(jpath);
    }
    env->DeleteLocalRef(jtypeface);
    env->DeleteLocalRef(cls);

    return std::make_pair(path, index);
}

std::shared_ptr<VROImage> VROPlatformLoadImageFromFile(std::string filename,
                                                       VROTextureInternalFormat format) {
    jobject bitmap = VROPlatformLoadBitmapFromFile(filename, format);
    if (bitmap == nullptr) {
        return {};
    }

    return std::make_shared<VROImageAndroid>(bitmap, format);
}

std::shared_ptr<VROImage> VROPlatformLoadImageFromAsset(std::string asset,
                                                        VROTextureInternalFormat format) {
    jobject bitmap = VROPlatformLoadBitmapFromAsset(asset, format);
    if (bitmap == nullptr) {
        return {};
    }
    return std::make_shared<VROImageAndroid>(bitmap, format);
}

jobject VROPlatformLoadBitmapFromAsset(std::string asset, VROTextureInternalFormat format) {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "loadBitmapFromAsset", "(Ljava/lang/String;Z)Landroid/graphics/Bitmap;");

    VRO_STRING string = VRO_NEW_STRING(asset.c_str());
    jobject jbitmap = env->CallObjectMethod(sPlatformUtil, jmethod, string,
                                            format == VROTextureInternalFormat::RGB565);

    env->DeleteLocalRef(string);
    env->DeleteLocalRef(cls);
    return jbitmap;
}

jobject VROPlatformLoadBitmapFromFile(std::string path, VROTextureInternalFormat format) {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "loadBitmapFromFile", "(Ljava/lang/String;Z)Landroid/graphics/Bitmap;");

    VRO_STRING string = VRO_NEW_STRING(path.c_str());
    jobject jbitmap = env->CallObjectMethod(sPlatformUtil, jmethod, string,
                                            format == VROTextureInternalFormat::RGB565);

    env->DeleteLocalRef(string);
    env->DeleteLocalRef(cls);
    return jbitmap;
}

VROTextureFormat VROPlatformGetBitmapFormat(jobject jbitmap) {
    JNIEnv *env;
    getJNIEnv(&env);

    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, jbitmap, &bitmapInfo);

    // TODO: add more format maps, the two below are the common ones.
    switch(bitmapInfo.format) {
        case ANDROID_BITMAP_FORMAT_RGB_565:
            return VROTextureFormat::RGB565;
        case ANDROID_BITMAP_FORMAT_RGBA_8888:
        default:
            return VROTextureFormat::RGB8;
    }
}

void *VROPlatformConvertBitmap(jobject jbitmap, int *bitmapLength, int *width, int *height, bool *hasAlpha) {
    JNIEnv *env;
    getJNIEnv(&env);

    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, jbitmap, &bitmapInfo);

    *width = bitmapInfo.width;
    *height = bitmapInfo.height;
    *bitmapLength = bitmapInfo.height * bitmapInfo.stride;

    jclass cls = env->GetObjectClass(jbitmap);
    jmethodID jbitmapHasAlpha = env->GetMethodID(cls, "hasAlpha", "()Z");
    *hasAlpha = env->CallBooleanMethod(jbitmap, jbitmapHasAlpha, jbitmap);

    void *bitmapData;
    AndroidBitmap_lockPixels(env, jbitmap, &bitmapData);

    void *safeData = malloc(*bitmapLength);
    memcpy(safeData, bitmapData, *bitmapLength);

    AndroidBitmap_unlockPixels(env, jbitmap);

    env->DeleteLocalRef(jbitmap);
    return safeData;
}

void VROPlatformSaveRGBAImage(void *data, int length, int width, int height, std::string filename) {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sPlatformUtil);

    jobject jbuffer = env->NewDirectByteBuffer(data, length);
    VRO_STRING jpath = VRO_NEW_STRING(filename.c_str());
    jmethodID jsaveRGBAImageToFile = env->GetMethodID(cls, "saveRGBAImageToFile", "(Ljava/nio/ByteBuffer;IILjava/lang/String;)V");

    env->CallVoidMethod(sPlatformUtil, jsaveRGBAImageToFile, jbuffer, width, height, jpath);
    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(jbuffer);
}

jobject VROPlatformCreateVideoSink(int textureId) {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "createVideoSink", "(I)Landroid/view/Surface;");
    jobject jsurface = env->CallObjectMethod(sPlatformUtil, jmethod, textureId);

    env->DeleteLocalRef(cls);
    return jsurface;
}

void VROPlatformDestroyVideoSink(int textureId) {
    // As Video finalizers in Java can get called after the renderer is destroyed,
    // we perform a null check here to prevent video sinks from getting cleaned up twice.
    if (sPlatformUtil == NULL){
        return;
    }

    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "destroyVideoSink", "(I)V");
    env->CallVoidMethod(sPlatformUtil, jmethod, textureId);

    env->DeleteLocalRef(cls);
}

int VROPlatformGetAudioSampleRate() {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "getAudioSampleRate", "()I");
    VRO_INT sampleRate = env->CallIntMethod(sPlatformUtil, jmethod);

    env->DeleteLocalRef(cls);
    return sampleRate;
}

int VROPlatformGetAudioBufferSize() {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->GetObjectClass(sPlatformUtil);
    jmethodID jmethod = env->GetMethodID(cls, "getAudioBufferSize", "()I");
    VRO_INT bufferSize = env->CallIntMethod(sPlatformUtil, jmethod);

    env->DeleteLocalRef(cls);
    return bufferSize;
}

int VROPlatformGenerateTask(std::function<void()> fcn) {
    std::lock_guard<std::mutex> lock(sTaskMapMutex);

    int taskId = ++sTaskIdGenerator;
    sTaskMap[taskId] = fcn;

    return taskId;
}

void VROPlatformRunTask(int taskId) {
    std::function<void()> fcn;
    {
        std::lock_guard<std::mutex> lock(sTaskMapMutex);
        auto it = sTaskMap.find(taskId);
        if (it != sTaskMap.end()) {
            fcn = it->second;
            sTaskMap.erase(it);
        }
    }

    if (fcn) {
        fcn();
    }
}

void VROPlatformDispatchAsyncBackground(std::function<void()> fcn) {
    int task = VROPlatformGenerateTask(fcn);

    if (!sPlatformUtil) {
        sBackgroundQueue.push_back(task);
        return;
    }

    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->FindClass("com/viro/core/internal/PlatformUtil");
    jmethodID jmethod = env->GetMethodID(cls, "dispatchAsyncBackground", "(I)V");
    env->CallVoidMethod(sPlatformUtil, jmethod, task);

    env->DeleteLocalRef(cls);
}

void VROPlatformDispatchAsyncRenderer(std::function<void()> fcn) {
    int task = VROPlatformGenerateTask(fcn);
    if (!sPlatformUtil) {
        sRendererQueue.push_back(task);
        return;
    }
    VROPlatformCallJavaFunction(sPlatformUtil,
                                "dispatchRenderer", "(I)V", task);
}

void VROPlatformDispatchAsyncApplication(std::function<void()> fcn){
    int task = VROPlatformGenerateTask(fcn);
    if (!sPlatformUtil) {
        sAsyncQueue.push_back(task);
        return;
    }
    VROPlatformCallJavaFunction(sPlatformUtil,
                                "dispatchApplication", "(I)V", task);
}

void VROPlatformFlushTaskQueues() {
    for (int task : sBackgroundQueue) {
        JNIEnv *env;
        getJNIEnv(&env);

        jclass cls = env->FindClass("com/viro/core/internal/PlatformUtil");
        jmethodID jmethod = env->GetMethodID(cls, "dispatchAsyncBackground", "(I)V");
        env->CallVoidMethod(sPlatformUtil, jmethod, task);

        env->DeleteLocalRef(cls);
    }
    sBackgroundQueue.clear();

    for (int task : sRendererQueue) {
        VROPlatformCallJavaFunction(sPlatformUtil,
                                    "dispatchRenderer", "(I)V", task);
    }
    sRendererQueue.clear();

    for (int task : sAsyncQueue) {
        VROPlatformCallJavaFunction(sPlatformUtil,
                                    "dispatchApplication", "(I)V", task);
    }
    sAsyncQueue.clear();
}

jobject VROPlatformGetClassLoader(JNIEnv *jni, jobject jcontext) {
    jclass contextClass = jni->GetObjectClass(jcontext);
    jclass contextClassClass = jni->GetObjectClass(contextClass);

    jmethodID method = jni->GetMethodID(contextClassClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject classLoader = jni->CallObjectMethod(contextClass, method);

    jni->DeleteLocalRef(contextClass);
    jni->DeleteLocalRef(contextClassClass);

    if (classLoader == 0) {
        perr("Failed to get class loader from activity context");
    }
    return classLoader;
}

jclass VROPlatformFindClass(JNIEnv *jni, jobject javaObject, const char *className) {
    jobject classLoader = VROPlatformGetClassLoader(jni, javaObject);
    jclass classLoaderClass = jni->FindClass("java/lang/ClassLoader");
    jmethodID loadClassMethodId = jni->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;" );

    VRO_STRING jclassName = jni->NewStringUTF(className);
    jclass cls = static_cast<jclass>(jni->CallObjectMethod(classLoader, loadClassMethodId, jclassName));
    if (cls == 0) {
        perr("Failed to locate class %s using activity class loader", className);
    }

    jni->DeleteLocalRef(classLoaderClass);
    jni->DeleteLocalRef(classLoader);
    jni->DeleteLocalRef(jclassName);

    return cls;
}

void VROPlatformCallJavaFunction(jobject javaObject,
                                 std::string functionName,
                                 std::string methodID, ...) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();

    jclass viroClass = env->GetObjectClass(javaObject);
    if (viroClass == nullptr) {
        perr("Unable to find class for making java calls [function %s, method %s]",
             functionName.c_str(), methodID.c_str());
        return;
    }
    
    jmethodID method = env->GetMethodID(viroClass, functionName.c_str(), methodID.c_str());
    if (method == nullptr) {
        perr("Unable to find method %s callback.", functionName.c_str());
        return;
    }
    
    va_list args;
    va_start(args, methodID);
    env->CallVoidMethodV(javaObject, method, args);
    if (env->ExceptionOccurred()) {
        perr("Exception occurred when calling %s.", functionName.c_str());
        env->ExceptionDescribe();
        std::string errorString = "A java exception has been thrown when calling " + functionName;
        throw std::runtime_error(errorString.c_str());
    }
    va_end(args);
    
    env->DeleteLocalRef(viroClass);
}

 jlong VROPlatformCallJavaLongFunction(jobject javaObject,
                                       std::string functionName,
                                       std::string methodID, ...) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();

    jclass viroClass = env->GetObjectClass(javaObject);
    if (viroClass == nullptr) {
        perr("Unable to find class for making java calls [function %s, method %s]",
             functionName.c_str(), methodID.c_str());
        return 0;
    }

    jmethodID method = env->GetMethodID(viroClass, functionName.c_str(), methodID.c_str());
    if (method == nullptr) {
        perr("Unable to find method %s callback.", functionName.c_str());
        return 0;
    }

    va_list args;
    va_start(args, methodID);
    jlong result = env->CallLongMethodV(javaObject, method, args);
    if (env->ExceptionOccurred()) {
        perr("Exception occurred when calling %s.", functionName.c_str());
        env->ExceptionDescribe();
        std::string errorString = "A java exception has been thrown when calling " + functionName;
        throw std::runtime_error(errorString.c_str());
    }
    va_end(args);

    env->DeleteLocalRef(viroClass);
    return result;
}

VRO_OBJECT VROPlatformConstructHostObject(std::string className,
                                          std::string constructorSig, ...) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->ExceptionClear();

    jclass cls = env->FindClass(className.c_str());
    jmethodID constructor = env->GetMethodID(cls, "<init>", constructorSig.c_str());

    va_list args;
    va_start(args, constructorSig);
    jobject object = env->NewObjectV(cls, constructor, args);
    if (env->ExceptionOccurred()) {
        perr("Exception occurred when calling constructor %s", constructorSig.c_str());
        env->ExceptionDescribe();

        std::string errorString = "A java exception has been thrown when calling constructor " + constructorSig;
        throw std::runtime_error(errorString.c_str());
    }
    va_end(args);

    env->DeleteLocalRef(cls);
    return object;
}

void VROPlatformSetBool(JNIEnv *env, jobject jObj, const char *fieldName, jboolean value) {
    jfieldID fieldId = env->GetFieldID(env->GetObjectClass(jObj), fieldName, "Z");
    if (fieldId == NULL) {
        pwarn("Attempted to set undefined field: %s", fieldName);
        return;
    }

    env->SetBooleanField(jObj, fieldId, value);
}

void VROPlatformSetInt(JNIEnv *env, jobject jObj, const char *fieldName, VRO_INT value) {
    jfieldID fieldId = env->GetFieldID(env->GetObjectClass(jObj), fieldName, "I");
    if (fieldId == NULL) {
        pwarn("Attempted to set undefined field: %s", fieldName);
        return;
    }

    env->SetIntField(jObj, fieldId, value);
}

void VROPlatformSetFloat(JNIEnv *env, jobject jObj, const char *fieldName, VRO_FLOAT value) {
    jfieldID fieldId = env->GetFieldID(env->GetObjectClass(jObj), fieldName, "F");
    if (fieldId == NULL){
        pwarn("Attempted to set undefined field: %s", fieldName);
        return;
    }

    env->SetFloatField(jObj, fieldId, value);
}

void VROPlatformSetString(JNIEnv *env, jobject jObj, const char *fieldName, std::string value) {
    jfieldID fieldId = env->GetFieldID(env->GetObjectClass(jObj), fieldName, "Ljava/lang/String;");
    if (fieldId == NULL){
        pwarn("Attempted to set undefined field: %s", fieldName);
        return;
    }

    VRO_STRING jValue = VRO_NEW_STRING(value.c_str());
    env->SetObjectField(jObj, fieldId, jValue);
}

void VROPlatformSetEnumValue(JNIEnv *env, jobject jObj, const char *fieldName,
                             std::string enumClassPathName, std::string enumValueStr) {

    // Assume an enumClassPathName example of the form: com/viro/Material
    // Assume an enumClasspathType example of the form: Lcom/viro/Material;
    std::string enumClassPathType = ("L" + enumClassPathName) + ";";

    // Grab the jEnumValue for the given C++ Enum string and Enum class.
    jclass enumClass = env->FindClass(enumClassPathName.c_str());
    jfieldID enumValueField = env->GetStaticFieldID(enumClass , enumValueStr.c_str(), enumClassPathType.c_str());
    jobject jEnumValue = env->GetStaticObjectField(enumClass, enumValueField);

    // Get the corresponding enum field in jObjClass.java to be set on and set it.
    jfieldID jEnumField = env->GetFieldID(env->GetObjectClass(jObj), fieldName, enumClassPathType.c_str());
    env->SetObjectField(jObj, jEnumField, jEnumValue);
    env->DeleteLocalRef(enumClass);
    env->DeleteLocalRef(jEnumValue);
}

void VROPlatformSetObject(JNIEnv *env, jobject jObj, const char *fieldName,
                          const char *fieldType, VRO_OBJECT object) {
    jfieldID fieldId = env->GetFieldID(env->GetObjectClass(jObj), fieldName, fieldType);
    if (fieldId == NULL) {
        pwarn("Attempted to set undefined field: %s", fieldName);
        return;
    }

    env->SetObjectField(jObj, fieldId, object);
}

std::string VROPlatformGetString(VRO_STRING jInputString, JNIEnv *env){
    if (env == NULL){
        env = VROPlatformGetJNIEnv();
        env->ExceptionClear();
    }

    if (jInputString == NULL){
        return "";
    }

    const char * inputString_c = VRO_STRING_GET_CHARS(jInputString);
    std::string sInputString(inputString_c);
    VRO_STRING_RELEASE_CHARS(jInputString, inputString_c);

    return sInputString;
}

std::string VROPlatformGetDeviceModel() {
    JNIEnv *env;
    getJNIEnv(&env);
    jclass cls = env->FindClass("android/os/Build");
    jfieldID modelFieldID = env->GetStaticFieldID(cls, "MODEL", "Ljava/lang/String;");

    VRO_STRING jModelString = (VRO_STRING) env->GetStaticObjectField(cls, modelFieldID);

    return VROPlatformGetString(jModelString, env);
}

std::string VROPlatformGetDeviceBrand() {
    JNIEnv *env;
    getJNIEnv(&env);
    jclass cls = env->FindClass("android/os/Build");
    jfieldID brandFieldID = env->GetStaticFieldID(cls, "BRAND", "Ljava/lang/String;");

    VRO_STRING jBrandString = (VRO_STRING) env->GetStaticObjectField(cls, brandFieldID);

    return VROPlatformGetString(jBrandString, env);
}

std::string VROPlatformGetCacheDirectory() {
    JNIEnv *env;
    getJNIEnv(&env);

    jclass cls = env->FindClass("com/viro/core/internal/PlatformUtil");
    jmethodID jmethod = env->GetMethodID(cls, "getCacheDirectory", "()Ljava/lang/String;");
    VRO_STRING jpath = (VRO_STRING) env->CallObjectMethod(sPlatformUtil, jmethod);

    return VROPlatformGetString(jpath, env);
}

void VROPlatformSetTrackingImageView(std::string filepath) {

    VROPlatformDispatchAsyncApplication([filepath]() {
        JNIEnv *env;
        getJNIEnv(&env);

        jclass cls = env->FindClass("com/viro/core/ViroViewARCore");
        jmethodID jmethod = env->GetStaticMethodID(cls, "setImageOnTrackingImageView", "(Ljava/lang/String;)Z");

        VRO_STRING string = VRO_NEW_STRING(filepath.c_str());

        env->CallStaticBooleanMethod(cls, jmethod, string);

        env->DeleteLocalRef(cls);
    });
}

void Java_com_viro_core_internal_PlatformUtil_runTask(JNIEnv *env, jclass clazz, VRO_INT taskId) {
    VROPlatformRunTask(taskId);
}

#pragma mark - WebAssembly
#elif VRO_PLATFORM_WASM

#include "emscripten.h"
#include "VROImageWasm.h"

std::string VROPlatformRandomString(size_t length) {
    auto randchar = []() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

std::string VROPlatformLoadResourceAsString(std::string resource, std::string type) {
    std::string path = "/" + resource + "." + type;
    return VROPlatformLoadFileAsString(path);
}

class VROPlatformWGetContext {
public:
    std::function<void(std::string, bool)> onSuccess;
    std::function<void()> onFailure;
};

void VROPlatformWGetDownloadCallback(unsigned int x, void *arg, const char *file) {
    pinfo("Downloaded file [%s]", file);
    
    VROPlatformWGetContext *context = (VROPlatformWGetContext *) arg;
    context->onSuccess(std::string(file), true);
    delete (context);
}

void VROPlatformWGetErrorCallback(unsigned int x, void *arg, int error) {
    pinfo("Error executing wget [%d]", error);

    VROPlatformWGetContext *context = (VROPlatformWGetContext *) arg;
    context->onFailure();
    delete (context);
}

void VROPlatformWGetStatusCallback(unsigned int x, void *arg, int percentage) {
    pinfo("    Downloaded %d%%", percentage);
}

std::string VROPlatformDownloadURLToFile(std::string url, bool *temp, bool *success) {
    // Synchronous download not supported on WASM
    pabort();
}

void VROPlatformDownloadURLToFileAsync(std::string url,
                                       std::function<void(std::string, bool)> onSuccess,
                                       std::function<void()> onFailure) {
    VROPlatformWGetContext *context = new VROPlatformWGetContext();
    context->onSuccess = onSuccess;
    context->onFailure = onFailure;
    
    std::string prefix = "/" + VROPlatformLastPathComponent(url, "download");
    std::string tempFile = prefix + "_" + VROPlatformRandomString(8);
    emscripten_async_wget2(url.c_str(), tempFile.c_str(), "GET", "", context,
                           &VROPlatformWGetDownloadCallback, &VROPlatformWGetErrorCallback, &VROPlatformWGetStatusCallback);
    pinfo("Downloading URL [%s]", url.c_str());
}

std::string VROPlatformCopyResourceToFile(std::string asset, bool *isTemp) {
    // In WebAssembly, "resources" are preloaded files at the root of the virtual filesystem
    *isTemp = false;
    return "/" + asset;
}

void VROPlatformDeleteFile(std::string filename) {
    // Not supported on WASM
}

std::shared_ptr<VROImage> VROPlatformLoadImageFromFile(std::string filename,
                                                       VROTextureInternalFormat format) {
    return std::make_shared<VROImageWasm>(filename, format);
}

void VROPlatformDispatchAsyncRenderer(std::function<void()> fcn) {
    // Multithreading not supported on WASM
    fcn();
}

void VROPlatformDispatchAsyncBackground(std::function<void()> fcn) {
    // Multithreading not supported on WASM
    fcn();
}

std::string VROPlatformFindValueInResourceMap(std::string key, std::map<std::string, std::string> resourceMap) {
    return "";
}

VRO_ENV VROPlatformGetJNIEnv() {
    return nullptr;
}

void VROPlatformCallJavaFunction(VRO_OBJECT object,
                                 std::string functionName,
                                 std::string methodID, ...) {

}

VRO_LONG VROPlatformCallJavaLongFunction(VRO_OBJECT object,
                                         std::string functionName,
                                         std::string methodID, ...) {
    return 0;
}

VRO_OBJECT VROPlatformConstructHostObject(std::string className,
                                          std::string constructorSignature, ...) {

}

void VROPlatformSetFloat(VRO_ENV env, VRO_OBJECT obj, const char *fieldName, VRO_FLOAT value) {

}

void VROPlatformSetString(VRO_ENV env, VRO_OBJECT obj, const char *fieldName, std::string value) {

}

void VROPlatformSetInt(VRO_ENV env, VRO_OBJECT obj, const char *fieldName, VRO_INT value) {

}

void VROPlatformSetBool(VRO_ENV env, VRO_OBJECT obj, const char *fieldName, VRO_BOOL value) {

}

void VROPlatformSetEnumValue(VRO_ENV env, VRO_OBJECT obj, const char *fieldName,
                             std::string enumClassPathName, std::string enumValueStr) {

}

void VROPlatformSetObject(VRO_ENV env, VRO_OBJECT obj, const char *fieldName,
                          const char *fieldType, VRO_OBJECT object) {

}

std::string VROPlatformGetString(VRO_STRING string, VRO_ENV env) {
    return string;
}

#endif
#pragma mark - iOS and Android
#if VRO_PLATFORM_IOS || VRO_PLATFORM_ANDROID

#include "VROStringUtil.h"
#include "vr/gvr/capi/include/gvr_audio.h"

int VROPlatformParseGVRAudioMaterial(std::string property) {
    if (VROStringUtil::strcmpinsensitive(property, "transparent")) {
        return GVR_AUDIO_MATERIAL_TRANSPARENT;
    } else if (VROStringUtil::strcmpinsensitive(property, "acoustic_ceiling_tiles")) {
        return GVR_AUDIO_MATERIAL_ACOUSTIC_CEILING_TILES;
    } else if (VROStringUtil::strcmpinsensitive(property, "brick_bare")) {
        return GVR_AUDIO_MATERIAL_BRICK_BARE;
    } else if (VROStringUtil::strcmpinsensitive(property, "brick_painted")) {
        return GVR_AUDIO_MATERIAL_BRICK_PAINTED;
    } else if (VROStringUtil::strcmpinsensitive(property, "concrete_block_coarse")) {
        return GVR_AUDIO_MATERIAL_CONCRETE_BLOCK_COARSE;
    } else if (VROStringUtil::strcmpinsensitive(property, "concrete_block_painted")) {
        return GVR_AUDIO_MATERIAL_CONCRETE_BLOCK_PAINTED;
    } else if (VROStringUtil::strcmpinsensitive(property, "curtain_heavy")) {
        return GVR_AUDIO_MATERIAL_CURTAIN_HEAVY;
    } else if (VROStringUtil::strcmpinsensitive(property, "fiber_glass_insulation")) {
        return GVR_AUDIO_MATERIAL_FIBER_GLASS_INSULATION;
    } else if (VROStringUtil::strcmpinsensitive(property, "glass_thin")) {
        return GVR_AUDIO_MATERIAL_GLASS_THIN;
    } else if (VROStringUtil::strcmpinsensitive(property, "glass_thick")) {
        return GVR_AUDIO_MATERIAL_GLASS_THICK;
    } else if (VROStringUtil::strcmpinsensitive(property, "grass")) {
        return GVR_AUDIO_MATERIAL_GRASS;
    } else if (VROStringUtil::strcmpinsensitive(property, "linoleum_on_concrete")) {
        return GVR_AUDIO_MATERIAL_LINOLEUM_ON_CONCRETE;
    } else if (VROStringUtil::strcmpinsensitive(property, "marble")) {
        return GVR_AUDIO_MATERIAL_MARBLE;
    } else if (VROStringUtil::strcmpinsensitive(property, "metal")) {
        return GVR_AUDIO_MATERIAL_METAL;
    } else if (VROStringUtil::strcmpinsensitive(property, "parquet_on_concrete")) {
        return GVR_AUDIO_MATERIAL_PARQUET_ON_CONCRETE;
    } else if (VROStringUtil::strcmpinsensitive(property, "plaster_rough")) {
        return GVR_AUDIO_MATERIAL_PLASTER_ROUGH;
    } else if (VROStringUtil::strcmpinsensitive(property, "plaster_smooth")) {
        return GVR_AUDIO_MATERIAL_PLASTER_SMOOTH;
    } else if (VROStringUtil::strcmpinsensitive(property, "plywood_panel")) {
        return GVR_AUDIO_MATERIAL_PLYWOOD_PANEL;
    } else if (VROStringUtil::strcmpinsensitive(property, "polished_concrete_or_tile")) {
        return GVR_AUDIO_MATERIAL_POLISHED_CONCRETE_OR_TILE;
    } else if (VROStringUtil::strcmpinsensitive(property, "sheet_rock")) {
        return GVR_AUDIO_MATERIAL_SHEET_ROCK;
    } else if (VROStringUtil::strcmpinsensitive(property, "water_or_ice_surface")) {
        return GVR_AUDIO_MATERIAL_WATER_OR_ICE_SURFACE;
    } else if (VROStringUtil::strcmpinsensitive(property, "wood_ceiling")) {
        return GVR_AUDIO_MATERIAL_WOOD_CEILING;
    } else if (VROStringUtil::strcmpinsensitive(property, "wood_panel")) {
        return GVR_AUDIO_MATERIAL_WOOD_PANEL;
    }
    return GVR_AUDIO_MATERIAL_TRANSPARENT;
}

#endif
