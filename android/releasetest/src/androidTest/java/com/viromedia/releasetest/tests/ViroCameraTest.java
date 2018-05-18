package com.viromedia.releasetest.tests;


import android.graphics.Color;

import com.viro.core.AmbientLight;
import com.viro.core.Animation;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.Box;
import com.viro.core.CameraListener;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Quaternion;
import com.viro.core.Renderer;
import com.viro.core.Text;
import com.viro.core.Vector;
import com.viro.core.Camera;
import com.viromedia.releasetest.ViroReleaseTestActivity;

import org.junit.Test;

import java.util.Arrays;
import java.util.EnumSet;

public class ViroCameraTest extends ViroBaseTest {
    private Camera mCamera;
    private Renderer mRenderer;
    private float mRotationAngle;
    @Override
    void configureTestScene() {

        ViroReleaseTestActivity activity = (ViroReleaseTestActivity)mActivity;
        mRenderer = activity.getViroView().getRenderer();
        final float[] lightDirection = {0, 0, -1};
        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(ambientLightJni);

        DirectionalLight directionalLight = new DirectionalLight(Color.WHITE, 2000.0f, new Vector(0, 0, -1));
        mScene.getRootNode().addLight(directionalLight);

        final Material material = new Material();
        material.setDiffuseColor(Color.BLUE);
        material.setLightingModel(Material.LightingModel.BLINN);

        // Creation of ViroBox
        final Node node = new Node();
        Box box = new Box(1, 1, 1);

        node.setGeometry(box);
        final float[] boxPosition = {0, 0, -5};
        node.setPosition(new Vector(boxPosition));
        box.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(node);

        mCamera = new Camera();
        mRotationAngle = 0.0f;
        mScene.getRootNode().setCamera(mCamera);
        mRenderer.setPointOfView(mScene.getRootNode());
    }

    @Test
    public void testCamera() {
        runUITest(() -> testCameraPosition());
        runUITest(() -> testCameraOrbitMode());
        runUITest(() -> testCameraRotationQuaternion());
        runUITest(() -> testCameraRotationEuler());
        runUITest(() -> testCameraCallbacks());
    }

    private void testCameraOrbitMode() {
        mMutableTestMethod = null;
        mCamera.setRotationType(Camera.RotationType.ORBIT);
        mCamera.setOrbitFocalPoint(new Vector(0, 0, -10));
        assertPass("Camera should orbit around box", ()->{
        mCamera.setRotationType(Camera.RotationType.STANDARD);});
    }

    private void testCameraPosition() {
        mMutableTestMethod = () -> {
            Vector position = mCamera.getPosition();
            if(position.z > -5f) {
                mCamera.setPosition(new Vector(0, 0, position.z + .2f));
            }
        };
        assertPass("Camera position moves back.", ()->{mCamera.setPosition(new Vector(0, 0, 0));});
    }

    private void testCameraRotationQuaternion() {
        mMutableTestMethod = () -> {
            mRotationAngle+=5.0f;
            Quaternion quaternion = ViroCameraTest.toQuaternion(0, Math.toRadians(mRotationAngle), 0);
            mCamera.setRotation(quaternion);
        };

        assertPass("Camera rotates upwards (via Quaternion).", () ->{mCamera.setRotation(ViroCameraTest.toQuaternion(0, 0, 0));});
    }

    private void testCameraRotationEuler() {
        mMutableTestMethod = () -> {
            mRotationAngle+=5.0f;
            mCamera.setRotation(new Vector(0, Math.toRadians(mRotationAngle), 0));
        };

        assertPass("Camera rotates to the left (via Euler angles).", ()->{
            mCamera.setRotation(new Vector());
            mRotationAngle = 0;
        });
    }

    // TODO: VIRO-2166 Move to Quaternion class
    private static Quaternion toQuaternion(double pitch, double roll, double yaw)
    {
        Quaternion q = new Quaternion();
        // Abbreviations for the various angular functions
        double cy = Math.cos(yaw * 0.5);
        double sy = Math.sin(yaw * 0.5);
        double cr = Math.cos(roll * 0.5);
        double sr = Math.sin(roll * 0.5);
        double cp = Math.cos(pitch * 0.5);
        double sp = Math.sin(pitch * 0.5);

        q.w = (float)((cy * cr * cp) + (sy * sr * sp));
        q.x = (float)((cy * sr * cp) - (sy * cr * sp));
        q.y = (float)((cy * cr * sp) + (sy * sr * cp));
        q.z = (float)((sy * cr * cp) - (cy * sr * sp));
        return q;
    }

    private void testCameraCallbacks(){
        Text output = new Text(mViroView.getViroContext(), "",
                "Roboto", 18,
                Color.WHITE, 9f, 2f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 4);

        Node textNode = new Node();
        textNode.setGeometry(output);
        textNode.setRotationPivot(new Vector(-4,0,0));
        textNode.setTransformBehaviors(EnumSet.of(Node.TransformBehavior.BILLBOARD));
        mScene.getRootNode().addChildNode(textNode);
        textNode.setPosition(new Vector(0f, 4f, -3.3f));

        // Set the camera at preset positions
        mCamera.setPosition(new Vector(0, 0, 0));
        mCamera.setRotation(new Vector(0,Math.toRadians(0),0));

        // Animate the camera
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(20000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.Linear);
        mCamera.setPosition(new Vector(0, 5, 0));
        mCamera.setRotation(new Vector(0,Math.toRadians(45),0));
        AnimationTransaction transaction = AnimationTransaction.commit();

        // Listen for camera transformation updates as the camera animates
        mViroView.setCameraListener(new CameraListener() {
            @Override
            public void onTransformUpdate(Vector position, Vector rotation, Vector forward) {
                Vector ro2 = new Vector(0,Math.toDegrees(rotation.y), 0);
                String msg = "You should see the position and rotation of the camera animate increase\n Pos: " + position.toString() + " Rot: " + ro2.toString();
                output.setText(msg);
            }
        });

        assertPass("Camera position Changes.", ()->{mCamera.setPosition(new Vector(0, 0, 0));});
        transaction.terminate();
        textNode.removeFromParentNode();
        textNode.dispose();
    }
}

