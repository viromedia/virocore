package com.viromedia.releasetest.tests;

import android.graphics.Color;

import com.viro.core.Box;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Polyline;
import com.viro.core.Sphere;
import com.viro.core.Vector;

import org.junit.Test;

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
        childOne.setPosition(new Vector(-1.5f, 0f, 0f));

        //create 2nd box.
        final Box boxTwo = new Box(1, 1, 1);
        final Material boxTwoMaterial = new Material();
        boxTwoMaterial.setDiffuseColor(Color.YELLOW);
        boxTwoMaterial.setLightingModel(Material.LightingModel.BLINN);
        boxTwo.setMaterials(Arrays.asList(boxTwoMaterial));
        childTwo.setGeometry(boxTwo);
        childTwo.setPosition(new Vector(0.0f, 1f, 0f));

        //create 3rd box, right node
        final Box boxThree = new Box(1, 1, 1);
        final Material boxThreeMaterial = new Material();
        boxThreeMaterial.setDiffuseColor(Color.GREEN);
        boxThreeMaterial.setLightingModel(Material.LightingModel.BLINN);
        boxThree.setMaterials(Arrays.asList(boxThreeMaterial));
        childThree.setGeometry(boxThree);
        childThree.setPosition(new Vector(1.5f, 0f, 0f));

        final Sphere sphere = new Sphere(.5f);
        final Material sphereMaterial = new Material();
        sphereMaterial.setDiffuseColor(Color.DKGRAY);
        sphereMaterial.setLightingModel(Material.LightingModel.BLINN);
        sphere.setMaterials(Arrays.asList(sphereMaterial));
        parentSphereNode.setGeometry(sphere);
        parentSphereNode.setPosition(new Vector(0f, 0f, -5f));
    }

    @Test
    public void testNode() {
        testNodeAddChildren();
        testConvertLocalToWorld();
        testConvertWorldToLocal();
        testNodePositionParent();
        testNodeRotationParent();
        testNodeScaleParent();
        testNodeVisibilityParent();
        testNodeOpacityParent();
        testNodePositionChild();
        testNodeRotationChild();
        testNodeScaleChild();
        testNodeVisibilityChild();
        testNodeOpacityChild();
        testNodeChangeGeometry();
        testNodeRemoveFromParent();
        testNodeRemoveAllChildren();
        testTransformBehaviorX();
        testTransformBehaviorY();
        testTransformBehaviorXY();
    }

    private void testNodeAddChildren() {
        parentSphereNode.addChildNode(childOne);
        parentSphereNode.addChildNode(childTwo);
        parentSphereNode.addChildNode(childThree);
        mScene.getRootNode().addChildNode(parentSphereNode);
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
        assertPass("All nodes are moving back till -10", () -> {
            parentSphereNode.setRotation(new Vector(0f, 0f, -5f));
        });
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
        assertPass("All nodes are rotating around all x,y,z axis", () -> {
            parentSphereNode.setRotation(new Vector(0f, 0f, 0f));
        });
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

        assertPass("All have smaller scale.", () -> {
            parentSphereNode.setScale(new Vector(1f, 1f, 1f));
        });
    }

    private void testNodeVisibilityParent() {
        mMutableTestMethod = () -> {
            if (parentSphereNode != null) {
                parentSphereNode.setVisible(!parentSphereNode.isVisible());
            }
        };

        assertPass("All nodes dissappear and reappear.", () -> {
            parentSphereNode.setVisible(true);
        });
    }

    private void testNodeOpacityParent() {
        mMutableTestMethod = () -> {
            if (parentSphereNode != null && parentSphereNode.getOpacity() > 0.0f) {
                parentSphereNode.setOpacity(parentSphereNode.getOpacity() - .1f);
            }
        };

        assertPass("All nodes fade out.", () -> {
            parentSphereNode.setOpacity(1.0f);
        });
    }

    private void testNodeOpacityChild() {
        mMutableTestMethod = () -> {
            if (childOne != null && childOne.getOpacity() > 0.0f) {
                childOne.setOpacity(childOne.getOpacity() - .1f);
            }
        };

        assertPass("Left node fades out.", () -> {
            childOne.setOpacity(1.0f);
        });
    }

    private void testNodeVisibilityChild() {
        mMutableTestMethod = () -> {
            childOne.setVisible(!childOne.isVisible());
            childThree.setVisible(!childThree.isVisible());
        };

        assertPass("Left and right nodes disappear and reappear.", () -> {
            childOne.setVisible(true);
            childThree.setVisible(true);
        });
    }

    private void testNodePositionChild() {
        mMutableTestMethod = () -> {
            if (childOne != null && childOne.getPositionRealtime().z > -10) {
                final Vector newPosition = new Vector(childOne.getPositionRealtime().x,
                        childOne.getPositionRealtime().y, childOne.getPositionRealtime().z - .5f);
                childOne.setPosition(newPosition);
            }
        };
        assertPass("Left node is moving back till -10", () -> {
            childOne.setPosition(new Vector(-1.5f, 0f, 0f));
        });
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
        assertPass("top node is rotating", () -> {
            childTwo.setRotation(new Vector(0f, 0f, 0f));
        });
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

        assertPass("Right node scale shrinks.", () -> {
            childThree.setScale(new Vector(1f, 1f, 1f));
        });
    }

    private void testNodeChangeGeometry() {
        final Box box = new Box(1, 1, 1);
        final Material boxMaterial = new Material();
        boxMaterial.setDiffuseColor(Color.BLUE);
        boxMaterial.setLightingModel(Material.LightingModel.BLINN);
        box.setMaterials(Arrays.asList(boxMaterial));
        parentSphereNode.setGeometry(box);
        parentSphereNode.setPosition(new Vector(0f, 0f, -5f));
        parentSphereNode.setGeometry(box);
        assertPass("Sphere should change to box.");
    }

    private void testNodeRemoveFromParent() {
        childOne.removeFromParentNode();
        assertPass("Left node should be removed.");
    }

    private void testNodeRemoveAllChildren() {
        parentSphereNode.removeAllChildNodes();
        assertPass("All nodes but middle box should be removed.", new TestCleanUpMethod() {
            @Override
            public void cleanUp() {
                parentSphereNode.addChildNode(childOne);
                parentSphereNode.addChildNode(childTwo);
                parentSphereNode.addChildNode(childThree);
            }
        });
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
        mYesButtonNode.setTransformBehaviors(transformBehavior);
        assertPass("Drag the Thumbs-Up above and below the X axis, and confirm the button " +
                "billboards along the axis", () -> {
            mYesButtonNode.setTransformBehaviors(EnumSet.of(Node.TransformBehavior.BILLBOARD_Y));
            mYesButtonNode.setPosition(new Vector(2.5f, -0.5f, -3.3f));
        });
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
        mYesButtonNode.setTransformBehaviors(transformBehavior);
        assertPass("Drag the Thumbs-Up left and right of the Y axis, and confirm the button " +
                "billboards along the axis", () -> {
            mYesButtonNode.setTransformBehaviors(EnumSet.of(Node.TransformBehavior.BILLBOARD_Y));
            mYesButtonNode.setPosition(new Vector(2.5f, -0.5f, -3.3f));
        });
    }

    private void testTransformBehaviorXY() {
        final EnumSet<Node.TransformBehavior> transformBehavior =
                EnumSet.of(Node.TransformBehavior.BILLBOARD);
        mYesButtonNode.setTransformBehaviors(transformBehavior);
        assertPass("Drag the Thumbs-Up far out in any quadrant and, and confirm the button " +
                "billboards along both the axis", () -> {
            mYesButtonNode.setTransformBehaviors(EnumSet.of(Node.TransformBehavior.BILLBOARD_Y));
            mYesButtonNode.setPosition(new Vector(2.5f, -0.5f, -3.3f));
        });
    }

    private void testConvertLocalToWorld() {
        parentSphereNode.setPosition(new Vector(0, 1, -1));
        childOne.setPosition(new Vector(0, 1, 1));

        Vector worldPosition = childOne.convertLocalPositionToWorldSpace(new Vector(0, -2, 0));
        assertPass("The world position printed here " + worldPosition + " is [0, 0, 0]");
    }

    private void testConvertWorldToLocal() {
        parentSphereNode.setPosition(new Vector(0, 2, 2));
        childOne.setPosition(new Vector(2, 0, 0));

        Vector localPosition = childOne.convertWorldPositionToLocalSpace(new Vector(1, 1, 1));
        assertPass("The local position printed here " + localPosition + " is [-1, -1, -1]");
    }
}
