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
import android.util.Log;

import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.Box;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Scene;
import com.viro.core.Sound;
import com.viro.core.Sphere;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.VideoTexture;

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
        final AmbientLight light = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(light);
    }

    @Test
    public void sceneTest() {
        testSceneBackgroundTexture();
        testSceneBackgroundRotation();
        testSceneBackgroundColor();
        testSceneBackgroundCube();
        testSceneBackgroundVideoTexture();
        testSceneTransition();
    }

    private void testSceneBackgroundTexture() {
        final Bitmap background = getBitmapFromAssets(mActivity, "360_westlake.jpg");
        final Texture backgroundTexture = new Texture(background, Texture.Format.RGBA8, true, true);
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
        assertPass("The scene background should display a video of a climber.");
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
        final Bitmap px = getBitmapFromAssets(mActivity, "px.png");
        final Bitmap nx = getBitmapFromAssets(mActivity, "nx.png");
        final Bitmap py = getBitmapFromAssets(mActivity, "py.png");
        final Bitmap ny = getBitmapFromAssets(mActivity, "ny.png");
        final Bitmap pz = getBitmapFromAssets(mActivity, "pz.png");
        final Bitmap nz = getBitmapFromAssets(mActivity, "nz.png");

        final Texture cubeTexture = new Texture(px, nx, py, ny,
                pz, nz, Texture.Format.RGBA8);

        mScene.setBackgroundCubeTexture(cubeTexture);

        assertPass("The scene should be a cube map.");
    }

    private void testSceneTransition() {
        // Set scene (with audio), set an onClickListener on a box, on Click change scene
        final AmbientLight light = new AmbientLight(Color.WHITE, 200f);
        final DirectionalLight directionalLight = new DirectionalLight();

        final Material material = new Material();
        material.setLightingModel(Material.LightingModel.BLINN);
        material.setDiffuseColor(Color.BLUE);

        final Scene scene1 = new ARScene();
        scene1.getRootNode().addLight(light);
        scene1.getRootNode().addLight(directionalLight);
        final Node boxNode = new Node();
        final Box box1 = new Box(2, 2, 2);
        box1.setMaterials(Arrays.asList(material));
        boxNode.setGeometry(box1);
        boxNode.setPosition(new Vector(0, -2.5f, -3.3f));
        scene1.getRootNode().addChildNode(boxNode);

        runOnUiThread(() -> {
            final Sound sound = new Sound(mViroView.getViroContext(),
                    Uri.parse("file:///android_asset/flies_mono.wav"), new Sound.PlaybackListener() {
                @Override
                public void onSoundReady(final Sound sound) {
                    sound.play();
                }

                @Override
                public void onSoundFinish(final Sound sound) {

                }

                @Override
                public void onSoundFail(final String error) {

                }
            });
            sound.setLoop(true);
        });

        final Scene scene2 = new ARScene();
        scene2.getRootNode().addLight(light);
        scene2.getRootNode().addLight(directionalLight);
        final Node sphereNode = new Node();
        final Sphere sphere = new Sphere(1);
        sphere.setMaterials(Arrays.asList(material));
        sphereNode.setGeometry(sphere);
        sphereNode.setPosition(new Vector(0, -2.5f, -3.3f));
        scene2.getRootNode().addChildNode(sphereNode);

        runOnUiThread(() -> {
            final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(),
                    Uri.parse("file:///android_asset/stereoVid360.mp4"), new VideoTexture.PlaybackListener() {
                @Override
                public void onVideoBufferStart(final VideoTexture video) {

                }

                @Override
                public void onVideoBufferEnd(final VideoTexture video) {

                }

                @Override
                public void onVideoFinish(final VideoTexture video) {

                }

                @Override
                public void onReady(final VideoTexture video) {
                    video.play();
                }

                @Override
                public void onVideoFailed(final String error) {

                }

                @Override
                public void onVideoUpdatedTime(final VideoTexture video, final float seconds, final float totalDuration) {

                }
            }, Texture.StereoMode.TOP_BOTTOM);

            videoTexture.setVolume(1);
            videoTexture.setLoop(true);
            final Material videoMaterial = new Material();
            videoMaterial.setDiffuseTexture(videoTexture);
            scene2.setBackgroundTexture(videoTexture);
        });

        final List<Scene> scenes = Arrays.asList(scene1, scene1, scene2, scene2);
        final Iterator<Scene> iterator = Iterables.cycle(scenes).iterator();
        mMutableTestMethod = () -> {
            mViroView.setScene(iterator.next());
        };
        assertPass("Alternating between two scenes");
    }
}
