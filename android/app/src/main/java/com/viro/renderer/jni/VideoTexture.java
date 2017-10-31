/*
 * Copyright © 2016 Viro Media. All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.jni.VideoTextureJni.java
 * Cpp JNI wrapper      : VideoTexture_JNI.cpp
 * Cpp Object           : VROVideoTextureAVP.cpp
 */
package com.viro.renderer.jni;

import android.net.Uri;

import java.lang.ref.WeakReference;

/**
 * VideoTexture plays video from local or remote sources. VideoTexture extends from {@link Texture},
 * so it can be used with any {@link Material} for display on any geometry. To make a video render
 * on a {@link Surface}, for example, use:
 * <p>
 * <tt>
 * <pre>
 * VideoTexture videoTexture = new VideoTexture(...);
 * Material material = new Material();
 * material.setDiffuseTexture(videoTexture);
 *
 * Surface surface = new Surface(10, 10);
 * surface.setMaterials(Arrays.asList(material));</pre>
 * </tt>
 * VideoTexture can also be rendered directly to the background of a {@link Scene}, which is useful
 * for displaying spherical, immersive video in VR. To do so, use {@link
 * Scene#setBackgroundTexture(Texture)}.
 */
public class VideoTexture extends Texture {

    /**
     * Callback interface for responding to video lifecycle and playback events.
     */
    public interface Delegate {
        /**
         * Invoked when the video begins buffering.
         *
         * @param video The {@link VideoTexture} displaying the video.
         */
        void onVideoBufferStart(VideoTexture video);

        /**
         * Invoked when the video ends buffering.
         *
         * @param video The {@link VideoTexture} displaying the video.
         */
        void onVideoBufferEnd(VideoTexture video);

        /**
         * Invoked when the video finishes. If the video is looping, this will be invoked each
         * time we reach the end of the video.
         *
         * @param video The {@link VideoTexture} displaying the video.
         */
        void onVideoFinish(VideoTexture video);

        /**
         * Invoked when the video is loaded and ready for playback.
         *
         * @param video The {@link VideoTexture} displaying the video.
         */
        void onReady(VideoTexture video);

        /**
         * Invoked when the video has failed to load.
         *
         * @param error The error message.
         */
        void onVideoFailed(String error);

        /**
         * Invoked continually as the video updates its position during playback.
         *
         * @param video The {@link VideoTexture} displaying the video.
         * @param seconds The current playback position, in seconds.
         * @param totalDuration The total duration of the video, in seconds.
         */
        void onVideoUpdatedTime(VideoTexture video, float seconds, float totalDuration);
    }

    private long mNativeDelegateRef;

    private WeakReference<Delegate> mDelegate = null;
    private boolean mReady = false;
    private float mVolume = 1.0f;
    private boolean mMuted = false;
    private boolean mPaused = true;
    private boolean mLoop = false;

    /**
     * Construct a new VideoTexture that will play the video at the provided URI.
     *
     * @param viroContext The {@link ViroContext} is required to play videos.
     * @param uri         The URI of the video. To load the video from an Android asset, use URI's
     *                    of the form <tt>file:///android_asset/[asset-name]</tt>.
     */
    public VideoTexture(ViroContext viroContext, Uri uri) {
        this(viroContext, uri, null);
    }

    /**
     * Construct a new VideoTexture that will play the video at the provided URI.
     *
     * @param viroContext The {@link ViroContext} is required to play videos.
     * @param uri         The URI of the video. To load the video from an Android asset, use URI's
     *                    of the form <tt>file:///android_asset/[asset-name]</tt>.
     * @param delegate    {@link Delegate} which can be used to respond to video loading and
     *                    playback events. May be null.
     */
    public VideoTexture(ViroContext viroContext, Uri uri, Delegate delegate) {
        this(viroContext, uri, delegate, null);
    }

    /**
     * Construct a new <i>stereo</i> VideoTexture that will play the video at the provided URI.
     * This constructor should be used when loading a stereo video. Stereo videos are designed to
     * simulate 3D by rendering slightly different images to each eye, creating a realistic depth
     * illusion. The StereoMode indicates how the stereo image is divided between the left eye and
     * the right eye.
     *
     * @param viroContext The {@link ViroContext} is required to play videos.
     * @param uri         The URI of the video. To load the video from an Android asset, use URI's
     *                    of the form <tt>file:///android_asset/[asset-name]</tt>.
     * @param delegate    {@link Delegate} which can be used to respond to video loading and
     *                    playback events. May be null.
     * @param stereoMode  The {@link StereoMode} indicating which half of the video to render to the
     *                    left eye, and which to render to the right eye. Null if the video is not
     *                    stereo.
     */
    public VideoTexture(ViroContext viroContext, Uri uri, Delegate delegate, Texture.StereoMode stereoMode) {
        mNativeRef = nativeCreateVideoTexture(viroContext.mNativeRef, stereoMode == null ? null : stereoMode.getStringValue());
        mNativeDelegateRef = nativeCreateVideoDelegate();
        if (delegate != null) {
            mDelegate = new WeakReference<Delegate>(delegate);
        }
        nativeAttachDelegate(mNativeRef, mNativeDelegateRef);
        nativeLoadSource(mNativeRef, uri.toString(), viroContext.mNativeRef);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this VideoTexture.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDeleteVideoTexture(mNativeRef);
            mNativeRef = 0;
        }

        if (mNativeDelegateRef != 0) {
            nativeDeleteVideoDelegate(mNativeDelegateRef);
            mNativeDelegateRef = 0;
        }
    }

    /**
     * Play the video.
     */
    public void play() {
        mPaused = false;
        nativePlay(mNativeRef);
    }

    /**
     * Pause the video.
     */
    public void pause() {
        mPaused = true;
        nativePause(mNativeRef);
    }

    /**
     * Return true if the video is currently playing. This returns false if the video
     * is loading or paused.
     *
     * @return True if the video is playing.
     */
    public boolean isPlaying() {
        return mReady && !mPaused;
    }

    /**
     * Return true if the video is paused. The video can be played by invoking {@link #play()}.
     *
     * @return True if the video is paused.
     */
    public boolean isPaused() {
        return mPaused;
    }

    /**
     * Return true if the video is loading.
     *
     * @return True if the video is loading.
     */
    public boolean isLoading() {
        return !mReady;
    }

    /**
     * Set the volume to the given value, where 0.0 is mute and 1.0 is the maximum volume. The
     * default is 1.0.
     *
     * @param volume The value between 0.0 and 1.0.
     */
    public void setVolume(float volume) {
        mVolume = volume;
        nativeSetVolume(mNativeRef, volume);
    }

    /**
     * Get the volume of the video, between 0.0 and 1.0.
     *
     * @return The volume.
     */
    public float getVolume() {
        return mVolume;
    }

    /**
     * Set to true to mute the video.
     *
     * @param muted True to mute.
     */
    public void setMuted(boolean muted) {
        mMuted = true;
        nativeSetMuted(mNativeRef, muted);
    }

    /**
     * Return true if the video is currently muted.
     *
     * @return True if muted.
     */
    public boolean isMuted() {
        return mMuted;
    }

    /**
     * Set to true to make the video automatically loop to the beginning when playback finishes.
     *
     * @param loop True to loop.
     */
    public void setLoop(boolean loop) {
        mLoop = loop;
        nativeSetLoop(mNativeRef, loop);
    }

    /**
     * Return true if the video is currently set to loop after finishing playback.
     *
     * @return True if loop is enabled.
     */
    public boolean getLoop() {
        return mLoop;
    }

    /**
     * Seek to the given point in the video, in seconds.
     *
     * @param seconds The seek position in seconds.
     */
    public void seekToTime(float seconds) {
        nativeSeekToTime(mNativeRef, seconds);
    }

    /**
     * Set the {@link Delegate}, which can be used to respond to video loading and playback
     * events.
     *
     * @param delegate The delegate to use for this video.
     */
    public void setVideoDelegate(Delegate delegate) {
        mDelegate = new WeakReference<Delegate>(delegate);
    }

    /*
     Native Functions called into JNI
     */
    private native long nativeCreateVideoTexture(long renderContext, String stereoType);
    private native long nativeCreateVideoDelegate();
    private native void nativeAttachDelegate(long nativeTexture, long nativeDelegate);
    private native void nativeDeleteVideoTexture(long nativeTexture);
    private native void nativeDeleteVideoDelegate(long nativeDelegate);
    private native void nativePause(long nativeTexture);
    private native void nativePlay(long nativeTexture);
    private native void nativeSetMuted(long nativeTexture, boolean muted);
    private native void nativeSetVolume(long nativeTexture, float volume);
    private native void nativeSetLoop(long nativeTexture, boolean loop);
    private native void nativeSeekToTime(long nativeTexture, float seconds);
    private native void nativeLoadSource(long nativeTexture, String url, long renderContext);

    /*
     Invoked from JNI.
     */
    /**
     * @hide
     */
    void playerWillBuffer() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != 0) {
            mDelegate.get().onVideoBufferStart(this);
        }
    }
    /**
     * @hide
     */
    void playerDidBuffer() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != 0) {
            mDelegate.get().onVideoBufferEnd(this);
        }
    }
    /**
     * @hide
     */
    void playerDidFinishPlaying() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != 0) {
            mDelegate.get().onVideoFinish(this);
        }
    }
    /**
     * @hide
     */
    void onVideoFailed(String error) {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != 0) {
            mDelegate.get().onVideoFailed(error);
        }
    }
    /**
     * @hide
     */
    void onReady() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != 0) {
            mDelegate.get().onReady(this);
        }
    }
    /**
     * @hide
     */
    void onVideoUpdatedTime(float currentTimeInSeconds, float totalTimeInSeconds) {
        if (mDelegate != null && mDelegate.get() != null){
            mDelegate.get().onVideoUpdatedTime(this, currentTimeInSeconds, totalTimeInSeconds);
        }
    }
}
