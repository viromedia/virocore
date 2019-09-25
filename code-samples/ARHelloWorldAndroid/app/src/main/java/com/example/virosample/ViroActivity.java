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
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import com.viro.core.ARAnchor;
import com.viro.core.ARNode;
import com.viro.core.ARPlaneAnchor;
import com.viro.core.ARScene;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.ClickListener;
import com.viro.core.ClickState;
import com.viro.core.DragListener;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;

import com.viro.core.OmniLight;
import com.viro.core.Surface;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import com.viro.core.ViroViewScene;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Activity that initializes Viro and ARCore. This activity builds an AR scene that continuously
 * detects planes. If you tap on a plane, an Android will appear at the location tapped. The
 * Androids are rendered using PBR (physically-based rendering).
 */
public class ViroActivity extends Activity {
    private static final String TAG = ViroActivity.class.getSimpleName();
    private ViroView mViroView;
    private ARScene mScene;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mViroView = new ViroViewARCore(this, new ViroViewARCore.StartupListener() {
            @Override
            public void onSuccess() {
                displayScene();
            }

            @Override
            public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {
                Log.e(TAG, "Error initializing AR [" + errorMessage + "]");
            }
        });
        setContentView(mViroView);
    }

    /**
     * Create an AR scene that tracks planes. Tapping on a plane places a 3D Object on the spot
     * tapped.
     */
    private void displayScene() {
        // Create the 3D AR scene, and display the point cloud
        mScene = new ARScene();
        mScene.displayPointCloud(true);

        // Create a TrackedPlanesController to visually display identified planes.
        TrackedPlanesController controller = new TrackedPlanesController(this, mViroView);

        // Spawn a 3D Droid on the position where the user has clicked on a tracked plane.
        controller.addOnPlaneClickListener(new ClickListener() {
            @Override
            public void onClick(int i, Node node, Vector clickPosition) {
                createDroidAtPosition(clickPosition);
            }

            @Override
            public void onClickState(int i, Node node, ClickState clickState, Vector vector) {
                //No-op
            }
        });

        mScene.setListener(controller);

        // Add some lights to the scene; this will give the Android's some nice illumination.
        Node rootNode = mScene.getRootNode();
        List<Vector> lightPositions = new ArrayList<Vector>();
        lightPositions.add(new Vector(-10,  10, 1));
        lightPositions.add(new Vector(10,  10, 1));

        float intensity = 300;
        List<Integer> lightColors = new ArrayList();
        lightColors.add(Color.WHITE);
        lightColors.add(Color.WHITE);

        for (int i = 0; i < lightPositions.size(); i++) {
            OmniLight light = new OmniLight();
            light.setColor(lightColors.get(i));
            light.setPosition(lightPositions.get(i));
            light.setAttenuationStartDistance(20);
            light.setAttenuationEndDistance(30);
            light.setIntensity(intensity);
            rootNode.addLight(light);
        }

        //Add an HDR environment map to give the Android's more interesting ambient lighting.
        Texture environment = Texture.loadRadianceHDRTexture(Uri.parse("file:///android_asset/ibl_newport_loft.hdr"));
        mScene.setLightingEnvironment(environment);

        mViroView.setScene(mScene);
    }

    /**
     * Create an Android object and have it appear at the given location.
     * @param position The location where the Android should appear.
     */
    private void createDroidAtPosition(Vector position) {
        // Create a droid on the surface
        final Bitmap bot = getBitmapFromAsset(this, "andy.png");
        final Object3D object3D = new Object3D();
        object3D.setPosition(position);

        mScene.getRootNode().addChildNode(object3D);

        // Load the Android model asynchronously.
        object3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/andy.obj"), Object3D.Type.OBJ, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                // When the model is loaded, set the texture associated with this OBJ
                Texture objectTexture = new Texture(bot, Texture.Format.RGBA8, false, false);
                Material material = new Material();
                material.setDiffuseTexture(objectTexture);

                // Give the material a more "metallic" appearance, so it reflects the environment map.
                // By setting its lighting model to PHYSICALLY_BASED, we enable PBR rendering on the
                // model.
                material.setRoughness(0.23f);
                material.setMetalness(0.7f);
                material.setLightingModel(Material.LightingModel.PHYSICALLY_BASED);

                object3D.getGeometry().setMaterials(Arrays.asList(material));
            }

            @Override
            public void onObject3DFailed(String s) {
            }
        });

        // Make the object draggable.
        object3D.setDragListener(new DragListener() {
            @Override
            public void onDrag(int i, Node node, Vector vector, Vector vector1) {
                // No-op.
            }
        });
        object3D.setDragType(Node.DragType.FIXED_DISTANCE);
    }

    /**
     * Tracks planes and renders a surface on them so the user can see where we've identified
     * planes.
     */
    private static class TrackedPlanesController implements ARScene.Listener {
        private WeakReference<Activity> mCurrentActivityWeak;
        private boolean searchingForPlanesLayoutIsVisible = false;
        private HashMap<String, Node> surfaces = new HashMap<String, Node>();
        private Set<ClickListener> mPlaneClickListeners = new HashSet<ClickListener>();

        @Override
        public void onTrackingUpdated(ARScene.TrackingState trackingState, ARScene.TrackingStateReason trackingStateReason) {
            //no-op
        }

        public TrackedPlanesController(Activity activity, View rootView) {
            mCurrentActivityWeak = new WeakReference<Activity>(activity);
            // Inflate viro_view_hud.xml layout to display a "Searching for surfaces" text view.
            View.inflate(activity, R.layout.viro_view_hud, ((ViewGroup) rootView));
        }

        public void addOnPlaneClickListener(ClickListener listener) {
            mPlaneClickListeners.add(listener);
        }

        public void removeOnPlaneClickListener(ClickListener listener) {
            if (mPlaneClickListeners.contains(listener)){
                mPlaneClickListeners.remove(listener);
            }
        }

        /**
         * Once a Tracked plane is found, we can hide the our "Searching for Surfaces" UI.
         */
        private void hideIsTrackingLayoutUI(){
            if (searchingForPlanesLayoutIsVisible){
                return;
            }
            searchingForPlanesLayoutIsVisible = true;

            Activity activity = mCurrentActivityWeak.get();
            if (activity == null){
                return;
            }

            View isTrackingFrameLayout = activity.findViewById(R.id.viro_view_hud);
            isTrackingFrameLayout.animate().alpha(0.0f).setDuration(2000);
        }

        @Override
        public void onAnchorFound(ARAnchor arAnchor, ARNode arNode) {
            // Spawn a visual plane if a PlaneAnchor was found
            if (arAnchor.getType() == ARAnchor.Type.PLANE){
                ARPlaneAnchor planeAnchor = (ARPlaneAnchor)arAnchor;

                // Create the visual geometry representing this plane
                Vector dimensions = planeAnchor.getExtent();
                Surface plane = new Surface(1,1);
                plane.setWidth(dimensions.x);
                plane.setHeight(dimensions.z);

                // Set a default material for this plane.
                Material material = new Material();
                material.setDiffuseColor(Color.parseColor("#BF000000"));
                plane.setMaterials(Arrays.asList(material));

                // Attach it to the node
                Node planeNode = new Node();
                planeNode.setGeometry(plane);
                planeNode.setRotation(new Vector(-Math.toRadians(90.0), 0, 0));
                planeNode.setPosition(planeAnchor.getCenter());

                // Attach this planeNode to the anchor's arNode
                arNode.addChildNode(planeNode);
                surfaces.put(arAnchor.getAnchorId(), planeNode);

                // Attach click listeners to be notified upon a plane onClick.
                planeNode.setClickListener(new ClickListener() {
                    @Override
                    public void onClick(int i, Node node, Vector vector) {
                        for (ClickListener listener : mPlaneClickListeners){
                            listener.onClick(i, node, vector);
                        }
                    }

                    @Override
                    public void onClickState(int i, Node node, ClickState clickState, Vector vector) {
                        //No-op
                    }
                });

                // Finally, hide isTracking UI if we haven't done so already.
                hideIsTrackingLayoutUI();
            }
        }

        @Override
        public void onAnchorUpdated(ARAnchor arAnchor, ARNode arNode) {
            if (arAnchor.getType() == ARAnchor.Type.PLANE){
                ARPlaneAnchor planeAnchor = (ARPlaneAnchor)arAnchor;

                // Update the mesh surface geometry
                Node node = surfaces.get(arAnchor.getAnchorId());
                Surface plane = (Surface) node.getGeometry();
                Vector dimensions = planeAnchor.getExtent();
                plane.setWidth(dimensions.x);
                plane.setHeight(dimensions.z);
            }
        }

        @Override
        public void onAnchorRemoved(ARAnchor arAnchor, ARNode arNode) {
            surfaces.remove(arAnchor.getAnchorId());
        }

        @Override
        public void onTrackingInitialized() {
            //No-op
        }

        @Override
        public void onAmbientLightUpdate(float lightIntensity, Vector lightColor) {
            //No-op
        }
    }

    private Bitmap getBitmapFromAsset(final Context context, String assetName) {
        AssetManager assetManager = context.getResources().getAssets();
        InputStream imageStream;
        try {
            imageStream = assetManager.open(assetName);
        } catch (IOException exception) {
            Log.w(TAG, "Unable to find image [" + assetName + "] in assets! Error: "
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
}

