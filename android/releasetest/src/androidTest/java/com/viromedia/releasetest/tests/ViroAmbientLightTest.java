/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.support.test.espresso.core.deps.guava.collect.Iterables;
import android.support.test.runner.AndroidJUnit4;

import com.viro.core.AmbientLight;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Sphere;
import com.viro.core.Surface;
import com.viro.core.Vector;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Created by manish on 10/25/17.
 */

@RunWith(AndroidJUnit4.class)
public class ViroAmbientLightTest extends ViroBaseTest {
    private static final String TAG = ViroAmbientLightTest.class.getName();
    private static final int INFLUENCE_BITMASK = 1;
    private AmbientLight mAmbientLight;
    private Node mVSurface;
    private Node mHSurface;
    private Node mSphere;
    @Override
    void configureTestScene() {
        mAmbientLight = new AmbientLight();
        mAmbientLight.setInfluenceBitMask(INFLUENCE_BITMASK);
        mScene.getRootNode().addLight(mAmbientLight);

        final DirectionalLight directionalLight = new DirectionalLight();
        directionalLight.setDirection(new Vector(-1, 0, 0));
        directionalLight.setInfluenceBitMask(INFLUENCE_BITMASK);
        mScene.getRootNode().addLight(directionalLight);
        // Configure verticle surface
        mVSurface = new Node();
        final Surface surface = new Surface(5, 5);
        final float[] surfacePos = {0, 0, -5};
        mVSurface.setPosition(new Vector(surfacePos));
        final Material surfaceMaterial = new Material();
        surfaceMaterial.setDiffuseColor(Color.WHITE);
        surfaceMaterial.setLightingModel(Material.LightingModel.BLINN);
        surface.setMaterials(Arrays.asList(surfaceMaterial));
        mVSurface.setGeometry(surface);
        mVSurface.setLightReceivingBitMask(INFLUENCE_BITMASK);
        mScene.getRootNode().addChildNode(mVSurface);

        // Configure horizontal surface
        mHSurface = new Node();
        final Surface hSurface = new Surface(5, 5);
        final float[] hSurfacePos = {0, -2f, -5};
        mHSurface.setPosition(new Vector(hSurfacePos));
        final Material hSurfaceMaterial = new Material();
        hSurfaceMaterial.setDiffuseColor(Color.WHITE);
        hSurfaceMaterial.setLightingModel(Material.LightingModel.BLINN);
        hSurface.setMaterials(Arrays.asList(hSurfaceMaterial));
        mHSurface.setGeometry(hSurface);
        mHSurface.setRotation(new Vector(-90, 0, 0));
        mHSurface.setLightReceivingBitMask(INFLUENCE_BITMASK);
        mScene.getRootNode().addChildNode(mHSurface);

        // Configure sphere
        mSphere = new Node();
        final Sphere sphere = new Sphere(0.5f);
        final Material sphereMaterial = new Material();
        sphereMaterial.setDiffuseColor(Color.WHITE);
        sphereMaterial.setLightingModel(Material.LightingModel.BLINN);
        mSphere.setShadowCastingBitMask(INFLUENCE_BITMASK);
        mSphere.setLightReceivingBitMask(INFLUENCE_BITMASK);
        mSphere.setGeometry(sphere);
        final float[] spherePos = {0, -0.5f, -2f};
        mSphere.setPosition(new Vector(spherePos));
        sphere.setMaterials(Arrays.asList(sphereMaterial));
        mScene.getRootNode().addChildNode(mSphere);
    }

    @Test
    public void testAmbientLight() {
        // Context of the app under test.
        runUITest(() -> testSetColor());
        runUITest(() -> testSetIntensity());
        runUITest(() -> testSetInfluenceBitMask());
    }

    private void testSetColor() {
        final List<Integer> colors = Arrays.asList(Color.WHITE, Color.RED, Color.GREEN, Color.BLUE, Color.MAGENTA, Color.CYAN);

        final Iterator<Integer> itr = Iterables.cycle(colors).iterator();

        mMutableTestMethod = () -> {
            mAmbientLight.setColor(itr.next());
        };
        assertPass("Cycling colors through WHITE, RED, GREEN, BLUE, MAGENTA, CYAN", () -> {
            mAmbientLight.setColor(Color.GREEN);
        });
    }

    private void testSetIntensity() {
        mMutableTestMethod = () -> {
            if (mAmbientLight != null) {
                final float currentIntensity = mAmbientLight.getIntensity();
                mAmbientLight.setIntensity((currentIntensity + 200) % 2000);
            }
        };
        assertPass("Cycling intensity from 0 to 2000, with +200 every second", () -> {
            mAmbientLight.setIntensity(1000);
        });
    }

    private void testSetInfluenceBitMask() {
        mMutableTestMethod = () -> {
            if (mAmbientLight != null) {
                final int currentBitMask = mAmbientLight.getInfluenceBitMask();
                mAmbientLight.setInfluenceBitMask(currentBitMask == INFLUENCE_BITMASK ? 2 : INFLUENCE_BITMASK);
            }
        };

        assertPass("Alternating ambientLight's influence bit mask betweeen 1 and 2", () -> {
            mSphere.setLightReceivingBitMask(INFLUENCE_BITMASK);
        });
    }
}
