/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests;

import android.content.Context;
import android.graphics.Color;
import android.support.test.InstrumentationRegistry;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.Image;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.OmniLight;
import com.viro.renderer.jni.Scene;
import com.viro.renderer.jni.Spotlight;
import com.viro.renderer.jni.Text;
import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.TextureFormat;
import com.viro.renderer.jni.Vector;
import com.viro.renderer.jni.ViroView;
import com.viromedia.renderertest.ViroReleaseTestActivity;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

import static org.awaitility.Awaitility.await;
import static org.junit.Assert.assertEquals;

/**
 * Created by manish on 10/25/17.
 */

@RunWith(AndroidJUnit4.class)
public class ViroAmbientLightTest {
    private static final String TAG = ViroAmbientLightTest.class.getName();
    private Boolean mGLInitialized = false;
    private ViroView mViroView;
    private ViroReleaseTestActivity mActivity;
    @Rule
    public ActivityTestRule<ViroReleaseTestActivity> rule
            = new ActivityTestRule<>(ViroReleaseTestActivity.class, true, true);

    @Before
    public void setUp() {
        System.out.println(TAG + "setUp called");
        mActivity = rule.getActivity();

        await().until(glInitialized());

        Scene scene = new Scene();
        Node rootNode = scene.getRootNode();
        List<Node> nodes = new ArrayList<>();

        mViroView = mActivity.getViroView();

        nodes = testBox(mActivity);
        for (Node node: nodes) {
            rootNode.addChildNode(node);
        }

        testSceneLighting(rootNode);
        mViroView.setScene(scene);
    }

    private List<Node> testBox(Context context) {
        Node node1 = new Node();
        Node node2 = new Node();

        Node node3 = new Node();
        Text textJni = new Text(mViroView.getViroContext(), "Test text 1 2 3", "Roboto", 24,
                Color.WHITE, 10, 4, Text.HorizontalAlignment.CENTER, Text.VerticalAlignment.CENTER, Text.LineBreakMode.NONE,
                Text.ClipMode.CLIP_TO_BOUNDS, 1);

        float[] position = {0, -1, -2};
        node3.setPosition(new Vector(position));
        node3.setGeometry(textJni);
        //node3.setEventDelegate(getGenericDelegate("Text"));

        // Create a new material with a diffuseTexture set to the image "boba.png"
        Image bobaImage = new Image("boba.png", TextureFormat.RGBA8);

        Texture bobaTexture = new Texture(bobaImage, TextureFormat.RGBA8, true, true);
        Material material = new Material();
//        material.setTexture(bobaTexture, "diffuseTexture");
        material.setColor(Color.BLUE, "diffuseColor");
        material.setLightingModel("Blinn");

        // Creation of ViroBox to the right and billboarded
        Box boxGeometry = new Box(2,4,2);
        node1.setGeometry(boxGeometry);
        float[] boxPosition = {5,0,-3};
        node1.setPosition(new Vector(boxPosition));
        boxGeometry.setMaterials(Arrays.asList(material));
        EnumSet<Node.TransformBehavior> behaviors = EnumSet.of(Node.TransformBehavior.BILLBOARD);
        node1.setTransformBehaviors(behaviors);
        node1.setEventDelegate(mActivity.getGenericDelegate("Box"));

        Box boxGeometry2 = new Box(2, 2, 2);
        node2.setGeometry(boxGeometry2);
        float[] boxPosition2 = {-2, 0, -3};
        node2.setPosition(new Vector(boxPosition2));
        boxGeometry2.setMaterials(Arrays.asList(material));
        node2.setEventDelegate(mActivity.getGenericDelegate("Box2"));

        return Arrays.asList(node1, node2, node3);
    }

    private void testSceneLighting(Node node) {
        float[] lightDirection = {0, 0, -1};
        AmbientLight ambientLightJni = new AmbientLight(Color.BLACK, 1000.0f);
        node.addLight(ambientLightJni);

        DirectionalLight directionalLightJni = new DirectionalLight(Color.BLUE, 1000.0f, new Vector(lightDirection));
        node.addLight(directionalLightJni);

        float[] omniLightPosition = {1,0,0};
        OmniLight omniLightJni = new OmniLight(Color.RED, 1000.0f, 1, 10, new Vector(omniLightPosition));
        node.addLight(omniLightJni);

        float[] spotLightPosition = {-2, 0, 3};
        Spotlight spotLightJni = new Spotlight(Color.YELLOW, 1000.0f, 1, 10, new Vector(spotLightPosition),
                new Vector(lightDirection), 2, 10);
        node.addLight(spotLightJni);
    }

    private Callable<Boolean> glInitialized() {
        return new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mActivity.isGlInitialized();
            }
        };
    }

    @Test
    public void dummy_test() {
        // Context of the app under test.
        Log.d(TAG, "useApp called");
        Context appContext = InstrumentationRegistry.getTargetContext();

        assertEquals("com.viromedia.renderertest.gvr", appContext.getPackageName());
        assertEquals(true, mActivity.isGlInitialized());
    }

    @After
    public void tearDown() throws Exception {
        synchronized (this) {
            TimeUnit.SECONDS.sleep(10);
        }
    }

}
