package com.viro.renderer;

/**
 * Created by radvani on 11/18/16.
 */

import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.util.Log;
import android.view.Surface;

import java.io.FileDescriptor;
import java.io.IOException;

/**
 * Wraps a {@link android.media.MediaPlayer} and can be controlled
 * via JNI.
 */
public class AVPlayer {

    private MediaPlayer _mediaPlayer;

    public AVPlayer() {
        _mediaPlayer = new MediaPlayer();


    }

    public boolean setDataSourceURL(String pathOrURL) {
        try {
            _mediaPlayer.setDataSource(pathOrURL);
            _mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            _mediaPlayer.prepare();

            return true;
        }catch(IOException e) {
            Log.w("Viro", "Failed to load path [" + pathOrURL + "] in AV Player!");
            return false;
        }
    }

    public boolean setDataSourceAsset(String asset, AssetManager assetManager) {
        try {
            AssetFileDescriptor afd = assetManager.openFd(asset);
            _mediaPlayer.setDataSource(afd);
            _mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            _mediaPlayer.prepare();

            return true;
        }catch(IOException e) {
            Log.w("Viro", "Failed to load asset [" + asset + "] in AV Player!");
            return false;
        }
    }

    public void setVideoSink(Surface videoSink) {
        _mediaPlayer.setSurface(videoSink);
    }

    public void stop() {
        _mediaPlayer.stop();
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
        _mediaPlayer.setVolume(volume, volume);
    }

    public void seekTo(int seconds) {
        _mediaPlayer.seekTo(seconds * 1000);
    }
}
