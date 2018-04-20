//
//  VRODefines.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/1/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODefines_h
#define VRODefines_h

#ifdef __OBJC__
#import "TargetConditionals.h" 
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#define VRO_PLATFORM_ANDROID 0
#define VRO_PLATFORM_IOS 1
#define VRO_PLATFORM_WASM 0
#define VRO_PLATFORM_MACOS 0
#else
#define VRO_PLATFORM_ANDROID 0
#define VRO_PLATFORM_IOS 0
#define VRO_PLATFORM_WASM 0
#define VRO_PLATFORM_MACOS 1
#endif // __OBJC__
#else
#ifdef WASM_PLATFORM
#define VRO_PLATFORM_ANDROID 0
#define VRO_PLATFORM_IOS 0
#define VRO_PLATFORM_WASM 1
#define VRO_PLATFORM_MACOS 0
#else  // !WASM_PLATFORM
#define VRO_PLATFORM_ANDROID 1
#define VRO_PLATFORM_IOS 0
#define VRO_PLATFORM_WASM 0
#define VRO_PLATFORM_MACOS 0

#define VRO_C_INCLUDE <jni.h>
#define VRO_ARGS JNIEnv *env, jobject obj,
#define VRO_ARGS_STATIC JNIEnv *env, jclass clazz,
#define VRO_NO_ARGS JNIEnv *env, jobject obj
#define VRO_NO_ARGS_STATIC JNIEnv *env, jclass clazz
#define VRO_REF jlong
#define VRO_BOOL jboolean
#define VRO_INT jint
#define VRO_LONG jlong
#define VRO_FLOAT jfloat
#define VRO_DOUBLE jdouble
#define VRO_STRING jstring
#define VRO_ARRAY jobjectArray
#define VRO_FLOAT_ARRAY jfloatArray
#define VRO_INT_ARRAY jintArray

#define VRO_ARRAY_LENGTH(array) \
    env->GetArrayLength(array);

#define VRO_NEW_FLOAT_ARRAY(size) \
    env->NewFloatArray(size);
#define VRO_FLOAT_ARRAY_SET(dest, start, len, src) \
    env->SetFloatArrayRegion(dest, start, len, src);

#define VRO_NEW_STRING_ARRAY(size) \
    env->NewObjectArray(size, env->FindClass("java/lang/String"), env->NewStringUTF(""));
#define VRO_STRING_ARRAY_GET(array, index) \
    (jstring) (env->GetObjectArrayElement(array, index));

#define VRO_STRING_ARRAY_SET(array, index, item) \
    jstring jkey = env->NewStringUTF(item.c_str()); \
    env->SetObjectArrayElement(array, index, jkey); \
    env->DeleteLocalRef(jkey); \

#endif
#endif // !__OBJC __

#define VRO_METAL 0

#endif /* VRODefines_h */
