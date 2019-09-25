//
//  Copyright (c) 2018-present, ViroMedia, Inc.
//  All rights reserved.
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

/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.jni.AnimatedTexture.java
 * Cpp JNI wrapper      : AnimatedTexture_JNI.cpp
 * Cpp Object           : VROAnimatedTextureOpenGL.cpp
 */
package com.viro.core;

import android.net.Uri;

/**
 * AnimatedTexture plays animated GIF images from local or remote sources. AnimatedTexture
 * extends from {@link Texture}, so it can be used with any {@link Material} for display
 * on any geometry. To make an AnimatedTexture for rendering GIF images on a {@link Quad},
 * for example, use:
 * <p>
 * <tt>
 * <pre>
 * AnimatedTexture animTexture = new AnimatedTexture(...);
 * Material material = new Material();
 * material.setDiffuseTexture(animTexture);
 *
 * Quad quad = new Quad(1, 1);
 * quad.setMaterials(Arrays.asList(material));</pre>
 * </tt>
 */
public class AnimatedTexture extends Texture {

    /**
     * Callback interface for responding to an AnimatedTexture's lifecycle events.
     */
    public interface OnLoadComplete {
        /**
         * Invoked when the AnimatedTexture is loaded and ready to be animated.
         *
         * @param video The {@link AnimatedTexture} containing the GIF to be animated.
         */
        void onSuccess(AnimatedTexture video);

        /**
         * Invoked when the AnimatedTexture has failed to load.
         *
         * @param error The error message.
         */
        void onFailure(String error);
    }

    private OnLoadComplete mCurrentLoadCallback = null;
    private boolean mReady = false;
    private boolean mPaused = true;
    private boolean mLoop = false;

    /**
     * Construct a new AnimatedTexture that will play the animation given the provided URI.
     * If a {@link OnLoadComplete} is provided, its success or failure callbacks will be
     * triggered once the image data has been loaded.
     */
    public AnimatedTexture(ViroContext context, Uri uri, OnLoadComplete listener) {
        mCurrentLoadCallback = listener;
        mNativeRef = nativeCreateAnimatedTexture();
        nativeLoadSource(mNativeRef, uri.toString(), this, context.mNativeRef);
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
     * Release native resources associated with this AnimationTexture.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDeleteAnimatedTexture(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Plays the animated texture. The animation automatically restarts if had previously
     * ended and loop is set to true.
     */
    public void play() {
        mPaused = false;
        nativePlay(mNativeRef);
    }

    /**
     * Pauses the animated texture.
     */
    public void pause() {
        mPaused = true;
        nativePause(mNativeRef);
    }

    /**
     * Return true if the AnimatedTexture is currently playing. This returns false if the
     * animation is loading or paused.
     *
     * @return True if the animation is playing.
     */
    public boolean isPlaying() {
        return mReady && !mPaused;
    }

    /**
     * Return true if the animation is paused. The video can be played by invoking {@link #play()}.
     *
     * @return True if the animation is paused.
     */
    public boolean isPaused() {
        return mPaused;
    }

    /**
     * Return true if the animation is loading.
     *
     * @return True if the animation is loading.
     */
    public boolean isLoading() {
        return !mReady;
    }

    /**
     * Set to true to make the AnimatedTexture automatically loop to the beginning when playback finishes.
     *
     * @param loop True to loop.
     */
    public void setLoop(boolean loop) {
        mLoop = loop;
        nativeSetLoop(mNativeRef, loop);
    }

    /**
     * Return true if the animated texture is currently set to loop after finishing playback.
     *
     * @return True if loop is enabled.
     */
    public boolean getLoop() {
        return mLoop;
    }

    /**
     * Called from JNI upon animated texture load completion.
     *
     * @hide
     */
    public void onSourceLoaded(boolean success, String error){
        mReady = success;

        mWidth = nativeGetTextureWidth(mNativeRef);
        mHeight = nativeGetTextureHeight(mNativeRef);

        if (mCurrentLoadCallback == null) {
            return;
        }

        if (success){
            mCurrentLoadCallback.onSuccess(this);
        } else {
            mCurrentLoadCallback.onFailure(error);
        }
    }

    /*
     Native Functions called into JNI
     */
    private native long nativeCreateAnimatedTexture();
    private native void nativeDeleteAnimatedTexture(long nativeTexture);
    private native void nativeLoadSource(long nativeTexture, String url, AnimatedTexture texture, long renderContext);
    private native void nativePause(long nativeTexture);
    private native void nativePlay(long nativeTexture);
    private native void nativeSetLoop(long nativeTexture, boolean loop);
}
