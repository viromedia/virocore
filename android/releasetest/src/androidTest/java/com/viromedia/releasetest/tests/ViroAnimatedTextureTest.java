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
import com.viro.core.AnimatedTexture;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Quad;
import com.viro.core.Vector;
import org.junit.Test;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class ViroAnimatedTextureTest extends ViroBaseTest {
    private Material mAnimMat;
    private Node mAnimNode;
    @Override
    void configureTestScene() {
        final DirectionalLight light = new DirectionalLight(Color.WHITE, 1000.0f, new Vector(0, 0, -1f));
        final Quad q = new Quad(0.75f,.75f);
        mAnimMat = new Material();
        q.setMaterial(mAnimMat);

        mAnimNode = new Node();
        mAnimNode.setPosition(new Vector(0,0,-1));
        mAnimNode.setGeometry(q);

        mScene.getRootNode().addChildNode(mAnimNode);
        mScene.getRootNode().addLight(light);
    }

    @Test
    public void testImages() {
        runUITest(() -> testAnimatedTexture());
        runUITest(() -> testAnimatedTexturePausePlay());
        runUITest(() -> testAnimatedTextureCallback());
        runUITest(() -> testAnimatedTextureURL());
        runUITest(() -> testVaryingGifFormats());
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

    private void testVaryingGifFormats() {
        mAnimNode.setVisible(false);

        ArrayList<String> testUri = new ArrayList<String>();
        testUri.add("file:///android_asset/test.gif");
        testUri.add("file:///android_asset/testingGifplz2.gif");
        testUri.add("file:///android_asset/testingGifplz3.gif");
        testUri.add("file:///android_asset/testingGifplz4.gif");
        testUri.add("file:///android_asset/testingGifplz5.gif");
        testUri.add("file:///android_asset/testingGifplz6.gif");
        testUri.add("file:///android_asset/testingGifplz7.gif");
        testUri.add("file:///android_asset/testingGifplz8.gif");
        testUri.add("file:///android_asset/testingGifplz9.gif");
        testUri.add("file:///android_asset/testingGifplz10.gif");
        testUri.add("file:///android_asset/testingGifplz11.gif");
        testUri.add("file:///android_asset/testingGifplz12.gif");
        testUri.add("http://i.kinja-img.com/gawker-media/image/upload/s--zhPWGLpa--/17yeis91c7prugif.gif");
        testUri.add("https://cdn.vox-cdn.com/thumbor/S4xR3SwNu1yrh_lvDDZArn6m_9A=/1600x0/filters:no_upscale()/cdn.vox-cdn.com/uploads/chorus_asset/file/8687957/tenor.gif");
        testUri.add("https://cdn.vox-cdn.com/thumbor/lv4CD8Y-BFnpagAg6vgGcBozr8Y=/1600x0/filters:no_upscale()/cdn.vox-cdn.com/uploads/chorus_asset/file/8692949/no_words_homer_into_brush.gif");
        testUri.add("https://cdn.vox-cdn.com/thumbor/96c6PgcP7N84eNii0Z8l9nWA-9c=/1600x0/filters:no_upscale()/cdn.vox-cdn.com/uploads/chorus_asset/file/8689071/My5Z2DO.gif");

        final Node groupNode = new Node();
        groupNode.setPosition(new Vector(-2,0,-3));
        int source = 0;
        for (int x = 0; x < 4 ; x ++) {
            for (int y = 0; y < 4 ; y ++) {

                if (source < testUri.size()){
                    addGIFNode(groupNode, new Vector(x, y, 0), testUri.get(source));
                    source++;
                }
            }
        }
        mScene.getRootNode().addChildNode(groupNode);
        assertPass("You should see a 4 x 4 grid of animated GIFs.");
    }


    public void addGIFNode(Node rootNode, Vector position, String gifStr){
        final Quad q = new Quad(0.75f,.75f);
        AnimatedTexture text = new AnimatedTexture(mViroView.getViroContext(),
                Uri.parse(gifStr), new AnimatedTexture.OnLoadComplete() {
            @Override
            public void onSuccess(AnimatedTexture texture) {
                //Material mat = new Material();
                //mat.setDiffuseColor(Color.parseColor("#00FF00"));
                //q.setMaterial(mat);
            }

            @Override
            public void onFailure(String error) {
                Material mat = new Material();
                mat.setDiffuseColor(Color.parseColor("#FF0000"));
                q.setMaterial(mat);
            }
        });

        final Material videoMaterial = new Material();
        videoMaterial.setDiffuseTexture(text);
        q.setMaterial(videoMaterial);
        text.setLoop(true);
        text.play();

        Node n = new Node();
        n.setGeometry(q);
        n.setPosition(position);
        rootNode.addChildNode(n);
    }
}
