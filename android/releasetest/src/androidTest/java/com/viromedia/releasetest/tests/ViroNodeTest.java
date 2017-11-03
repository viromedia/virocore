package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.util.Log;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Sphere;
import com.viro.renderer.jni.Vector;

import org.junit.Test;

import java.util.Arrays;

/**
 * ViroNodeTest - Tests all properties of {@link Node} except those related to Physics, Events and Animation as those
 * are covered in other tests.
 */

public class ViroNodeTest  extends ViroBaseTest {

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
        DirectionalLight light = new DirectionalLight(Color.WHITE, 1000.0f, new Vector(0, 0, -1f));
        mScene.getRootNode().addLight(light);

        //create 1st box.
        Box box = new Box(1, 1, 1);
        Material boxOneMaterial = new Material();
        boxOneMaterial.setDiffuseColor(Color.RED);
        boxOneMaterial.setLightingModel(Material.LightingModel.BLINN);
        box.setMaterials(Arrays.asList(boxOneMaterial));
        childOne.setGeometry(box);
        childOne.setPosition(new Vector(-1.5f, 0f, 0f));

        //create 2nd box.
        Box boxTwo = new Box(1, 1, 1);
        Material boxTwoMaterial = new Material();
        boxTwoMaterial.setDiffuseColor(Color.YELLOW);
        boxTwoMaterial.setLightingModel(Material.LightingModel.BLINN);
        boxTwo.setMaterials(Arrays.asList(boxTwoMaterial));
        childTwo.setGeometry(boxTwo);
        childTwo.setPosition(new Vector(0.0f, 1f, 0f));

        //create 3rd box, right node
        Box boxThree = new Box(1, 1, 1);
        Material boxThreeMaterial = new Material();
        boxThreeMaterial.setDiffuseColor(Color.GREEN);
        boxThreeMaterial.setLightingModel(Material.LightingModel.BLINN);
        boxThree.setMaterials(Arrays.asList(boxThreeMaterial));
        childThree.setGeometry(boxThree);
        childThree.setPosition(new Vector(1.5f, 0f, 0f));

        Sphere sphere = new Sphere(.5f);
        Material sphereMaterial = new Material();
        sphereMaterial.setDiffuseColor(Color.DKGRAY);
        sphereMaterial.setLightingModel(Material.LightingModel.BLINN);
        sphere.setMaterials(Arrays.asList(sphereMaterial));
        parentSphereNode.setGeometry(sphere);
        parentSphereNode.setPosition(new Vector(0f, 0f, -5f));

    }

    @Test
    public void testNode() {
        testNodeAddChildren();

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
        //testNodeTransformBehaviors();
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
                Vector newPosition = new Vector(parentSphereNode.getPositionRealtime().x, parentSphereNode.getPositionRealtime().y, parentSphereNode.getPositionRealtime().z - .5f);
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
                Vector newRotation = new Vector(parentSphereNode.getRotationEulerRealtime().x + +25f, parentSphereNode.getRotationEulerRealtime().y + +25f, parentSphereNode.getRotationEulerRealtime().z + 25f);
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

                Vector newScale = new Vector(parentSphereNode.getScaleRealtime().x - .2f, parentSphereNode.getScaleRealtime().y - .2f, parentSphereNode.getScaleRealtime().z - .2f);
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
                Vector newPosition = new Vector(childOne.getPositionRealtime().x, childOne.getPositionRealtime().y, childOne.getPositionRealtime().z - .5f);
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
                Vector newRotation = new Vector(childTwo.getRotationEulerRealtime().x, childTwo.getRotationEulerRealtime().y, childTwo.getRotationEulerRealtime().z + 25f);
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
                Vector newScale = new Vector(childThree.getScaleRealtime().x - .2f, childThree.getScaleRealtime().y - .2f, childThree.getScaleRealtime().z - .2f);
                childThree.setScale(newScale);
            }
        };

        assertPass("Right node scale shrinks.", () -> {
            childThree.setScale(new Vector(1f, 1f, 1f));
        });
    }

    private void testNodeChangeGeometry() {
        Box box = new Box(1, 1, 1);
        Material boxMaterial = new Material();
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
        ;
        assertPass("Left node should be removed.");
    }

    private void testNodeRemoveAllChildren() {
        parentSphereNode.removeAllChildNodes();
        assertPass("All nodes but middle box should be removed.");
    }

}
