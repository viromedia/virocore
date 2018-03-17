package com.viromedia.releasetest.tests;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.support.test.espresso.core.deps.guava.collect.Iterables;
import android.util.Log;

import com.viro.core.ARAnchor;
import com.viro.core.ARNode;
import com.viro.core.ARPointCloud;
import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.PointCloudUpdateListener;
import com.viro.core.Surface;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Created by vadvani on 11/7/17.
 * This includes tests for all methods for ARScene except the onAnchor callbacks. Those are tested in ViroARNodeTest.
 */

public class ViroARSceneTest extends ViroBaseTest {

    private ARScene mARScene;
    private Node mBoxNode;
    private AmbientLight mAmbientLight;
    private Text mTestText;

    private boolean mAmbientLightUpdateTestStarted = false;
    @Override
    void configureTestScene() {
        mAmbientLight = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(mAmbientLight);
        mARScene = (ARScene)mScene;

        final Bitmap bobaBitmap = getBitmapFromAssets(mActivity, "boba.png");
        final Texture bobaTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);
        final Material material = new Material();
        material.setDiffuseTexture(bobaTexture);
        material.setLightingModel(Material.LightingModel.BLINN);
        Box box = new Box(1, 1, 1);
        box.setMaterials(Arrays.asList(material));
        mBoxNode = new Node();
        mBoxNode.setGeometry(box);
        mBoxNode.setPosition(new Vector(0, 0, -4));
        mScene.getRootNode().addChildNode(mBoxNode);

        mTestText = new Text(mViroView.getViroContext(),
                "No input yet.", "Roboto", 12, Text.FontStyle.Normal, Text.FontWeight.Regular,
                Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.CLIP_TO_BOUNDS, 0);

        Node textNode = new Node();
        textNode.setPosition(new Vector(0, -1f, -4f));
        textNode.setGeometry(mTestText);
        mScene.getRootNode().addChildNode(textNode);

        mARScene.setListener(new ARScene.Listener() {
            @Override
            public void onTrackingInitialized() {
                mTestText.setText("AR Initialized callback received!");
            }

            @Override
            public void onAmbientLightUpdate(float lightIntensity, float colorTemperature) {
                if(mAmbientLightUpdateTestStarted) {
                    mTestText.setText("Ambient light intensity:" + lightIntensity + ", colorTemp: " + colorTemperature);
                    mAmbientLight.setIntensity(lightIntensity);
                }
            }

            @Override
            public void onAnchorFound(ARAnchor anchor, ARNode node) {

            }

            @Override
            public void onAnchorUpdated(ARAnchor anchor, ARNode node) {

            }

            @Override
            public void onAnchorRemoved(ARAnchor anchor, ARNode node) {

            }
        });
    }

    @Test
    public void testARScene() {
            testARInitialized();
            testARAmbientLightValues();
            testPointCloudUpdateCallback();
            testDisplayPointCloudOn();
            testPointCloudScale();
            testPointCloudSurface();
            setPointCloudMaxPoints();
            testDisplayPointCloudOff();

    }

    private void testARInitialized() {
        assertPass("AR init text appears saying callback received.");
    }

    private void testARAmbientLightValues() {
        mAmbientLightUpdateTestStarted = true;
        assertPass("AR text udpates to show new ambient light values, ambient light changes to reflect new changes.");
    }

    private void testPointCloudScale() {
        final List<Float> scaleSizes= Arrays.asList(0.1f, 0.2f, 0.3f, 0.4f, 0.5f);
        final Iterator<Float> itr = Iterables.cycle(scaleSizes).iterator();
       mMutableTestMethod = ()->{
            Float scaleNum = itr.next();
            mARScene.setPointCloudSurfaceScale(new Vector(scaleNum, scaleNum, scaleNum));
       };
        assertPass("Point cloud scale should change over time.", () -> {
            mARScene.resetPointCloudSurface();
        });
    }

    private void testDisplayPointCloudOn() {
        mARScene.displayPointCloud(true);
        assertPass("Display point cloud ");
    }

    private void testPointCloudUpdateCallback() {

        final Text pointCloudText = new Text(mViroView.getViroContext(),
                "Waiting for cloud updates.", "Roboto", 12, Text.FontStyle.Normal, Text.FontWeight.Regular,
                Color.WHITE, 4f, 4f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.CLIP_TO_BOUNDS, 0);

        final Node textNode = new Node();
        textNode.setPosition(new Vector(0, 1f, -4f));
        textNode.setGeometry(pointCloudText);
        mScene.getRootNode().addChildNode(textNode);
        mARScene.setPointCloudUpdateListener(new PointCloudUpdateListener() {
            @Override
            public void onUpdate(ARPointCloud pointCloud) {
                Log.i("ViroARSceneTest", "Point cloud values: " + pointCloud.size());
                float []pointCloudArray = pointCloud.getPoints();
                String pointCloudStr = "Point clouds: ";
                for(int i =0; i< pointCloud.size(); i++) {
                    float x= pointCloudArray[i*4+0];
                    float y = pointCloudArray[i*4 + 1];
                    float z = pointCloudArray[i*4 + 2];
                    pointCloudStr += "(" + x + "," + y + "," + z  + "), ";
                    Log.i("ViroARSceneTest", "point i" + i + "(x,y,z)->" + "(" + x + "," + y + "," + z  + ")");
                }

                pointCloudText.setText(pointCloudStr);
            }
        });

        assertPass("Point cloud callback should update with new values", () -> {
            mARScene.setPointCloudUpdateListener(null);
            textNode.removeFromParentNode();
        });
    }

    private void testPointCloudSurface() {
        Surface surfaceOne = new Surface(.1f, .1f);
        Surface surfaceTwo = new Surface(.1f, .1f);

        Material material = new Material();
        material.setLightingModel(Material.LightingModel.BLINN);
        material.setDiffuseColor(Color.BLUE);
        surfaceOne.setMaterials(Arrays.asList(material));

        Material materialTwo = new Material();
        materialTwo.setLightingModel(Material.LightingModel.BLINN);
        materialTwo.setDiffuseColor(Color.RED);
        surfaceTwo.setMaterials(Arrays.asList(materialTwo));

        final List<Surface> surfaces = Arrays.asList(surfaceOne, surfaceTwo);
        final Iterator<Surface> itr = Iterables.cycle(surfaces).iterator();
        mMutableTestMethod = ()->{
            Surface surface = itr.next();
            mARScene.setPointCloudSurface(surface);
        };
        assertPass("Point cloud surfaces should loop change from red to blue.", () -> {
            mARScene.resetPointCloudSurface();
        });
    }

    private void setPointCloudMaxPoints() {
        final List<Integer> maxPoints = Arrays.asList(1, 5, 200);
        final Iterator<Integer> itr = Iterables.cycle(maxPoints).iterator();

        mMutableTestMethod = ()->{
            mARScene.setPointCloudMaxPoints(itr.next());
        };
        assertPass("Max cloud points loops from 1, 5 to 200", () -> {
            mARScene.resetPointCloudSurface();
        });
    }

    private void testDisplayPointCloudOff() {
        mARScene.displayPointCloud(false);
        assertPass("Display point cloud is OFF.");
    }
}
