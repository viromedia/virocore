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

import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Sound;
import com.viro.renderer.jni.Text;
import com.viro.renderer.jni.Vector;

import org.junit.Test;

import java.io.File;

/**
 * Created by manish on 11/6/17.
 */

public class ViroSoundTest extends ViroBaseTest {
    private Sound mSound;
    private Text mDelegateText;

    @Override
    void configureTestScene() {
        mDelegateText = new Text(mViroView.getViroContext(), "Delegate text", "Roboto", 25, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final Node textNode = new Node();
        textNode.setPosition(new Vector(-1.5f, 1f, -3.3f));
        textNode.setGeometry(mDelegateText);
        mScene.getRootNode().addChildNode(textNode);
        mSound = new Sound(mViroView.getViroContext(), Uri.fromFile(new File("file:///android_asset/metronome.mp3")), new Sound.Delegate() {
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
        assertPass("Sound check");

    }
}
