/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.AmbientLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Quad;
import com.viro.core.Surface;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Created by vadvani on 10/30/17.
 */

public class ViroQuadTest extends ViroBaseTest {
    private Quad mSurface;
    private Node mNode;

    @Override
    void configureTestScene() {

        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(ambientLightJni);

        // Creation of first sphere using radius constructor.
        final Material material = new Material();
        material.setDiffuseColor(Color.RED);
        material.setLightingModel(Material.LightingModel.BLINN);

        mSurface = new Quad(1, 1);
        mNode = new Node();
        mNode.setGeometry(mSurface);
        final float[] position = {0, 0, -10};
        mNode.setPosition(new Vector(position));
        mSurface.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(mNode);
    }

    @Test
    public void surfaceTest() {
        runUITest(() -> surfaceWidth());
        runUITest(() -> surfaceHeight());
        runUITest(() -> surfaceUVs());
    }

    public void surfaceWidth() {
        mMutableTestMethod = () -> {
            if (mSurface != null && (mSurface.getWidth() < 10)) {
                mSurface.setWidth(mSurface.getWidth() + 1);
            }
        };
        assertPass("Quad width increased in size.");
    }

    public void surfaceHeight() {
        mMutableTestMethod = () -> {
            if (mSurface != null && (mSurface.getHeight() < 10)) {
                mSurface.setHeight(mSurface.getHeight() + 1);
            }
        };
        assertPass("Quad height increased in size.");
    }

    public void surfaceUVs() {
        Bitmap specBitmap = this.getBitmapFromAssets(mActivity, "earth_normal.jpg");
        Texture specTexture = new Texture(specBitmap, Texture.Format.RGBA8, true, true);
        specTexture.setWrapS(Texture.WrapMode.REPEAT);
        specTexture.setWrapT(Texture.WrapMode.REPEAT);
        final Material material = new Material();
        material.setDiffuseTexture(specTexture);
        material.setLightingModel(Material.LightingModel.BLINN);
        final List<Float> tilesList = Arrays.asList(2f, 4f, 8f);

        final Iterator<Float> itr = Iterables.cycle(tilesList).iterator();

        mMutableTestMethod = () -> {
            Float numTilesUV = itr.next();
            mSurface = new Quad(5, 5, 0, 0, numTilesUV , numTilesUV);
            mSurface.setMaterials(Arrays.asList(material));
            mNode.setGeometry(mSurface);
        };
        assertPass("Quad tiles material 2, 4 and 8 times with UV coordinates.");
    }
}
