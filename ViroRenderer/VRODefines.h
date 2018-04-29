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

#include <emscripten.h>
#include <emscripten/val.h>

#define VRO_C_INCLUDE <string>
#define VRO_ENV void*
#define VRO_ARGS ,
#define VRO_ARGS_STATIC ,
#define VRO_NO_ARGS
#define VRO_NO_ARGS_STATIC
#define VRO_REF int
#define VRO_BOOL bool
#define VRO_INT int
#define VRO_LONG uint64_t
#define VRO_FLOAT float
#define VRO_DOUBLE double
#define VRO_OBJECT void*
#define VRO_WEAK void*

#define VRO_STRING std::string
#define VRO_NEW_STRING(chars) \
    std::string(chars)
#define VRO_STRING_LENGTH(string) \
    string.size()
#define VRO_STRING_GET_CHARS(str) \
    str.c_str()
#define VRO_STRING_RELEASE_CHARS(str, chars) \

#define VRO_STRING_GET_CHARS_WIDE(str) \
    str.c_str()
#define VRO_STRING_RELEASE_CHARS_WIDE(str, chars) \
    

#define VRO_ARRAY std::vector<emscripten::val>
#define VRO_ARRAY_LENGTH(array) \
    array.size()
#define VRO_ARRAY_GET(array, index) \
    array[index]
#define VRO_ARRAY_SET(array, index, object) \
    array[index] = object
#define VRO_NEW_ARRAY(size, cls) \
    std::vector<emscripten::val>(size)

#define VRO_FLOAT_ARRAY std::vector<float>
#define VRO_NEW_FLOAT_ARRAY(size) \
    std::vector<float>(size)
#define VRO_FLOAT_ARRAY_SET(dest, start, len, src) \
    dest.insert(dest.start() + start, &src[0], &src[len]);
#define VRO_FLOAT_ARRAY_GET_ELEMENTS(array) \
    array.data()
#define VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(array, elements) \


#define VRO_DOUBLE_ARRAY std::vector<double>
#define VRO_NEW_DOUBLE_ARRAY(size) \
    std::vector<double>(size)
#define VRO_DOUBLE_ARRAY_SET(dest, start, len, src) \
    dest.insert(dest.start() + start, &src[0], &src[len]);
#define VRO_DOUBLE_ARRAY_GET_ELEMENTS(array) \
    array.data()
#define VRO_DOUBLE_ARRAY_RELEASE_ELEMENTS(array, elements) \


#define VRO_INT_ARRAY std::vector<int>
#define VRO_LONG_ARRAY std::vector<uint64_t>
#define VRO_NEW_LONG_ARRAY(size) \
    std::vector<uint64_t>(size)
#define VRO_LONG_ARRAY_SET(dest, start, len, src) \
    dest.insert(dest.start() + start, &src[0], &src[len]);
#define VRO_LONG_ARRAY_GET_ELEMENTS(array) \
    array.data()
#define VRO_LONG_ARRAY_RELEASE_ELEMENTS(array, elements) \


#define VRO_NEW_STRING_ARRAY(size) \
    std::vector<std::string>(size)
#define VRO_STRING_ARRAY_GET(array, index) \
    array[index]
#define VRO_STRING_ARRAY_SET(array, index, item) \
    array[index] = item

#define VRO_NEW_GLOBAL_REF(object) \
    object
#define VRO_NEW_LOCAL_REF(object) \
    object
#define VRO_NEW_WEAK_GLOBAL_REF(object) \
    object

#define VRO_DELETE_GLOBAL_REF(object) \

#define VRO_DELETE_LOCAL_REF(object) \

#define VRO_DELETE_WEAK_GLOBAL_REF(object) \


#define VRO_BUFFER_GET_CAPACITY(buffer) \
    0
#define VRO_BUFFER_GET_ADDRESS(buffer) \
    0

#define VRO_METHOD(return_type, method_name) \
    return_type method_name

#else  // !WASM_PLATFORM
#define VRO_PLATFORM_ANDROID 1
#define VRO_PLATFORM_IOS 0
#define VRO_PLATFORM_WASM 0
#define VRO_PLATFORM_MACOS 0

#define VRO_C_INCLUDE <jni.h>
#define VRO_ENV JNIEnv*
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
#define VRO_OBJECT jobject
#define VRO_WEAK jweak

#define VRO_STRING jstring
#define VRO_NEW_STRING(chars) \
    env->NewStringUTF(chars);
#define VRO_STRING_LENGTH(string) \
    env->GetStringLength(string)
#define VRO_STRING_GET_CHARS(str) \
    env->GetStringUTFChars(str, NULL)
#define VRO_STRING_RELEASE_CHARS(str, chars) \
    env->ReleaseStringUTFChars(str, chars)
#define VRO_STRING_GET_CHARS_WIDE(str) \
    env->GetStringChars(str, NULL)
#define VRO_STRING_RELEASE_CHARS_WIDE(str, chars) \
    env->ReleaseStringChars(str, chars)

#define VRO_ARRAY jobjectArray
#define VRO_ARRAY_LENGTH(array) \
    env->GetArrayLength(array)
#define VRO_ARRAY_GET(array, index) \
    env->GetObjectArrayElement(array, index)
#define VRO_ARRAY_SET(array, index, object) \
    env->SetObjectArrayElement(array, index, object)
#define VRO_NEW_ARRAY(size, cls) \
    env->NewObjectArray(size, env->FindClass(cls), NULL)

#define VRO_FLOAT_ARRAY jfloatArray
#define VRO_NEW_FLOAT_ARRAY(size) \
    env->NewFloatArray(size)
#define VRO_FLOAT_ARRAY_SET(dest, start, len, src) \
    env->SetFloatArrayRegion(dest, start, len, src)
#define VRO_FLOAT_ARRAY_GET_ELEMENTS(array) \
    env->GetFloatArrayElements(array, 0)
#define VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(array, elements) \
    env->ReleaseFloatArrayElements(array, elements, 0)

#define VRO_DOUBLE_ARRAY jdoubleArray
#define VRO_NEW_DOUBLE_ARRAY(size) \
    env->NewDoubleArray(size)
#define VRO_DOUBLE_ARRAY_SET(dest, start, len, src) \
    env->SetDoubleArrayRegion(dest, start, len, src)
#define VRO_DOUBLE_ARRAY_GET_ELEMENTS(array) \
    env->GetDoubleArrayElements(array, 0)
#define VRO_DOUBLE_ARRAY_RELEASE_ELEMENTS(array, elements) \
    env->ReleaseDoubleArrayElements(array, elements, 0)

#define VRO_INT_ARRAY jintArray
#define VRO_LONG_ARRAY jlongArray
#define VRO_NEW_LONG_ARRAY(size) \
    env->NewLongArray(size)
#define VRO_LONG_ARRAY_SET(dest, start, len, src) \
    env->SetLongArrayRegion(dest, start, len, src)
#define VRO_LONG_ARRAY_GET_ELEMENTS(array) \
    env->GetLongArrayElements(array, 0)
#define VRO_LONG_ARRAY_RELEASE_ELEMENTS(array, elements) \
    env->ReleaseLongArrayElements(array, elements, 0)

#define VRO_NEW_STRING_ARRAY(size) \
    env->NewObjectArray(size, env->FindClass("java/lang/String"), env->NewStringUTF(""));
#define VRO_STRING_ARRAY_GET(array, index) \
    (VRO_STRING) (env->GetObjectArrayElement(array, index));
#define VRO_STRING_ARRAY_SET(array, index, item) \
    jstring jkey = env->NewStringUTF(item.c_str()); \
    env->SetObjectArrayElement(array, index, jkey); \
    env->DeleteLocalRef(jkey);

#define VRO_NEW_GLOBAL_REF(object) \
    env->NewGlobalRef(object)
#define VRO_NEW_LOCAL_REF(object) \
    env->NewLocalRef(object)
#define VRO_NEW_WEAK_GLOBAL_REF(object) \
    env->NewWeakGlobalRef(object)

#define VRO_DELETE_GLOBAL_REF(object) \
    env->DeleteGlobalRef(object)
#define VRO_DELETE_LOCAL_REF(object) \
    env->DeleteLocalRef(object)
#define VRO_DELETE_WEAK_GLOBAL_REF(object) \
    env->DeleteWeakGlobalRef(object)

#define VRO_BUFFER_GET_CAPACITY(buffer) \
    env->GetDirectBufferCapacity(buffer)
#define VRO_BUFFER_GET_ADDRESS(buffer) \
    env->GetDirectBufferAddress(buffer)

#endif
#endif // !__OBJC __

#define VRO_METAL 0

#endif /* VRODefines_h */
