/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public class SoundField implements BaseSound {
    final protected long mNativeRef;

    private boolean mReady = false;
    private SoundDelegate mDelegate;

    public SoundField(String path, RenderContext renderContext,
                      SoundDelegate delegate, boolean local) {

        mNativeRef = nativeCreateSoundField(path, local, renderContext.mNativeRef);

        mDelegate = delegate;
    }

    public SoundField(SoundData data, RenderContext renderContext,
                      SoundDelegate delegate) {
        mNativeRef = nativeCreateSoundFieldWithData(data.mNativeRef, renderContext.mNativeRef);
        mDelegate = delegate;
    }

    @Override
    public void destroy() {
        nativeDestroySoundField(mNativeRef);
    }

    @Override
    public void play() {
        nativePlaySoundField(mNativeRef);
    }

    @Override
    public void pause() {
        nativePauseSoundField(mNativeRef);
    }

    @Override
    public void setVolume(float volume) {
        nativeSetVolume(mNativeRef, volume);
    }

    @Override
    public void setMuted(boolean muted) {
        nativeSetMuted(mNativeRef, muted);
    }

    @Override
    public void setLoop(boolean loop) {
        nativeSetLoop(mNativeRef, loop);
    }

    @Override
    public void setRotation(float[] rotation) {
        nativeSetRotation(mNativeRef, rotation[0], rotation[1], rotation[2]);
    }

    @Override
    public void setPosition(float[] position) {
        // no-op only for SpatialSound
    }

    @Override
    public void setDistanceRolloff(String model, float minDistance, float maxDistance) {
        // no-op only for SpatialSound
    }

    @Override
    public void seekToTime(float seconds) {
        nativeSeekToTime(mNativeRef, seconds);
    }

    @Override
    public void setDelegate(SoundDelegate delegate) {
        mDelegate = delegate;
        // call the delegate.onSoundReady() if we're already ready.
        if (mReady) {
            delegate.onSoundReady();
        }
    }

    @Override
    public void soundIsReady() {
        mReady = true;
        if (mDelegate != null) {
            mDelegate.onSoundReady();
        }
    }

    @Override
    public void soundDidFinish() {
        if (mDelegate != null) {
            mDelegate.onSoundFinish();
        }
    }

    @Override
    public void soundDidFail(String error) {
        if (mDelegate != null) {
            mDelegate.onSoundFail(error);
        }
    }

    private native long nativeCreateSoundField(String url, boolean local, long renderContextRef);
    private native long nativeCreateSoundFieldWithData(long dataRef, long renderContextRef);
    private native void nativeDestroySoundField(long mNativeRef);
    private native void nativePlaySoundField(long nativeRef);
    private native void nativePauseSoundField(long nativeRef);
    private native void nativeSetVolume(long nativeRef, float volume);
    private native void nativeSetMuted(long mNativeRef, boolean muted);
    private native void nativeSetLoop(long mNativeRef, boolean loop);
    private native void nativeSeekToTime(long mNativeRef, float seconds);
    private native void nativeSetRotation(long mNativeRef, float degreesX, float degreesY, float degreesZ);
}
