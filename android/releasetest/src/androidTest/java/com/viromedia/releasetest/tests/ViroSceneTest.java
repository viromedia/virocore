/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.Vector;
import com.viro.renderer.jni.VideoTexture;

import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Created by vadvani on 11/6/17.
 *
 * Tests all scene methods except setSoundRoom, addParticleEmitter, and removeParticleEmitter.
 */

public class ViroSceneTest extends ViroBaseTest {

    @Override
    void configureTestScene() {
        AmbientLight light = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(light);
    }

    @Test
    public void sceneTest() {
        testSceneBackgroundTexture();
        testSceneBackgroundRotation();
        testSceneBackgroundColor();
        testSceneBackgroundCube();
        testSceneBackgroundVideoTexture();
    }

    private void testSceneBackgroundTexture() {
        Bitmap background = this.getBitmapFromAssets(mActivity, "360_westlake.jpg");
        final Texture backgroundTexture = new Texture(background, Texture.TextureFormat.RGBA8, true, true);
        mScene.setBackgroundTexture(backgroundTexture);
        assertPass("The scene background should display a texture of westlake.");
    }

    private void testSceneBackgroundVideoTexture() {
        runOnUiThread(()->{
            final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(),
                    Uri.parse("https://s3.amazonaws.com/viro.video/Climber2Top.mp4"));
            mScene.setBackgroundTexture(videoTexture);
            videoTexture.setLoop(true);
            videoTexture.play();
        });
        assertPass("The scene  background should display a video of a climber.");
    }

    private void testSceneBackgroundRotation() {
        final List<Float> rotations = Arrays.asList(0f, 45f, 90f, 135f, 180f, 225f, 270f);
        final Iterator<Float> itr = Iterables.cycle(rotations).iterator();
        mMutableTestMethod = () -> {
            mScene.setBackgroundRotation(new Vector(0, 0, Math.toRadians(itr.next())));
        };
        assertPass("The scene background should rotate.");
    }

    private void testSceneBackgroundColor() {
        final List<Integer> bgColors = Arrays.asList(Color.WHITE, Color.YELLOW, Color.BLUE, Color.RED);
        final Iterator<Integer> itr = Iterables.cycle(bgColors).iterator();
        mMutableTestMethod = () -> {
            mScene.setBackgroundCubeWithColor(itr.next());
        };
        assertPass("The scene should rotate thru colors.");
    }

    private void testSceneBackgroundCube() {
        mMutableTestMethod = null;
        final Bitmap px = this.getBitmapFromAssets(mActivity, "px.png");
        final Bitmap nx = this.getBitmapFromAssets(mActivity, "nx.png");
        final Bitmap py = this.getBitmapFromAssets(mActivity, "py.png");
        final Bitmap ny = this.getBitmapFromAssets(mActivity, "ny.png");
        final Bitmap pz = this.getBitmapFromAssets(mActivity, "pz.png");
        final Bitmap nz = this.getBitmapFromAssets(mActivity, "nz.png");

        final Texture cubeTexture = new Texture(px, nx, py, ny,
                pz, nz, Texture.TextureFormat.RGBA8);

        mScene.setBackgroundCubeTexture(cubeTexture);

        assertPass("The scene should be a cube map.");
    }

}
