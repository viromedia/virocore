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

import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.Image;
import com.viro.renderer.jni.Vector;

import org.junit.Test;


public class ViroTextureTest extends ViroBaseTest {

    @Override
    void configureTestScene() {
        DirectionalLight light = new DirectionalLight(Color.WHITE, 1000.0f, new Vector(0, 0, -1f));
        mScene.getRootNode().addLight(light);
        Image image = new Image();

    }

    @Test
    public void testImages() {
        testImageConstructorString();
        testImageConstructorBitmap();
        testImage
    }



}
