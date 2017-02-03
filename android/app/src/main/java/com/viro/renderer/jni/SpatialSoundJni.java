/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public class SpatialSoundJni implements BaseSoundJni {
    final protected long mNativeRef;

    private boolean mReady = false;
    private SoundDelegate mDelegate;
    private NodeJni mParentNode = null;

    public SpatialSoundJni(String path, RenderContextJni renderContext,
                           SoundDelegate delegate, boolean local) {
        if (local) {
            mNativeRef = nativeCreateSpatialSoundFromFile(path, renderContext.mNativeRef);
        } else {
            mNativeRef = nativeCreateSpatialSoundFromUrl(path, renderContext.mNativeRef);
        }
        mDelegate = delegate;
    }

    public SpatialSoundJni(SoundDataJni data, RenderContextJni renderContext,
                    SoundDelegate delegate) {
        mNativeRef = nativeCreateSpatialSoundWithData(data.mNativeRef, renderContext.mNativeRef);
        mDelegate = delegate;
    }

    public void destroy() {
        if (mParentNode != null && !mParentNode.mDestroyed) {
            nativeDetachFromNode(mNativeRef, mParentNode.mNativeRef);
            mParentNode = null;
        }
        nativeDestroySpatialSound(mNativeRef);
    }

    public void attachToNode(NodeJni node) {
        mParentNode = node;
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }

    public void detachFromNode(NodeJni node) {
        nativeDetachFromNode(mNativeRef, node.mNativeRef);
    }

    public void play() {
        nativePlaySpatialSound(mNativeRef);
    }

    public void pause() {
        nativePauseSpatialSound(mNativeRef);
    }

    public void setVolume(float volume) {
        nativeSetVolume(mNativeRef, volume);
    }

    public void setMuted(boolean muted) {
        nativeSetMuted(mNativeRef, muted);
    }

    public void setLoop(boolean loop) {
        nativeSetLoop(mNativeRef, loop);
    }

    public void seekToTime(float seconds) {
        nativeSeekToTime(mNativeRef, seconds);
    }

    public void setDelegate(SoundDelegate delegate) {
        mDelegate = delegate;
        // call the delegate.onSoundReady() if we're already ready.
        if (mReady) {
            delegate.onSoundReady();
        }
    }

    @Override
    public void setRotation(float[] rotation) {
        // no-op only for SoundField
    }

    public void setPosition(float[] position) {
        nativeSetPosition(mNativeRef, position[0], position[1], position[2]);
    }

    @Override
    public void setDistanceRolloff(String model, float minDistance, float maxDistance) {
        nativeSetDistanceRolloff(mNativeRef, model, minDistance, maxDistance);
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

    private native long nativeCreateSpatialSoundFromFile(String filename, long renderContextRef);
    private native long nativeCreateSpatialSoundFromUrl(String url, long renderContextRef);
    private native long nativeCreateSpatialSoundWithData(long nativeRef, long dataRef);
    private native void nativeDestroySpatialSound(long nativeRef);
    private native void nativeAttachToNode(long nativeRef, long nodeRef);
    private native void nativeDetachFromNode(long nativeRef, long nodeRef);
    private native void nativePlaySpatialSound(long nativeRef);
    private native void nativePauseSpatialSound(long nativeRef);
    private native void nativeSetVolume(long nativeRef, float volume);
    private native void nativeSetMuted(long nativeRef, boolean muted);
    private native void nativeSetLoop(long nativeRef, boolean loop);
    private native void nativeSeekToTime(long nativeRef, float seconds);
    private native void nativeSetPosition(long nativeRef, float posX, float posY, float posZ);
    private native void nativeSetDistanceRolloff(long nativeRef, String model, float minDistance, float maxDistance);
}
