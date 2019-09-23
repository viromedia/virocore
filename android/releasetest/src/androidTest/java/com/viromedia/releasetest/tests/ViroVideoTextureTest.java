//
//  Copyright (c) 2017-present, ViroMedia, Inc.
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

package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.net.Uri;
import android.support.test.espresso.core.deps.guava.collect.Iterables;
import android.util.Log;

import com.viro.core.AmbientLight;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.VideoTexture;

import org.junit.Test;

import java.util.ArrayList;
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


        mDelegateText = new Text(mViroView.getViroContext(), "Delegate text", "Roboto", 25,
                Color.WHITE, 3f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final Node textNode = new Node();
        textNode.setPosition(new Vector(0f, 1f, -3.3f));
        textNode.setGeometry(mDelegateText);
        mScene.getRootNode().addChildNode(textNode);

        final VideoTexture.PlaybackListener delegate = new VideoTexture.PlaybackListener() {
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
        mVideoTexture.setVolume(1);
        mVideoTexture.setMuted(false);
        mVideoTexture.setLoop(true);
        mVideoTexture.play();
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
    }

    @Test
    public void testVideoTexture() {
        runUITest(() -> testPlayPause());
        runUITest(() -> testSetVolume());
        runUITest(() -> testSetMuted());
        runUITest(() -> testSetLoop());
        runUITest(() -> testSeekToTime());
        runUITest(() -> testSetDelegate());
        runUITest(() -> testSetDelegate());
        runUITest(() -> testSetRotation());
    }

    private void animateBox() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(30000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        mBoxNode.setRotation(new Vector(0, 6.28, 0));
        AnimationTransaction.commit();

    }

    private void testPlayPause() {
        mVideoTexture.play();
        final List<Integer> intArray = new ArrayList<Integer>();
        intArray.add(new Integer(0));
        mMutableTestMethod = () -> {
            if (mVideoTexture.isPlaying()) {
                Log.i("ViroVideoTexture", "isPausing!!");
                if(intArray.get(0) > 5) {
                    mVideoTexture.pause();
                    mDelegateText.setText("Video Paused");
                    Log.i("ViroVideoTexture", "Video is pausing!!");
                }
                int newCount = intArray.get(0);
                newCount++;
                intArray.clear();
                intArray.add(newCount);
            } else {
                mVideoTexture.play();
                mDelegateText.setText("Video Playing");
            }
        };

        assertPass("Toggling video play / pause");
    }

    private void testSetVolume() {
        mVideoTexture.setVolume(0);

        mMutableTestMethod = () -> {
            mDelegateText.setText("Video volume: " + mVideoTexture.getVolume());
            mVideoTexture.setVolume((mVideoTexture.getVolume() + 0.1f) % 1);
        };
        assertPass("Increasing volume by +1 every second");
    }

    private void testSetMuted() {
        mMutableTestMethod = () -> {
            mDelegateText.setText("Video muted: " + mVideoTexture.isMuted() + "\n This should toggle, TODO BUG VIRO-2192 ");
            mVideoTexture.setMuted(!mVideoTexture.isMuted());
        };
        assertPass("Toggling mute / unmute every second");
    }

    private void testSetLoop() {
        mVideoTexture.setLoop(false);
        mVideoTexture.seekToTime(0);
        mDelegateText.setText("video loop: " + mVideoTexture.getLoop());

        assertPass("Looping == false, confirm video does not loop after it finishes");
    }

    private void testSeekToTime() {
        final List<Double> seekTimes = Arrays.asList(10.0, 15.0, 20.0, 25.0);

        mMutableTestMethod = () -> {
            final Random rand = new Random();
            final Double seekToTime = seekTimes.get(rand.nextInt(seekTimes.size()));
            mDelegateText.setText("Video seekToTime: " + seekToTime);
            mVideoTexture.seekToTime(seekToTime.floatValue());
        };
        assertPass("Seek to random times");
    }

    private void testSetDelegate() {
        final VideoTexture.PlaybackListener delegate1 = new VideoTexture.PlaybackListener() {
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

        final VideoTexture.PlaybackListener delegate2 = new VideoTexture.PlaybackListener() {
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

        final List<VideoTexture.PlaybackListener> delegates = Arrays.asList(delegate1, delegate2);
        final Iterator<VideoTexture.PlaybackListener> itr = Iterables.cycle(delegates).iterator();

        mMutableTestMethod = () -> {

            mVideoTexture.setPlaybackListener(itr.next());
        };

        assertPass("Toggling between two delegates");
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