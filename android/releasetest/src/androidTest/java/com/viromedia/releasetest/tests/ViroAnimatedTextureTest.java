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
import com.viro.core.AnimatedTexture;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Quad;
import com.viro.core.Vector;
import org.junit.Test;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class ViroAnimatedTextureTest extends ViroBaseTest {
    private Material mAnimMat;
    @Override
    void configureTestScene() {
        final DirectionalLight light = new DirectionalLight(Color.WHITE, 1000.0f, new Vector(0, 0, -1f));
        final Quad q = new Quad(0.75f,.75f);
        mAnimMat = new Material();
        q.setMaterial(mAnimMat);

        Node n = new Node();
        n.setPosition(new Vector(0,0,-1));
        n.setGeometry(q);

        mScene.getRootNode().addChildNode(n);
        mScene.getRootNode().addLight(light);
    }

    @Test
    public void testImages() {
        runUITest(() -> testAnimatedTexture());
        runUITest(() -> testAnimatedTexturePausePlay());
        runUITest(() -> testAnimatedTextureCallback());
        runUITest(() -> testAnimatedTextureURL());
    }

    private void testAnimatedTexture() {
        AnimatedTexture text = new AnimatedTexture(mViroView.getViroContext(),
                Uri.parse("file:///android_asset/testingGifplz.gif"), new AnimatedTexture.OnLoadComplete() {
            @Override
            public void onSuccess(AnimatedTexture texture) {
                // No-op
            }

            @Override
            public void onFailure(String error) {
                // No-op
            }
        });

        mAnimMat.setDiffuseTexture(text);
        assertPass("You should see an animated textured cat.");
    }

    private void testAnimatedTexturePausePlay() {
        AnimatedTexture text = new AnimatedTexture(mViroView.getViroContext(),
                Uri.parse("file:///android_asset/testingGifplz.gif"), new AnimatedTexture.OnLoadComplete() {
            @Override
            public void onSuccess(AnimatedTexture texture) {
                // No-op
            }

            @Override
            public void onFailure(String error) {
                // No-op
            }
        });

        mAnimMat.setDiffuseTexture(text);

        final List<Boolean> playList = Arrays.asList(false, true, false, true, false, true, false, true, false, true);
        final Iterator<Boolean> itr = Iterables.cycle(playList).iterator();
        mMutableTestMethod = () -> {
            if (itr.next().booleanValue()){
                text.play();
            } else {
                text.pause();
            }
        };

        assertPass("Animated Texture should toggle between play/pause every second.");
    }

    private void testAnimatedTextureCallback() {
        AnimatedTexture text = new AnimatedTexture(mViroView.getViroContext(),
                Uri.parse("file:///android_asset/testingGifplz.gif"), new AnimatedTexture.OnLoadComplete() {
            @Override
            public void onSuccess(AnimatedTexture texture) {
                mAnimMat.setDiffuseTexture(null);
                mAnimMat.setDiffuseColor(Color.parseColor("#00FF00"));
            }

            @Override
            public void onFailure(String error) {
                // No-op
            }
        });

        mAnimMat.setDiffuseTexture(text);
        assertPass("You should see a green surface - shows AnimatedTexture Callback is successful.");
    }
    private void testAnimatedTextureURL() {
        AnimatedTexture text = new AnimatedTexture(mViroView.getViroContext(),
                Uri.parse("https://cdn.vox-cdn.com/thumbor/lv4CD8Y-BFnpagAg6vgGcBozr8Y=/1600x0/filters:no_upscale()/cdn.vox-cdn.com/uploads/chorus_asset/file/8692949/no_words_homer_into_brush.gif"), new AnimatedTexture.OnLoadComplete() {
            @Override
            public void onSuccess(AnimatedTexture texture) {
                // No-op
            }

            @Override
            public void onFailure(String error) {
                // No-op
            }
        });

        mAnimMat.setDiffuseTexture(text);
        assertPass("You should see an animated homer going into the bush.");
    }
}
