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

import com.viro.core.AmbientLight;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;


/**
 * Created by vadvani on 11/6/17.
 */

public class ViroAnimationTransactionTest extends ViroBaseTest {

    private Node boxNode;
    private Box mBox;
    @Override
    void configureTestScene() {
        final AmbientLight light = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(light);
        boxNode = new Node();
        mBox = new Box(1, 1, 1);
        final Material material = new Material();
        material.setLightingModel(Material.LightingModel.BLINN);
        material.setDiffuseColor(Color.BLUE);
        mBox.setMaterials(Arrays.asList(material));
        boxNode.setGeometry(mBox);
        boxNode.setPosition(new Vector(-3,0, -3));
        mScene.getRootNode().addChildNode(boxNode);

    }

    @Test
    public void animationTest() {
            testAnimationPosition();
            testAnimationRotation();
            testAnimationScale();
            testAnimationOpacity();
            testAnimationEasing();
            testAnimationMaterial();
            testAnimationChained();
    }

    private void testAnimationPosition() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setPosition(new Vector(3, 0, -3));
        AnimationTransaction.commit();
        assertPass("Position animates from x -3 to 3.", () -> {
            boxNode.setPosition(new Vector(0, 0, -3));
        });
    }

    private void testAnimationRotation() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setRotation(new Vector(0, 0.78, 0.78));
        AnimationTransaction.commit();
        assertPass("Animate rotation to 45 degrees on y and z axis.", ()->{
            boxNode.setRotation(new Vector(0, 0, 0));
        });
    }

    private void testAnimationScale() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setScale(new Vector(.2f, .2f, 2f));
        AnimationTransaction.commit();
        assertPass("Animate animates from scale 1 to .2", ()-> {
            boxNode.setScale(new Vector(1f, 1f, 1f));
        });
    }

    private void testAnimationMaterial() {
        final Material material = new Material();
        material.setLightingModel(Material.LightingModel.BLINN);
        material.setDiffuseColor(Color.RED);
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(4000);
        mBox.setMaterials(Arrays.asList(material));
        AnimationTransaction.commit();
        assertPass("Material blends from blue to red.");
    }

    private void testAnimationOpacity() {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(3000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
        boxNode.setOpacity(0.2f);
        AnimationTransaction.commit();
        assertPass("Animate opacity from 1 to 0.2", ()->{
            boxNode.setOpacity(1.0f);
        });
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

        assertPass("Chained Animation should move box back then rotate.", ()->{
            boxNode.setPosition(new Vector(0, 0, -3));
            boxNode.setRotation(new Vector(0, 0, 0));
        });
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
        assertPass("Animation easing is set to:" + animTimingFunction.toString(), ()->{
            boxNode.setPosition(new Vector(0, 0, -3));
        });
    }

}
