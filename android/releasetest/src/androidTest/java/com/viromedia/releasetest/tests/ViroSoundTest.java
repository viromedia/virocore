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

import com.viro.core.Node;
import com.viro.core.Sound;
import com.viro.core.Text;
import com.viro.core.Vector;
import com.viro.core.ViroContext;

import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

/**
 * Created by manish on 11/6/17.
 */

public class ViroSoundTest extends ViroBaseTest {
    private Sound mSound;
    private Text mDelegateText;

    @Override
    void configureTestScene() {
        mDelegateText = new Text(mViroView.getViroContext(), "Delegate text", "Roboto", 25,
                Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final Node textNode = new Node();
        textNode.setPosition(new Vector(0f, 1f, -3.3f));
        textNode.setGeometry(mDelegateText);
        mScene.getRootNode().addChildNode(textNode);

        final ViroContext context = mViroView.getViroContext();
        if (mSound == null) {
            mSound = new Sound(mViroView.getViroContext(), Uri.parse("file:///android_asset/mpthreetest.mp3"), null);
        }

        mSound.setLoop(true);
        mSound.setVolume(1);
        mSound.setMuted(false);

        mSound.setPlaybackListener(new Sound.PlaybackListener() {
            @Override
            public void onSoundReady(final Sound sound) {
                mDelegateText.setText("onSoundReady called. Playing song");
                sound.play();
            }

            @Override
            public void onSoundFinish(final Sound sound) {
                mDelegateText.setText("onSoundFinish called");
            }

            @Override
            public void onSoundFail(final String error) {
                mDelegateText.setText("onSoundFail called");
            }
        });

    }

    @Test
    public void testSound() {
        runUITest(() -> testPlayPause());
        runUITest(() -> testSetVolume());
        runUITest(() -> testSetMuted());
        runUITest(() -> testSetLoop());
        runUITest(() -> testSeekToTime());
        runUITest(() -> testSetDelegate());
    }

    private void testPlayPause() {

        mMutableTestMethod = () -> {
            if (mSound.isPlaying()) {
                mSound.pause();
                mDelegateText.setText("Sound Paused");
            } else {
                mSound.play();
                mDelegateText.setText("Sound Playing");
            }
        };

        assertPass("Toggling sound play / pause");
    }

    private void testSetVolume() {
        mSound.play();
        mSound.setVolume(0);

        mMutableTestMethod = () -> {
            mDelegateText.setText("Sound volume: " + mSound.getVolume());
            mSound.setVolume((mSound.getVolume() + 0.1f) % 1);
        };
        assertPass("Increasing volume by +1 every second");
    }

    private void testSetMuted() {
        mMutableTestMethod = () -> {
            mDelegateText.setText("Sound muted: " + mSound.isMuted());
            mSound.setMuted(!mSound.isMuted());
        };
        assertPass("Toggling mute / unmute every second");
    }

    private void testSetLoop() {
        mSound.setLoop(false);
        mSound.seekToTime(0);
        mDelegateText.setText("Sound loop: " + mSound.getLoop());

        assertPass("Looping == false, confirm it does not loop after it finishes");
    }

    private void testSeekToTime() {
        final List<Double> seekTimes = Arrays.asList(10.0, 15.0, 20.0, 25.0);

        mMutableTestMethod = () -> {
            final Random rand = new Random();
            final Double seekToTime = seekTimes.get(rand.nextInt(seekTimes.size()));
            mDelegateText.setText("Sound seekToTime: " + seekToTime);
            mSound.seekToTime(seekToTime.floatValue());
        };
        assertPass("Seek to random times");
    }

    // TODO VIRO-2181 setting null delegate will cause an NPE
    private void testSetDelegate() {
        final Sound.PlaybackListener delegate1 = new Sound.PlaybackListener() {
            @Override
            public void onSoundReady(final Sound sound) {
                mDelegateText.setText("DELEGATE 1 onSoundReady called. Playing song");
                mSound.play();
            }

            @Override
            public void onSoundFinish(final Sound sound) {
                mDelegateText.setText("DELEGATE 1 onSoundFinish called. Playing song");

            }

            @Override
            public void onSoundFail(final String error) {
                mDelegateText.setText("DELEGATE 1 onSoundFail called. Playing song");
            }
        };

        final Sound.PlaybackListener delegate2 = new Sound.PlaybackListener() {
            @Override
            public void onSoundReady(final Sound sound) {
                mDelegateText.setText("DELEGATE 2 onSoundReady called. Playing song");
                mSound.play();
            }

            @Override
            public void onSoundFinish(final Sound sound) {
                mDelegateText.setText("DELEGATE 2 onSoundFinish called. Playing song");

            }

            @Override
            public void onSoundFail(final String error) {
                mDelegateText.setText("DELEGATE 2 onSoundFail called. Playing song");
            }
        };

        final List<Sound.PlaybackListener> delegates = Arrays.asList(delegate1, delegate2);
        final Iterator<Sound.PlaybackListener> itr = Iterables.cycle(delegates).iterator();

        mMutableTestMethod = () -> {

            mSound.setPlaybackListener(itr.next());
        };

        assertPass("Toggling between two delegates");
    }
}
