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
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.Box;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Sphere;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;


public class ViroTextureTest extends ViroBaseTest {

    private Box mBox;
    private Sphere mSphere;
    private Node mNode;
    private Node mSphereNode;
    private Texture mTexture;
    private Texture mSphereTexture;
    private static final int BUFFER_SIZE = 1024 * 4;

    @Override
    void configureTestScene() {
        final DirectionalLight light = new DirectionalLight(Color.WHITE, 1000.0f, new Vector(0, 0, -1f));
        mScene.getRootNode().addLight(light);

        mBox = new Box(1, 1, 1);
        mNode = new Node();
        mNode.setPosition(new Vector(0, 0, -3));

        mNode.setGeometry(mBox);
        mScene.getRootNode().addChildNode(mNode);

        mSphere = new Sphere(.5f);
        mSphereNode = new Node();
        mSphereNode.setPosition(new Vector(1, 0, -3));
        mSphereNode.setGeometry(mSphere);

        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        mSphereTexture  = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);
        Material material = new Material();
        material.setDiffuseTexture(mSphereTexture);
        material.setLightingModel(Material.LightingModel.BLINN);

        mSphere.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(mSphereNode);
    }

    @Test
    public void testImages() {
        runUITest(() -> testTextureBitmapConstructorMipMapOn());
        runUITest(() -> testBitMapTextureWrapS());
        runUITest(() -> testBitmapTextureWrapT());
        runUITest(() -> testBitmapMinificationFilter());
        runUITest(() -> testBitmapMagnificationFilter());
        runUITest(() -> testTextureBitmapConstructorMipMapOff());
        runUITest(() -> testTextureBitMapConstructorStereoMode());
        runUITest(() -> testTextureDataConstructor());
        runUITest(() -> testTextureDataConstructorStereoMode());
        runUITest(() -> testTextureCubeMap());
    }

    private void testTextureBitmapConstructorMipMapOn() {
        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        mTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);
        Material material = new Material();
        material.setDiffuseTexture(mTexture);
        material.setLightingModel(Material.LightingModel.BLINN);
        mBox.setMaterials(Arrays.asList(material));
        mNode.setPosition(new Vector(0, 0, -2));
        mMutableTestMethod = () -> {

            Vector position = mNode.getPositionRealtime();
            mNode.setPosition(new Vector(0, 0, position.z - .5f));

        };
        assertPass("Box is textured with mips maps ON.", ()-> {mNode.setPosition(new Vector(0, 0, -3));});

    }

    private void testTextureBitmapConstructorMipMapOff() {
        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        mTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);
        Material material = new Material();
        material.setDiffuseTexture(mTexture);
        mBox.setMaterials(Arrays.asList(material));
        mNode.setPosition(new Vector(0, 0, -1));
        mMutableTestMethod = () -> {

            Vector position = mNode.getPositionRealtime();
            mNode.setPosition(new Vector(0, 0, position.z - .5f));

        };
        assertPass("Box is texturing with mips maps OFF.");
    }


    private void testTextureCubeMap() {
        final Bitmap px = this.getBitmapFromAssets(mActivity,"px.png");
        final Bitmap nx = this.getBitmapFromAssets(mActivity, "nx.png");
        final Bitmap py = this.getBitmapFromAssets(mActivity,"py.png");
        final Bitmap ny = this.getBitmapFromAssets(mActivity, "ny.png");
        final Bitmap pz = this.getBitmapFromAssets(mActivity, "pz.png");
        final Bitmap nz = this.getBitmapFromAssets(mActivity, "nz.png");

        mTexture  = new Texture(px, nx, py, ny,
                pz, nz, Texture.Format.RGBA8);
        mScene.setBackgroundCubeTexture(mTexture);
        assertPass("Background of scene should be cube texture.");
    }

    private void testTextureBitMapConstructorStereoMode() {
        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "stereo1.jpg");
        mTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true, Texture.StereoMode.LEFT_RIGHT);
        Material material = new Material();
        material.setDiffuseTexture(mTexture);
        mBox.setMaterials(Arrays.asList(material));
        mNode.setPosition(new Vector(0, 0, -1));
        mMutableTestMethod = () -> {

            Vector position = mNode.getPositionRealtime();
            mNode.setPosition(new Vector(0, 0, position.z - .5f));

        };
        assertPass("Box is texturing with stereo image.");

    }

    private void testTextureDataConstructor() {
        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        mTexture = new Texture(getRBGAFromBitmap(bobaBitmap), bobaBitmap.getWidth(), bobaBitmap.getHeight(),
                                          Texture.Format.RGBA8, Texture.Format.RGBA8,
                                          true, false, null);
        Material material = new Material();
        material.setDiffuseTexture(mTexture);
        mBox.setMaterials(Arrays.asList(material));
        mNode.setPosition(new Vector(0, 0, -1));
        mMutableTestMethod = () -> {

            Vector position = mNode.getPositionRealtime();
            mNode.setPosition(new Vector(0, 0, position.z - .5f));

        };
        assertPass("Loading from Bitmap, converting to raw, and loading into Texture. Should see textured box.");
    }

    private void testTextureDataConstructorStereoMode() {

    }

    private void testBitMapTextureWrapS() {

        final List<Texture.WrapMode> wrapModes = Arrays.asList(Texture.WrapMode.CLAMP, Texture.WrapMode.MIRROR, Texture.WrapMode.REPEAT);

        final Iterator<Texture.WrapMode> itr = Iterables.cycle(wrapModes).iterator();

        mMutableTestMethod = () -> {
            mSphereTexture.setWrapS(itr.next());
            Material material = new Material();
            material.setDiffuseTexture(mSphereTexture);
            mSphere.setMaterials(Arrays.asList(material));
        };

        assertPass("Loop texture modes for setWrapS from CLAMP, MIRROR, REPEAT.");
    }

    private void testBitmapTextureWrapT() {
        final List<Texture.WrapMode> wrapModes = Arrays.asList(Texture.WrapMode.CLAMP, Texture.WrapMode.MIRROR, Texture.WrapMode.REPEAT);

        final Iterator<Texture.WrapMode> itr = Iterables.cycle(wrapModes).iterator();

        mMutableTestMethod = () -> {
            mSphereTexture.setWrapT(itr.next());
            Material material = new Material();
            material.setDiffuseTexture(mSphereTexture);
            mSphere.setMaterials(Arrays.asList(material));
        };

        assertPass("Loop texture modes for setWrapT from CLAMP, MIRROR, REPEAT.");
    }

    private void testBitmapMinificationFilter() {
        final List<Texture.FilterMode> wrapModes = Arrays.asList(Texture.FilterMode.LINEAR, Texture.FilterMode.NEAREST);

        final Iterator<Texture.FilterMode> itr = Iterables.cycle(wrapModes).iterator();

        mMutableTestMethod = () -> {
            mSphereTexture.setMinificationFilter(itr.next());
            Material material = new Material();
            material.setDiffuseTexture(mSphereTexture);
            mSphere.setMaterials(Arrays.asList(material));
        };

        assertPass("Loop texture modes for setMinificationFilter from LINEAR, NEAREST.");
    }

    private void testBitmapMagnificationFilter() {
        final List<Texture.FilterMode> wrapModes = Arrays.asList(Texture.FilterMode.LINEAR, Texture.FilterMode.NEAREST);

        final Iterator<Texture.FilterMode> itr = Iterables.cycle(wrapModes).iterator();

        mMutableTestMethod = () -> {
            mSphereTexture.setMagnificationFilter(itr.next());
            Material material = new Material();
            material.setDiffuseTexture(mSphereTexture);
            mSphere.setMaterials(Arrays.asList(material));
        };

        assertPass("Loop texture modes for setMagnificationFilter from LINEAR, NEAREST.");
    }

    private ByteBuffer getRBGAFromBitmap(Bitmap bitmap) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int componentsPerPixel = 4;
        int totalPixels = width * height;
        int totalBytes = totalPixels * componentsPerPixel;

        byte[] rgbValues = new byte[totalBytes];
        int[] argbPixels = new int[totalPixels];
        bitmap.getPixels(argbPixels, 0, width, 0, 0, width, height);
        for (int i = 0; i < totalPixels; i++) {
            int argbPixel = argbPixels[i];
            int red = Color.red(argbPixel);
            int green = Color.green(argbPixel);
            int blue = Color.blue(argbPixel);
            int alpha = Color.alpha(argbPixel);
            rgbValues[i * componentsPerPixel + 0] = (byte) red;
            rgbValues[i * componentsPerPixel + 1] = (byte) green;
            rgbValues[i * componentsPerPixel + 2] = (byte) blue;
            rgbValues[i * componentsPerPixel + 3] = (byte) alpha;
        }

        ByteBuffer buffer = ByteBuffer.allocateDirect(rgbValues.length);
        buffer.put(rgbValues);
        buffer.flip();
        return buffer;
    }

    public static byte[] toByteArray(InputStream is) throws IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        try {
            byte[] b = new byte[BUFFER_SIZE];
            int n = 0;
            while ((n = is.read(b)) != -1) {
                output.write(b, 0, n);
            }
            return output.toByteArray();
        } finally {
            output.close();
        }
    }


}
