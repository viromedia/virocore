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
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.AmbientLight;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Text;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;


/**
 * Created by vadvani on 11/6/17.
 */

public class ViroAnimationTransactionTest extends ViroBaseTest {

    private Node boxNode;
    private Box mBox;
    private Material mMaterial;
    private AnimationTransaction mTransaction;
    @Override
    void configureTestScene() {
        final AmbientLight light = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(light);
        boxNode = new Node();
        mBox = new Box(1, 1, 1);
        mMaterial = new Material();
        mMaterial.setLightingModel(Material.LightingModel.BLINN);
        mMaterial.setDiffuseColor(Color.BLUE);
        mBox.setMaterials(Arrays.asList(mMaterial));
        boxNode.setGeometry(mBox);
        boxNode.setPosition(new Vector(0,0, -3));
        boxNode.setRotation(new Vector(0, 0, 0));
        boxNode.setScale(new Vector(1f, 1f, 1f));
        boxNode.setOpacity(1.0f);
        mScene.getRootNode().addChildNode(boxNode);

    }

    @Test
    public void animationTest() {
        runUITest(() -> testAnimationPosition());
        runUITest(() -> testAnimationRotation());
        runUITest(() -> testAnimationScale());
        runUITest(() -> testAnimationOpacity());
        runUITest(() -> testAnimationEasing());
        runUITest(() -> testAnimationMaterial());
        runUITest(() -> testAnimationChained());
        runUITest(() -> testAnimationLoop());
    }

    private void testAnimationPosition() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setPosition(new Vector(3, 0, -3));
        AnimationTransaction.commit();
        assertPass("Position animates from x -3 to 3.");
    }

    private void testAnimationRotation() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setRotation(new Vector(0, 0.78, 0.78));
        AnimationTransaction.commit();
        assertPass("Animate rotation to 45 degrees on y and z axis.");
    }

    private void testAnimationScale() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setScale(new Vector(.2f, .2f, 2f));
        AnimationTransaction.commit();
        assertPass("Animate animates from scale 1 to .2");
    }

    private void testAnimationMaterial() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(4000);
        mBox.getMaterials().get(0).setDiffuseColor(Color.RED);
        AnimationTransaction.commit();
        assertPass("Material blends from blue to red.");
    }

    private void testAnimationOpacity() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setOpacity(0.2f);
        AnimationTransaction.commit();
        assertPass("Animate opacity from 1 to 0.2");
    }

    private void testAnimationChained() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(2000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.Linear);
        boxNode.setPosition(new Vector(0, 0, -6));
        AnimationTransaction.setListener((transaction)-> {
                AnimationTransaction.begin();
                    AnimationTransaction.setAnimationDuration(2000);
            boxNode.setRotation(new Vector(0, 1.04, 0));
                AnimationTransaction.commit();
        });
        AnimationTransaction.commit();

        assertPass("Chained Animation should move box back then rotate.");
    }

    private void testAnimationLoop() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(1000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        AnimationTransaction.setAnimationLoop(true);
        boxNode.setRotation(new Vector(0, 0.78, 0.78));
        final List<Integer> colors = Arrays.asList(Color.WHITE, Color.RED, Color.GREEN, Color.BLUE, Color.MAGENTA, Color.CYAN);
        final Iterator<Integer> itr = Iterables.cycle(colors).iterator();
        AnimationTransaction.setListener(transaction -> {
            mMaterial.setDiffuseColor(itr.next());
            mBox.setMaterials(Arrays.asList(mMaterial));
        });
        mTransaction = AnimationTransaction.commit();
        assertPass("Loop Animate rotation to 45 degrees on y and z axis," +
                " should change color every second");
    }
    private void testAnimationEasing() {
        for (final AnimationTimingFunction animationTimingFunction : AnimationTimingFunction.values()) {
            testEasing(animationTimingFunction);
        }
    }

    private void testEasing(final AnimationTimingFunction animTimingFunction) {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(2000);
        AnimationTransaction.setTimingFunction(animTimingFunction);
        boxNode.setPosition(new Vector(0, 0, -5));
        AnimationTransaction.commit();
        assertPass("Animation easing is set to:" + animTimingFunction.toString());
    }

}
