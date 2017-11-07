/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.AsyncTask;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.amazonaws.util.IOUtils;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Sphere;
import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.Vector;

import org.junit.Test;

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

    private class DownloadImageTask extends AsyncTask<String, Void, ByteBuffer> {


        protected ByteBuffer doInBackground(String... urls) {
            String urldisplay = urls[0];
            try {
                InputStream in = new java.net.URL(urldisplay).openStream();
                byte[] bytes = IOUtils.toByteArray(in);

                ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bytes.length);//ByteBuffer.wrap(bytes);
                byteBuffer.put(bytes, 0, bytes.length);
                return byteBuffer;
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }

        protected void onPostExecute(ByteBuffer byteBuffer) {

            mTexture = new Texture(byteBuffer, 4500, 3000, Texture.TextureFormat.RGBA8, Texture.TextureFormat.RGBA8, true, true, null);
            Material material = new Material();
            material.setDiffuseTexture(mTexture);
            mBox.setMaterials(Arrays.asList(material));
        }
    }

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
        mSphereTexture  = new Texture(bobaBitmap, Texture.TextureFormat.RGBA8, true, true);
        Material material = new Material();
        material.setDiffuseTexture(mSphereTexture);
        material.setLightingModel(Material.LightingModel.BLINN);

        mSphere.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(mSphereNode);
    }

    @Test
    public void testImages() {
        testTextureBitmapConstructorMipMapOn();
        testBitMapTextureWrapS();
        testBitmapTextureWrapT();
        testBitmapMinificationFilter();
        testBitmapMagnificationFilter();
        testTextureBitmapConstructorMipMapOff();
        testTextureBitMapConstructorStereoMode();
        //testTextureDataConstructor();
        //testTextureDataConstructorStereoMode();
        testTextureCubeMap();

    }

    private void testTextureBitmapConstructorMipMapOn() {
        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        mTexture = new Texture(bobaBitmap, Texture.TextureFormat.RGBA8, true, true);
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
        mTexture = new Texture(bobaBitmap, Texture.TextureFormat.RGBA8, true, true);
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
                pz, nz, Texture.TextureFormat.RGBA8);
        mScene.setBackgroundCubeTexture(mTexture);
        assertPass("Background of scene should be cube texture.");
    }

    private void testTextureBitMapConstructorStereoMode() {
        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "stereo1.png");
        mTexture = new Texture(bobaBitmap, Texture.TextureFormat.RGBA8, true, true, Texture.StereoMode.LEFT_RIGHT);
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
        new DownloadImageTask().execute("http://static.pexels.com/photos/45888/landscape-scotland-nature-highlands-and-islands-45888.jpeg");
        assertPass("Loading texture from URL and converting to ByteBuffer. Should see scottish highlands show up.");
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

}
