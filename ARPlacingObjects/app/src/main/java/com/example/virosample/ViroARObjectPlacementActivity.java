package com.example.virosample;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.net.Uri;

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
import com.viro.core.ViroViewARCore;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;


/**
 * A ViroCore AR sample activity.
 * <p>
 * This activity builds an ar scene that lets the user place and drag objects in an ARScene. Click on the Viro
 * button to get a dialog of objects to place in the scene. Drag, rotate and scale the object using pinch and rotate
 * gestures.
 */
public class ViroARObjectPlacementActivity extends ViroActivity {

    // Constants used to determine if plane or point is within bounds. Units in meters.
    static final float MIN_DISTANCE = .2f;
    static final float MAX_DISTANCE = 10f;

    /*
      Private class that contains logic for placing, dragging, rotating and scaling a 3D object in AR.
     */
    private class Draggable3dObject {
        private String mFileName;
        private float rotateStart;
        private float scaleStart;


        public Draggable3dObject(String filename) {
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
                  //TODO: Display toast saying model loaded successfully.
                }

                @Override
                public void onObject3DFailed(String s) {
                    Toast.makeText(ViroARObjectPlacementActivity.this, "An error occured when loading the 3d Object!", Toast.LENGTH_LONG).show();
                }
            });

            // Make the object draggable.
            object3D.setDragType(Node.DragType.FIXED_TO_WORLD);
            mScene.getRootNode().addChildNode(object3D);

        }
    }

    /*
    Reference to the arScene we will be creating within this activity
    */
    private ARScene mScene;

    /*
     List of draggable 3d objects in our scene.
     */
    private List<Draggable3dObject> mDraggableObjects;

    /*
     Constructor for activity. Creates a list of draggable objects we'll create.
     */
    public ViroARObjectPlacementActivity() {
        mDraggableObjects = new ArrayList<Draggable3dObject>();
    }

    @Override
    public void onRendererStart() {
        mScene = new ARScene();
        //add a listener to the scene so we can update 'AR Init' text.
        mScene.setListener(new ARSceneListener(this, mViroView));
        //add a light to the scene so our models can show up.
        mScene.getRootNode().addLight(new AmbientLight(Color.WHITE, 1000f));
        mViroView.setScene(mScene);
        View.inflate(this, R.layout.viro_view_ar_hit_test_hud, ((ViewGroup) mViroView));
    }


    private void placeObject(final String fileName) {
        ViroViewARCore viewARView = (ViroViewARCore)mViroView;
        final Vector cameraPos  = viewARView.getLastCameraPositionRealtime();
        viewARView.performARHitTestWithRay(viewARView.getLastCameraForwardRealtime(), new ARHitTestListener() {
            @Override
            public void onHitTestFinished(ARHitTestResult[] arHitTestResults) {
                if(arHitTestResults != null ) {
                    if(arHitTestResults.length > 0) {
                        for (int i = 0; i < arHitTestResults.length; i++) {
                            ARHitTestResult result = arHitTestResults[i];
                            float distance = result.getPosition().distance(cameraPos);
                            if(distance > MIN_DISTANCE && distance < MAX_DISTANCE) {
                                // If we found a plane of feature point greater than .2 and less than 10 meters away
                                // then choose it!
                                add3dDraggableObject(fileName, result.getPosition());
                                return;
                            }
                        }
                    }
                }
                Toast.makeText(ViroARObjectPlacementActivity.this, "Unable to find suitable point or plane to place object!", Toast.LENGTH_LONG).show();
            }
        });
    }


    /*
      Add a 3d object with the given filename to the scene at the specified world position.
     */
    private void add3dDraggableObject(String filename, Vector position) {
        Draggable3dObject draggable3dObject = new Draggable3dObject(filename);
        mDraggableObjects.add(draggable3dObject);
        draggable3dObject.addModelToPosition(position);
    }

    /*
     *Dialog menu of virtual objects we can place in the real world.
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

    /*
     Private class that implements ARScene.Listener callbacks. In this example we use this to notify the user
     AR is initialized.
     */
    private static class ARSceneListener implements ARScene.Listener {
        private WeakReference<Activity> mCurrentActivityWeak;

        public ARSceneListener(Activity activity, View rootView) {
            mCurrentActivityWeak = new WeakReference<Activity>(activity);
        }

        @Override
        public void onTrackingInitialized() {
            Activity activity = mCurrentActivityWeak.get();
            if (activity == null){
                return;
            }

            TextView initText = (TextView)activity.findViewById(R.id.initText);
            initText.setText("AR is initialized.");
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
}
