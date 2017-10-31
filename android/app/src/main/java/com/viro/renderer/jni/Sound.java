/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.net.Uri;

/**
 * Viro provides a full-featured audio engine. Sprinkling environments with sound helps to add true
 * immersion to your scenes; they draw the listener's attention and provide cues on where to look
 * next. Viro supports three forms of sound: stereo (for music and non-spatial sound effects),
 * spatial (where the sound is positioned within the scene graph), and sound fields (for atmospheric
 * background noise).
 * <p>
 * Sound is the class for basic stereo sounds.
 */
public class Sound implements BaseSound {

    /**
     * Callback interface for responding to {@link Sound} events.
     */
    public interface Delegate {

        /**
         * Invoked when a {@link Sound} has finished loading and is ready to play without delay.
         *
         * @param sound The sound.
         */
        void onSoundReady(Sound sound);

        /**
         * Invoked when a {@link Sound} has completed. This is also invoked at the end of each loop of
         * a sound.
         *
         * @param sound The sound.
         */
        void onSoundFinish(Sound sound);

        /**
         * Invoked when a {@link Sound} failed to load.
         *
         * @param error The associated error message.
         */
        void onSoundFail(String error);
    }

    long mNativeRef;

    private Delegate mDelegate;
    private boolean mReady = false;
    private float mVolume = 1.0f;
    private boolean mMuted = false;
    private boolean mPaused = true;
    private boolean mLoop = false;

    /**
     * Construct a new Sound.
     *
     * @param viroContext The {@link ViroContext} is required to play sounds.
     * @param uri         The URI of the sound. To load the sound from an Android asset, use URI's
     *                    of the form <tt>file:///android_asset/[asset-name]</tt>.
     * @param delegate    {@link Delegate} which can be used to respond to sound loading and
     *                    playback events. May be null.
     */
    public Sound(ViroContext viroContext, Uri uri, Delegate delegate) {
        // we don't currently use local because the underlying player is Android's MediaPlayer, if/when
        // we move to GVR audio, we'll want to treat them differently.
        mDelegate = delegate;
        mNativeRef = nativeCreateSound(uri.toString(), viroContext.mNativeRef);

        // Setup is called after creation because setup may end up invoking the delegate, and we
        // need mNativeRef set to a valid pointer before we do so
        nativeSetup(mNativeRef);
    }

    /**
     * @hide
     */
    public Sound(String path, ViroContext viroContext, Delegate delegate) {
        mDelegate = delegate;
        mNativeRef = nativeCreateSound(path, viroContext.mNativeRef);

        // Setup is called after creation because setup may end up invoking the delegate, and we
        // need mNativeRef set to a valid pointer before we do so
        nativeSetup(mNativeRef);
    }

    /**
     * @hide
     */
    public Sound(SoundData data, ViroContext viroContext, Delegate delegate) {
        mDelegate = delegate;
        mNativeRef = nativeCreateSoundWithData(data.mNativeRef, viroContext.mNativeRef);
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
     * Release native resources associated with this Sound.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroySound(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Play the Sound.
     */
    @Override
    public void play() {
        mPaused = false;
        nativePlaySound(mNativeRef);
    }

    /**
     * Pause the Sound.
     */
    @Override
    public void pause() {
        mPaused = true;
        nativePauseSound(mNativeRef);
    }

    /**
     * Return true if the Sound is currently playing. This returns false if the Sound is loading
     * or paused.
     *
     * @return True if the Sound is playing.
     */
    public boolean isPlaying() {
        return mReady && !mPaused;
    }

    /**
     * Return true if the Sound is paused. The Sound can be played by invoking {@link #play()}.
     *
     * @return True if the Sound is paused.
     */
    public boolean isPaused() {
        return mPaused;
    }

    /**
     * Return true if the Sound is loading.
     *
     * @return True if the Sound is loading.
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
    @Override
    public void setVolume(float volume) {
        mVolume = volume;
        nativeSetVolume(mNativeRef, volume);
    }

    /**
     * Get the volume of the Sound, between 0.0 and 1.0.
     *
     * @return The volume.
     */
    public float getVolume() {
        return mVolume;
    }

    /**
     * Set to true to mute the Sound.
     *
     * @param muted True to mute.
     */
    @Override
    public void setMuted(boolean muted) {
        mMuted = true;
        nativeSetMuted(mNativeRef, muted);
    }

    /**
     * Return true if the Sound is currently muted.
     *
     * @return True if muted.
     */
    public boolean isMuted() {
        return mMuted;
    }

    /**
     * Set to true to make the Sound automatically loop to the beginning when playback finishes.
     *
     * @param loop True to loop.
     */
    @Override
    public void setLoop(boolean loop) {
        mLoop = loop;
        nativeSetLoop(mNativeRef, loop);
    }

    /**
     * Return true if the Sound is currently set to loop after finishing playback.
     *
     * @return True if loop is enabled.
     */
    public boolean getLoop() {
        return mLoop;
    }

    /**
     * Seek to the given point in the Sound, in seconds.
     *
     * @param seconds The seek position in seconds.
     */
    @Override
    public void seekToTime(float seconds) {
        nativeSeekToTime(mNativeRef, seconds);
    }

    /**
     * Set the {@link Delegate}, which can be used to respond to Sound loading and playback
     * events.
     *
     * @param delegate The SoundDelegate to use for this Sound.
     */
    public void setDelegate(Delegate delegate) {
        mDelegate = delegate;
        // call the delegate.onSoundReady() if we're already ready.
        if (mReady) {
            delegate.onSoundReady(this);
        }
    }

    /**
     * Get the {@link Delegate} used to receive callbacks for this Sound.
     *
     * @return The SoundDelegate, or null if none is attached.
     */
    public Delegate getDelegate() {
        return mDelegate;
    }

    /**
     * @hide
     */
    @Override
    public void soundIsReady() {
        mReady = true;
        if (mDelegate != null) {
            mDelegate.onSoundReady(this);
        }
    }

    /**
     * @hide
     */
    @Override
    public void soundDidFinish() {
        if (mDelegate != null) {
            mDelegate.onSoundFinish(this);
        }
    }

    /**
     * @hide
     * @param error
     */
    @Override
    public void soundDidFail(String error) {
        if (mDelegate != null) {
            mDelegate.onSoundFail(error);
        }
    }

    private native long nativeCreateSound(String uri, long renderContextRef);
    private native long nativeCreateSoundWithData(long dataRef, long renderContextRef);
    private native void nativeSetup(long mNativeRef);
    private native void nativeDestroySound(long mNativeRef);
    private native void nativePlaySound(long nativeRef);
    private native void nativePauseSound(long nativeRef);
    private native void nativeSetVolume(long nativeRef, float volume);
    private native void nativeSetMuted(long mNativeRef, boolean muted);
    private native void nativeSetLoop(long mNativeRef, boolean loop);
    private native void nativeSeekToTime(long mNativeRef, float seconds);
}
