//
//  VROAVPlayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <jni.h>
#include "VROAVPlayer.h"
#include "VROPlatformUtil.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_AVPlayer_##method_name

extern "C" {
    inline jlong jptr(VROAVPlayer *nativePlayer) {
        return reinterpret_cast<intptr_t>(nativePlayer);
    }

    inline VROAVPlayer *native(jlong ptr) {
        return reinterpret_cast<VROAVPlayer *>(ptr);
    }

    JNI_METHOD(void, nativeOnFinished)(JNIEnv *env,
                                            jclass clazz,
                                            jlong nativePlayerRef) {
        std::weak_ptr<VROAVPlayerDelegate> delegateWeak
                = native(nativePlayerRef)->getDelegate();
        if(auto tmp = delegateWeak.lock()){
            tmp->onFinished();
        }
    }

    JNI_METHOD(void, nativeOnPrepared)(JNIEnv *env,
                                       jclass clazz,
                                       jlong nativePlayerRef) {
        std::weak_ptr<VROAVPlayerDelegate> delegateWeak
                = native(nativePlayerRef)->getDelegate();
        if(auto tmp = delegateWeak.lock()){
            tmp->onPrepared();
        }
    }

    JNI_METHOD(void, nativeOnError)(JNIEnv *env,
                                    jclass clazz,
                                    jlong nativePlayerRef,
                                    jstring error) {
        std::weak_ptr<VROAVPlayerDelegate> delegateWeak
            = native(nativePlayerRef)->getDelegate();
        if(auto tmp = delegateWeak.lock()){
            const char *cError = env->GetStringUTFChars(error, NULL);
            std::string sError(cError);

            tmp->onError(sError);
            env->ReleaseStringUTFChars(error, cError);
        }
    }
}

static const char *AVPlayerClass = "com/viro/renderer/AVPlayer";
VROAVPlayer::VROAVPlayer() :
    _jsurface(nullptr),
    _textureId(0) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->FindClass(AVPlayerClass);
    jobject jcontext = VROPlatformGetJavaAppContext();
    jmethodID jmethod = env->GetMethodID(cls, "<init>", "(JLandroid/content/Context;)V");

    /**
     * Pass into AVPlayer a long address referencing it's corresponding
     * native object.
     */
    jlong myLongVal = jptr(this);
    jobject javPlayer = env->NewObject(cls, jmethod, myLongVal, jcontext);

    env->DeleteLocalRef(cls);
    _javPlayer = env->NewGlobalRef(javPlayer);
}

VROAVPlayer::~VROAVPlayer() {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "destroy", "()V");
    env->CallVoidMethod(_javPlayer, jmethod);
    env->DeleteLocalRef(cls);

    env->DeleteGlobalRef(_javPlayer);

    if (_jsurface) {
        env->DeleteGlobalRef(_jsurface);
    }
    if (_textureId) {
        VROPlatformDestroyVideoSink(_textureId);
    }
}

bool VROAVPlayer::setDataSourceURL(const char *resourceOrUrl) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jstring jstring = env->NewStringUTF(resourceOrUrl);
    jobject jcontext = VROPlatformGetJavaAppContext();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "setDataSourceURL", "(Ljava/lang/String;Landroid/content/Context;)Z");
    jboolean result = env->CallBooleanMethod(_javPlayer, jmethod, jstring, jcontext);

    env->DeleteLocalRef(jstring);
    env->DeleteLocalRef(cls);

    return result;
}

bool VROAVPlayer::setDataSourceAsset(const char *assetName) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jobject jassetManager = VROPlatformGetJavaAssetManager();

    jstring jstring = env->NewStringUTF(assetName);

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "setDataSourceAsset", "(Ljava/lang/String;Landroid/content/res/AssetManager;)Z");
    jboolean result = env->CallBooleanMethod(_javPlayer, jmethod, jstring, jassetManager);

    env->DeleteLocalRef(jstring);
    env->DeleteLocalRef(cls);

    return result;
}

void VROAVPlayer::setSurface(GLuint textureId) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    if (_jsurface) {
        env->DeleteGlobalRef(_jsurface);
    }
    if (_textureId) {
        VROPlatformDestroyVideoSink(_textureId);
    }

    _textureId = textureId;
    jobject jsurface = VROPlatformCreateVideoSink(textureId);
    _jsurface = env->NewGlobalRef(jsurface);

    bindVideoSink();
}

void VROAVPlayer::bindVideoSink() {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "setVideoSink", "(Landroid/view/Surface;)V");
    env->CallVoidMethod(_javPlayer, jmethod, _jsurface);

    env->DeleteLocalRef(cls);
}

void VROAVPlayer::pause() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "pause", "()V");
    env->CallVoidMethod(_javPlayer, jmethod);
    env->DeleteLocalRef(cls);
}

void VROAVPlayer::play() {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "play", "()V");
    env->CallVoidMethod(_javPlayer, jmethod);

    env->DeleteLocalRef(cls);
}

void VROAVPlayer::reset() {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "reset", "()V");
    env->CallVoidMethod(_javPlayer, jmethod);

    env->DeleteLocalRef(cls);
}

bool VROAVPlayer::isPaused() {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "isPaused", "()Z");
    bool paused = env->CallBooleanMethod(_javPlayer, jmethod);

    env->DeleteLocalRef(cls);
    return paused;
}

void VROAVPlayer::seekToTime(int seconds) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "seekToTime", "(I)V");
    env->CallVoidMethod(_javPlayer, jmethod, seconds);

    env->DeleteLocalRef(cls);
}

int VROAVPlayer::getCurrentTimeInSeconds() {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "getCurrentTimeInSeconds", "()I");
    jint seconds = env->CallIntMethod(_javPlayer, jmethod);

    env->DeleteLocalRef(cls);
    return seconds;
}

int VROAVPlayer::getVideoDurationInSeconds() {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "getVideoDurationInSeconds", "()I");
    jint seconds = env->CallIntMethod(_javPlayer, jmethod);

    env->DeleteLocalRef(cls);
    return seconds;
}

void VROAVPlayer::setMuted(bool muted) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "setMuted", "(Z)V");
    env->CallVoidMethod(_javPlayer, jmethod, muted);

    env->DeleteLocalRef(cls);
}

void VROAVPlayer::setVolume(float volume) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "setVolume", "(F)V");
    env->CallVoidMethod(_javPlayer, jmethod, volume);

    env->DeleteLocalRef(cls);
}

void VROAVPlayer::setLoop(bool loop) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "setLoop", "(Z)V");
    env->CallVoidMethod(_javPlayer, jmethod, loop);

    env->DeleteLocalRef(cls);
}

