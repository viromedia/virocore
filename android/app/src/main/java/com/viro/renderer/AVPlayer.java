/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.net.Uri;
import android.util.Log;
import android.view.Surface;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.DefaultLoadControl;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayer;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.audio.AudioRendererEventListener;
import com.google.android.exoplayer2.decoder.DecoderCounters;
import com.google.android.exoplayer2.extractor.DefaultExtractorsFactory;
import com.google.android.exoplayer2.extractor.ExtractorsFactory;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.trackselection.AdaptiveVideoTrackSelection;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.trackselection.TrackSelectionArray;
import com.google.android.exoplayer2.upstream.DataSource;
import com.google.android.exoplayer2.upstream.DefaultBandwidthMeter;
import com.google.android.exoplayer2.upstream.DefaultDataSourceFactory;
import com.google.android.exoplayer2.upstream.RawResourceDataSource;
import com.google.android.exoplayer2.util.Util;

import java.io.IOException;

/**
 * Wraps the Android ExoPlayer and can be controlled via JNI.
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

    private SimpleExoPlayer mExoPlayer;
    private float mVolume;
    private long mNativeReference;
    private boolean mLoop;
    private State mState;
    private boolean mMute;
    private int mPrevExoPlayerState = -1;
    private boolean mWasBuffering = false;

    public AVPlayer(long nativeReference, Context context) {
        mVolume = 1.0f;
        mNativeReference = nativeReference;
        mLoop = false;
        mState = State.IDLE;
        mMute = false;

        TrackSelection.Factory trackSelectionFactory = new AdaptiveVideoTrackSelection.Factory(new DefaultBandwidthMeter());
        DefaultTrackSelector trackSelector = new DefaultTrackSelector(trackSelectionFactory);
        mExoPlayer = ExoPlayerFactory.newSimpleInstance(context, trackSelector, new DefaultLoadControl());

        mExoPlayer.setAudioDebugListener(new AudioRendererEventListener() {
            @Override
            public void onAudioEnabled(DecoderCounters counters) {
            }
            @Override
            public void onAudioSessionId(int audioSessionId) {
            }
            @Override
            public void onAudioDecoderInitialized(String decoderName, long initializedTimestampMs, long initializationDurationMs) {
                Log.i(TAG, "AVPlayer audio decoder initialized " + decoderName);
            }
            @Override
            public void onAudioInputFormatChanged(Format format) {
                Log.i(TAG, "AVPlayer audio input format changed to " + format);
            }
            @Override
            public void onAudioTrackUnderrun(int bufferSize, long bufferSizeMs, long elapsedSinceLastFeedMs) {
            }
            @Override
            public void onAudioDisabled(DecoderCounters counters) {
            }
        });
        mExoPlayer.addListener(new ExoPlayer.EventListener() {
            @Override
            public void onTimelineChanged(Timeline timeline, Object manifest) {
            }
            @Override
            public void onTracksChanged(TrackGroupArray trackGroups, TrackSelectionArray trackSelections) {
            }
            @Override
            public void onLoadingChanged(boolean isLoading) {
            }

            @Override
            public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
                // this function sometimes gets called back w/ the same playbackState.
                if (mPrevExoPlayerState == playbackState) {
                    return;
                }
                mPrevExoPlayerState = playbackState;
                switch (playbackState) {
                    case ExoPlayer.STATE_BUFFERING:
                        if (!mWasBuffering) {
                            nativeWillBuffer(mNativeReference);
                            mWasBuffering = true;
                        }
                        break;
                    case ExoPlayer.STATE_READY:
                        if (mWasBuffering) {
                            nativeDidBuffer(mNativeReference);
                            mWasBuffering = false;
                        }
                        break;
                    case ExoPlayer.STATE_ENDED:
                        if (mLoop) {
                            mExoPlayer.seekToDefaultPosition();
                        }
                        nativeOnFinished(mNativeReference);
                        break;
                }
            }

            @Override
            public void onPlayerError(ExoPlaybackException error) {
                Log.w(TAG, "AVPlayer encountered error [" + error + "]", error);

                String message = null;
                if (error.type == ExoPlaybackException.TYPE_RENDERER) {
                    message = error.getRendererException().getLocalizedMessage();
                }
                else if (error.type == ExoPlaybackException.TYPE_SOURCE) {
                    message = error.getSourceException().getLocalizedMessage();
                }
                else if (error.type == ExoPlaybackException.TYPE_UNEXPECTED) {
                    message = error.getUnexpectedException().getLocalizedMessage();
                }
                nativeOnError(mNativeReference, message);
            }
            @Override
            public void onPositionDiscontinuity() {
            }
        });
    }

    public boolean setDataSourceURL(String resourceOrURL, final Context context) {
        try {
            reset();

            Uri uri = Uri.parse(resourceOrURL);
            DataSource.Factory dataSourceFactory;
            ExtractorsFactory extractorsFactory = new DefaultExtractorsFactory();
            if (resourceOrURL.startsWith("res")) {
                // the uri we get is in the form res:/#######, so we want the path
                // which is `/#######`, and the id is the path minus the first char
                int id = Integer.parseInt(uri.getPath().substring(1));
                uri = RawResourceDataSource.buildRawResourceUri(id);
                dataSourceFactory = new DataSource.Factory() {
                    @Override
                    public DataSource createDataSource() {
                        return new RawResourceDataSource(context, null);
                    }
                };
            } else {
                dataSourceFactory = new DefaultDataSourceFactory(context,
                        Util.getUserAgent(context, "ViroAVPlayer"), new DefaultBandwidthMeter());
            }
            Log.i(TAG, "AVPlayer setting URL to [" + uri + "]");

            MediaSource mediaSource = new ExtractorMediaSource(uri, dataSourceFactory, extractorsFactory,
                    null, null);

            mExoPlayer.prepare(mediaSource);
            mExoPlayer.seekToDefaultPosition();
            mState = State.PREPARED;

            Log.i(TAG, "AVPlayer prepared for playback");
            nativeOnPrepared(mNativeReference);

            return true;
        }catch(Exception e) {
            Log.w(TAG, "AVPlayer failed to load video at URL [" + resourceOrURL + "]", e);
            reset();

            return false;
        }
    }

    public void setVideoSink(Surface videoSink) {
        mExoPlayer.setVideoSurface(videoSink);
    }

    public void reset() {
        mExoPlayer.stop();
        mExoPlayer.seekToDefaultPosition();
        mState = State.IDLE;

        Log.i(TAG, "AVPlayer reset");
    }

    public void destroy() {
        reset();
        mExoPlayer.release();

        Log.i(TAG, "AVPlayer destroyed");
    }

    public void play() {
        if (mState == State.PREPARED || mState == State.PAUSED) {
            mExoPlayer.setPlayWhenReady(true);
            mState = State.STARTED;
        }
        else {
            Log.w(TAG, "AVPlayer could not play video in " + mState.toString() + " state");
        }
    }

    public void pause() {
        if (mState == State.STARTED) {
            mExoPlayer.setPlayWhenReady(false);
            mState = State.PAUSED;
        }
        else {
            Log.w(TAG, "AVPlayer could not pause video in " + mState.toString() + " state");
        }
    }

    public boolean isPaused() {
        return mState != State.STARTED;
    }

    public void setLoop(boolean loop) {
        mLoop = loop;
        if (mExoPlayer.getPlaybackState() == ExoPlayer.STATE_ENDED){
            mExoPlayer.seekToDefaultPosition();
        }
    }

    public void setVolume(float volume) {
        mVolume = volume;
        if (!mMute){
            mExoPlayer.setVolume(mVolume);
        }
    }

    public void setMuted(boolean muted) {
        mMute = muted;
        if (muted) {
            mExoPlayer.setVolume(0);
        }
        else {
            mExoPlayer.setVolume(mVolume);
        }
    }

    public void seekToTime(int seconds) {
        if (mState == State.IDLE) {
            Log.w(TAG, "AVPlayer could not seek while in IDLE state");
            return;
        }

        mExoPlayer.seekTo(seconds * 1000);
    }

    public int getCurrentTimeInSeconds() {
        if (mState == State.IDLE) {
            Log.w(TAG, "AVPlayer could not get current time in IDLE state");
            return 0;
        }

        return (int) (mExoPlayer.getCurrentPosition() / 1000);
    }

    public int getVideoDurationInSeconds() {
        if (mState == State.IDLE) {
            Log.w(TAG, "AVPlayer could not get video duration in IDLE state");
            return 0;
        } else if (mExoPlayer.getDuration() == C.TIME_UNSET) {
            return 0;
        }

        return (int) (mExoPlayer.getDuration() / 1000);
    }

    /**
     * Native Callbacks
     */
    private native void nativeOnPrepared(long ref);
    private native void nativeOnFinished(long ref);
    private native void nativeWillBuffer(long ref);
    private native void nativeDidBuffer(long ref);
    private native void nativeOnError(long ref, String error);
}

