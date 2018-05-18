/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Bitmap;
import android.graphics.Color;

import com.viro.core.Box;
import com.viro.core.Camera;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Sphere;

import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;


public class ViroMaterialTest extends ViroBaseTest {

    private Material mMaterial;
    private Sphere mSphere;
    private Node mSphereNode;
    private Box mBox;
    private Node mNodeBox;
    private DirectionalLight mDirectionalLight;
    private ArrayList<Material> mMaterialBoxList;
    private Texture mBobaTexture;

    @Override
    void configureTestScene() {
        mScene.setBackgroundCubeWithColor(Color.BLUE);
        mDirectionalLight = new DirectionalLight(Color.WHITE, 2000.0f, new Vector(0, 0, -1));
        mScene.getRootNode().addLight(mDirectionalLight);

        Bitmap bobaBitmap = this.getBitmapFromAssets(mActivity, "boba.png");
        mBobaTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);

        mMaterial = new Material();
        mMaterial.setDiffuseTexture(mBobaTexture);
        mMaterial.setDiffuseColor(Color.RED);
        mMaterial.setLightingModel(Material.LightingModel.BLINN);

        mSphere = new Sphere(5);
        mSphere.setMaterials(Arrays.asList(mMaterial));

        mSphereNode = new Node();
        mSphereNode.setGeometry(mSphere);
        float[] spherePosition = {0, 0, -12};
        mSphereNode.setPosition(new Vector(spherePosition));
        mScene.getRootNode().addChildNode(mSphereNode);

        mBox = new Box(2,2,2);
        mNodeBox = new Node();
        mNodeBox.setOpacity(0.8f);
        float[] boxPosition = {0, 0, -4};
        mNodeBox.setPosition(new Vector(boxPosition));
        mNodeBox.setRotation(new Vector(0f, 45f, 0f));
        mMaterialBoxList = new ArrayList();
        int[] colorArray = {Color.DKGRAY, Color.GRAY, Color.LTGRAY, Color.MAGENTA, Color.CYAN, Color.YELLOW};
        for (int i = 0; i < 6; i++) {
            Material material = new Material();
            material.setLightingModel(Material.LightingModel.CONSTANT);
            material.setDiffuseColor(colorArray[i]);
            mMaterialBoxList.add(material);
        }
        mBox.setMaterials(mMaterialBoxList);
        mNodeBox.setGeometry(mBox);

        Camera camera = new Camera();
        camera.setFieldOfView(90);
        mScene.getRootNode().setCamera(camera);
        mViroView.setPointOfView(mScene.getRootNode());
    }

    @Test
    public void testMaterial() {
        runUITest(() -> testMaterialLightingModelConstant());
        runUITest(() -> testMaterialLightingModelBlinn());
        runUITest(() -> testMaterialLightingModelPhong());
        runUITest(() -> testMaterialLightingModelLambert());

        runUITest(() -> testMaterialSpecularTexture());
        runUITest(() -> testMaterialNormalMapTexture());
        runUITest(() -> testMaterialShininess());

        runUITest(() -> testMaterialBlendNone());
        runUITest(() -> testMaterialBlendAlpha());
        runUITest(() -> testMaterialBlendAdd());
        runUITest(() -> testMaterialBlendSubtract());
        runUITest(() -> testMaterialBlendMultiply());
        runUITest(() -> testMaterialBlendScreen());

        runUITest(() -> testMaterialTransparencyModeAOne());
        runUITest(() -> testMaterialTransparencyModeRGBZero());

        runUITest(() -> testMaterialCullMode());
        runUITest(() -> testMaterialReadsFromDepthBuffer());
        runUITest(() -> testMaterialWritesToDepthBuffer());
        runUITest(() -> testMaterialBloomThreshold());
    }

    private void testMaterialLightingModelConstant() {
        mMaterial.setLightingModel(Material.LightingModel.CONSTANT);
        assertPass("Lighting model on material is Constant");
    }

    private void testMaterialLightingModelBlinn() {
        mMaterial.setLightingModel(Material.LightingModel.BLINN);
        mSphere.setMaterials(Arrays.asList(mMaterial));
        assertPass("Lighting model on material is Blinn");
    }

    private void testMaterialLightingModelPhong() {
        mMaterial.setLightingModel(Material.LightingModel.PHONG);
        assertPass("Lighting model on material is Phong");
    }

    private void testMaterialLightingModelLambert() {
        mMaterial.setLightingModel(Material.LightingModel.LAMBERT);
        assertPass("Lighting model on material is Lambert");
    }

    private void testMaterialSpecularTexture() {
        Bitmap specBitmap = this.getBitmapFromAssets(mActivity, "specular.png");
        Texture specTexture = new Texture(specBitmap, Texture.Format.RGBA8, true, true);
        mMaterial.setSpecularTexture(specTexture);
        mSphere.setMaterials(Arrays.asList(mMaterial));
        assertPass("Added specular texture to the material");
    }

    private void testMaterialNormalMapTexture() {
        Bitmap specBitmap = this.getBitmapFromAssets(mActivity, "earth_normal.jpg");
        Texture specTexture = new Texture(specBitmap, Texture.Format.RGBA8, true, true);
        mMaterial.setNormalMap(specTexture);
        mSphere.setMaterials(Arrays.asList(mMaterial));
        assertPass("Added normal map to the material.");
    }

    private void testMaterialShininess() {
        mMutableTestMethod = () -> {
            if (mMaterial != null && mMaterial.getShininess() < 10) {
                mMaterial.setShininess(mMaterial.getShininess() + .5f);
                mSphere.setMaterials(Arrays.asList(mMaterial));
            }
        };
        assertPass("Shininess increases over time.");
    }

    private void testMaterialBlendNone() {
        mScene.setBackgroundCubeWithColor(Color.WHITE);
        mSphereNode.setOpacity(0.5f);
        mMaterial.setDiffuseTexture(null);
        mMaterial.setBlendMode(Material.BlendMode.NONE);
        assertPass("Blend mode is NONE: red sphere over white background");
    }

    private void testMaterialBlendAlpha() {
        mScene.setBackgroundCubeWithColor(Color.WHITE);
        mSphereNode.setOpacity(0.5f);
        mMaterial.setDiffuseTexture(null);
        mMaterial.setBlendMode(Material.BlendMode.ALPHA);
        assertPass("Blend mode is Alpha: faded red sphere over white background");
    }

    private void testMaterialBlendAdd() {
        mScene.setBackgroundCubeWithColor(Color.GREEN);
        mSphereNode.setOpacity(0.5f);
        mMaterial.setDiffuseTexture(null);
        mMaterial.setBlendMode(Material.BlendMode.ADD);
        assertPass("Blend mode is ADD: yellow sphere over green background");
    }

    private void testMaterialBlendSubtract() {
        mScene.setBackgroundCubeWithColor(Color.YELLOW);
        mMaterial.setDiffuseTexture(null);
        mMaterial.setBlendMode(Material.BlendMode.SUBTRACT);
        assertPass("Blend mode is SUBTRACT: green sphere over yellow background");
    }

    private void testMaterialBlendMultiply() {
        mScene.setBackgroundCubeWithColor(Color.GRAY);
        mMaterial.setDiffuseTexture(null);
        mMaterial.setDiffuseColor(Color.GRAY);
        mMaterial.setBlendMode(Material.BlendMode.MULTIPLY);
        assertPass("Blend mode is MULTIPLY: sphere is darker gray than background");
    }

    private void testMaterialBlendScreen() {
        mScene.setBackgroundCubeWithColor(Color.GRAY);
        mMaterial.setDiffuseTexture(null);
        mMaterial.setDiffuseColor(Color.GRAY);
        mMaterial.setBlendMode(Material.BlendMode.SCREEN);
        assertPass("Blend mode is SCREEN: sphere is lighter gray than background");
    }

    private void testMaterialTransparencyModeAOne() {
        mScene.setBackgroundCubeWithColor(Color.BLUE);
        mMaterial.setDiffuseColor(Color.RED);
        mMaterial.setDiffuseTexture(mBobaTexture);
        mMaterial.setTransparencyMode(Material.TransparencyMode.A_ONE);
        mSphere.setMaterials(Arrays.asList(mMaterial));
        assertPass("TransparencyMode set to A_ONE");
    }


    private void testMaterialTransparencyModeRGBZero() {
        mScene.setBackgroundCubeWithColor(Color.BLUE);
        mMaterial.setDiffuseColor(Color.RED);
        mMaterial.setDiffuseTexture(mBobaTexture);
        mMaterial.setTransparencyMode(Material.TransparencyMode.RGB_ZERO);
        mSphere.setMaterials(Arrays.asList(mMaterial));
        assertPass("TransparencyMode set to RGB_ZERO");
    }

    private void testMaterialCullMode() {
        mScene.getRootNode().addChildNode(mNodeBox);
        mMutableTestMethod = () -> {
            Material.CullMode newCullMode = Material.CullMode.BACK;
            if (mMaterialBoxList.get(0).getCullMode() == Material.CullMode.BACK) {
                 newCullMode  = Material.CullMode.FRONT;
            } else if(mMaterialBoxList.get(0).getCullMode() == Material.CullMode.FRONT) {
                newCullMode = Material.CullMode.NONE;
            } else if(mMaterialBoxList.get(0).getCullMode() == Material.CullMode.NONE) {
                newCullMode = Material.CullMode.BACK;
            }

            for(Material material: mMaterialBoxList) {
                material.setCullMode(newCullMode);
            }
            mBox.setMaterials(mMaterialBoxList);
        };
        assertPass("CullMode loops from FRONT to BACK to NONE.");
    }

    private void testMaterialReadsFromDepthBuffer() {
        mMutableTestMethod = () -> {
            mMaterial.setReadsFromDepthBuffer(!mMaterial.getReadsFromDepthBuffer());
            mSphere.setMaterials(Arrays.asList(mMaterial));
        };
        assertPass("Sphere should flip from being drawn in front of box to behind.");
    }

    private void testMaterialWritesToDepthBuffer() {
        mNodeBox.setPosition(new Vector(0, 0, -15));
        mMutableTestMethod = () -> {
            mMaterial.setWritesToDepthBuffer(!mMaterial.getWritesToDepthBuffer());
            mSphere.setMaterials(Arrays.asList(mMaterial));
        };
        assertPass("Box should appear and disappear.");
    }

    private void testMaterialBloomThreshold() {
        mNodeBox.setPosition(new Vector(0, 0, -15));
        mMaterial.setBloomThreshold(0.0f);
        mMutableTestMethod = () -> {
            if (mMaterial.getBloomThreshold() <= 1.0f) {
                mMaterial.setBloomThreshold(mMaterial.getBloomThreshold() + .1f);
            }
        };
        assertPass("Bloom threshold changes low to high (sphere goes from bright to dim)");
    }
}
