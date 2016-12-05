/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Android Java Object  : com.viromedia.bridge.component.node.control.VideoSurface
 * Java JNI Wrapper     : com.viro.renderer.VideoSurfaceJni.java
 * Cpp JNI wrapper      : VideoSurface_JNI.cpp
 * Cpp Object           : VROVideoSurface
 */
public class VideoSurfaceJni extends BaseGeometry{
    public VideoSurfaceJni(float width, float height, String url, RenderContextJni context) {
        mNativeRef = nativeCreateVideo(width, height, url, context.mRef);
    }
    public void delete() {
        nativeDeleteVideo(mNativeRef);
    }
    public void pause () {
        nativePause(mNativeRef);
    }
    public void play (){
        nativePlay(mNativeRef);
    }
    public boolean isPaused() {
        return nativeIsPaused(mNativeRef);
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
    public native long nativeCreateVideo(float width, float height,
                                  String urlString, long renderContextRef);
    public native long nativeDeleteVideo(long nativeRenderer);
    private native void nativePause(long nativeRenderer);
    private native void nativePlay(long nativeRenderer);
    private native boolean nativeIsPaused(long nativeRenderer);
    private native void nativeSetMuted(long nativeRenderer, boolean muted);
    private native void nativeSetVolume(long nativeRenderer, float volume);
    private native void nativeSetLoop(long nativeRenderer, boolean loop);
    private native void nativeSeekToTime(long nativeRenderer, int seconds);

    @Override
    public void attachToNode(NodeJni node) {
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }
    private native void nativeAttachToNode(long videoReference, long nodeReference);

    /**
     * Delegate Callback functions called from JNI
     */
    private VideoDelegate mDelegate = null;
    public interface VideoDelegate{
        void onVideoFinish();
    }

    public void setVideoDelegate(VideoDelegate delegate){
        mDelegate = delegate;
    }

    public void nativeOnVideoFinished(){
        if (mDelegate != null){
            mDelegate.onVideoFinish();
        }
    }
}
