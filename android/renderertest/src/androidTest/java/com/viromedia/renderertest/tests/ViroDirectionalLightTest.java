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
import android.support.test.runner.AndroidJUnit4;

import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Surface;
import com.viro.renderer.jni.Vector;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.Arrays;

/**
 * Created by manish on 10/25/17.
 */

@RunWith(AndroidJUnit4.class)
public class ViroDirectionalLightTest extends ViroBaseTest {
    private static final String TAG = ViroDirectionalLightTest.class.getName();

    private DirectionalLight mDirectionalLight;

    @Override
    void configureTestScene() {
//        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
//        mScene.getRootNode().addLight(ambientLightJni);

        final Node verticleSurfaceNode = new Node();
        final Surface surface = new Surface(6, 6);
        final float[] surfacePos = {0, 2, -4};
        verticleSurfaceNode.setPosition(new Vector(surfacePos));
        final Material surfaceMaterial = new Material();
        surfaceMaterial.setDiffuseColor(Color.GREEN);
        surfaceMaterial.setLightingModel(Material.LightingModel.LAMBERT);
        surface.setMaterials(Arrays.asList(surfaceMaterial));
        verticleSurfaceNode.setGeometry(surface);
        mScene.getRootNode().addChildNode(verticleSurfaceNode);

        final Node boxNode = new Node();
        final Box box = new Box(1, 1, 1);
        final Material boxMaterial = new Material();
        boxMaterial.setDiffuseColor(Color.BLUE);
        boxMaterial.setLightingModel(Material.LightingModel.LAMBERT);

        boxNode.setGeometry(box);
        final float[] boxPosition = {0, -0.5f, -2};
        boxNode.setPosition(new Vector(boxPosition));
        box.setMaterials(Arrays.asList(boxMaterial));
        mScene.getRootNode().addChildNode(boxNode);

        // Add directional light in -Z direction
        mDirectionalLight = new DirectionalLight();
        mDirectionalLight.setColor(Color.GREEN);
        mScene.getRootNode().addLight(mDirectionalLight);
    }

    @Test
    public void testDirectionalLight() {

        testIntensityChange();
        testColorChange();

        testShadowBiasChange();
    }

    private void testColorChange() {
        // test
        // change color to green
        assertPass("running testColorChange()");
    }

    private void testIntensityChange() {
        assertPass("running testIntensityChange()");
    }

    private void testShadowBiasChange() {
        assertPass("running testShadowBiasChange()");
    }
}
