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
import android.os.Handler;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Polyline;
import com.viro.renderer.jni.Vector;

import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * Created by manish on 11/6/17.
 */

public class ViroPolylineTest extends ViroBaseTest {
    private Node polylineNode;
    private Polyline polyline;
    private Handler mHandler;

    @Override
    void configureTestScene() {
        final AmbientLight ambientLight = new AmbientLight(Color.WHITE, 200);
        mScene.getRootNode().addLight(ambientLight);

        final Material material = new Material();
        material.setDiffuseColor(Color.RED);
        material.setLightingModel(Material.LightingModel.CONSTANT);
        material.setCullMode(Material.CullMode.NONE);
        final float[][] points = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}};
        polyline = new Polyline(points, 0.1f);
        polylineNode.setGeometry(polyline);
        polylineNode.setPosition(new Vector(0, -1f, -3.3f));
        mScene.getRootNode().addChildNode(polylineNode);
    }

    @Test
    public void polylineTest() {
        testAppendPoints();
        testSetPoints();
        testSetThickness();
    }

    private void testAppendPoints() {
        mMutableTestMethod = () -> {
            polyline.appendPoint(new Vector(getRandCoord(), getRandCoord(), getRandCoord()));
        };

        assertPass("Setting new points every second");
    }

    private void testSetPoints() {

        mMutableTestMethod = () -> {
            final List<Vector> points = Arrays.asList(new Vector(getRandCoord(), getRandCoord(), getRandCoord()),
                    new Vector(getRandCoord(), getRandCoord(), getRandCoord()),
                    new Vector(getRandCoord(), getRandCoord(), getRandCoord()),
                    new Vector(getRandCoord(), getRandCoord(), getRandCoord()));
            polyline.setPoints(points);
        };

        assertPass("Setting new points every second");
    }

    private int getRandCoord() {
        if (Math.random() % 2 == 0) {
            return 0;
        } else {
            return 1;
        }
    }

    private void testSetThickness() {
        mMutableTestMethod = () -> {
            polyline.setThickness(polyline.getThickness() + 0.1f);
        };

        assertPass("Increasing thickness every second", () -> {
            polyline.setThickness(0.1f);
        });
    }
}
