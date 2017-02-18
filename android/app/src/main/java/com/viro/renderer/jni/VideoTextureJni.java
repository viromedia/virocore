/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.jni.VideoTextureJni.java
 * Cpp JNI wrapper      : VideoTexture_JNI.cpp
 * Cpp Object           : VROVideoTextureAVP.cpp
 */
public class VideoTextureJni {
    private static long INVALID_REF = Long.MAX_VALUE;
    protected long mNativeRef = INVALID_REF;

    public VideoTextureJni() {
        nativeCreateVideoTexture();
    }
    public void delete() {
        nativeDeleteVideoTexture(mNativeRef);
        mNativeRef = INVALID_REF;
    }
    public void loadSource(String url, RenderContextJni renderContext){
        nativeLoadSource(mNativeRef, url, renderContext.mNativeRef);
    }
    public boolean isReady(){
        return mNativeRef != INVALID_REF;
    }

    public void pause () {
        nativePause(mNativeRef);
    }
    public void play (){
        nativePlay(mNativeRef);
    }
    public void setMuted(boolean muted){
        nativeSetMuted(mNativeRef, muted);
    }
    public void setVolume(float volume) {
        nativeSetVolume(mNativeRef, volume);
    }
    public void setLoop(boolean loop) {
        nativeSetLoop(mNativeRef, loop);
    }
    public void seekToTime(int seconds){
        nativeSeekToTime(mNativeRef, seconds);
    }

    /**
     * Native Functions called into JNI
     */
    public native void nativeCreateVideoTexture();
    public native void nativeDeleteVideoTexture(long nativeTexture);
    private native void nativePause(long nativeRennativeTexturederer);
    private native void nativePlay(long nativeTexture);
    private native void nativeSetMuted(long nativeTexture, boolean muted);
    private native void nativeSetVolume(long nativeTexture, float volume);
    private native void nativeSetLoop(long nativeTexture, boolean loop);
    private native void nativeSeekToTime(long nativeTexture, int seconds);
    private native void nativeLoadSource(long nativeTexture, String url, long renderContext);

    /**
     * Delegate Callback functions called from JNI
     */
    private VideoDelegate mDelegate = null;
    public interface VideoDelegate{
        void onVideoFinish();
        void onReady();
        void onVideoUpdatedTime(int seconds, int totalDuration);
    }

    public void setVideoDelegate(VideoDelegate delegate){
        mDelegate = delegate;
        if (isReady()){
            mDelegate.onReady();
        }
    }

    public void playerDidFinishPlaying(){
        if (mDelegate != null && mNativeRef != INVALID_REF){
            mDelegate.onVideoFinish();
        }
    }

    public void onReady(long ref) {
        mNativeRef = ref;
        if (mDelegate != null && mNativeRef != INVALID_REF) {
            mDelegate.onReady();
        }
    }

    public void onVideoUpdatedTime(int currentTimeInSeconds, int totalTimeInSeconds) {
        if (mDelegate != null){
            mDelegate.onVideoUpdatedTime(currentTimeInSeconds, totalTimeInSeconds);
        }
    }
}
