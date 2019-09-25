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

import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Log;

import com.viro.core.AmbientLight;
import com.viro.core.Geometry;
import com.viro.core.Submesh;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;

public class ViroGeometryTest extends ViroBaseTest {

    private Node mRootNode;
    private AmbientLight mAmbientLight;

    @Override
    void configureTestScene() {
        mAmbientLight = new AmbientLight(Color.WHITE, 1000f);
        mRootNode = mScene.getRootNode();
        mRootNode.addLight(mAmbientLight);
    }

    @Test
    public void test() {
        runUITest(() -> testSimpleGeometry());
        runUITest(() -> testGeometryMissingData());
        runUITest(() -> testGeometryWithTexture());
        runUITest(() -> testGeometryTwoElements());
    }

    public void testSimpleGeometry() {
        Node node = new Node();
        node.setPosition(new Vector(0,-1, -3));

        Geometry geo = new Geometry();
        geo.setVertices(Arrays.asList(new Vector(1, 0, 0), new Vector(0, 1, 0), new Vector(-1, 0, 0)));
        geo.setNormals(Arrays.asList(new Vector(0, 0, 1), new Vector(0, 0, 1), new Vector(0, 0, 1)));
        geo.setTextureCoordinates(Arrays.asList(new Vector(1, 0, 0), new Vector(0, 1, 0), new Vector(-1, 0, 0)));

        Material material = new Material();
        material.setDiffuseColor(Color.BLUE);
        geo.setMaterials(Arrays.asList(material));

        Submesh submesh = new Submesh();
        submesh.setTriangleIndices(Arrays.asList(0, 1, 2));
        geo.setSubmeshes(Arrays.asList(submesh));

        node.setGeometry(geo);
        mRootNode.addChildNode(node);

        assertPass("You should see one blue triangle", ()->{
            node.removeFromParentNode();
        });
    }

    public void testGeometryMissingData() {
        Node node = new Node();
        node.setPosition(new Vector(0,-1, -3));

        Geometry geo = new Geometry();
        geo.setVertices(Arrays.asList(new Vector(1, 0, 0), new Vector(0, 1, 0), new Vector(-1, 0, 0)));

        Material material = new Material();
        material.setDiffuseColor(Color.RED);
        geo.setMaterials(Arrays.asList(material));

        Submesh element = new Submesh();
        element.setTriangleIndices(Arrays.asList(0, 1, 2));
        geo.setSubmeshes(Arrays.asList(element));

        node.setGeometry(geo);
        mRootNode.addChildNode(node);

        assertPass("You should see one red triangle", ()->{
            node.removeFromParentNode();
        });
    }

    public void testGeometryWithTexture() {
        Node node = new Node();
        node.setPosition(new Vector(-0.5,-1, -3));

        Geometry geo = new Geometry();
        geo.setVertices(Arrays.asList(new Vector(0, 0, 0), new Vector(1, 0, 0), new Vector(1, 1, 0), new Vector(0, 1, 0)));
        geo.setNormals( Arrays.asList(new Vector(0, 0, 1), new Vector(0, 0, 1), new Vector(0, 0, 1), new Vector(0, 0, 1)));
        geo.setTextureCoordinates(Arrays.asList(new Vector(0, 0, 0), new Vector(1, 0, 0), new Vector(1, 1, 0), new Vector(0, 1, 0)));

        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        Texture texture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);

        Material material = new Material();
        material.setDiffuseTexture(texture);
        geo.setMaterials(Arrays.asList(material));

        Submesh submesh = new Submesh();
        submesh.setTriangleIndices(Arrays.asList(0, 1, 2, 2, 3, 0));
        geo.setSubmeshes(Arrays.asList(submesh));

        node.setGeometry(geo);
        mRootNode.addChildNode(node);

        assertPass("You should see a square with a pug texture", ()->{
            node.removeFromParentNode();
        });
    }

    public void testGeometryTwoElements() {
        Node node = new Node();
        node.setPosition(new Vector(-0.5,-1, -3));

        Geometry geo = new Geometry();
        geo.setVertices(Arrays.asList(new Vector(-1, 0, 0), new Vector(0, 0, 0), new Vector(0, 1, 0), new Vector(-1, 1, 0),
                                      new Vector(1, 0, 0), new Vector(2, 0, 0), new Vector(2, 1, 0), new Vector(1, 1, 0)));
        geo.setNormals( Arrays.asList(new Vector(0, 0, 1), new Vector(0, 0, 1), new Vector(0, 0, 1), new Vector(0, 0, 1),
                                      new Vector(0, 0, 1), new Vector(0, 0, 1), new Vector(0, 0, 1), new Vector(0, 0, 1)));
        geo.setTextureCoordinates(Arrays.asList(new Vector(0, 0, 0), new Vector(1, 0, 0), new Vector(1, 1, 0), new Vector(0, 1, 0),
                                                new Vector(0, 0, 0), new Vector(1, 0, 0), new Vector(1, 1, 0), new Vector(0, 1, 0)));

        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        Texture texture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);

        Material materialA = new Material();
        materialA.setDiffuseTexture(texture);
        Material materialB = new Material();
        materialB.setDiffuseTexture(texture);
        geo.setMaterials(Arrays.asList(materialA, materialB));

        Submesh submeshA = new Submesh();
        submeshA.setTriangleIndices(Arrays.asList(0, 1, 2, 2, 3, 0));
        Submesh submeshB = new Submesh();
        submeshB.setTriangleIndices(Arrays.asList(4, 5, 6, 6, 7, 4));
        geo.setSubmeshes(Arrays.asList(submeshA, submeshB));

        node.setGeometry(geo);
        mRootNode.addChildNode(node);

        assertPass("You should see two squares with pug textures", ()->{
            node.removeFromParentNode();
        });
    }

}
