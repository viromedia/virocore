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
    private MediaPlayer _mediaPlayer;
    private float _volume;
    private long mNativeReference;

    public AVPlayer(long nativeReference) {
        _mediaPlayer = new MediaPlayer();
        _volume = 1.0f;
        mNativeReference = nativeReference;

        // Attach listeners to be called back into native
        _mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mediaPlayer) {
                nativeOnFinished(mNativeReference);
            }
        });

        _mediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mp) {
                nativeOnPrepared(mNativeReference);
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
                _mediaPlayer.setDataSource(resourceOrURL);
            }
            _mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            _mediaPlayer.prepare();

            return true;
        }catch(IOException e) {
            Log.w("Viro", "Failed to load path [" + resourceOrURL + "] in AV Player!", e);
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
            } else {
                // TODO: Figure out how to setDataSource for API Level < 24
            }

            return true;
        }catch(IOException e) {
            Log.w("Viro", "Failed to load asset [" + asset + "] in AV Player!");
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
    }

    public void play() {
        _mediaPlayer.start();
    }

    public void pause() {
        _mediaPlayer.pause();
    }

    public boolean isPaused() {
        return !_mediaPlayer.isPlaying();
    }

    public void setLoop(boolean loop) {
        _mediaPlayer.setLooping(loop);
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

    public void seekToTime(float seconds) {
        _mediaPlayer.seekTo((int) (seconds * 1000));
    }

    /**
     * Native Callbacks
     */
    private native void nativeOnFinished(long ref);
    private native void nativeOnPrepared(long ref);
}

