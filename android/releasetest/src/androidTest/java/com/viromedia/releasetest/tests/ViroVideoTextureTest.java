/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.net.Uri;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.AnimationTimingFunction;
import com.viro.renderer.jni.AnimationTransaction;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Text;
import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.Vector;
import com.viro.renderer.jni.VideoTexture;

import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

/**
 * Created by manish on 11/8/17.
 */

public class ViroVideoTextureTest extends ViroBaseTest {
    private VideoTexture mVideoTexture;
    private Box mBox;
    private Node mBoxNode;
    private Text mDelegateText;

    @Override
    void configureTestScene() {
        final AmbientLight ambientLight = new AmbientLight(Color.WHITE, 300);
        mScene.getRootNode().addLight(ambientLight);


        runOnUiThread(() -> {
            final VideoTexture.Delegate delegate = new VideoTexture.Delegate() {
                private static final String TAG = "VideoDelegate: ";

                @Override
                public void onVideoBufferStart(final VideoTexture video) {
                    mDelegateText.setText(TAG + "onVideoBufferStart");
                }

                @Override
                public void onVideoBufferEnd(final VideoTexture video) {
                    mDelegateText.setText(TAG + "onVideoBufferEnd");
                }

                @Override
                public void onVideoFinish(final VideoTexture video) {
                    mDelegateText.setText(TAG + "onVideoFinish");
                }

                @Override
                public void onReady(final VideoTexture video) {
                    mDelegateText.setText(TAG + "onReady");
                    video.play();
                }

                @Override
                public void onVideoFailed(final String error) {
                    mDelegateText.setText(TAG + "onVideoFailed");
                }

                @Override
                public void onVideoUpdatedTime(final VideoTexture video, final float seconds, final float totalDuration) {
                    mDelegateText.setText(TAG + "onVideoUpdatedTime seconds = " + seconds
                            + ", totalDuration = " + totalDuration);
                }
            };

            mVideoTexture = new VideoTexture(mViroView.getViroContext(),
                    Uri.parse("file:///android_asset/stereoVid360.mp4"), delegate, Texture.StereoMode.TOP_BOTTOM);
        });
        mVideoTexture.setVolume(1);
        mVideoTexture.setLoop(true);
        final Material videoMaterial = new Material();
        videoMaterial.setDiffuseTexture(mVideoTexture);
        mScene.setBackgroundTexture(mVideoTexture);

        mBoxNode = new Node();
        mBox = new Box(2, 2, 2);
        final Material boxMaterial = new Material();
        boxMaterial.setLightingModel(Material.LightingModel.BLINN);

        mBox.setMaterials(Arrays.asList(videoMaterial, boxMaterial));
        mBoxNode.setGeometry(mBox);
        mBoxNode.setPosition(new Vector(0, -2.5f, -3.3f));
        mBoxNode.setRotation(new Vector(0.78f, 0.78f, 0));

        mScene.getRootNode().addChildNode(mBoxNode);
        animateBox();
        mDelegateText = new Text(mViroView.getViroContext(), "Delegate text", "Roboto", 25,
                Color.WHITE, 3f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final Node textNode = new Node();
        textNode.setPosition(new Vector(0f, 1f, -3.3f));
        textNode.setGeometry(mDelegateText);
        mScene.getRootNode().addChildNode(textNode);
    }

    @Test
    public void testVideoTexture() {
        testPlayPause();
        testSetVolume();
        testSetMuted();
        testSetLoop();
        testSeekToTime();
        testSetDelegate();
        testSetDelegate();
        testSetRotation();
    }

    private void animateBox() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(30000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        mBoxNode.setRotation(new Vector(0, 6.28, 0));
        AnimationTransaction.commit();

    }

    private void testPlayPause() {

        mMutableTestMethod = () -> {
            if (mVideoTexture.isPlaying()) {
                mVideoTexture.pause();
                mDelegateText.setText("Video Paused");
            } else {
                mVideoTexture.play();
                mDelegateText.setText("Video Playing");
            }
        };

        assertPass("Toggling video play / pause", () -> {
            mVideoTexture.play();
            animateBox();
        });
    }

    private void testSetVolume() {
        mVideoTexture.setVolume(0);

        mMutableTestMethod = () -> {
            mDelegateText.setText("Video volume: " + mVideoTexture.getVolume());
            mVideoTexture.setVolume((mVideoTexture.getVolume() + 0.1f) % 1);
        };
        assertPass("Increasing volume by +1 every second", () -> {
            mVideoTexture.setVolume(1);
            animateBox();
        });
    }

    private void testSetMuted() {
        mMutableTestMethod = () -> {
            mDelegateText.setText("Video muted: " + mVideoTexture.isMuted() + "\n This should toggle, TODO BUG VIRO-2192 ");
            mVideoTexture.setMuted(!mVideoTexture.isMuted());
        };
        assertPass("Toggling mute / unmute every second", () -> {
            mVideoTexture.setMuted(false);
            animateBox();
        });
    }

    private void testSetLoop() {
        mVideoTexture.setLoop(false);
        mVideoTexture.seekToTime(0);
        mDelegateText.setText("video loop: " + mVideoTexture.getLoop());

        assertPass("Looping == false, confirm video does not loop after it finishes", () -> {
            mVideoTexture.setLoop(true);
            animateBox();
        });
    }

    private void testSeekToTime() {
        final List<Double> seekTimes = Arrays.asList(10.0, 15.0, 20.0, 25.0);

        mMutableTestMethod = () -> {
            final Random rand = new Random();
            final Double seekToTime = seekTimes.get(rand.nextInt(seekTimes.size()));
            mDelegateText.setText("Video seekToTime: " + seekToTime);
            mVideoTexture.seekToTime(seekToTime.floatValue());
        };
        assertPass("Seek to random times", () -> {
            mVideoTexture.seekToTime(0);
            animateBox();
        });
    }

    private void testSetDelegate() {
        final VideoTexture.Delegate delegate1 = new VideoTexture.Delegate() {
            private static final String TAG = "VideoDelegate1: ";

            @Override
            public void onVideoBufferStart(final VideoTexture video) {
                mDelegateText.setText(TAG + "onVideoBufferStart");
            }

            @Override
            public void onVideoBufferEnd(final VideoTexture video) {
                mDelegateText.setText(TAG + "onVideoBufferEnd");
            }

            @Override
            public void onVideoFinish(final VideoTexture video) {
                mDelegateText.setText(TAG + "onVideoFinish");
            }

            @Override
            public void onReady(final VideoTexture video) {
                mDelegateText.setText(TAG + "onReady");
            }

            @Override
            public void onVideoFailed(final String error) {
                mDelegateText.setText(TAG + "onVideoFailed");
            }

            @Override
            public void onVideoUpdatedTime(final VideoTexture video, final float seconds, final float totalDuration) {
                mDelegateText.setText(TAG + "onVideoUpdatedTime seconds = " + seconds
                        + ", totalDuration = " + totalDuration);
            }
        };

        final VideoTexture.Delegate delegate2 = new VideoTexture.Delegate() {
            private static final String TAG = "VideoDelegate2: ";

            @Override
            public void onVideoBufferStart(final VideoTexture video) {
                mDelegateText.setText(TAG + "onVideoBufferStart");
            }

            @Override
            public void onVideoBufferEnd(final VideoTexture video) {
                mDelegateText.setText(TAG + "onVideoBufferEnd");
            }

            @Override
            public void onVideoFinish(final VideoTexture video) {
                mDelegateText.setText(TAG + "onVideoFinish");
            }

            @Override
            public void onReady(final VideoTexture video) {
                mDelegateText.setText(TAG + "onReady");
            }

            @Override
            public void onVideoFailed(final String error) {
                mDelegateText.setText(TAG + "onVideoFailed");
            }

            @Override
            public void onVideoUpdatedTime(final VideoTexture video, final float seconds, final float totalDuration) {
                mDelegateText.setText(TAG + "onVideoUpdatedTime seconds = " + seconds
                        + ", totalDuration = " + totalDuration);
            }
        };

        final List<VideoTexture.Delegate> delegates = Arrays.asList(delegate1, delegate2);
        final Iterator<VideoTexture.Delegate> itr = Iterables.cycle(delegates).iterator();

        mMutableTestMethod = () -> {

            mVideoTexture.setDelegate(itr.next());
        };

        assertPass("Toggling between two delegates", () -> {
            animateBox();
        });
    }

    private void testSetRotation() {
        final List<Vector> rotationVectors = Arrays.asList(new Vector(0, 1.57, 0),
                new Vector(0, 3.14, 0), new Vector(0, 4.712, 0));
        final Iterator<Vector> itr = Iterables.cycle(rotationVectors).iterator();
        mMutableTestMethod = () -> {
            mScene.setBackgroundRotation(itr.next());
            mDelegateText.setText("setBackGroundRotation with 90 degrees / 3.14 radians along Y-axis");
        };

        assertPass("VideoTexture seems to rotate correctly");

    }
}