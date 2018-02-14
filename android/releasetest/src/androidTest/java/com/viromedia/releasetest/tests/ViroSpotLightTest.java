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
import android.view.animation.Animation;

import com.viro.core.AmbientLight;
import com.viro.core.AnimationTransaction;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Sphere;
import com.viro.core.Spotlight;
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
public class ViroSpotLightTest extends ViroBaseTest {
    private static final String TAG = ViroSpotLightTest.class.getName();
    private static final int INFLUENCE_BITMASK = 1;
    private Spotlight mSpotLight;

    @Override
    void configureTestScene() {
        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 100.0f);
        mScene.getRootNode().addLight(ambientLightJni);

        // Add directional light in -Z direction
        mSpotLight = new Spotlight();
        mSpotLight.setInfluenceBitMask(INFLUENCE_BITMASK);

        mScene.getRootNode().addLight(mSpotLight);

        // Configure verticle surface
        final Node verticleSurfaceNode = new Node();
        final Surface surface = new Surface(5, 5);
        final float[] surfacePos = {0, 0, -5};
        verticleSurfaceNode.setPosition(new Vector(surfacePos));
        final Material surfaceMaterial = new Material();
        surfaceMaterial.setDiffuseColor(Color.WHITE);
        surfaceMaterial.setLightingModel(Material.LightingModel.BLINN);
        surface.setMaterials(Arrays.asList(surfaceMaterial));
        verticleSurfaceNode.setGeometry(surface);
        verticleSurfaceNode.setLightReceivingBitMask(INFLUENCE_BITMASK);
        mScene.getRootNode().addChildNode(verticleSurfaceNode);

        // Configure horizontal surface
        final Node horizontalSurfaceNode = new Node();
        final Surface hSurface = new Surface(5, 5);
        final float[] hSurfacePos = {0, -2f, -5};
        horizontalSurfaceNode.setPosition(new Vector(hSurfacePos));
        final Material hSurfaceMaterial = new Material();
        hSurfaceMaterial.setDiffuseColor(Color.WHITE);
        hSurfaceMaterial.setLightingModel(Material.LightingModel.BLINN);
        hSurface.setMaterials(Arrays.asList(hSurfaceMaterial));
        horizontalSurfaceNode.setGeometry(hSurface);
        horizontalSurfaceNode.setRotation(new Vector(-90, 0, 0));
        horizontalSurfaceNode.setLightReceivingBitMask(INFLUENCE_BITMASK);
        mScene.getRootNode().addChildNode(horizontalSurfaceNode);

        // Configure sphere
        final Node sphereNode = new Node();
        final Sphere sphere = new Sphere(0.5f);
        final Material sphereMaterial = new Material();
        sphereMaterial.setDiffuseColor(Color.WHITE);
        sphereMaterial.setLightingModel(Material.LightingModel.BLINN);
        sphereNode.setShadowCastingBitMask(INFLUENCE_BITMASK);
        sphereNode.setLightReceivingBitMask(INFLUENCE_BITMASK);
        sphereNode.setGeometry(sphere);
        final float[] spherePos = {0, -0.5f, -2f};
        sphereNode.setPosition(new Vector(spherePos));
        sphere.setMaterials(Arrays.asList(sphereMaterial));
        mScene.getRootNode().addChildNode(sphereNode);

    }

    @Test
    public void testSpotlight() {
        testSetColor();
        testSetIntensity();
        testSetCastsShadow();
        testSetDirection();
        testSetPosition();
        testSetDirection();
        testSetInnerAngle();
        testSetOuterAngle();
        testTemperature();
    }

    /**
     * Base Light.java props
     **/

    private void testSetColor() {
        final List<Integer> colors = Arrays.asList(Color.WHITE, Color.RED, Color.GREEN, Color.BLUE, Color.MAGENTA, Color.CYAN);

        final Iterator<Integer> itr = Iterables.cycle(colors).iterator();

        mMutableTestMethod = () -> {
            mSpotLight.setColor(itr.next());
        };
        assertPass("Cycling colors through WHITE, RED, GREEN, BLUE, MAGENTA, CYAN", () -> {
            mSpotLight.setColor(Color.GREEN);
        });

    }

    private void testSetIntensity() {
        mMutableTestMethod = () -> {
            if (mSpotLight != null) {
                final float currentIntensity = mSpotLight.getIntensity();
                mSpotLight.setIntensity((currentIntensity + 200) % 2000);
            }
        };
        assertPass("Cycling intensity from 0 to 2000, with +200 every second", () -> {
            mSpotLight.setIntensity(1000);
        });
    }

    private void testSetCastsShadow() {
        mMutableTestMethod = () -> {
            if (mSpotLight != null) {
                mSpotLight.setCastsShadow(!mSpotLight.getCastsShadow());
            }
        };
        assertPass("Toggling casts shadow", () -> {
            mSpotLight.setCastsShadow(false);
        });
    }

    private void testSetInfluenceBitMask() {
        assertPass("running testSetCastsShadow()");

    }


    private void testSetDirection() {
        final List<Vector> directions = Arrays.asList(new Vector(0, 0, -1), new Vector(0, -1, -1), new Vector(0, -1, 0));
        final Iterator<Vector> itr = Iterables.cycle(directions).iterator();
        mMutableTestMethod = () -> {
            if (mSpotLight != null) {
                mSpotLight.setDirection(itr.next());
            }
        };

        assertPass("Changing direction of light from -z to -y axis", () -> {
            mSpotLight.setDirection(directions.get(0));
        });
        assertPass("running testSetDirection()");

    }

    private void testSetPosition() {
        final List<Vector> positions = Arrays.asList(new Vector(0, 0, -1), new Vector(0, -1, -1), new Vector(0, -1, 0));
        final Iterator<Vector> itr = Iterables.cycle(positions).iterator();
        mMutableTestMethod = () -> {
            if (mSpotLight != null) {
                mSpotLight.setPosition(itr.next());
            }
        };
        assertPass("running testSetPosition()");
    }

    private void testSetInnerAngle() {
        final List<Float> innerAnglesList = Arrays.asList((float)Math.toRadians(20), (float)Math.toRadians(35), (float)Math.toRadians(55));
        final Iterator<Float> itr = Iterables.cycle(innerAnglesList).iterator();
        mMutableTestMethod = () -> {
            if (mSpotLight != null) {
                mSpotLight.setPosition(new Vector(0, 0, 0));
                mSpotLight.setDirection(new Vector(0, 0, -1));

                AnimationTransaction.begin();
                AnimationTransaction.setAnimationDuration(500);
                Float next = itr.next();
                mSpotLight.setInnerAngle(next);
                mSpotLight.setOuterAngle(next);
                AnimationTransaction.commit();
            }
        };
        assertPass("running testSetInnerAngle()");

    }

    private void testSetOuterAngle() {
        final List<Float> outerAnglesList = Arrays.asList((float)Math.toRadians(20), (float)Math.toRadians(35), (float)Math.toRadians(55));
        final Iterator<Float> itr = Iterables.cycle(outerAnglesList).iterator();
        mMutableTestMethod = () -> {
            if (mSpotLight != null) {
                mSpotLight.setPosition(new Vector(0, 0, 0));
                mSpotLight.setDirection(new Vector(0, 0, -1));

                AnimationTransaction.begin();
                AnimationTransaction.setAnimationDuration(500);
                mSpotLight.setInnerAngle((float) Math.toRadians(20));
                mSpotLight.setOuterAngle(itr.next());
                AnimationTransaction.commit();
            }
        };
        assertPass("running testSetOuterAngle()");
    }

    private void testTemperature() {
        final List<Float> temperatureList = Arrays.asList(1000f, 2000f, 3000f, 4000f, 5000f, 6000f, 7000f, 10000f, 15000f, 20000f);
        final Iterator<Float> itr = Iterables.cycle(temperatureList).iterator();
        mMutableTestMethod = () -> {
            if (mSpotLight != null) {
                AnimationTransaction.begin();
                AnimationTransaction.setAnimationDuration(500);
                mSpotLight.setColor(Color.WHITE);
                mSpotLight.setTemperature(itr.next());
                AnimationTransaction.commit();
            }
        };
        assertPass("running testTemperature()");
    }

}
