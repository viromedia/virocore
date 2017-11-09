package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.net.Uri;

import com.viro.core.Box;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Sphere;
import com.viro.core.Text;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;

/**
 * Created by vadvani on 11/7/17.
 */

public class ViroAREventsTest extends ViroBaseTest {

    private Node boxNode;
    private Object3D objectNode;
    private Node sphereNode;
    private Text eventText;

    @Override
    void configureTestScene() {
        DirectionalLight light = new DirectionalLight(Color.WHITE, 1000.0f, new Vector(0, 0, -1f));
        mScene.getRootNode().addLight(light);

        //add box node.
        Box box = new Box(1, 1, 1);
        Material boxOneMaterial = new Material();
        boxOneMaterial.setDiffuseColor(Color.RED);
        boxOneMaterial.setLightingModel(Material.LightingModel.BLINN);
        box.setMaterials(Arrays.asList(boxOneMaterial));
        boxNode = new Node();
        boxNode.setPosition(new Vector(-1.5f, 1f, -4f));
        boxNode.setGeometry(box);


        //add object node.
        objectNode = new Object3D();
        objectNode.loadModel(Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, null);
        objectNode.setPosition(new Vector(0.0f, 1f, -4f));

        //add sphere node.
        Sphere sphere = new Sphere(.5f);
        Material sphereMaterial = new Material();
        sphereMaterial.setDiffuseColor(Color.YELLOW);
        sphereMaterial.setLightingModel(Material.LightingModel.BLINN);
        sphere.setMaterials(Arrays.asList(boxOneMaterial));
        sphereNode = new Node();
        sphereNode.setGeometry(sphere);
        sphereNode.setPosition(new Vector(1.5f, 1.0f, -4f));


        mScene.getRootNode().addChildNode(boxNode);
        mScene.getRootNode().addChildNode(objectNode);
        mScene.getRootNode().addChildNode(sphereNode);
        eventText = new Text(mViroView.getViroContext(),
                "Event not registered.", "Roboto", 12, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.CLIP_TO_BOUNDS, 0);

        Node textNode = new Node();
        textNode.setPosition(new Vector(0, -1f, -4f));
        textNode.setGeometry(eventText);
        mScene.getRootNode().addChildNode(textNode);
    }

    @Test
    public void testAREvents() {
        testEventsARDragListener();
        testEventsDragListener();
        testEventsGesturePinchListener();
        testEventsGestureRotateListener();
    }

    private void testEventsARDragListener() {
        boxNode.setDragType(Node.DragType.FIXED_TO_WORLD);
        objectNode.setDragType(Node.DragType.FIXED_TO_WORLD);
        sphereNode.setDragType(Node.DragType.FIXED_TO_WORLD);
        boxNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("DragFixedToWorld on box, worldLoc: " + ViroEventsTest.vectorString(worldLocation) + ", localLoc: " + ViroEventsTest.vectorString(localLocation));
        });

        objectNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("DragFixedToWorld on obj., worldLoc: " + ViroEventsTest.vectorString(worldLocation) + ", localLoc: " + ViroEventsTest.vectorString(localLocation));
        });

        sphereNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("DragFixedToWorld on sphere, worldLoc: " + ViroEventsTest.vectorString(worldLocation) + ", localLoc: " + ViroEventsTest.vectorString(localLocation));
        });

        assertPass("For ARCORE: All objects can be dragged in the world.");
    }

    private void testEventsDragListener() {
        boxNode.setDragType(Node.DragType.FIXED_DISTANCE);
        objectNode.setDragType(Node.DragType.FIXED_DISTANCE);
        sphereNode.setDragType(Node.DragType.FIXED_DISTANCE);
        boxNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("DragFixed on box, worldLoc: " + ViroEventsTest.vectorString(worldLocation) + ", localLoc: " + ViroEventsTest.vectorString(localLocation));
        });

        objectNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("DragFixed on obj., worldLoc: " + ViroEventsTest.vectorString(worldLocation) + ", localLoc: " + ViroEventsTest.vectorString(localLocation));
        });

        sphereNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("DragFixed on sphere, worldLoc: " + ViroEventsTest.vectorString(worldLocation) + ", localLoc: " + ViroEventsTest.vectorString(localLocation));
        });

        assertPass("All objects can be dragged within fixed distance(not the world).");
    }

    private void testEventsGesturePinchListener() {
        boxNode.setGesturePinchListener((source, node, scaleFactor, pinchState) -> {
            eventText.setText("Pinch scale on box, scaleFactor "  + scaleFactor + ", pinchState: " + pinchState.toString());
            boxNode.setScale(new Vector(scaleFactor, scaleFactor, scaleFactor));
        });

        objectNode.setGesturePinchListener((source, node, scaleFactor, pinchState) -> {
            eventText.setText("Pinch scale on Obj, scaleFactor "  + scaleFactor + ", pinchState: " + pinchState.toString());
            objectNode.setScale(new Vector(scaleFactor, scaleFactor, scaleFactor));
        });

        sphereNode.setGesturePinchListener((source, node, scaleFactor, pinchState) -> {
            eventText.setText("Pinch scale on Sphere, scaleFactor "  + scaleFactor + ", pinchState: " + pinchState.toString());
            sphereNode.setScale(new Vector(scaleFactor, scaleFactor, scaleFactor));
        });

        assertPass("All objects can be pinched and scaled. Text updated with proper pinchStates.");

    }

    private void testEventsGestureRotateListener() {
        boxNode.setGestureRotateListener((source, node, rotation, rotateState) -> {
            eventText.setText("Rotate gesture on box, rotation: "  + rotation + ", rotateState: " + rotateState.toString());
            boxNode.setRotation(new Vector(0, rotation, 0));
        });

        objectNode.setGestureRotateListener((source, node, rotation, rotateState) -> {
            eventText.setText("Rotate gesture on box, rotation: "  + rotation + ", rotateState: " + rotateState.toString());
            objectNode.setRotation(new Vector(0, rotation, 0));
        });

        sphereNode.setGestureRotateListener((source, node, rotation, rotateState) -> {
            eventText.setText("Rotate gesture on box, rotation: "  + rotation + ", rotateState: " + rotateState.toString());
            sphereNode.setRotation(new Vector(0, rotation, 0));
        });

        assertPass("All objects can be rotated via rotate gesture. Text updated with proper rotateStates.");
    }
}
