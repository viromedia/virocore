/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.core;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.viro.core.internal.ExecutableAnimation;

import java.lang.ref.WeakReference;

/**
 * Animation represents a set of skeletal or keyframe animations that can be executed. These
 * animations are techniques for animating complex geometries; for example, to make a humanoid
 * character walk. They are typically authored in 3D graphics software, and exported as
 * FBX files, which can then be loaded into Viro via {@link Object3D}. To retrieve an Animation,
 * use {@link Node#getAnimation(String)}.
 */
public class Animation {

    private static final String TAG = "Viro";

    /**
     * Represents the playback state of an Animation.
     */
    public enum State {
        /**
         * The animation is scheduled to run. It has been played, but is waiting for its
         * <tt>delay</tt> time to expire.
         */
        SCHEDULED,
        /**
         * The animation is currently running.
         */
        RUNNING,
        /**
         * The animation is paused.
         */
        PAUSED,
        /**
         * The animation is not running, and is not scheduled.
         */
        STOPPED
    }

    /**
     * Callback interface to respond to {@link Animation} events.
     */
    public interface Delegate {

        /**
         * Invoked when the {@link Animation} starts. The Animation starts after {@link #play()} is
         * invoked, and after the start <tt>delay</tt> expires. If the Animation is looping, this is
         * called at the start of each loop.
         *
         * @param animation The Animation that started.
         */
        public void onAnimationStart(Animation animation);

        /**
         * Invoked when the {@link Animation} finishes. The Animation finishes when its playback
         * has completed. If the Animation is looping, this is called after each loop completes.
         * This callback is also triggered if an animation is terminated.
         *
         * @param animation The animation that ended.
         * @param canceled  True if the animation was ended via {@link #stop()}, false if the
         *                  animation ran to completion and ended naturally.
         */
        public void onAnimationFinish(Animation animation, boolean canceled);
    }

    private Node mNode;
    private ExecutableAnimation mExecutableAnimation;
    private long mDelayInMilliseconds = 0;
    private boolean mLoop = false;
    private State mState = State.STOPPED;
    private Delegate mDelegate;

    /*
     * This is a handler to the main thread where we queue up our delayed runner
     */
    private Handler mMainLoopHandler = new Handler(Looper.getMainLooper());

    /*
     * This runnable is what we use to start animations with a delay.
     */
    private Runnable mDelayedRunner = new Runnable() {
        @Override
        public void run() {
            startAnimation();
        }
    };

    Animation(ExecutableAnimation animation, Node node) {
        mExecutableAnimation = animation;
        mNode = node;
    }

    /**
     * Set to true to make the Animation automatically loop to the beginning when playback finishes.
     *
     * @param loop True to loop.
     */
    public void setLoop(boolean loop) {
        mLoop = loop;
    }

    /**
     * Return true if the Animation is currently set to loop after playback.
     *
     * @return True if loop is enabled.
     */
    public boolean getLoop() {
        return mLoop;
    }

    /**
     * Set the delay, in milliseconds, that the Animation will wait before running after {@link #play()}
     * is invoked.
     *
     * @param delay The delay in milliseconds.
     */
    public void setDelay(long delay) {
        mDelayInMilliseconds = delay;
    }

    /**
     * Get the delay, in millieconds, that the Animation will wait before running.
     *
     * @return The delay in milliseconds.
     */
    public long getDelay() {
        return mDelayInMilliseconds;
    }

    /**
     * Set the {@link Delegate}, which can be used to respond to Animation playback
     * events.
     *
     * @param delegate The delegate to use for this Animation.
     */
    public void setDelegate(Delegate delegate) {
        mDelegate = delegate;
    }

    /**
     * Get the {@link Delegate} used to receive callbacks for this Animation.
     *
     * @return The delegate, or null if none is attached.
     */
    public Delegate getDelegate() {
        return mDelegate;
    }

    /**
     * Start a {@link State#STOPPED} Animation or resume a {@link State#PAUSED} Animation. If the
     * Animation is paused, then it will immediately resume. If the Animation is {@link
     * State#STOPPED}, then it will first wait <tt>delay</tt> milliseconds before starting.
     */
    public void play() {
        if (mState == State.PAUSED) {
            mExecutableAnimation.resume();
            mState = State.RUNNING;
        }
        else if (mState == State.STOPPED) {
            mState = State.SCHEDULED;
            if(mDelayInMilliseconds <= 0) {
                startAnimation();
            }
            else {
                mMainLoopHandler.postDelayed(mDelayedRunner, (long) mDelayInMilliseconds);
            }
        }
        else {
            Log.w(TAG, "Unable to play animation in state " + mState.name());
        }
    }

    /**
     * Pauses a {@link State#RUNNING} Animation or terminates a {@link State#SCHEDULED} Animation.
     * The Animation can be resumed by invoking {@link #play()}. If the Animation was scheduled --
     * meaning it was waiting for its <tt>delay</tt> period to expire -- then this function will
     * stop the animation.
     */
    public void pause() {
        if (mState == State.RUNNING) {
            mExecutableAnimation.pause();
            mState = State.PAUSED;
        }
        else if (mState == State.SCHEDULED) {
            mState = State.STOPPED;
            mMainLoopHandler.removeCallbacks(mDelayedRunner);
        }
    }

    /**
     * Terminates any {@link State#SCHEDULED}, {@link State#RUNNING} or {@link State#PAUSED} Animation.
     */
    public void stop() {
        if (mState == State.SCHEDULED) {
            mMainLoopHandler.removeCallbacks(mDelayedRunner);
        }
        else if (mState == State.RUNNING || mState == State.PAUSED) {
            if (mExecutableAnimation != null) {
                mExecutableAnimation.terminate();
            }
        }
        mState = State.STOPPED;
    }

    /*
     * This method starts a scheduled animation
     */
    private void startAnimation() {
        if (mState != State.SCHEDULED) {
            Log.i(TAG, "Aborted starting new animation, was no longer scheduled");
            mState = State.STOPPED;
            return;
        }
        onStartAnimation();

        final WeakReference<Animation> weakSelf = new WeakReference<>(this);
        mExecutableAnimation.execute(mNode, new ExecutableAnimation.AnimationDelegate() {
            @Override
            public void onFinish(ExecutableAnimation animation) {
                Animation self = weakSelf.get();
                if (self != null) {
                    self.onFinishAnimation(animation);
                }
            }
        });
        mState = State.RUNNING;
    }

    /*
     * This method is called when an animation starts. It notifies the delegate.
     */
    private void onStartAnimation() {
        if (mDelegate != null) {
            mDelegate.onAnimationStart(this);
        }
    }

    /*
     * This method is called by the JNI when the animation ends. It notifies the delegate and
     * handles looping logic.
     */
    private void onFinishAnimation(ExecutableAnimation animation) {
        if (mDelegate != null) {
            mDelegate.onAnimationFinish(this, false);
        }

        mState = State.SCHEDULED;
        if (mLoop) {
            startAnimation();
        }
    }
}
