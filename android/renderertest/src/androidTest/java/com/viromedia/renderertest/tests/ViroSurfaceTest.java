/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests;

import android.graphics.Color;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Sphere;
import com.viro.renderer.jni.Surface;
import com.viro.renderer.jni.Vector;

import org.junit.Test;

import java.util.Arrays;

/**
 * Created by vadvani on 10/30/17.
 */

public class ViroSurfaceTest extends ViroBaseTest {
    private Surface mSurface;

    @Override
    void configureTestScene() {

        AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(ambientLightJni);

        // Creation of first sphere using radius constructor.
        Material material = new Material();
        material.setDiffuseColor(Color.RED);
        material.setLightingModel(Material.LightingModel.BLINN);

        mSurface = new Surface(1, 1);
        Node node = new Node();
        node.setGeometry(mSurface);
        float[] position = {0,0,-10};
        node.setPosition(new Vector(position));
        mSurface.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(node);
    }

    @Test
    public void surfaceTest() {
        //surfaceWidth();
        surfaceHeight();
    }

    public void surfaceWidth() {
        mMutableTestMethod = () -> {
            if(mSurface != null  && (mSurface.getWidth() < 10)) {
                mSurface.setWidth(mSurface.getWidth() +1);
            }
        };
        assertPass("Surface width increased in size.");
    }

    public void surfaceHeight() {
        mMutableTestMethod = () -> {
            if(mSurface != null  && (mSurface.getHeight() < 10)) {
                mSurface.setHeight(mSurface.getHeight() +1);
            }
        };
        assertPass("Surface height increased in size.");
    }
}
