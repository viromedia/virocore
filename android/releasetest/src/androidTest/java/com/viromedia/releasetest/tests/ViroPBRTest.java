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
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;

import com.viro.core.Box;
import com.viro.core.Camera;
import com.viro.core.DirectionalLight;
import com.viro.core.Geometry;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.OmniLight;
import com.viro.core.Sphere;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


public class ViroPBRTest extends ViroBaseTest {

    private Node mSphereContainerNode;
    private Camera mCamera;

    @Override
    void configureTestScene() {
        Node rootNode = mScene.getRootNode();

        List<Vector> lightPositions = new ArrayList<Vector>();
        lightPositions.add(new Vector(-10,  10, 1));
        lightPositions.add(new Vector(10,  10, 1));
        lightPositions.add(new Vector(-10, -10, 1));
        lightPositions.add(new Vector( 10, -10, 1));

        float intensity = 300;
        List<Integer> lightColors = new ArrayList<Integer>();
        lightColors.add(Color.WHITE);
        lightColors.add(Color.WHITE);
        lightColors.add(Color.WHITE);
        lightColors.add(Color.WHITE);

        int rows = 7;
        int columns = 7;
        float spacing = 2.5f;

        mSphereContainerNode = new Node();

        // Render an array of spheres, varying roughness and metalness
        for (int row = 0; row < rows; ++row) {
            float radius = 1;
            float metalness = (float) row / (float) rows;

            for (int col = 0; col < columns; ++col) {
                // Clamp the roughness to [0.05, 1.0] as perfectly smooth surfaces (roughness of 0.0)
                // tend to look off on direct lighting
                float roughness = Math.min(Math.max((float) col / (float) columns, 0.05f), 1.0f);

                Sphere sphere = new Sphere(radius, 20, 20, true);
                Material material = new Material();
                material.setDiffuseColor(Color.rgb(128, 128, 128));
                material.setRoughness(roughness);
                material.setMetalness(metalness);
                material.setLightingModel(Material.LightingModel.PHYSICALLY_BASED);
                sphere.setMaterials(Arrays.asList(material));

                Node sphereNode = new Node();
                sphereNode.setPosition(new Vector((float)(col - (columns / 2)) * spacing,
                                                  (float)(row - (rows    / 2)) * spacing,
                                                  -9.0f));
                sphereNode.setGeometry(sphere);
                mSphereContainerNode.addChildNode(sphereNode);
            }
        }

        rootNode.addChildNode(mSphereContainerNode);

        for (int i = 0; i < lightPositions.size(); i++) {
            OmniLight light = new OmniLight();
            light.setColor(lightColors.get(i));
            light.setPosition(lightPositions.get(i));
            light.setAttenuationStartDistance(20);
            light.setAttenuationEndDistance(30);
            light.setIntensity(intensity);
            rootNode.addLight(light);
        }

        mCamera = new Camera();
        mCamera.setFieldOfView(90);

        Node cameraNode = new Node();
        cameraNode.setPosition(new Vector(0, 0, 0));
        cameraNode.setCamera(mCamera);
        rootNode.addChildNode(cameraNode);

        //mViroView.setPointOfView(cameraNode);
    }

    @Test
    public void testPBR() {
        runUITest(() -> testNewportLoftEnvironment());
        runUITest(() -> testMansOutsideEnvironment());
        runUITest(() -> testRidgecrestRoadEnvironment());
        runUITest(() -> testWoodenDoorEnvironment());
        runUITest(() -> testTextured());
    }

    private void testNewportLoftEnvironment() {
        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/ibl_newport_loft.hdr"));
        mScene.setLightingEnvironment(environment);
        mScene.setBackgroundTexture(environment);

        assertPass("PBR (Newport Loft), roughness increasing X, metalness increasing Y");
    }

    private void testMansOutsideEnvironment() {
        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/ibl_mans_outside.hdr"));
        mScene.setLightingEnvironment(environment);
        mScene.setBackgroundTexture(environment);

        assertPass("PBR (Outside), roughness increasing X, metalness increasing Y");
    }

    private void testRidgecrestRoadEnvironment() {
        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/ibl_ridgecrest_road.hdr"));
        mScene.setLightingEnvironment(environment);
        mScene.setBackgroundTexture(environment);

        assertPass("PBR (Ridgecrest Road), roughness increasing X, metalness increasing Y");
    }

    private void testWoodenDoorEnvironment() {
        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/ibl_wooden_door.hdr"));
        mScene.setLightingEnvironment(environment);
        mScene.setBackgroundTexture(environment);

        assertPass("PBR (Wooden Door), roughness increasing X, metalness increasing Y");
    }

    private void testTextured() {
        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/ibl_newport_loft.hdr"));
        mScene.setLightingEnvironment(environment);
        mScene.setBackgroundTexture(environment);

        Texture albedo = loadTexture("pbr_sample_albedo.png", true);
        Texture roughness = loadTexture("pbr_sample_roughness.png", false);
        Texture metalness = loadTexture("pbr_sample_metallic.png", false);
        Texture normal = loadTexture("pbr_sample_normal.png", false);
        Texture ao = loadTexture("pbr_sample_ao.png", true);

        for (int i = 0; i < mSphereContainerNode.getChildNodes().size(); i++) {
            Node sphereNode = mSphereContainerNode.getChildNodes().get(i);
            Geometry sphere = sphereNode.getGeometry();

            Material material = sphere.getMaterials().get(0);
            material.setDiffuseTexture(albedo);
            material.setRoughnessMap(roughness);
            material.setMetalnessMap(metalness);
            material.setNormalMap(normal);
            material.setAmbientOcclusionMap(ao);
        }

        assertPass("PBR (Newport Loft), textured spheres");
    }

    private Texture loadTexture(final String assetName, boolean sRGB) {
        final InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = mActivity.getAssets().open(assetName);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (final IOException e) {
            return null;
        }
        return new Texture(bitmap, Texture.Format.RGBA8, sRGB, true);
    }
}
