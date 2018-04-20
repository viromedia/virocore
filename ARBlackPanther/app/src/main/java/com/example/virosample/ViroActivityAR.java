
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
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.viro.core.ARAnchor;
import com.viro.core.ARImageTarget;
import com.viro.core.ARNode;
import com.viro.core.ARScene;
import com.viro.core.Animation;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.OmniLight;
import com.viro.core.Spotlight;
import com.viro.core.Surface;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

/**
 * Activity that initializes Viro and ARCore. This activity demonstrates how we can
 * add and easily track multiple imageTracker to Node targets within an ARScene.
 */
public class ViroActivityAR extends Activity {
    private static final String TAG = ViroActivityAR.class.getSimpleName();
    protected ViroView mViroView;
    private ARScene mScene;
    private ARImageTarget mImageTarget;
    private Node mBlackPantherGroupNode;
    private Node mSurfaceShadowNode;
    private AssetManager mAssetManager;
    private Object3D mblackPantherModel;

    private boolean mObjLoaded = false;
    private boolean mImageTargetFound = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mViroView = new ViroViewARCore(this, new ViroViewARCore.StartupListener() {
            @Override
            public void onSuccess() {
                // Override this function to start building your scene here! We provide a sample
                // "Hello World" scene
                onRenderCreate();
            }

            @Override
            public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {
                // Fail as you wish!
            }
        });
        setContentView(mViroView);
    }

    private void onRenderCreate() {
        // Create the base ARScene
        mScene = new ARScene();

        // Create an the ARImageTarget from which to get transformation updates.
        Bitmap teslaLogoTargetBmp = getBitmapFromAssets("logo.jpg");
        mImageTarget = new ARImageTarget(teslaLogoTargetBmp, ARImageTarget.Orientation.Up, 0.188f);

        // Create our Black Panther Node Group
        mBlackPantherGroupNode = initBlackPantherGroupNode();
        mBlackPantherGroupNode.addChildNode(initLightsNode());

        mViroView.setScene(mScene);
        trackImageNodeTargets();
    }

    private Node initBlackPantherGroupNode() {
        Node blackPantherModelNode = new Node();
        mblackPantherModel = new Object3D();
        mblackPantherModel.setRotation(new Vector(Math.toRadians(-90), 0, 0));
        mblackPantherModel.setScale(new Vector(0.2f, 0.2f, 0.2f));
        mblackPantherModel.loadModel(Uri.parse("file:///android_asset/blackpanther/object_bpanther_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                mObjLoaded = true;
                startPantherExperience();
            }

            @Override
            public void onObject3DFailed(final String error) {
                Log.e(TAG,"Black Panther Object Failed to load.");
            }
        });

        mblackPantherModel.setVisible(false);
        blackPantherModelNode.addChildNode(mblackPantherModel);
        return blackPantherModelNode;
    }

    private Node initLightsNode() {
        Vector omniLightPositions [] = {    new Vector(-3, 3, 0.3),
                new Vector(3, 3, 1),
                new Vector(-3,-3,1),
                new Vector(3, -3, 1)};
        Node groupLightNode = new Node();
        for (Vector pos : omniLightPositions){
            final OmniLight light = new OmniLight();
            light.setPosition(pos);
            light.setColor(Color.parseColor("#FFFFFF"));
            light.setIntensity(20);
            light.setAttenuationStartDistance(6);
            light.setAttenuationEndDistance(9);
            groupLightNode.addLight(light);
        }

        Spotlight spotLight = new Spotlight();
        spotLight.setPosition(new Vector(0,5,-0.5));
        spotLight.setColor(Color.parseColor("#FFFFFF"));
        spotLight.setDirection(new Vector(0, -1, 0));
        spotLight.setIntensity(50);
        spotLight.setShadowOpacity(0.4f);
        spotLight.setShadowMapSize(2048);
        spotLight.setShadowNearZ(2f);
        spotLight.setShadowFarZ(7f);
        spotLight.setInnerAngle(5);
        spotLight.setOuterAngle(20);
        spotLight.setCastsShadow(true);
        groupLightNode.addLight(spotLight);

        // Add the lighting environment for PBR
        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/hotel_room_2k.hdr"));
        mScene.setLightingEnvironment(environment);

        // Add our shadow planes.
        final Material material = new Material();
        material.setShadowMode(Material.ShadowMode.TRANSPARENT);
        Surface surface = new Surface(3, 3);
        surface.setMaterials(Arrays.asList(material));
        mSurfaceShadowNode = new Node();
        mSurfaceShadowNode.setRotation(new Vector(Math.toRadians(-90), 0, 0));
        mSurfaceShadowNode.setGeometry(surface);
        mSurfaceShadowNode.setPosition(new Vector(0, 0, 0.0));
        mSurfaceShadowNode.setVisible(false);
        groupLightNode.addChildNode(mSurfaceShadowNode);

        groupLightNode.setRotation(new Vector(Math.toRadians(-90), 0, 0));
        return groupLightNode;
    }

    private void trackImageNodeTargets(){
        // Finally link the two together and set the scene.
        mScene.addARImageTarget(mImageTarget);
        mScene.getRootNode().addChildNode(mBlackPantherGroupNode);
        mScene.setListener(new ARScene.Listener() {
            @Override
            public void onTrackingInitialized() {
                // No-op
            }

            @Override
            public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {
                // No-op
            }

            @Override
            public void onAmbientLightUpdate(float lightIntensity, float colorTemperature) {
                // No-op
            }

            @Override
            public void onAnchorFound(ARAnchor anchor, ARNode arNode) {
                String anchorId = anchor.getAnchorId();
                if (!mImageTarget.getId().equalsIgnoreCase(anchorId)) {
                    return;
                }

                Vector anchorPos = anchor.getPosition();
                Vector pos = new Vector(anchorPos.x, anchorPos.y - 0.4 , anchorPos.z - 0.15);
                mBlackPantherGroupNode.setPosition(pos);
                mBlackPantherGroupNode.setRotation(anchor.getRotation());
                mImageTargetFound = true;
                startPantherExperience();
            }

            @Override
            public void onAnchorUpdated(ARAnchor anchor, ARNode arNode) {
                //No-op
            }

            @Override
            public void onAnchorRemoved(ARAnchor anchor, ARNode arNode) {
                String anchorId = anchor.getAnchorId();
                if (!mImageTarget.getId().equalsIgnoreCase(anchorId)) {
                    return;
                }

                mBlackPantherGroupNode.setVisible(false);
            }
        });
    }

    private void startPantherExperience(){
        if (!mObjLoaded || !mImageTargetFound){
            return;
        }
        // Animate the black panther's jump animation
        final Animation animationJump = mblackPantherModel.getAnimation("01");
        animationJump.setListener(new Animation.Listener() {
            @Override
            public void onAnimationStart(Animation animation) {
                mblackPantherModel.setVisible(true);
                mSurfaceShadowNode.setVisible(true);
                //No-op
            }

            @Override
            public void onAnimationFinish(Animation animation, boolean canceled) {
                // After jump animation is finished set the panther's idle animation
                final Animation animationIdle = mblackPantherModel.getAnimation("02");
                animationIdle.play();
            }
        });
        animationJump.play();
    }

    private Bitmap getBitmapFromAssets(String assetName) {
        if (mAssetManager == null) {
            mAssetManager = getResources().getAssets();
        }

        InputStream imageStream;
        try {
            imageStream = mAssetManager.open(assetName);
        } catch (IOException exception) {
            Log.w("Viro", "Unable to find image [" + assetName + "] in assets! Error: "
                    + exception.getMessage());
            return null;
        }
        return BitmapFactory.decodeStream(imageStream);
    }

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
    protected void onPause() {
        super.onPause();
        mViroView.onActivityPaused(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        mViroView.onActivityStopped(this);
    }
}