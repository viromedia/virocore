/*
 * Copyright (c) 2017-present, Viro, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.virosample;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;

import com.viro.core.ARAnchor;
import com.viro.core.ARImageTarget;
import com.viro.core.ARNode;
import com.viro.core.ARScene;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.ClickListener;
import com.viro.core.ClickState;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Sphere;
import com.viro.core.Spotlight;
import com.viro.core.Surface;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.Map;

/**
 * This activity demonstrates how to use an ARImageTarget. When a Tesla logo is
 * detected, a 3D Tesla car is created over the logo, along with controls that let
 * the user customize the car.
 */
public class ViroActivityAR extends Activity implements ARScene.Listener {
    private static final String TAG = ViroActivityAR.class.getSimpleName();
    private ViroView mViroView;
    private ARScene mScene;
    private Node mCarModelNode;
    private Node mColorChooserGroupNode;
    private Map<String, Pair<ARImageTarget, Node>> mTargetedNodesMap;
    private HashMap<CAR_MODEL, Texture> mCarColorTextures = new HashMap<>();

    private enum CAR_MODEL{
        WHITE("object_car_main_Base_Color.png",         new Vector(231,231,231)),
        BLUE("object_car_main_Base_Color_blue.png",     new Vector(19, 42, 143)),
        GREY("object_car_main_Base_Color_grey.png",     new Vector(75, 76, 79)),
        RED("object_car_main_Base_Color_red.png",       new Vector(168, 0, 0)),
        YELLOW("object_car_main_Base_Color_yellow.png", new Vector(200, 142, 31));

        private final String mDiffuseSource;
        private final Vector mUIPickerColorSource;

        CAR_MODEL(String carSrc, Vector pickerColorSrc){
            mDiffuseSource = carSrc;
            mUIPickerColorSource = pickerColorSrc;
        }

        public String getCarSrc(){
            return mDiffuseSource;
        }
        public Vector getColorPickerSrc(){
            return mUIPickerColorSource;
        }
    }

    // +---------------------------------------------------------------------------+
    //  Initialization
    // +---------------------------------------------------------------------------+

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mTargetedNodesMap = new HashMap<String, Pair<ARImageTarget, Node>>();
        mViroView = new ViroViewARCore(this, new ViroViewARCore.StartupListener() {
            @Override
            public void onSuccess() {
                onRenderCreate();
            }

            @Override
            public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {
                Log.e(TAG, "Error initializing AR [" + errorMessage + "]");
            }
        });
        setContentView(mViroView);
    }

    /*
     Create the main ARScene. We add an ARImageTarget representing the Tesla logo to the scene,
     then we create a Node (teslaNode) that consists of the Tesla car and the controls to
     customize it. This Node is not yet added to the Scene -- we will wait until the Tesla logo
     is found.
     */
    private void onRenderCreate() {
        // Create the base ARScene
        mScene = new ARScene();
        mScene.setListener(this);
        mViroView.setScene(mScene);

        // Create an ARImageTarget for the Tesla logo
        Bitmap teslaLogoTargetBmp = getBitmapFromAssets("logo.png");
        ARImageTarget teslaTarget = new ARImageTarget(teslaLogoTargetBmp, ARImageTarget.Orientation.Up, 0.188f);
        mScene.addARImageTarget(teslaTarget);

        // Build the Tesla car Node and add it to the Scene. Set it to invisible: it will be made
        // visible when the ARImageTarget is found.
        Node teslaNode = new Node();
        initCarModel(teslaNode);
        initColorPickerModels(teslaNode);
        initSceneLights(teslaNode);
        teslaNode.setVisible(false);
        mScene.getRootNode().addChildNode(teslaNode);

        // Link the Node with the ARImageTarget, such that when the image target is
        // found, we'll render the Node.
        linkTargetWithNode(teslaTarget, teslaNode);
    }

    /*
     Link the given ARImageTarget with the provided Node. When the ARImageTarget is
     found in the scene (by onAnchorFound below), the Node will be made visible and
     the target's transformations will be applied to the Node, thereby rendering the
     Node over the target.
     */
    private void linkTargetWithNode(ARImageTarget imageToDetect, Node nodeToRender){
        String key = imageToDetect.getId();
        mTargetedNodesMap.put(key, new Pair(imageToDetect, nodeToRender));
    }

    // +---------------------------------------------------------------------------+
    //  ARScene.Listener Implementation
    // +---------------------------------------------------------------------------+

    /*
     When an ARImageTarget is found, lookup the target's corresponding Node in the
     mTargetedNodesMap. Make the Node visible and apply the target's transformations
     to the Node. This makes the Node appear correctly over the target.

     (In this case, this makes the Tesla 3D model and color pickers appear directly
      over the detected Tesla logo)
     */
    @Override
    public void onAnchorFound(ARAnchor anchor, ARNode arNode) {
        String anchorId = anchor.getAnchorId();
        if (!mTargetedNodesMap.containsKey(anchorId)) {
            return;
        }

        Node imageTargetNode = mTargetedNodesMap.get(anchorId).second;
        Vector rot = new Vector(0,anchor.getRotation().y, 0);
        imageTargetNode.setPosition(anchor.getPosition());
        imageTargetNode.setRotation(rot);
        imageTargetNode.setVisible(true);
        animateCarVisible(mCarModelNode);

        // Stop the node from moving in place once found
        ARImageTarget imgTarget = mTargetedNodesMap.get(anchorId).first;
        mScene.removeARImageTarget(imgTarget);
        mTargetedNodesMap.remove(anchorId);
    }

    @Override
    public void onAnchorRemoved(ARAnchor anchor, ARNode arNode) {
        String anchorId = anchor.getAnchorId();
        if (!mTargetedNodesMap.containsKey(anchorId)) {
            return;
        }

        Node imageTargetNode = mTargetedNodesMap.get(anchorId).second;
        imageTargetNode.setVisible(false);
    }

    @Override
    public void onAnchorUpdated(ARAnchor anchor, ARNode arNode) {
        // No-op
    }

    // +---------------------------------------------------------------------------+
    //  Scene Building Methods
    // +---------------------------------------------------------------------------+

    /*
     Init, loads the the Tesla Object3D, and attaches it to the passed in groupNode.
     */
    private void initCarModel(Node groupNode) {
        // Creation of ObjectJni to the right
        Object3D fbxCarNode = new Object3D();
        fbxCarNode.setScale(new Vector(0.00f, 0.00f, 0.00f));
        fbxCarNode.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/object_car.obj"), Object3D.Type.OBJ, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                preloadCarColorTextures(object);
            }

            @Override
            public void onObject3DFailed(final String error) {
                Log.e(TAG,"Car Model Failed to load.");
            }
        });

        groupNode.addChildNode(fbxCarNode);
        mCarModelNode = fbxCarNode;

        // Set click listeners.
        mCarModelNode.setClickListener(new ClickListener() {
            @Override
            public void onClick(int i, Node node, Vector vector) {
                // Animate toggling the groupColor picker.
                boolean setVisibility = !mColorChooserGroupNode.isVisible();
                mColorChooserGroupNode.setVisible(setVisibility);
                animateColorPickerVisible(setVisibility, mColorChooserGroupNode);
            }

            @Override
            public void onClickState(int i, Node node, ClickState clickState, Vector vector) {
                // No-op.
            }
        });
    }

    /*
     Constructs a group of sphere color pickers and attaches them to the passed in group Node.
     These sphere pickers when click will change the diffuse texture of our tesla model.
     */
    private void initColorPickerModels(Node groupNode) {
        mColorChooserGroupNode = new Node();
        mColorChooserGroupNode.setTransformBehaviors(EnumSet.of(Node.TransformBehavior.BILLBOARD_Y));
        mColorChooserGroupNode.setPosition(new Vector(0,0.25,0));
        float pickerPositions [] = {-.2f, -.1f, 0f, .1f, .2f};
        int i = 0;

        // Loop through car color model colors
        for (final CAR_MODEL model : CAR_MODEL.values()) {
            // Create our sphere picker geometry
            final Node colorSphereNode = new Node();
            float posX = pickerPositions[i++];
            colorSphereNode.setPosition(new Vector(posX, 0, 0));
            Sphere colorSphere = new Sphere(0.03f);

            // Create sphere picker color that correlates to the car model's color
            Material material = new Material();
            Vector c = model.getColorPickerSrc();
            material.setDiffuseColor(Color.rgb((int)c.x, (int)c.y, (int)c.z));
            material.setLightingModel(Material.LightingModel.PHYSICALLY_BASED);

            // Finally, set the sphere's properties
            colorSphere.setMaterials(Arrays.asList(material));
            colorSphereNode.setGeometry(colorSphere);
            colorSphereNode.setShadowCastingBitMask(0);
            mColorChooserGroupNode.addChildNode(colorSphereNode);

            // Set clickListener on spheres
            colorSphereNode.setClickListener(new ClickListener() {
                @Override
                public void onClick(int i, Node node, Vector vector) {
                    //mCarModelNode.getGeometry().setMaterials();
                    Texture texture = mCarColorTextures.get(model);
                    Material mat = mCarModelNode.getGeometry().getMaterials().get(0);
                    mat.setDiffuseTexture(texture);
                    animateColorPickerClicked(colorSphereNode);
                }

                @Override
                public void onClickState(int i, Node node, ClickState clickState, Vector vector) {
                    // No-op.
                }
            });
        }

        mColorChooserGroupNode.setScale(new Vector(0,0,0));
        mColorChooserGroupNode.setVisible(false);
        groupNode.addChildNode(mColorChooserGroupNode);
    }

    private void initSceneLights(Node groupNode){
        Node rootLightNode = new Node();

        // Construct a spot light for shadows
        Spotlight spotLight = new Spotlight();
        spotLight.setPosition(new Vector(0,5,0));
        spotLight.setColor(Color.parseColor("#FFFFFF"));
        spotLight.setDirection(new Vector(0,-1,0));
        spotLight.setIntensity(300);
        spotLight.setInnerAngle(5);
        spotLight.setOuterAngle(25);
        spotLight.setShadowMapSize(2048);
        spotLight.setShadowNearZ(2);
        spotLight.setShadowFarZ(7);
        spotLight.setShadowOpacity(.7f);
        spotLight.setCastsShadow(true);
        rootLightNode.addLight(spotLight);

        // Add our shadow planes.
        final Material material = new Material();
        material.setShadowMode(Material.ShadowMode.TRANSPARENT);
        Surface surface = new Surface(2, 2);
        surface.setMaterials(Arrays.asList(material));
        Node surfaceShadowNode = new Node();
        surfaceShadowNode.setRotation(new Vector(Math.toRadians(-90), 0, 0));
        surfaceShadowNode.setGeometry(surface);
        surfaceShadowNode.setPosition(new Vector(0, 0, -0.7));
        rootLightNode.addChildNode(surfaceShadowNode);
        groupNode.addChildNode(rootLightNode);

        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/garage_1k.hdr"));
        mScene.setLightingEnvironment(environment);
    }

    private Material preloadCarColorTextures(Node node){
        final Texture metallicTexture = new Texture(getBitmapFromAssets("object_car_main_Metallic.png"),
                Texture.Format.RGBA8, true, true);
        final Texture roughnessTexture = new Texture(getBitmapFromAssets("object_car_main_Roughness.png"),
                Texture.Format.RGBA8, true, true);

        Material material = new Material();
        material.setMetalnessMap(metallicTexture);
        material.setRoughnessMap(roughnessTexture);
        material.setLightingModel(Material.LightingModel.PHYSICALLY_BASED);
        node.getGeometry().setMaterials(Arrays.asList(material));

        // Loop through color.
        for (CAR_MODEL model : CAR_MODEL.values()) {
            Bitmap carBitmap = getBitmapFromAssets(model.getCarSrc());
            final Texture carTexture = new Texture(carBitmap, Texture.Format.RGBA8, true, true);
            mCarColorTextures.put(model, carTexture);

            // Preload our textures into the model
            material.setDiffuseTexture(carTexture);
        }

        material.setDiffuseTexture(mCarColorTextures.get(CAR_MODEL.WHITE));
        return material;
    }

    // +---------------------------------------------------------------------------+
    //  Image Loading
    // +---------------------------------------------------------------------------+

    private Bitmap getBitmapFromAssets(final String assetName) {
        final InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = getAssets().open(assetName);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (final IOException e) {
            throw new IllegalArgumentException("Loading bitmap failed!", e);
        }
        return bitmap;
    }

    // +---------------------------------------------------------------------------+
    //  Animation Utilities
    // +---------------------------------------------------------------------------+

    private void animateScale(Node node, long duration, Vector targetScale,
                              AnimationTimingFunction fcn, final Runnable runnable) {
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(duration);
        AnimationTransaction.setTimingFunction(fcn);
        node.setScale(targetScale);
        if (runnable != null){
            AnimationTransaction.setListener(new AnimationTransaction.Listener() {
                @Override
                public void onFinish(AnimationTransaction animationTransaction) {
                    runnable.run();
                }
            });
        }
        AnimationTransaction.commit();
    }

    private void animateColorPickerVisible(boolean isVisible, Node groupNode) {
        if (isVisible){
            animateScale(groupNode, 500, new Vector(1,1,1), AnimationTimingFunction.Bounce, null);
        } else {
            animateScale(groupNode, 200, new Vector(0,0,0), AnimationTimingFunction.Bounce, null);
        }
    }

    private void animateCarVisible(Node car) {
        animateScale(car, 500, new Vector(0.09f, 0.09f, 0.09f), AnimationTimingFunction.EaseInEaseOut, null);
    }

    private void animateColorPickerClicked(final Node picker){
        animateScale(picker, 50, new Vector(0.8f, 0.8f, 0.8f), AnimationTimingFunction.EaseInEaseOut, new Runnable() {
            @Override
            public void run() {
                animateScale(picker, 50, new Vector(1,1,1), AnimationTimingFunction.EaseInEaseOut, null);
            }
        });
    }

    // +---------------------------------------------------------------------------+
    //  Lifecycle
    // +---------------------------------------------------------------------------+

    @Override
    protected void onStart() {
        super.onStart();
        mViroView.onActivityStarted(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mViroView.onActivityResumed(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        mViroView.onActivityPaused(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        mViroView.onActivityStopped(this);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mViroView.onActivityDestroyed(this);
    }

    @Override
    public void onTrackingInitialized() {
        // No-op
    }

    @Override
    public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {
        // No-op
    }

    @Override
    public void onAmbientLightUpdate(float value, Vector v) {
        // No-op
    }
}