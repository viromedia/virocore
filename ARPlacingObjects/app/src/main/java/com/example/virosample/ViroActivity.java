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
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.viro.core.ARAnchor;
import com.viro.core.ARHitTestListener;
import com.viro.core.ARHitTestResult;
import com.viro.core.ARNode;
import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.DragListener;
import com.viro.core.GesturePinchListener;
import com.viro.core.GestureRotateListener;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.PinchState;

import com.viro.core.RotateState;
import com.viro.core.Vector;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * Activity that initializes Viro and ARCore. This activity builds an AR scene that lets the user
 * place and drag objects. Tap on the 'Viro' button to get a dialog of objects to place in the scene.
 * Once placed, the objects can be dragged, rotated, and scaled using pinch and rotate gestures.
 */
public class ViroActivity extends Activity {
    private static final String TAG = ViroActivity.class.getSimpleName();

    // Constants used to determine if plane or point is within bounds. Units in meters.
    private static final float MIN_DISTANCE = 0.2f;
    private static final float MAX_DISTANCE = 10f;
    private ViroView mViroView;

    /**
     * The ARScene we will be creating within this activity.
     */
    private ARScene mScene;

    /**
     * List of draggable 3D objects in our scene.
     */
    private List<Draggable3DObject> mDraggableObjects;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mDraggableObjects = new ArrayList<Draggable3DObject>();
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
     * Contains logic for placing, dragging, rotating, and scaling a 3D object in AR.
     */
    private class Draggable3DObject {

        private String mFileName;
        private float rotateStart;
        private float scaleStart;

        public Draggable3DObject(String filename) {
            mFileName = filename;
        }

        private void addModelToPosition(Vector position) {
            final Object3D object3D = new Object3D();
            object3D.setPosition(position);
            // Shrink the objects as the original size is too large.
            object3D.setScale(new Vector(.2f, .2f, .2f));
            object3D.setGestureRotateListener(new GestureRotateListener() {
                @Override
                public void onRotate(int i, Node node, float rotation, RotateState rotateState) {
                    if(rotateState == RotateState.ROTATE_START) {
                        rotateStart = object3D.getRotationEulerRealtime().y;
                    }
                    float totalRotationY = rotateStart + rotation;
                    object3D.setRotation(new Vector(0, totalRotationY, 0));
                }
            });

            object3D.setGesturePinchListener(new GesturePinchListener() {
                @Override
                public void onPinch(int i, Node node, float scale, PinchState pinchState) {
                    if(pinchState == PinchState.PINCH_START) {
                        scaleStart = object3D.getScaleRealtime().x;
                    } else {
                        object3D.setScale(new Vector(scaleStart * scale, scaleStart * scale, scaleStart * scale));
                    }
                }
            });

            object3D.setDragListener(new DragListener() {
                @Override
                public void onDrag(int i, Node node, Vector vector, Vector vector1) {

                }
            });

            // Load the Android model asynchronously.
            object3D.loadModel(Uri.parse(mFileName), Object3D.Type.FBX, new AsyncObject3DListener() {
                @Override
                public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {

                }

                @Override
                public void onObject3DFailed(String s) {
                    Toast.makeText(ViroActivity.this, "An error occured when loading the 3D Object!",
                            Toast.LENGTH_LONG).show();
                }
            });

            // Make the object draggable.
            object3D.setDragType(Node.DragType.FIXED_TO_WORLD);
            mScene.getRootNode().addChildNode(object3D);
        }
    }

    private void displayScene() {
        mScene = new ARScene();
        // Add a listener to the scene so we can update the 'AR Initialized' text.
        mScene.setListener(new ARSceneListener(this, mViroView));
        // Add a light to the scene so our models show up
        mScene.getRootNode().addLight(new AmbientLight(Color.WHITE, 1000f));
        mViroView.setScene(mScene);
        View.inflate(this, R.layout.viro_view_ar_hit_test_hud, ((ViewGroup) mViroView));
    }

    /**
     * Perform a hit-test and place the object (identified by its file name) at the intersected
     * location.
     *
     * @param fileName The resource name of the object to place.
     */
    private void placeObject(final String fileName) {
        ViroViewARCore viewARView = (ViroViewARCore) mViroView;
        final Vector cameraPos = viewARView.getLastCameraPositionRealtime();
        viewARView.performARHitTestWithRay(viewARView.getLastCameraForwardRealtime(), new ARHitTestListener() {
            @Override
            public void onHitTestFinished(ARHitTestResult[] arHitTestResults) {
                if (arHitTestResults != null && arHitTestResults.length > 0) {
                    for (int i = 0; i < arHitTestResults.length; i++) {
                        ARHitTestResult result = arHitTestResults[i];
                        float distance = result.getPosition().distance(cameraPos);
                        if (distance > MIN_DISTANCE && distance < MAX_DISTANCE) {
                            // If we found a plane or feature point further than 0.2m and less 10m away,
                            // then choose it!
                            add3DDraggableObject(fileName, result.getPosition());
                            return;
                        }
                    }
                }
                Toast.makeText(ViroActivity.this, "Unable to find suitable point or plane to place object!",
                        Toast.LENGTH_LONG).show();
            }
        });
    }

    /**
     * Add a 3D object with the given filename to the scene at the specified world position.
     */
    private void add3DDraggableObject(String filename, Vector position) {
        Draggable3DObject draggable3DObject = new Draggable3DObject(filename);
        mDraggableObjects.add(draggable3DObject);
        draggable3DObject.addModelToPosition(position);
    }

    /**
     * Dialog menu displaying the virtual objects we can place in the real world.
     */
    public void showPopup(View v) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        CharSequence itemsList[] = {"Coffee mug", "Flowers", "Smile Emoji"};
        builder.setTitle("Choose an object")
                .setItems(itemsList, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        switch (which) {
                            case 0:
                                placeObject("file:///android_asset/object_coffee_mug.vrx");
                                break;
                            case 1:
                                placeObject("file:///android_asset/object_flowers.vrx");
                                break;
                            case 2:
                                placeObject("file:///android_asset/emoji_smile.vrx");
                                break;
                        }
                    }
                });
        Dialog d = builder.create();
        d.show();
    }

    /**
     * Private class that implements ARScene.Listener callbacks. In this example we use this to notify
     * the user when AR is initialized.
     */
    private static class ARSceneListener implements ARScene.Listener {
        private WeakReference<Activity> mCurrentActivityWeak;
        private boolean mInitialized;
        public ARSceneListener(Activity activity, View rootView) {
            mCurrentActivityWeak = new WeakReference<Activity>(activity);
            mInitialized = false;
        }

        @Override
        public void onTrackingUpdated(ARScene.TrackingState trackingState,
                                      ARScene.TrackingStateReason trackingStateReason) {
            if (!mInitialized && trackingState == ARScene.TrackingState.NORMAL) {
                Activity activity = mCurrentActivityWeak.get();
                if (activity == null) {
                    return;
                }

                TextView initText = (TextView) activity.findViewById(R.id.initText);
                initText.setText("AR is initialized");
                mInitialized = true;
            }
        }

        @Override
        public void onTrackingInitialized() {
            // This method is deprecated.
        }

        @Override
        public void onAmbientLightUpdate(float v, float v1) {

        }

        @Override
        public void onAnchorFound(ARAnchor arAnchor, ARNode arNode) {

        }

        @Override
        public void onAnchorRemoved(ARAnchor arAnchor, ARNode arNode) {

        }

        @Override
        public void onAnchorUpdated(ARAnchor arAnchor, ARNode arNode) {

        }
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
