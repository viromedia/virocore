//
//  Copyright (c) 2018-present, ViroMedia, Inc.
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
import android.support.test.espresso.core.deps.guava.collect.Iterables;
import android.util.Log;
import android.graphics.Point;


import com.viro.core.AmbientLight;

import com.viro.core.FuseListener;
import com.viro.core.HitTestListener;
import com.viro.core.HitTestResult;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Sphere;
import com.viro.core.Vector;


import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 *
 * Created by vadvani on 10/15/18.
 */

public class ViroHitTest extends ViroBaseTest  {

    private HitTestListener mHitTestListener;
    @Override
    void configureTestScene() {
        AmbientLight light = new AmbientLight(Color.WHITE, 2000.0f);
        mScene.getRootNode().addLight(light);

        final Object3D objectJni = new Object3D();
        objectJni.setPosition(new Vector(0,0,-200));

        mViroView.setVRModeEnabled(true);


        final Material material = new Material();
        material.setDiffuseColor(Color.RED);
        material.setLightingModel(Material.LightingModel.BLINN);

        final Node node = new Node();
        node.setHighAccuracyEvents(true);
        node.setTag("SPHERE");
        Sphere sphereOne = new Sphere(7);
        node.setGeometry(sphereOne);
        final float[] spherePosition = {0f, 0, -15};
        node.setPosition(new Vector(spherePosition));
        sphereOne.setMaterials(Arrays.asList(material));
        node.setFuseListener(new FuseListener() {
            @Override
            public void onFuse(int source, Node node) {

            }
        });

        mScene.getRootNode().addChildNode(node);
    }

    @Test
    public void testARHitTest() {
        runUITest(() -> testHitTestWithScreenPointHighAccuracyTest());
        runUITest(() -> testHitTestWithScreenPointBoundaryTest());
        runUITest(() -> testHitTestWithRayHighAccuracyTest());
        runUITest(() -> testHitTestWithRayBoundaryTest());
    }

    public void testHitTestWithScreenPointBoundaryTest() {
        Vector forward = mViroView.getLastCameraForwardRealtime();
        mHitTestListener = new HitTestListener() {
            @Override
            public void onHitTestFinished(HitTestResult[] results) {
                if(results != null && results.length > 0) {
                    Sphere sphere = new Sphere(.1f);
                    Material material = new Material();
                    material.setLightingModel(Material.LightingModel.BLINN);
                    material.setDiffuseColor(Color.GREEN);
                    sphere.setMaterials(Arrays.asList(material));
                    Node nodeSphere = new Node();
                    Vector intPoint = results[0].getIntersectionPoint();
                    nodeSphere.setPosition(results[0].getIntersectionPoint());
                    nodeSphere.setGeometry(sphere);
                    if (intPoint.z <= -5) {
                        mScene.getRootNode().addChildNode(nodeSphere);
                    }
                }
            }
        };

        mMutableTestMethod = () -> {
            mViroView.performSceneHitTestWithPoint(new Point(400, 500), true, mHitTestListener);
        };

        assertPass("ViroHitTest: Intersected screen point(400, 500) where sphere should be GREEN. points should follow sphere geometry. ", ()->{

        });
    }

    public void testHitTestWithScreenPointHighAccuracyTest() {
        Vector forward = mViroView.getLastCameraForwardRealtime();
        Vector fowardWithDistance = new Vector(forward.x * 2.f, forward.y * 2.f, forward.z * 2.f);
        mHitTestListener = new HitTestListener() {
            @Override
            public void onHitTestFinished(HitTestResult[] results) {
                if(results != null && results.length > 0) {
                    Sphere sphere = new Sphere(.1f);
                    Material material = new Material();
                    material.setLightingModel(Material.LightingModel.BLINN);
                    material.setDiffuseColor(Color.BLUE);
                    sphere.setMaterials(Arrays.asList(material));
                    Node nodeSphere = new Node();
                    Vector intPoint = results[0].getIntersectionPoint();
                    nodeSphere.setPosition(results[0].getIntersectionPoint());
                    nodeSphere.setGeometry(sphere);
                    if (intPoint.z <= -5) {
                        mScene.getRootNode().addChildNode(nodeSphere);
                    }
                }
            }
        };

        mMutableTestMethod = () -> {
            mViroView.performSceneHitTestWithPoint(new Point(400, 500), false, mHitTestListener);
        };

        assertPass("ViroHitTest: Intersected screen point(400,500) on sphere bounding box. Should be BLUE.", ()->{

        });
    }

    public void testHitTestWithRayHighAccuracyTest() {
        Vector forward = mViroView.getLastCameraForwardRealtime();
        mHitTestListener = new HitTestListener() {
            @Override
            public void onHitTestFinished(HitTestResult[] results) {
                if(results != null && results.length > 0) {
                    Sphere sphere = new Sphere(.1f);
                    Material material = new Material();
                    material.setLightingModel(Material.LightingModel.BLINN);
                    material.setDiffuseColor(Color.BLACK);
                    sphere.setMaterials(Arrays.asList(material));
                    Node nodeSphere = new Node();
                    nodeSphere.setPosition(results[0].getIntersectionPoint());
                    nodeSphere.setGeometry(sphere);
                    mScene.getRootNode().addChildNode(nodeSphere);
                }
            }
        };

        final List<Vector> points = Arrays.asList(new Vector(-3, 0, -10),new Vector(0, 0, -10) ,new Vector(3, 0, -10) );
        final Iterator<Vector> itr = Iterables.cycle(points).iterator();
        mMutableTestMethod = () -> {
            mViroView.performSceneHitTestRay(new Vector(0,0,0),itr.next(), false, mHitTestListener);
        };

        assertPass("ViroHitTest: Intersected points of Sphere Bounding Box. Should be BLACK.", ()->{

        });
    }

    public void testHitTestWithRayBoundaryTest() {
        Vector forward = mViroView.getLastCameraForwardRealtime();
        mHitTestListener = new HitTestListener() {
            @Override
            public void onHitTestFinished(HitTestResult[] results) {
                if(results != null && results.length > 0) {
                    Sphere sphere = new Sphere(.1f);
                    Material material = new Material();
                    material.setLightingModel(Material.LightingModel.BLINN);
                    material.setDiffuseColor(Color.YELLOW);
                    sphere.setMaterials(Arrays.asList(material));
                    Node nodeSphere = new Node();
                    nodeSphere.setPosition(results[0].getIntersectionPoint());
                    nodeSphere.setGeometry(sphere);
                    mScene.getRootNode().addChildNode(nodeSphere);
                }
            }
        };
        final List<Vector> points = Arrays.asList(new Vector(-3, 0, -10),new Vector(0, 0, -10) ,new Vector(3, 0, -10) );
        final Iterator<Vector> itr = Iterables.cycle(points).iterator();
        mMutableTestMethod = () -> {
            mViroView.performSceneHitTestRay(new Vector(0,0,0), itr.next(), true, mHitTestListener);
        };

        assertPass("ViroHitTest: Intersected points of sphere. Should be YELLOW.", ()->{

        });
    }
}
