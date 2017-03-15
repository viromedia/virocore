/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;

/**
 * Wraps a {@link android.media.MediaPlayer} and can be controlled
 * via JNI.
 */
public class AVPlayer {

    private static final String TAG = "Viro";

    /**
     * These states mimic the underlying stats in the Android
     * MediaPlayer. We need to ensure we don't violate any state
     * in the Android MediaPlayer, else it becomes invalid.
     */
    private enum State {
        IDLE,
        PREPARED,
        PAUSED,
        STARTED,
    }

    private MediaPlayer _mediaPlayer;
    private float _volume;
    private long mNativeReference;
    private boolean _loop;
    private State _state;

    public AVPlayer(long nativeReference) {
        _mediaPlayer = new MediaPlayer();
        _volume = 1.0f;
        mNativeReference = nativeReference;
        _loop = false;
        _state = State.IDLE;

        // Attach listeners to be called back into native
        _mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mediaPlayer) {
                if (_loop) {
                    _state = State.PAUSED;
                    play();
                }
                nativeOnFinished(mNativeReference);
            }
        });

        _mediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mp) {
                nativeOnPrepared(mNativeReference);
            }
        });

        _mediaPlayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {
            @Override
            public boolean onError(MediaPlayer mediaPlayer, int what, int extra) {
                Log.w(TAG, "AVPlayer encountered error [" + what + ", extra: " + extra + "]");
                return true;
            }
        });
    }

    public boolean setDataSourceURL(String resourceOrURL, Context context) {
        try {
            reset();
            if (resourceOrURL.startsWith("res")) {
                Uri uri = Uri.parse(resourceOrURL);
                // The MediaPlayer doesn't like resources in the form res:/#######
                // so we need to convert it to: android.resource://[package]/[res id]
                Uri newUri = Uri.parse(ContentResolver.SCHEME_ANDROID_RESOURCE + "://"
                        + context.getPackageName() + uri.getPath());
                _mediaPlayer.setDataSource(context, newUri);
            } else {
                Log.i(TAG, "AVPlayer setting URL to ["  + resourceOrURL + "]");
                _mediaPlayer.setDataSource(resourceOrURL);
            }
            _mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            _mediaPlayer.prepare();
            _state = State.PREPARED;

            Log.i(TAG, "AVPlayer prepared for playback");

            return true;
        }catch(IOException e) {
            Log.w(TAG, "AVPlayer failed to load video at URL [" + resourceOrURL + "]", e);
            _mediaPlayer.reset();

            return false;
        }
    }

    public boolean setDataSourceAsset(String asset, AssetManager assetManager) {
        try {
            reset();
            AssetFileDescriptor afd = assetManager.openFd(asset);

            // MediaPlayer.setDataSource(AssetFileDescriptor) was introduced w/ API Level 24 (Nougat)
            if (android.os.Build.VERSION.SDK_INT >= 24) {
                _mediaPlayer.setDataSource(afd);
                _mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
                _mediaPlayer.prepare();
                _state = State.PREPARED;
            } else {
                // TODO: Figure out how to setDataSource for API Level < 24
            }

            return true;
        }catch(IOException e) {
            Log.w(TAG, "AVPlayer failed to load asset [" + asset + "]");
            return false;
        }
    }

    public void setVideoSink(Surface videoSink) {
        _mediaPlayer.setSurface(videoSink);
    }

    public void reset() {
        if (_mediaPlayer.isPlaying()) {
            _mediaPlayer.stop();
        }
        _mediaPlayer.reset();
        _state = State.IDLE;

        Log.i(TAG, "AVPlayer reset");
    }

    public void destroy() {
        reset();
        _mediaPlayer.setOnCompletionListener(null);
        _mediaPlayer.setOnPreparedListener(null);
        _mediaPlayer.release();

        Log.i(TAG, "AV player destroyed");
    }

    public void play() {
        if (_state == State.PREPARED || _state == State.PAUSED) {
            _mediaPlayer.start();
            _state = State.STARTED;
        }
        else {
            Log.w(TAG, "AVPlayer could not play video in " + _state.toString() + " state");
        }
    }

    public void pause() {
        if (_state == State.STARTED) {
            _mediaPlayer.pause();
            _state = State.PAUSED;
        }
        else {
            Log.w(TAG, "AVPlayer could not pause video in " + _state.toString() + " state");
        }
    }

    public boolean isPaused() {
        return !_mediaPlayer.isPlaying();
    }

    public void setLoop(boolean loop) {
        _loop = loop;
    }

    public void setVolume(float volume) {
        _volume = volume;
        _mediaPlayer.setVolume(_volume, _volume);
    }

    public void setMuted(boolean muted) {
        if (muted) {
            _mediaPlayer.setVolume(0, 0);
        }
        else {
            _mediaPlayer.setVolume(_volume, _volume);
        }
    }

    public void seekToTime(int seconds) {
        if (_state == State.IDLE) {
            Log.w(TAG, "AVPlayer could not seek while in IDLE state");
            return;
        }

        _mediaPlayer.seekTo(seconds * 1000);
    }

    public int getCurrentTimeInSeconds() {
        if (_state == State.IDLE) {
            Log.w(TAG, "AVPlayer " + this + " could not get current time in IDLE state");
            return 0;
        }

        return _mediaPlayer.getCurrentPosition() / 1000;
    }

    public int getVideoDurationInSeconds(){
        if (_state == State.IDLE) {
            Log.w(TAG, "AVPlayer could not get video duration in IDLE state");
            return 0;
        }

        return _mediaPlayer.getDuration() / 1000;
    }

    /**
     * Native Callbacks
     */
    private native void nativeOnFinished(long ref);
    private native void nativeOnPrepared(long ref);
}

