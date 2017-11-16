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
import android.graphics.Color;
import android.net.Uri;
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
import com.viro.core.RendererStartListener;
import com.viro.core.Surface;
import com.viro.core.Texture;
import com.viro.core.Vector;

import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

/**
 * A ViroCore AR sample activity.
 * <p>
 * This activity builds an ar scene that renders tracked ar planes. Clicking on a point on
 * tracked ar planes will also spawn a 3d droid that can be dragged around.
 */
public class ViroARPlanesDemoActivity extends ViroActivity implements RendererStartListener {
    /*
     Reference to the arScene we will be creating within this activity
     */
    private ARScene mScene;

    /**
     * Create an AR scene that tracks planes. Clicking on a plane places a 3D Object on that spot.
     */
    @Override
    public void onRendererStart() {
        // Create the 3d ar scene, and display the point clouds.
        mScene = new ARScene();
        mScene.displayPointCloud(true);

        // Create an TrackedPlanesController to visually display tracked planes
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
        mViroView.setScene(mScene);
    }

    private void createDroidAtPosition(Vector position){
        // Create a droid on the surface
        final Bitmap bot = ViroHelper.getBitmapFromAsset(this, "andy.png");
        final Object3D object3D = new Object3D();
        object3D.setPosition(position);
        mScene.getRootNode().addChildNode(object3D);

        // Load the Android model asynchronously.
        object3D.loadModel(Uri.parse("file:///android_asset/andy.obj"), Object3D.Type.OBJ, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                // When the model is loaded, set the texture associated with this OBJ.
                Texture objectTexture = new Texture(bot, Texture.Format.RGBA8, false, false);
                Material material = new Material();
                material.setDiffuseTexture(objectTexture);
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
     * An TrackedPlanesController that tracks planes and renders a surface on them.
     */
    private static class TrackedPlanesController implements ARScene.Listener {
        private WeakReference<Activity> mCurrentActivityWeak;
        private boolean searchingForPlanesLayoutIsVisible = false;
        private HashMap<String, Node> surfaces = new HashMap<String, Node>();
        private Set<ClickListener> mPlaneClickListeners = new HashSet<ClickListener>();

        public TrackedPlanesController(Activity activity, View rootView){
            mCurrentActivityWeak = new WeakReference<Activity>(activity);

            // Inflate viro_view_hud.xml layout to display a "Searching for surfaces" text view.
            View.inflate(activity, R.layout.viro_view_hud, ((ViewGroup) rootView));
        }

        /**
         * Register click listener for other components to listen for click events that occur
         * on tracked planes. In this example, a listener is registered during scene creation,
         * so as spawn 3d droids on a click.
         */
        public void addOnPlaneClickListener(ClickListener listener){
            mPlaneClickListeners.add(listener);
        }

        public void removeOnPlaneClickListener(ClickListener listener){
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
        public void onAmbientLightUpdate(float v, float v1) {
            //No-op
        }
    }
}
