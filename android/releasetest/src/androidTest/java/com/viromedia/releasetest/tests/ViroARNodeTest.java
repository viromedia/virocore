package com.viromedia.releasetest.tests;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Log;

import com.viro.core.ARAnchor;
import com.viro.core.ARNode;
import com.viro.core.ARPlaneAnchor;
import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.Box;
import com.viro.core.Material;
import com.viro.core.Node;

import com.viro.core.Surface;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Vector;

import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * Created by vadvani on 11/7/17.
 * This includes tests for all methods for ARScene except the onAnchor callbacks. Those are tested in ViroARNodeTest.
 */

public class ViroARNodeTest extends ViroBaseTest {

    private ARScene mARScene;
    private Node mBoxNode;
    private AmbientLight mAmbientLight;
    private Text mTestText;

    private boolean mAnchorFoundTestStarted = false;
    private boolean mAnchorUpdatedTestStarted = false;
    private boolean mNodePauseUpdatesTestStarted = false;

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

        mTestText = new Text(mViroView.getViroContext(),
                "No input yet", "Roboto", 12,
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
            public void onAmbientLightUpdate(float lightIntensity, Vector color) {

            }

            @Override
            public void onAnchorFound(ARAnchor anchor, ARNode arNode) {
                Log.i("Viro", "Anchor found for node " + arNode.getNativeRef());
                if (mAnchorFoundTestStarted) {
                    Text text  = new Text(mViroView.getViroContext(),
                            "Text attached to ARNODE!", "Roboto", 12,
                            Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                            Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
                    Node textNode = new Node();
                    textNode.setGeometry(text);
                    textNode.setPosition(new Vector(0f, .2f, 0f));

                    Material material = new Material();
                    material.setDiffuseColor(Color.RED);
                    material.setLightingModel(Material.LightingModel.BLINN);

                    if (anchor instanceof ARPlaneAnchor) {
                        ARPlaneAnchor arPlaneAnchor  = (ARPlaneAnchor)anchor;
                        Surface surface = new Surface(arPlaneAnchor.getExtent().x, arPlaneAnchor.getExtent().z);
                        surface.setMaterials(Arrays.asList(material));

                        Node surfaceNode = new Node();
                        surfaceNode.setGeometry(surface);
                        surfaceNode.setPosition(new Vector(0, 0, 0));
                        surfaceNode.setRotation(new Vector(-Math.toRadians(90.0), 0, 0));
                        arNode.addChildNode(surfaceNode);
                    }

                    arNode.addChildNode(textNode);
                }
            }

            @Override
            public void onAnchorUpdated(ARAnchor anchor, ARNode arNode) {
                Log.i("Viro", "Updating anchor for node " + arNode.getNativeRef());

                if (mAnchorUpdatedTestStarted) {
                    Log.i("Viro", "Inside If block");
                    List<Node> childNodes = arNode.getChildNodes();
                    Log.i("Viro", "Number of child nodes " + arNode.getChildNodes().size());

                    for (Node childNode : childNodes) {
                        if (childNode.getGeometry() instanceof Text) {
                            Log.i("Viro", "Foudn text node");
                            Text text = (Text) childNode.getGeometry();
                            String anchorString = "";

                            anchorString = "Anchor Type:" + anchor.getType().toString() + ", pos:" + anchor.getPosition().toString() + ", rotation: " + anchor.getRotation().toString() + "scale: " + anchor.getScale().toString();
                            if (anchor instanceof ARPlaneAnchor) {
                                ARPlaneAnchor anchorPlane = (ARPlaneAnchor) anchor;
                                anchorString += ("Center:" + anchorPlane.getCenter().toString() + ", " + "Extent: " + anchorPlane.getExtent().toString() + ", Alignment: " + anchorPlane.getAlignment().toString());
                            }
                            text.setText(anchorString);
                        }

                        if (childNode.getGeometry() instanceof Surface) {
                            Log.i("Viro", "Found surface");

                            ARPlaneAnchor planeAnchor = (ARPlaneAnchor)anchor;
                            Surface surface = (Surface)childNode.getGeometry();
                            surface.setWidth(planeAnchor.getExtent().x);
                            surface.setHeight(planeAnchor.getExtent().z);
                            childNode.setScale(planeAnchor.getScale());
                        }
                    }
                }

                if(mNodePauseUpdatesTestStarted) {
                    arNode.setPauseUpdates(true);
                }
            }

            @Override
            public void onAnchorRemoved(ARAnchor anchor, ARNode arNode) {
               mTestText.setText("An anchor has been removed!");
            }

            @Override
            public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {

            }
        });
    }

    @Override
    void resetTestState() {
        /*
         This test must be run in order, because it relies on an anchor being found, then updates
         the anchor, etc.
         */
        mAnchorFoundTestStarted = false;
        mAnchorUpdatedTestStarted = false;
        mNodePauseUpdatesTestStarted = false;
    }

    @Test
    public void testARScene() {
        runUITest(() -> testOnAnchorFound());
        runUITest(() -> testOnAnchorUpdated());
        runUITest(() -> testPauseUpdates());
        runUITest(() -> testOnAnchorRemoved());
    }


    private void testOnAnchorFound() {
        mAnchorFoundTestStarted = true;
        assertPass("Red planes are created where planar surfaces are found, with anchored text");
    }

    private void testOnAnchorUpdated() {
        mAnchorUpdatedTestStarted = true;
        assertPass("Anchors are updated with anchor information showing up in text");
    }

    private void testOnAnchorRemoved() {
        mAnchorUpdatedTestStarted = true;
        assertPass("Move around until anchor removed message appears where init message appeared. Plane should disappear.");
    }

    private void testPauseUpdates() {
        mNodePauseUpdatesTestStarted = true;
        assertPass("Anchors are paused, no updates should be occurring to the node attached to the anchor");
    }

}
