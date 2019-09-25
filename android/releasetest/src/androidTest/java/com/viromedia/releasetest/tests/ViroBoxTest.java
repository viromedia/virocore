//
//  Copyright (c) 2017-present, ViroMedia, Inc.
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

import com.viro.core.AmbientLight;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;

/**
 * Created by vadvani on 10/26/17.
 */


public class ViroBoxTest extends ViroBaseTest {
    private Box mBox;

    public ViroBoxTest() {
        super();
        mBox = null;
    }

    @Override
    void configureTestScene() {
        final float[] lightDirection = {0, 0, -1};
        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(ambientLightJni);

        final Material material = new Material();
        material.setDiffuseColor(Color.BLUE);
        material.setLightingModel(Material.LightingModel.BLINN);

        // Creation of ViroBox
        final Node node = new Node();
        mBox = new Box(1, 1, 1);

        node.setGeometry(mBox);
        final float[] boxPosition = {0, -0.5f, -5};
        node.setPosition(new Vector(boxPosition));
        mBox.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(node);
    }

    @Test
    public void boxTest() {
        runUITest(() -> boxWidthTest());
        runUITest(() -> boxHeightTest());
        runUITest(() -> boxLengthTest());
    }


    public void boxWidthTest() {
        mMutableTestMethod = () -> {
            if (mBox != null && mBox.getWidth() < 5) {
                mBox.setWidth(mBox.getWidth() + 1);
            }
        };
        assertPass("Box changed in width from 1 to 5");
    }

    public void boxHeightTest() {
        mMutableTestMethod = () -> {
            if (mBox != null && mBox.getHeight() < 5) {
                mBox.setHeight(mBox.getHeight() + 1);
            }
        };
        assertPass("Box changed in height from 1 to 5");
    }

    public void boxLengthTest() {
        mMutableTestMethod = () -> {
            if (mBox != null && mBox.getLength() < 5) {
                mBox.setLength(mBox.getLength() + 1);
            }
        };
        assertPass("Box changed in length from 1 to 5");
    }
}
