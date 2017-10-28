package com.viromedia.renderertest.tests;

import android.graphics.Color;

import com.google.ar.core.Config;
import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Vector;
import com.viro.renderer.jni.Node;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Created by vadvani on 10/26/17.
 */


public class ViroBoxTest extends ViroBaseTest {
    private Box mBox;

    public ViroBoxTest() {
        super();
        mBox = null;
    }

    @Override
    void configureTestScene() {
        float[] lightDirection = {0, 0, -1};
        AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(ambientLightJni);

        Material material = new Material();
        material.setDiffuseColor(Color.BLUE);
        material.setLightingModel(Material.LightingModel.BLINN);

        // Creation of ViroBox
        Node node = new Node();
        mBox = new Box(2,4,2);

        node.setGeometry(mBox);
        float[] boxPosition = {0,0,-5};
        node.setPosition(new Vector(boxPosition));
        mBox.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(node);
    }

    @Test
    public void boxTest() {
        boxWidthTest();
        boxHeightTest();

        boxLengthTest();
    }

    @Test
    public void boxWidthTest() {
        mMutableTestMethod = () -> {
            if(mBox != null ) {
                mBox.setWidth(mBox.getWidth() +1);
            }
        };
    }

    public void boxHeightTest() {
        mMutableTestMethod = () -> {
            if(mBox != null) {
                mBox.setHeight(mBox.getHeight() +1);
            }
        };

    }


    public void boxLengthTest() {
        mMutableTestMethod = () -> {
            if(mBox != null) {
                mBox.setLength(mBox.getLength() +1);
            }
        };

    }
}
