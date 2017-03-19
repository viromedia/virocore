/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public class SoundJni implements BaseSoundJni {

    final protected long mNativeRef;

    private boolean mReady = false;
    private SoundDelegate mDelegate;

    /**
     * Constructor for the SoundJni class
     *
     * @param path - path to the file or web resource
     * @param renderContext - the renderContext
     * @param delegate - the SoundDelegate
     * @param local - whether or not the given path is a local or web resource
     */
    public SoundJni(String path, RenderContextJni renderContext,
                    SoundDelegate delegate, boolean local) {
        // we don't currently use local because the underlying player is Android's MediaPlayer, if/when
        // we move to GVR audio, we'll want to treat them differently.
        mDelegate = delegate;
        mNativeRef = nativeCreateSound(path, renderContext.mNativeRef);
    }

    public SoundJni(SoundDataJni data, RenderContextJni renderContext,
                    SoundDelegate delegate) {
        mDelegate = delegate;
        mNativeRef = nativeCreateSoundWithData(data.mNativeRef, renderContext.mNativeRef);
    }

    @Override
    public void destroy() {
        nativeDestroySound(mNativeRef);
    }

    @Override
    public void play() {
        nativePlaySound(mNativeRef);
    }

    @Override
    public void pause() {
        nativePauseSound(mNativeRef);
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
    public void setRotation(float[] rotation) {
        // no-op SoundField only
    }

    @Override
    public void setPosition(float[] position) {
        // no-op SpatialSound only
    }

    @Override
    public void setDistanceRolloff(String model, float minDistance, float maxDistance) {
        // no-op SpatialSound only
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

    private native long nativeCreateSound(String filename, long renderContextRef);
    private native long nativeCreateSoundWithData(long dataRef, long renderContextRef);
    private native void nativeDestroySound(long mNativeRef);
    private native void nativePlaySound(long nativeRef);
    private native void nativePauseSound(long nativeRef);
    private native void nativeSetVolume(long nativeRef, float volume);
    private native void nativeSetMuted(long mNativeRef, boolean muted);
    private native void nativeSetLoop(long mNativeRef, boolean loop);
    private native void nativeSeekToTime(long mNativeRef, float seconds);
}
