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
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Polygon;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class ViroPolygonTest extends ViroBaseTest {

    private Node mPolygonNode;
    private Handler mHandler;
    private Material mMaterial;`

    @Override
    void configureTestScene() {
        mMaterial = new Material();
        mMaterial.setDiffuseColor(Color.RED);
        mMaterial.setLightingModel(Material.LightingModel.CONSTANT);

        mPolygonNode = new Node();
        mPolygonNode.setPosition(new Vector(0, 0, -5));
        mScene.getRootNode().addChildNode(mPolygonNode);
    }

    @Test
    public void polylineTest() {
        runUITest(() -> testConvex());
        runUITest(() -> testConcave());
        runUITest(() -> testHoles());
    }

    private void testConvex() {
        ArrayList<Vector> pointsList = new ArrayList<Vector>();
        pointsList.add(new Vector(-1, -1, 0));
        pointsList.add(new Vector(-1, 1, 0));
        pointsList.add(new Vector(1, 1, 0));
        pointsList.add(new Vector(1, -1, 0));

        Polygon polygon = new Polygon(pointsList, 0, 0, 1, 1);
        polygon.setMaterials(Arrays.asList(mMaterial));
        mPolygonNode.setGeometry(polygon);

        assertPass("Square polygon");
    }

    private void testConcave() {
        List<Vector> path = new ArrayList<Vector>();
        path.add(new Vector( -1,      -1, 0 ));
        path.add(new Vector( -1,       1, 0 ));
        path.add(new Vector(  1,       1, 0 ));
        path.add(new Vector(  1,     0.5, 0 ));
        path.add(new Vector(  0.5,   0.5, 0 ));
        path.add(new Vector(  0.5,  0.25, 0 ));
        path.add(new Vector(  1,    0.25, 0 ));
        path.add(new Vector(  1,      -1, 0 ));

        Polygon polygon = new Polygon(path, 0, 0, 1, 1);
        polygon.setMaterials(Arrays.asList(mMaterial));
        mPolygonNode.setGeometry(polygon);

        assertPass("Square polygon with an indent on the right");
    }

    private void testHoles() {
        List<Vector> path = new ArrayList<Vector>();
        path.add(new Vector( -1,      -1, 0 ));
        path.add(new Vector( -1,       1, 0 ));
        path.add(new Vector(  1,       1, 0 ));
        path.add(new Vector(  1,     0.5, 0 ));
        path.add(new Vector(  0.5,   0.5, 0 ));
        path.add(new Vector(  0.5,  0.25, 0 ));
        path.add(new Vector(  1,    0.25, 0 ));
        path.add(new Vector(  1,      -1, 0 ));

        List<Vector> holeA = new ArrayList<Vector>();
        holeA.add(new Vector( -0.75, -0.75, 0 ));
        holeA.add(new Vector( -0.75, -0.50, 0 ));
        holeA.add(new Vector( -0.50, -0.50, 0 ));
        holeA.add(new Vector( -0.50, -0.75, 0 ));

        List<Vector> holeB = new ArrayList<Vector>();
        holeB.add(new Vector( 0.75, -0.75, 0 ));
        holeB.add(new Vector( 0.75, -0.50, 0 ));
        holeB.add(new Vector( 0.50, -0.50, 0 ));
        holeB.add(new Vector( 0.50, -0.75, 0 ));

        List<List<Vector>> holes = new ArrayList<List<Vector>>();
        holes.add(holeA);
        holes.add(holeB);

        Polygon polygon = new Polygon(path, holes, 0, 0, 1, 1);
        polygon.setMaterials(Arrays.asList(mMaterial));
        mPolygonNode.setGeometry(polygon);

        assertPass("Square polygon with an indent on the right and two holes on the bottom");
    }
}
