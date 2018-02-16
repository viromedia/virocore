/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;


import android.net.Uri;

import com.viro.core.internal.BaseSound;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

/**
 * SpatialSound is positioned audio that can travel throughout a scene. For example, you can create
 * a SpatialSound that animates along with a bird, chirping as it flies through the scene.
 * SpatialSound is also compatible with reverb effects, which enables you to simulate the acoustic
 * qualities of a wide variety of environments. To use reverb, create a sound room via {@link
 * Scene#setSoundRoom(ViroContext, Vector, Scene.AudioMaterial, Scene.AudioMaterial,
 * Scene.AudioMaterial)}.
 * <p>
 * For an extended discussion of sound in Viro, refer to the <a
 * href="https://virocore.viromedia.com/docs/audio">Audio Guide</a>.
 */
public class SpatialSound implements BaseSound {

    /**
     * Callback interface for responding to {@link SpatialSound} events.
     */
    public interface PlaybackListener {

        /**
         * Invoked when a {@link SpatialSound} has finished loading and is ready to play without
         * delay.
         *
         * @param sound The sound.
         */
        void onSoundReady(SpatialSound sound);

        /**
         * Invoked when a {@link SpatialSound} failed to load.
         *
         * @param error The associated error message.
         */
        void onSoundFail(String error);
    }

    /**
     * The attenuation function that determines how the volume of the SpatialSound changes with
     * distance from the sound's position.
     */
    public enum Rolloff {
        /**
         * The volume of the SpatialSound drops off linearly with distance.
         */
        LINEAR("linear"),
        /**
         * The volume of the SpatialSound drops off logarithmically with distance.
         */
        LOGARITHMIC("logarithmic"),
        /**
         * The SpatialSound will not attenuate at any distance.
         */
        NONE("none");

        private String mStringValue;
        private Rolloff(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, Rolloff> map = new HashMap<String, Rolloff>();
        static {
            for (Rolloff value : Rolloff.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static Rolloff valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    long mNativeRef;
    WeakReference<Node> mParentNode = null;

    private PlaybackListener mListener;
    private boolean mReady = false;
    private float mVolume = 1.0f;
    private boolean mMuted = false;
    private boolean mPaused = true;
    private boolean mLoop = false;

    /**
     * Construct a new SpatialSound.
     *
     * @param viroContext The {@link ViroContext} is required to play sounds.
     * @param uri         The URI of the sound. To load the sound from an Android asset, use URI's
     *                    of the form <tt>file:///android_asset/[asset-name]</tt>.
     * @param listener    {@link PlaybackListener} which can be used to respond to sound loading and
     *                    playback events. May be null.
     */
    public SpatialSound(ViroContext viroContext, Uri uri, PlaybackListener listener) {
        mNativeRef = nativeCreateSpatialSound(uri.toString(), viroContext.mNativeRef);
        mListener = listener;
    }

    /**
     * @hide
     */
    public SpatialSound(SoundData data, ViroContext viroContext,
                        PlaybackListener delegate) {
        mNativeRef = nativeCreateSpatialSoundWithData(data.mNativeRef, viroContext.mNativeRef);
        mListener = delegate;
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
     * Release native resources associated with this SpatialSound.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            removeFromParent();;
            nativeDestroySpatialSound(mNativeRef);
            mNativeRef = 0;
        }
    }

    private void removeFromParent() {
        Node node = mParentNode == null ? null : mParentNode.get();
        if (node != null && !node.mDestroyed) {
            node.removeSound(this);
        }
    }

    /**
     * Play the SpatialSound.
     */
    public void play() {
        mPaused = false;
        nativePlaySpatialSound(mNativeRef);
    }

    /**
     * Pause the SpatialSound.
     */
    public void pause() {
        mPaused = true;
        nativePauseSpatialSound(mNativeRef);
    }

    /**
     * Return true if the SpatialSound is currently playing. This returns false if the SpatialSound
     * is loading or paused.
     *
     * @return True if the SpatialSound is playing.
     */
    public boolean isPlaying() {
        return mReady && !mPaused;
    }

    /**
     * Return true if the SpatialSound is paused. The SpatialSound can be played by invoking {@link #play()}.
     *
     * @return True if the SpatialSound is paused.
     */
    public boolean isPaused() {
        return mPaused;
    }

    /**
     * Return true if the SpatialSound is loading.
     *
     * @return True if the SpatialSound is loading.
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
     * Get the volume of the SpatialSound, between 0.0 and 1.0.
     *
     * @return The volume.
     */
    public float getVolume() {
        return mVolume;
    }

    /**
     * Set to true to mute the SpatialSound.
     *
     * @param muted True to mute.
     */
    @Override
    public void setMuted(boolean muted) {
        mMuted = muted;
        nativeSetMuted(mNativeRef, muted);
    }

    /**
     * Return true if the SpatialSound is currently muted.
     *
     * @return True if muted.
     */
    public boolean isMuted() {
        return mMuted;
    }

    /**
     * Set to true to make the SpatialSound automatically loop to the beginning when playback finishes.
     *
     * @param loop True to loop.
     */
    @Override
    public void setLoop(boolean loop) {
        mLoop = loop;
        nativeSetLoop(mNativeRef, loop);
    }

    /**
     * Return true if the SpatialSound is currently set to loop after finishing playback.
     *
     * @return True if loop is enabled.
     */
    public boolean getLoop() {
        return mLoop;
    }

    /**
     * Seek to the given point in the SpatialSound, in seconds.
     *
     * @param seconds The seek position in seconds.
     */
    public void seekToTime(float seconds) {
        nativeSeekToTime(mNativeRef, seconds);
    }

    /**
     * Set the {@link PlaybackListener}, which can be used to respond to SpatialSound loading and playback
     * events.
     *
     * @param listener The listener to use for this Sound.
     */
    public void setPlaybackListener(PlaybackListener listener) {
        mListener = listener;
        if (mReady) {
            listener.onSoundReady(this);
        }
    }

    /**
     * Get the {@link PlaybackListener} used to receive callbacks for this SpatialSound.
     *
     * @return The listener, or null if none is attached.
     */
    public PlaybackListener getPlaybackListener() {
        return mListener;
    }

    /**
     * Set the position of this SpatialSound within the coordinate system of its parent {@link
     * Node}.
     *
     * @param position The position of this SpatialSound as a {@link Vector}.
     */
    public void setPosition(Vector position) {
        nativeSetPosition(mNativeRef, position.x, position.y, position.z);
    }

    /**
     * Specify how this SpatialSound should attenuate in volume with distance from the user. The
     * sound will begin attenuating at <tt>minDistance</tt> and reach zero volume at
     * <tt>maxDistance</tt>. The volume level of the sound between those two values is determined by
     * the {@link Rolloff} model chosen.
     *
     * @param rolloff     The {@link Rolloff} which determines how the sound attenuates between
     *                    <tt>minDistance</tt> and <tt>maxDistance</tt>.
     * @param minDistance The distance at which the sound begins attenuating.
     * @param maxDistance The distance at which the sound finishes attenuated; e.g, when it reaches
     *                    zero volume.
     */
    public void setDistanceRolloff(Rolloff rolloff, float minDistance, float maxDistance) {
        nativeSetDistanceRolloff(mNativeRef, rolloff.getStringValue(), minDistance, maxDistance);
    }

    /**
     * @hide
     */
    @Override
    public void soundIsReady() {
        mReady = true;
        if (mListener != null) {
            mListener.onSoundReady(this);
        }
    }

    /**
     * @hide
     */
    @Override
    public void soundDidFinish() {
        // GVR does not support this yet, so this will never be called
    }

    /**
     * @hide
     * @param error
     */
    @Override
    public void soundDidFail(String error) {
        if (mListener != null) {
            mListener.onSoundFail(error);
        }
    }

    private native long nativeCreateSpatialSound(String uri, long renderContextRef);
    private native long nativeCreateSpatialSoundWithData(long nativeRef, long dataRef);
    private native void nativeDestroySpatialSound(long nativeRef);
    private native void nativePlaySpatialSound(long nativeRef);
    private native void nativePauseSpatialSound(long nativeRef);
    private native void nativeSetVolume(long nativeRef, float volume);
    private native void nativeSetMuted(long nativeRef, boolean muted);
    private native void nativeSetLoop(long nativeRef, boolean loop);
    private native void nativeSeekToTime(long nativeRef, float seconds);
    private native void nativeSetPosition(long nativeRef, float posX, float posY, float posZ);
    private native void nativeSetDistanceRolloff(long nativeRef, String model, float minDistance, float maxDistance);
}
