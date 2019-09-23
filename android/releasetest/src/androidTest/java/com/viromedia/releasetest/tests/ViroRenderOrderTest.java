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

import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;

import com.viro.core.AmbientLight;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Spotlight;
import com.viro.core.Surface;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;

/**
 * Created by radvani on 4/11/18.
 */

public class ViroRenderOrderTest extends ViroBaseTest {

    private Surface mBackRedSurface;
    private Surface mFrontGreenSurface;
    private Box mBox;

    private Node mBackRedNode;
    private Node mFrontGreenNode;
    private Node mBoxNode;

    private Texture mTransparentTexture;

    @Override
    void configureTestScene() {
        final AmbientLight ambient = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(ambient);

        final Material material = new Material();
        material.setDiffuseColor(Color.RED);
        material.setLightingModel(Material.LightingModel.BLINN);

        mBackRedSurface = new Surface(8, 8);
        mBackRedSurface.setMaterials(Arrays.asList(new Material.MaterialBuilder().
                diffuseColor(Color.RED).
                lightingModel(Material.LightingModel.BLINN).build()));

        mBackRedNode = new Node();
        mBackRedNode.setName("Red");
        mBackRedNode.setGeometry(mBackRedSurface);
        mBackRedNode.setPosition(new Vector(0, 0, -7));

        mFrontGreenSurface = new Surface(2f, 2f);
        mFrontGreenSurface.setMaterials(Arrays.asList(new Material.MaterialBuilder().
                diffuseColor(Color.GREEN).
                lightingModel(Material.LightingModel.BLINN).build()));

        mFrontGreenNode = new Node();
        mFrontGreenNode.setName("Green");
        mFrontGreenNode.setGeometry(mFrontGreenSurface);
        mFrontGreenNode.setPosition(new Vector(0, 0, -2.5));

        // Distance from camera is measured from the center of the geometry's bounding box.
        // By creating a box that extends depth-wise far back, we can test that rendering order
        // is taking opacity into account before distance to camera. In terms of distance from
        // camera, the red plane is in back, then the box, then the green plane.
        mBox = new Box(0.25f, 0.25f, 5.0f);
        mBox.setMaterials(Arrays.asList(new Material.MaterialBuilder().
                diffuseColor(Color.WHITE).
                lightingModel(Material.LightingModel.BLINN).build()));

        mBoxNode = new Node();
        mBoxNode.setName("Box");
        mBoxNode.setGeometry(mBox);
        mBoxNode.setPosition(new Vector(0, 0, -3.5));

        mScene.getRootNode().addChildNode(mBackRedNode);
        mScene.getRootNode().addChildNode(mFrontGreenNode);
        mScene.getRootNode().addChildNode(mBoxNode);

        Bitmap specBitmap = this.getBitmapFromAssets(mActivity, "transparent.png");
        mTransparentTexture = new Texture(specBitmap, Texture.Format.RGBA8, true, true);

        mFrontGreenNode.setVisible(true);
        mFrontGreenNode.setOpacity(1.0f);
        mBackRedNode.setVisible(true);
        mBackRedNode.setOpacity(1.0f);
        mBoxNode.setVisible(true);
        mBoxNode.setOpacity(1.0f);
        mBox.getMaterials().get(0).setDiffuseColor(Color.WHITE);
        mBox.getMaterials().get(0).setDiffuseTexture(null);
        mFrontGreenSurface.getMaterials().get(0).setDiffuseTexture(null);
        mBackRedSurface.getMaterials().get(0).setReadsFromDepthBuffer(true);
        mBackRedSurface.getMaterials().get(0).setWritesToDepthBuffer(true);
        mFrontGreenSurface.getMaterials().get(0).setReadsFromDepthBuffer(true);
        mFrontGreenSurface.getMaterials().get(0).setWritesToDepthBuffer(true);
        mBackRedSurface.getMaterials().get(0).setDiffuseTexture(null);
    }


    @Test
    public void renderOrderTest() {
        runUITest(() -> allOpaque());
        runUITest(() -> boxAndGreenSurfaceTransparent());
        runUITest(() -> greenBoxTransparentFromNodeOpacity());
        runUITest(() -> greenBoxTransparentFromDiffuseColor());
        runUITest(() -> greenBoxTransparentFromTexture());
        runUITest(() -> boxAndRedSurfaceTransparent());
        runUITest(() -> greenOverRed());
        runUITest(() -> redOverGreen());
        runUITest(() -> lamborghini());
    }

    public void allOpaque() {
        mBoxNode.setVisible(false);

        // Simply test that two opaque surfaces render correctly with the depth
        // buffer (order should not matter).
        assertPass("Green in front of red");
    }

    public void boxAndGreenSurfaceTransparent() {
        mBackRedNode.setVisible(false);

        // By setting the opacity of both the box and the surface down, they're both marked
        // as transparent, so the render order is determined by furthest distance from camera.
        // The box's center is further from the camera so it is rendered before the green surface.
        // As a result, we will *not* see the green surface behind the partially transparent box.
        mBoxNode.setOpacity(0.25f);
        mFrontGreenNode.setOpacity(0.99f);
        assertPass("Green surface can *NOT* be seen behind the partially transparent box");
    }

    public void greenBoxTransparentFromNodeOpacity() {
        mBackRedNode.setVisible(false);

        // By setting the opacity down, we change the render order such that the green surface
        // will be rendered first (it's opaque), and the white box is rendered after (it's transparent).
        // Therefore we should see the green surface behind the partially transparent white box.
        mBoxNode.setOpacity(0.25f);
        assertPass("[Opacity based] Green surface can be seen behind the partially transparent white box");
    }

    public void greenBoxTransparentFromDiffuseColor() {
        mBackRedNode.setVisible(false);

        // Same as above except we make the box transparent by turning down its color alpha
        mBox.getMaterials().get(0).setDiffuseColor(Color.argb(64, 255, 255, 255));
        assertPass("[Color based] Green surface can be seen behind the partially transparent white box");
    }

    public void greenBoxTransparentFromTexture() {
        mBackRedNode.setVisible(false);

        // Same as above except we make the box transparent by giving it a transparent texture
        mBox.getMaterials().get(0).setDiffuseTexture(mTransparentTexture);
        assertPass("Green surface can be seen behind the transparent squiggles");
    }

    public void boxAndRedSurfaceTransparent() {
        mFrontGreenNode.setVisible(false);

        // By setting the opacity of both the box and the surface down, they're both marked
        // as transparent. However, since the red surface is further away from the camera than
        // the box, it's still rendered first
        mBoxNode.setOpacity(0.25f);
        mBackRedNode.setOpacity(0.99f);
        assertPass("Red surface can be seen behind the partially transparent box");
    }

    public void greenOverRed() {
        mBoxNode.setVisible(false);

        // Set the green plane to partially transparent, and the red should be seen behind it
        mFrontGreenSurface.getMaterials().get(0).setDiffuseTexture(mTransparentTexture);
        assertPass("Red surface can be seen behind transparent squiggles");
    }

    public void redOverGreen() {
        mBoxNode.setVisible(false);

        // Set the red plane to partially transparent so that, despite being further away from the
        // camera, it is rendered after the green. This only works if we disable depth testing
        // (otherwise the red node will never appear above the green node).
        mBackRedSurface.getMaterials().get(0).setReadsFromDepthBuffer(false);
        mBackRedSurface.getMaterials().get(0).setWritesToDepthBuffer(false);
        mFrontGreenSurface.getMaterials().get(0).setReadsFromDepthBuffer(false);
        mFrontGreenSurface.getMaterials().get(0).setWritesToDepthBuffer(false);

        // The red surface is rendered on top of the green, but you should still see the green
        // surface through the squiggles.
        mBackRedSurface.getMaterials().get(0).setDiffuseTexture(mTransparentTexture);
        assertPass("Green surface can be seen behind transparent squiggles (edges of squiggles are black)");
    }

    public void lamborghini() {
        final Object3D lamborghini = new Object3D();
        lamborghini.setPosition(new Vector(0, -2.5, -6));
        lamborghini.setScale(new Vector(0.015f, 0.015f, 0.015f));
        lamborghini.setRotation(new Vector(0, Math.PI / 2.0f, 0));
        lamborghini.loadModel(mViroView.getViroContext(), (Uri.parse("file:///android_asset/lamborghini_v2.vrx")), Object3D.Type.FBX,  new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {

            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        Spotlight light = new Spotlight();
        light.setColor(Color.WHITE);
        light.setPosition(new Vector(0, 10, 10));
        light.setDirection(new Vector(0, -1.0, -1.0));
        light.setAttenuationStartDistance(25);
        light.setAttenuationEndDistance(50);
        light.setInnerAngle(35);
        light.setOuterAngle(60);
        light.setIntensity(1000);

        mScene.getRootNode().addLight(light);
        mScene.getRootNode().addChildNode(lamborghini);

        mFrontGreenNode.setVisible(false);
        mBoxNode.setVisible(false);
        assertPass("Lamborghini is displayed, and the interior and red surface can be seen through the window");
    }

}
