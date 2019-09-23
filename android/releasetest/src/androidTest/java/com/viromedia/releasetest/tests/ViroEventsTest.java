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

import android.graphics.Color;
import android.net.Uri;
import android.util.Log;

import com.viro.core.Box;
import com.viro.core.ClickState;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Sphere;
import com.viro.core.Text;
import com.viro.core.Vector;
import com.viro.core.ClickListener;

import org.junit.Test;

import java.util.Arrays;

/**
 * Created by vadvani on 11/3/17.
 */

public class ViroEventsTest extends ViroBaseTest {
    ClickListener boxClickListener;
    private Node boxNode;
    private Object3D objectNode;
    private Node sphereNode;
    private Text eventText;
    private static final String DEFAULT_EVENT_TEXT = "No events Detected.";

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
        boxNode.setPosition(new Vector(-1.5f, 1f, -3f));
        boxNode.setGeometry(box);


        //add object node.
        objectNode = new Object3D();
        objectNode.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, null);
        objectNode.setPosition(new Vector(0.0f, 1f, -3f));

        //add sphere node.
        Sphere sphere = new Sphere(.5f);
        Material sphereMaterial = new Material();
        sphereMaterial.setDiffuseColor(Color.YELLOW);
        sphereMaterial.setLightingModel(Material.LightingModel.BLINN);
        sphere.setMaterials(Arrays.asList(boxOneMaterial));
        sphereNode = new Node();
        sphereNode.setGeometry(sphere);
        sphereNode.setPosition(new Vector(1.5f, 1.0f, -3f));


        mScene.getRootNode().addChildNode(boxNode);
        mScene.getRootNode().addChildNode(objectNode);
        mScene.getRootNode().addChildNode(sphereNode);
        eventText = new Text(mViroView.getViroContext(),
                DEFAULT_EVENT_TEXT, "Roboto", 20,
                Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.CLIP_TO_BOUNDS, 0);

        Node textNode = new Node();
        textNode.setPosition(new Vector(0, 3.0f, -4f));
        textNode.setGeometry(eventText);
        mScene.getRootNode().addChildNode(textNode);

        objectNode.setClickListener(null);
        sphereNode.setClickListener(null);
        boxNode.setClickListener(null);
        boxClickListener = null;
        boxNode.setHoverListener(null);
        objectNode.setHoverListener(null);
        sphereNode.setHoverListener(null);
        boxNode.setFuseListener(null);
        objectNode.setFuseListener(null);
        sphereNode.setFuseListener(null);
        sphereNode.setDragListener(null);
        objectNode.setDragListener(null);
        boxNode.setDragListener(null);
        sphereNode.setGesturePinchListener(null);
        objectNode.setGesturePinchListener(null);
        boxNode.setGesturePinchListener(null);
        sphereNode.setGestureRotateListener(null);
        objectNode.setGestureRotateListener(null);
        boxNode.setGestureRotateListener(null);
        sphereNode.setTouchpadTouchListener(null);
        objectNode.setTouchpadTouchListener(null);
        boxNode.setTouchpadTouchListener(null);
        sphereNode.setTouchpadSwipeListener(null);
        objectNode.setTouchpadSwipeListener(null);
        boxNode.setTouchpadSwipeListener(null);
        sphereNode.setTouchpadScrollListener(null);
        objectNode.setTouchpadScrollListener(null);
        boxNode.setTouchpadScrollListener(null);
        eventText.setText(DEFAULT_EVENT_TEXT);

    }

    @Test
    public void testEvents() {
        runUITest(() -> testEventsClickListener());
        runUITest(() -> testEventsHoverListener());
        runUITest(() -> testEventsDragListener());
        runUITest(() -> testEventsFuseListener());
        runUITest(() -> testEventsTouchpadTouchListener());
        runUITest(() -> testEventsTouchpadSwipeListener());
        runUITest(() -> testEventsTouchpadScrollListener());
        //TODO: need to test bubbling of events from child to parent.
    }


    private void testEventsClickListener() {
        Log.i("ViroEventsTest", "in testEventsClickListener()");
        //boxNode.setClickListener((source, node, clickState, location) -> {
        //    eventText.setText("Clicked on box.");
        //    Log.i("ViroEventsTest", "CLICKED ON BOX!!");
        //});

        boxNode.setClickListener(new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                eventText.setText("Clicked on box node.");
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {

            }
        });

        boxClickListener = new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                eventText.setText("Clicked on 3d object.");
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {

            }
        };

        objectNode.setClickListener(boxClickListener);

        sphereNode.setClickListener(new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                eventText.setText("Clicked on sphere object.");
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {

            }
        });

        assertPass("All objects are clickable.");
    }

    private void testEventsHoverListener() {
        boxNode.setHoverListener((source, node, isHovering, location) -> {
            if (isHovering){
                eventText.setText("Hovered over box.");
            }
        });

        objectNode.setHoverListener((source, node, isHovering, location) -> {
            if (isHovering) {
                eventText.setText("Hovered over 3d object.");
            }
        });

        sphereNode.setHoverListener((source, node, isHovering, location) -> {
            if (isHovering) {
                eventText.setText("Hovered over sphere.");
            }
        });

        assertPass("All events hover.");
    }


    private void testEventsFuseListener() {
        boxNode.setFuseListener((source, node) -> {
            eventText.setText("Set fuse on box.");
        });

        objectNode.setFuseListener((source, node) -> {
            eventText.setText("Set fuse on 3d object");
        });

        sphereNode.setFuseListener((source, node) -> {
            eventText.setText("Set fuse on Sphere.");
        });

        assertPass("All nodes respond to onFuse.");
    }

    private void testEventsDragListener() {
        boxNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("Dragging the box. WorldLoc:" + ViroEventsTest.vectorString(worldLocation) + ", LocalLoc:" + ViroEventsTest.vectorString(localLocation));
        });

        objectNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("Dragging the object. WorldLoc:" + ViroEventsTest.vectorString(worldLocation) + ", LocalLoc:" + ViroEventsTest.vectorString(localLocation));
        });

        sphereNode.setDragListener((source, node, worldLocation, localLocation) -> {
            eventText.setText("Dragging the sphere. WorldLoc:" + ViroEventsTest.vectorString(worldLocation) + ", LocalLoc:" + ViroEventsTest.vectorString(localLocation));
        });

        assertPass("All objects respond to drag events with world loc. and local loc.");
    }

    private void testEventsGesturePinchListener() {
        //(int source, Node node, float scaleFactor, PinchState pinchState)
        boxNode.setGesturePinchListener((source, node, scaleFactor, pinchState) -> {
            eventText.setText("Pinching on box.");
            boxNode.setScale(new Vector(scaleFactor, scaleFactor, scaleFactor));
        });

        objectNode.setGesturePinchListener((source, node, scaleFactor, pinchState) -> {
            eventText.setText("Pinching on 3D Object.");
            objectNode.setScale(new Vector(scaleFactor, scaleFactor, scaleFactor));
        });

        sphereNode.setGesturePinchListener((source, node, scaleFactor, pinchState) -> {
            eventText.setText("Pinching on Sphere");
            sphereNode.setScale(new Vector(scaleFactor, scaleFactor, scaleFactor));
        });
        assertPass("All nodes respond to pinch.");
    }

    private void testEventsGestureRotateListener() {

        boxNode.setGestureRotateListener((source, node, rotateRadians, rotateState) -> {
            eventText.setText("Rotating on Box.");
        });

        objectNode.setGestureRotateListener((source, node, rotateRadians, rotateState) -> {
            eventText.setText("Rotating on 3d Object.");
        });

        sphereNode.setGestureRotateListener((source, node, rotateRadians, rotateState) -> {
            eventText.setText("Rotating on Sphere.");
        });

        assertPass("All nodes respond to rotate.");
    }

    private void testEventsTouchpadTouchListener() {
        boxNode.setTouchpadTouchListener((source, node, touchState, x, y) -> {
            eventText.setText("Touch registered on Box. TouchState: " + touchState.toString());
        });

        objectNode.setTouchpadTouchListener((source, node, touchState, x, y) -> {
            eventText.setText("Touch registered on 3d Object. TouchState: " + touchState.toString());
        });

        sphereNode.setTouchpadTouchListener((source, node, touchState, x, y) -> {
            eventText.setText("Touch registered on Sphere. TouchState: " + touchState.toString());
        });

        assertPass("For GearVR: All objects can be touched with touch pad, Touch state changes.");
    }

    private void testEventsTouchpadSwipeListener() {
        boxNode.setTouchpadSwipeListener((source, node, swipeState) -> {
            eventText.setText("Touchpad listener registered on Box. SwipeState: " + swipeState.toString());
        });

        objectNode.setTouchpadSwipeListener((source, node, swipeState) -> {
            eventText.setText("Touchpad listener registered on 3d Object. SwipeState: " + swipeState.toString());
        });

        sphereNode.setTouchpadSwipeListener((source, node, swipeState) -> {
            eventText.setText("Touchpad listener registered on Sphere. SwipeState: " + swipeState.toString());
        });

        assertPass("For GearVR: All objects register swipe event with touch pad.");
    }

    private void testEventsTouchpadScrollListener() {
        boxNode.setTouchpadScrollListener((source, node, x, y) -> {
            eventText.setText("Scroll listener registered on box. (x,y):" + "[" + x + "," + y + "]");
        });

        objectNode.setTouchpadScrollListener((source, node, x, y) -> {
            eventText.setText("Scroll listener registered on object. (x,y):" + "[" + x + "," + y + "]");
        });

        sphereNode.setTouchpadScrollListener((source, node, x, y) -> {
            eventText.setText("Scroll listener registered on sphere. (x,y):" + "[" + x + "," + y + "]");
        });

        assertPass("For GearVR: All objects register scroll event with touch pad.");
    }

    public static String vectorString(Vector vec) {
        String vectorString = "[" + vec.x + "," + vec.y + "," + vec.x + "]";
        return vectorString;
    }


}
