package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.support.test.annotation.UiThreadTest;
import android.util.Log;

import com.viro.core.Box;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Matrix;
import com.viro.core.Node;
import com.viro.core.Polyline;
import com.viro.core.Quaternion;
import com.viro.core.Sphere;
import com.viro.core.Vector;

import org.junit.Test;

import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.List;

/**
 * ViroNodeTest - Tests all properties of {@link Node} except those related to Physics, Events and Animation as those
 * are covered in other tests.
 */

public class ViroNodeTest extends ViroBaseTest {

    private Node childOne;
    private Node childTwo;
    private Node childThree;
    private Node parentSphereNode;

    @Override
    void configureTestScene() {
        childOne = new Node();
        childTwo = new Node();
        childThree = new Node();
        parentSphereNode = new Node();
        final DirectionalLight light = new DirectionalLight(Color.WHITE, 1000.0f, new Vector(0, 0, -1f));
        mScene.getRootNode().addLight(light);

        //create 1st box.
        final Box box = new Box(1, 1, 1);
        final Material boxOneMaterial = new Material();
        boxOneMaterial.setDiffuseColor(Color.RED);
        boxOneMaterial.setLightingModel(Material.LightingModel.BLINN);
        box.setMaterials(Arrays.asList(boxOneMaterial));
        childOne.setGeometry(box);
        childOne.setOpacity(1.0f);
        childOne.setVisible(true);
        childOne.setPosition(new Vector(-1.5f, 0f, 0f));

        //create 2nd box.
        final Box boxTwo = new Box(1, 1, 1);
        final Material boxTwoMaterial = new Material();
        boxTwoMaterial.setDiffuseColor(Color.YELLOW);
        boxTwoMaterial.setLightingModel(Material.LightingModel.BLINN);
        boxTwo.setMaterials(Arrays.asList(boxTwoMaterial));
        childTwo.setGeometry(boxTwo);
        childTwo.setRotation(new Vector(0f, 0f, 0f));
        childTwo.setPosition(new Vector(0.0f, 1f, 0f));

        //create 3rd box, right node
        final Box boxThree = new Box(1, 1, 1);
        final Material boxThreeMaterial = new Material();
        boxThreeMaterial.setDiffuseColor(Color.GREEN);
        boxThreeMaterial.setLightingModel(Material.LightingModel.BLINN);
        boxThree.setMaterials(Arrays.asList(boxThreeMaterial));
        childThree.setGeometry(boxThree);
        childThree.setVisible(true);
        childThree.setScale(new Vector(1f, 1f, 1f));
        childThree.setPosition(new Vector(1.5f, 0f, 0f));

        final Sphere sphere = new Sphere(.5f);
        final Material sphereMaterial = new Material();
        sphereMaterial.setDiffuseColor(Color.DKGRAY);
        sphereMaterial.setLightingModel(Material.LightingModel.BLINN);
        sphere.setMaterials(Arrays.asList(sphereMaterial));
        parentSphereNode.setGeometry(sphere);
        parentSphereNode.setPosition(new Vector(0f, 0f, -5f));
        parentSphereNode.setRotation(new Vector(0f, 0f, 0f));
        parentSphereNode.setScale(new Vector(1f, 1f, 1f));
        parentSphereNode.setVisible(true);
        parentSphereNode.setOpacity(1.0f);

        parentSphereNode.addChildNode(childOne);
        parentSphereNode.addChildNode(childTwo);
        parentSphereNode.addChildNode(childThree);
        mScene.getRootNode().addChildNode(parentSphereNode);

        if (mYesButtonNode != null) {
            mYesButtonNode.setTransformBehaviors(EnumSet.of(Node.TransformBehavior.BILLBOARD_Y));
            mYesButtonNode.setPosition(new Vector(2.5f, -0.5f, -3.3f));
        }
    }

    @Test
    public void testNode() {
        runUITest(() -> testNodeAddChildren());
        runUITest(() -> testEulerRotationOfChild());
        runUITest(() -> testConvertLocalToWorld());
        runUITest(() -> testConvertWorldToLocal());
        runUITest(() -> testConvertLocalToWorldWithSleep());
        runUITest(() -> testConvertWorldToLocalWithSleep());
        runUITest(() -> testNodePositionParent());
        runUITest(() -> testNodeRotationParent());
        runUITest(() -> testNodeScaleParent());
        runUITest(() -> testNodeVisibilityParent());
        runUITest(() -> testNodeOpacityParent());
        runUITest(() -> testNodePositionChild());
        runUITest(() -> testNodeRotationChild());
        runUITest(() -> testNodeScaleChild());
        runUITest(() -> testNodeVisibilityChild());
        runUITest(() -> testNodeOpacityChild());
        runUITest(() -> testNodeChangeGeometry());
        runUITest(() -> testNodeRemoveFromParent());
        runUITest(() -> testNodeRemoveAllChildren());
        runUITest(() -> testTransformBehaviorX());
        runUITest(() -> testTransformBehaviorY());
        runUITest(() -> testTransformBehaviorXY());
    }

    private void testNodeAddChildren() {
        // Test the configureScene construction
        assertPass("A left, middle, right and top node appear.");
    }

    private void testNodePositionParent() {
        mMutableTestMethod = () -> {
            if (parentSphereNode != null && parentSphereNode.getPositionRealtime().z > -10) {
                final Vector newPosition = new Vector(parentSphereNode.getPositionRealtime().x,
                        parentSphereNode.getPositionRealtime().y,
                        parentSphereNode.getPositionRealtime().z - .5f);
                parentSphereNode.setPosition(newPosition);
            }
        };
        assertPass("All nodes are moving back to -10");
    }

    private void testNodeRotationParent() {
        mMutableTestMethod = () -> {
            if (parentSphereNode != null) {
                final Vector newRotation =
                        new Vector(parentSphereNode.getRotationEulerRealtime().x,
                                   parentSphereNode.getRotationEulerRealtime().y + Math.toRadians(25),
                                   parentSphereNode.getRotationEulerRealtime().z);
                parentSphereNode.setRotation(newRotation);
            }
        };
        assertPass("All nodes are rotating around all x,y,z axis");
    }


    private void testNodeScaleParent() {
        mMutableTestMethod = () -> {
            if (parentSphereNode != null && parentSphereNode.getScaleRealtime().z > .2f) {

                final Vector newScale = new Vector(parentSphereNode.getScaleRealtime().x - .2f,
                        parentSphereNode.getScaleRealtime().y - .2f,
                        parentSphereNode.getScaleRealtime().z - .2f);
                parentSphereNode.setScale(newScale);
            }
        };

        assertPass("All have smaller scale");
    }

    private void testNodeVisibilityParent() {
        mMutableTestMethod = () -> {
            if (parentSphereNode != null) {
                parentSphereNode.setVisible(!parentSphereNode.isVisible());
            }
        };

        assertPass("All nodes dissappear and reappear");
    }

    private void testNodeOpacityParent() {
        mMutableTestMethod = () -> {
            if (parentSphereNode != null && parentSphereNode.getOpacity() > 0.0f) {
                parentSphereNode.setOpacity(parentSphereNode.getOpacity() - .1f);
            }
        };

        assertPass("All nodes fade out");
    }

    private void testNodeOpacityChild() {
        mMutableTestMethod = () -> {
            if (childOne != null && childOne.getOpacity() > 0.0f) {
                childOne.setOpacity(childOne.getOpacity() - .1f);
            }
        };

        assertPass("Left node fades out");
    }

    private void testNodeVisibilityChild() {
        mMutableTestMethod = () -> {
            childOne.setVisible(!childOne.isVisible());
            childThree.setVisible(!childThree.isVisible());
        };

        assertPass("Left and right nodes disappear and reappear");
    }

    private void testNodePositionChild() {
        mMutableTestMethod = () -> {
            if (childOne != null && childOne.getPositionRealtime().z > -10) {
                final Vector newPosition = new Vector(childOne.getPositionRealtime().x,
                        childOne.getPositionRealtime().y, childOne.getPositionRealtime().z - .5f);
                childOne.setPosition(newPosition);
            }
        };
        assertPass("Left node is moving back to -10");
    }

    private void testNodeRotationChild() {
        mMutableTestMethod = () -> {
            if (childTwo != null && childTwo.getRotationEulerRealtime().z < 180) {
                final Vector newRotation = new Vector(childTwo.getRotationEulerRealtime().x,
                        childTwo.getRotationEulerRealtime().y,
                        childTwo.getRotationEulerRealtime().z + Math.toRadians(25));
                childTwo.setRotation(newRotation);
            }
        };
        assertPass("Top node is rotating");
    }


    private void testNodeScaleChild() {
        mMutableTestMethod = () -> {
            if (childThree != null && childThree.getScaleRealtime().z > .2f) {
                final Vector newScale = new Vector(childThree.getScaleRealtime().x - .2f,
                        childThree.getScaleRealtime().y - .2f,
                        childThree.getScaleRealtime().z - .2f);
                childThree.setScale(newScale);
            }
        };

        assertPass("Right node scale shrinks");
    }

    private void testNodeChangeGeometry() {
        final Box box = new Box(1, 1, 1);
        final Material boxMaterial = new Material();
        boxMaterial.setDiffuseColor(Color.BLUE);
        boxMaterial.setLightingModel(Material.LightingModel.BLINN);
        box.setMaterials(Arrays.asList(boxMaterial));
        parentSphereNode.setGeometry(box);
        parentSphereNode.setPosition(new Vector(0f, 0f, -5f));
        assertPass("Center geometry should now be a box");
    }

    private void testNodeRemoveFromParent() {
        childOne.removeFromParentNode();
        assertPass("Left node should be removed.");
    }

    private void testNodeRemoveAllChildren() {
        parentSphereNode.removeAllChildNodes();
        assertPass("All nodes but middle box should be removed.");
    }

    private void testTransformBehaviorX() {
        final Node polylineNode;
        final Polyline polyline;

        final Material material = new Material();
        material.setDiffuseColor(Color.GREEN);
        material.setLightingModel(Material.LightingModel.CONSTANT);
        material.setCullMode(Material.CullMode.NONE);
        final float[][] points = {{-2, 0, 0}, {-1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {2, 0, 0}};
        List<Vector> pointsList = new ArrayList();
        pointsList.add(new Vector(-2,0,0));
        pointsList.add(new Vector(-1,0,0));
        pointsList.add(new Vector(0,0,0));
        pointsList.add(new Vector(1,0,0));
        pointsList.add(new Vector(2,0,0));

        polyline = new Polyline(pointsList, 0.1f);
        polyline.setMaterials(Arrays.asList(material));
        polylineNode = new Node();
        polylineNode.setGeometry(polyline);
        polylineNode.setPosition(new Vector(0, 0f, -3.3f));
        mScene.getRootNode().addChildNode(polylineNode);
        final EnumSet<Node.TransformBehavior> transformBehavior =
                EnumSet.of(Node.TransformBehavior.BILLBOARD_X);
        if (mYesButtonNode != null) {
            mYesButtonNode.setTransformBehaviors(transformBehavior);
        }
        assertPass("Drag the Thumbs-Up above and below the X axis, and confirm the button " +
                "billboards along the axis");
    }

    private void testTransformBehaviorY() {
        final Node polylineNode;
        final Polyline polyline;

        final Material material = new Material();
        material.setDiffuseColor(Color.GREEN);
        material.setLightingModel(Material.LightingModel.CONSTANT);
        material.setCullMode(Material.CullMode.NONE);
        final float[][] points = {{0, -2, 0}, {0, -1, 0}, {0, 0, 0}, {0, 1, 0}, {0, 2, 0}};


        List<Vector> pointsList = new ArrayList();
        pointsList.add(new Vector(0,-2,0));
        pointsList.add(new Vector(0,-1,0));
        pointsList.add(new Vector(0,0,0));
        pointsList.add(new Vector(0,1,0));
        pointsList.add(new Vector(0,2,0));

        polyline = new Polyline(pointsList, 0.1f);
        polyline.setMaterials(Arrays.asList(material));
        polylineNode = new Node();
        polylineNode.setGeometry(polyline);
        polylineNode.setPosition(new Vector(0, 0f, -3.3f));
        mScene.getRootNode().addChildNode(polylineNode);
        final EnumSet<Node.TransformBehavior> transformBehavior =
                EnumSet.of(Node.TransformBehavior.BILLBOARD_Y);
        if (mYesButtonNode != null) {
            mYesButtonNode.setTransformBehaviors(transformBehavior);
        }
        assertPass("Drag the Thumbs-Up left and right of the Y axis, and confirm the button " +
                "billboards along the axis");
    }

    private void testTransformBehaviorXY() {
        final EnumSet<Node.TransformBehavior> transformBehavior =
                EnumSet.of(Node.TransformBehavior.BILLBOARD);
        if (mYesButtonNode != null) {
            mYesButtonNode.setTransformBehaviors(transformBehavior);
        }
        assertPass("Drag the Thumbs-Up far out in any quadrant and, and confirm the button " +
                "billboards along both the axis");
    }

    private void testConvertLocalToWorld() {
        parentSphereNode.removeAllChildNodes();
        parentSphereNode.addChildNode(childOne);

        parentSphereNode.setPosition(new Vector(0, 1, -1));
        childOne.setPosition(new Vector(0, 1, 1));

        Vector worldPosition = childOne.convertLocalPositionToWorldSpace(new Vector(0, -2, 0));
        assertPass("The world position printed here " + worldPosition + " is [0, 0, 0]");
    }

    private void testConvertWorldToLocal() {
        parentSphereNode.removeAllChildNodes();
        parentSphereNode.addChildNode(childOne);

        parentSphereNode.setPosition(new Vector(0, 2, 2));
        childOne.setPosition(new Vector(2, 0, 0));

        Vector localPosition = childOne.convertWorldPositionToLocalSpace(new Vector(1, 1, 1));
        assertPass("The local position printed here " + localPosition + " is [-1, -1, -1]");
    }

    private void testConvertLocalToWorldWithSleep() {
        // The sleep here tests that the transforms are being updated even when the
        // scene graph changes, and even after the rendering thread has had time to
        // process those changes
        parentSphereNode.removeAllChildNodes();
        try {
            Thread.sleep(500);
        }catch(InterruptedException e) {

        }
        parentSphereNode.addChildNode(childOne);

        parentSphereNode.setPosition(new Vector(0, 1, -1));
        childOne.setPosition(new Vector(0, 1, 1));

        Vector worldPosition = childOne.convertLocalPositionToWorldSpace(new Vector(0, -2, 0));
        assertPass("The world position printed here " + worldPosition + " is [0, 0, 0]");
    }

    private void testConvertWorldToLocalWithSleep() {
        // The sleep here tests that the transforms are being updated even when the
        // scene graph changes, and even after the rendering thread has had time to
        // process those changes
        parentSphereNode.removeAllChildNodes();
        try {
            Thread.sleep(500);
        }catch(InterruptedException e) {

        }
        parentSphereNode.addChildNode(childOne);

        parentSphereNode.setPosition(new Vector(0, 2, 2));
        childOne.setPosition(new Vector(2, 0, 0));

        Vector localPosition = childOne.convertWorldPositionToLocalSpace(new Vector(1, 1, 1));
        assertPass("The local position printed here " + localPosition + " is [-1, -1, -1]");
    }

    private void testEulerRotationOfChild() {
        parentSphereNode.setRotation(new Vector(Math.toRadians(45), Math.toRadians(0), Math.toRadians(0)));
        childOne.setRotation(new Vector(Math.toRadians(45), Math.toRadians(0), Math.toRadians(0)));

        float localRotationX = childOne.getRotationEulerRealtime().x;
        Matrix worldTransform = childOne.getWorldTransformRealTime();
        Vector worldScale = worldTransform.extractScale();
        Quaternion worldRotation = worldTransform.extractRotation(worldScale);
        Vector worldEuler = worldRotation.toEuler();

        NumberFormat formatter = new DecimalFormat("#0.00");
        assertPass("Local rotation here [" + formatter.format(Math.toDegrees(localRotationX)) + "] is ~45, world rotation ["
                + formatter.format(Math.toDegrees(worldEuler.x)) + "] is ~90");
    }
}
