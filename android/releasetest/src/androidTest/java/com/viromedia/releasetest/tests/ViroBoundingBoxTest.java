package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.net.Uri;

import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.OmniLight;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class ViroBoundingBoxTest extends ViroBaseTest {
    private Node mAnchorNode;
    private Node mStageNode;
    private Node mPartNode;
    private Node mModelNode;

    public ViroBoundingBoxTest() {
        super();

    }

    @Override
    void configureTestScene() {
        // Add some lights to the scene; this will give the Android's some nice illumination.
        Node rootNode = mScene.getRootNode();
        List<Vector> lightPositions = new ArrayList<Vector>();
        lightPositions.add(new Vector(-10, 10, 1));
        lightPositions.add(new Vector(10, 10, 1));

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
    }

    @Test
    public void boundingBoxTest() {

        runUITest(() -> boxLocalTest1());
        runUITest(() -> boxLocalTest2());
        runUITest(() -> boxLocalTest3());
        runUITest(() -> boxFragmentedTest());
        runUITest(() -> boxWorldTest());

    }

    private void buildLocalScene() {
        Node rootNode = mScene.getRootNode();

        // Forward Testing
        mAnchorNode = new Node();
        rootNode.addChildNode(mAnchorNode);

        mStageNode = new Node();
        mStageNode.setName("Stage");
        mStageNode.setRotation(new Vector(0.0, -Math.PI / 4, 0.0));
        mAnchorNode.addChildNode(mStageNode);

        mPartNode = new Node();
        mPartNode.setName("Part");
        mPartNode.setPosition(new Vector(0.0, 0.3, 0.0));
        mStageNode.addChildNode(mPartNode);

        mModelNode = new Node();
        mModelNode.setName("Model");
        mPartNode.addChildNode(mModelNode);

        Box box = new Box(1.0f, 1.0f, 1.0f);
        Material material = new Material();
        material.setDiffuseColor(Color.argb(100, 255, 0, 0));
        box.setMaterials(Arrays.asList(material));
        mModelNode.setScale(new Vector(0.5f, 0.5f, 0.5f));
        mModelNode.setGeometry(box);
    }

    private void renderText(String text) {
        Node textNode = new Node();
        Text text1 = new Text(mViroView.getViroContext(), text,
                "Roboto", 20,
                Color.WHITE, 3f, 2f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.BOTTOM, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 50);
        textNode.setGeometry(text1);
        textNode.setPosition(new Vector(0.25, -0, -4));

        mScene.getRootNode().addChildNode(textNode);
    }

    public void boxLocalTest1() {
        buildLocalScene();
        renderText(mModelNode.getBoundingBoxLocal().toString());

        assertPass("Bounding box is [minX: -0.5, maxX: 0.5, minY: -0.5, maxY: 0.5, minZ: -0.5, maxZ: 0.5]", () -> {
            mAnchorNode = null;
        });
    }

    public void boxLocalTest2() {
        buildLocalScene();
        renderText(mPartNode.getBoundingBoxLocal().toString());
        assertPass("Bounding box is [minX: -0.25, maxX: 0.25, minY: -0.25, maxY: 0.25, minZ: -0.25, maxZ: 0.25]");
    }

    public void boxLocalTest3() {
        buildLocalScene();
        renderText(mStageNode.getBoundingBoxLocal().toString());
        assertPass("Bounding box is [minX: -0.25, maxX: 0.25, minY: 0.05, maxY: 0.55, minZ: -0.25, maxZ: 0.25]");
    }

    public void boxFragmentedTest() {
        buildLocalScene();

        Box box = new Box(2.0f, 2.0f, 2.0f);
        Material material = new Material();
        material.setDiffuseColor(Color.argb(100, 255, 0, 0));
        box.setMaterials(Arrays.asList(material));

        Node secondModelNode = new Node();
        secondModelNode.setGeometry(box);
        secondModelNode.setPosition(new Vector(4, 4, 4));

        mModelNode.addChildNode(secondModelNode);

        renderText(mModelNode.getBoundingBoxLocal().toString());
        assertPass("Bounding box is [minX: -0.5, maxX: 5.0, minY: -0.5, maxY: 5.0, minZ: -0.5, maxZ: 5.0]");
    }

    public void boxWorldTest() {
        buildLocalScene();
        renderText(mStageNode.getBoundingBox().toString());
        assertPass("Bounding box is [minX: -0.353, maxX: 0.353, minY: 0.05, maxY: 0.55, minZ: -0.353, maxZ: 0.353]");
    }
}