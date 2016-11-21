//
//  VROAVPlayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAVPlayer.h"
#include "VROPlatformUtil.h"

static const char *AVPlayerClass = "com/viro/renderer/AVPlayer";

inline jlong jptr(VROAVPlayer *nativePlayer) {
    return reinterpret_cast<intptr_t>(nativePlayer);
}

inline VROAVPlayer *native(jlong ptr) {
    return reinterpret_cast<VROAVPlayer *>(ptr);
}

VROAVPlayer::VROAVPlayer() :
    _jsurface(nullptr) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->FindClass(AVPlayerClass);
    jmethodID jmethod = env->GetMethodID(cls, "<init>", "()V");
    jobject javPlayer = env->NewObject(cls, jmethod);

    env->DeleteLocalRef(cls);
    _javPlayer = env->NewGlobalRef(javPlayer);
}

VROAVPlayer::~VROAVPlayer() {
    JNIEnv *env = VROPlatformGetJNIEnv();
    env->DeleteGlobalRef(_javPlayer);

    if (_jsurface) {
        env->DeleteGlobalRef(_jsurface);
    }
}

bool VROAVPlayer::setDataSourceURL(const char *fileOrURL) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jstring jstring = env->NewStringUTF(fileOrURL);

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "setDataSourceURL", "(Ljava/lang/String;)Z");
    jboolean result = env->CallBooleanMethod(_javPlayer, jmethod, jstring);

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

void VROAVPlayer::seekToTime(float seconds) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    jclass cls = env->GetObjectClass(_javPlayer);
    jmethodID jmethod = env->GetMethodID(cls, "seekToTime", "(F)V");
    env->CallVoidMethod(_javPlayer, jmethod, seconds);

    env->DeleteLocalRef(cls);
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

