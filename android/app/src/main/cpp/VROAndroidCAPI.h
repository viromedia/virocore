//
//  Created by Raj Advani on 5/2/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef ANDROID_VROANDROIDCAPI_H
#define ANDROID_VROANDROIDCAPI_H

#include <jni.h>
#include "PersistentRef.h"

#define VRO_ENV JNIEnv*
#define VRO_ARGS JNIEnv *env, jobject obj,
#define VRO_ARGS_STATIC JNIEnv *env, jclass clazz,
#define VRO_NO_ARGS JNIEnv *env, jobject obj
#define VRO_NO_ARGS_STATIC JNIEnv *env, jclass clazz
#define VRO_METHOD_PREAMBLE
#define VRO_BOOL jboolean
#define VRO_INT jint
#define VRO_LONG jlong
#define VRO_FLOAT jfloat
#define VRO_DOUBLE jdouble
#define VRO_CHAR_WIDE jchar

#define VRO_REF(type) jlong
#define VRO_REF_NEW(type, ptr) ({ \
    PersistentRef<type> *persistentRef = new PersistentRef<type>(ptr); \
    reinterpret_cast<jlong>(persistentRef); })
#define VRO_REF_GET(type, ref) ({ \
    PersistentRef<type> *persistentRef = reinterpret_cast<PersistentRef<type> *>(ref); \
    persistentRef->get(); })
#define VRO_REF_DELETE(type, ref) \
    delete reinterpret_cast<PersistentRef<type> *>(ref);
#define VRO_REF_NULL(ref) \
    ref == 0

#define VRO_OBJECT jobject
#define VRO_OBJECT_NULL NULL
#define VRO_IS_OBJECT_NULL(object) \
    (object == NULL)
#define VRO_WEAK jweak

#define VRO_STRING jstring
#define VRO_NEW_STRING(chars) \
    env->NewStringUTF(chars)
#define VRO_STRING_LENGTH(string) \
    env->GetStringLength(string)
#define VRO_STRING_GET_CHARS(str) \
    env->GetStringUTFChars(str, NULL)
#define VRO_STRING_RELEASE_CHARS(str, chars) \
    env->ReleaseStringUTFChars(str, chars)
#define VRO_IS_STRING_EMPTY(str) \
    (str == NULL || env->GetStringLength(str) == 0)
#define VRO_STRING_STL(str) ({ \
    std::string str_s = ""; \
    if (str != NULL) { \
        const char *str_c = VRO_STRING_GET_CHARS(str); \
        str_s = std::string(str_c); \
        VRO_STRING_RELEASE_CHARS(str, str_c); \
    } \
    str_s; })

#define VRO_STRING_WIDE jstring
#define VRO_IS_WIDE_STRING_EMPTY(str) \
    (str == NULL || env->GetStringLength(str) == 0)
#define VRO_STRING_GET_CHARS_WIDE(str, wide_str) \
    const jchar *text_c = env->GetStringChars(str, NULL); \
    jsize textLength = env->GetStringLength(str); \
    wide_str.assign(text_c, text_c + textLength); \
    env->ReleaseStringChars(str, text_c)

#define VRO_ARRAY(type) jobjectArray
#define VRO_ARRAY_LENGTH(array) \
    (int) env->GetArrayLength(array)
#define VRO_ARRAY_GET(array, index) \
    env->GetObjectArrayElement(array, index)
#define VRO_ARRAY_SET(array, index, object) \
    env->SetObjectArrayElement(array, index, object)
#define VRO_NEW_ARRAY(size, type, cls) \
    env->NewObjectArray(size, env->FindClass(cls), NULL)

#define VRO_OBJECT_ARRAY jobjectArray
#define VRO_OBJECT_ARRAY_GET(array, index) \
    env->GetObjectArrayElement(array, index)
#define VRO_OBJECT_ARRAY_SET(array, index, object) \
    env->SetObjectArrayElement(array, index, object)
#define VRO_NEW_OBJECT_ARRAY(size, cls) \
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
#define VRO_NEW_INT_ARRAY(size) \
    env->NewIntArray(size)
#define VRO_INT_ARRAY_SET(dest, start, len, src) \
    env->SetIntArrayRegion(dest, start, len, src)
#define VRO_INT_ARRAY_GET_ELEMENTS(array) \
    env->GetIntArrayElements(array, 0)
#define VRO_INT_ARRAY_RELEASE_ELEMENTS(array, elements) \
    env->ReleaseIntArrayElements(array, elements, 0)

#define VRO_REF_ARRAY(type) jlongArray
#define VRO_NEW_REF_ARRAY(size, type) \
    env->NewLongArray(size)
#define VRO_REF_ARRAY_SET(dest, start, len, src) \
    env->SetLongArrayRegion(dest, start, len, src)
#define VRO_REF_ARRAY_GET_ELEMENTS(array) \
    env->GetLongArrayElements(array, 0)
#define VRO_REF_ARRAY_RELEASE_ELEMENTS(array, elements) \
    env->ReleaseLongArrayElements(array, elements, 0)

#define VRO_STRING_ARRAY jobjectArray
#define VRO_NEW_STRING_ARRAY(size) \
    env->NewObjectArray(size, env->FindClass("java/lang/String"), env->NewStringUTF(""))
#define VRO_STRING_ARRAY_GET(array, index) \
    (VRO_STRING) (env->GetObjectArrayElement(array, index))
#define VRO_STRING_ARRAY_SET(array, index, item) \
    jstring jkey = env->NewStringUTF(item.c_str()); \
    env->SetObjectArrayElement(array, index, jkey); \
    env->DeleteLocalRef(jkey)

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

#endif //ANDROID_VROANDROIDCAPI_H
