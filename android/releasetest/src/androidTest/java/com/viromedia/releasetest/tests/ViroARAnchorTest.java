package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.graphics.Point;
import android.net.Uri;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import com.viro.core.ARAnchor;
import com.viro.core.ARHitTestListener;
import com.viro.core.ARHitTestResult;
import com.viro.core.ARNode;
import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.Animation;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Quaternion;
import com.viro.core.Sphere;
import com.viro.core.Vector;
import com.viro.core.ViroViewARCore;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Created by radvani on 5/22/18.
 */

public class ViroARAnchorTest extends ViroBaseTest {

    private ViroViewARCore mViewARCore;

    @Override
    void configureTestScene() {
        AmbientLight ambient = new AmbientLight(Color.WHITE, 500.0f);
        mScene.getRootNode().addLight(ambient);

        DirectionalLight light = new DirectionalLight(Color.WHITE, 500.0f, new Vector(0, -1, 0));
        mScene.getRootNode().addLight(light);

        mViewARCore = (ViroViewARCore)mViroView;

        ((ARScene)mScene).setListener(new ARScene.Listener() {
            @Override
            public void onTrackingInitialized() {

            }

            @Override
            public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {

            }

            @Override
            public void onAmbientLightUpdate(float intensity, Vector color) {

            }

            @Override
            public void onAnchorFound(ARAnchor anchor, ARNode arNode) {
                //Log.i("Viro", "Anchor found with node " + arNode.getNativeRef());
            }

            @Override
            public void onAnchorUpdated(ARAnchor anchor, ARNode arNode) {
                //Log.i("Viro", "Anchor updated with node " + arNode.getNativeRef());
            }

            @Override
            public void onAnchorRemoved(ARAnchor anchor, ARNode arNode) {
                //Log.i("Viro", "Anchor removed with node " + arNode.getNativeRef());
            }
        });
    }

    @Test
    public void testARHitTest() {
        runUITest(() -> testHitResultAnchors());
        runUITest(() -> testAddArbitraryAnchors());
        //runUITest(() -> testPerformARHitTestWithPosition());
        //runUITest(() -> testPerformARHitTest());
    }

    private void testHitResultAnchors() {
        final ViroViewARCore view = (ViroViewARCore) mViroView;
        Point point = new Point((int) (view.getWidth() / 2.0), (int) (view.getHeight() / 2.0f));

        final int[] count = new int[1];
        count[0] = 0;

        final List<ARNode> anchoredNodes = new ArrayList<>();

        mMutableTestMethod = () -> {
            if (count[0] == 20) {
                for (ARNode node : anchoredNodes) {
                    node.detach();
                }
                anchoredNodes.clear();
                return;
            }

            view.performARHitTest(point,
                    new ARHitTestListener() {
                        @Override
                        public void onHitTestFinished(ARHitTestResult[] results) {
                            for (ARHitTestResult result : results) {
                                if (result.getType() == ARHitTestResult.Type.FEATURE_POINT) {
                                    ARNode node = result.createAnchoredNode();
                                    if (node != null) {
                                        node.addChildNode(loadObjectNode(1, .1f, new Vector(0, 0, 0), Color.WHITE));
                                        anchoredNodes.add(node);
                                    }

                                } else if (result.getType() == ARHitTestResult.Type.PLANE) {
                                    ARNode node = result.createAnchoredNode();
                                    if (node != null) {
                                        node.addChildNode(loadObjectNode(1, .1f, new Vector(0, 0, 0), Color.RED));
                                        anchoredNodes.add(node);
                                    }
                                }
                            }
                        }
                    });
            count[0]++;
        };

        assertPass("20 hit tests are performed: a yellow star is created for point intersections, a reddish star for plane intersections. After the tests, all the stars are detached.");
    }

    private void testAddArbitraryAnchors() {
        final ViroViewARCore view = (ViroViewARCore) mViroView;
        final ARScene scene = (ARScene) mScene;
        Point point = new Point((int) (view.getWidth() / 2.0), (int) (view.getHeight() / 2.0f));

        final int[] count = new int[1];
        count[0] = 0;

        final Vector anchorPosition = new Vector(0, -0.5, -0.5);
        final Vector anchorRotation = new Vector(0, 0, 0);
        final List<ARNode> anchoredNodes = new ArrayList<>();

        mMutableTestMethod = () -> {
            if (count[0] == 20) {
                for (ARNode node : anchoredNodes) {
                    node.detach();
                }
                anchoredNodes.clear();
                return;
            }

            ARNode node = scene.createAnchoredNode(anchorPosition, new Quaternion(anchorRotation));
            if (node != null) {
                node.addChildNode(loadObjectNode(1, .075f, new Vector(0, 0, 0), Color.WHITE));
                anchoredNodes.add(node);

                anchorPosition.y += 0.10f;
                anchorPosition.z -= 0.10f;
                anchorRotation.y += (Math.toRadians(10));
                count[0]++;
            }
        };

        assertPass("1 anchor is created each second with a star. Each successive anchor is rotated more about the Y axis, and pushed further up and further away. All detach after 20 added.");
    }

    private Object3D loadObjectNode(final int bitmask, final float scale, final Vector position, final int color) {
        final Object3D objectNode = new Object3D();
        objectNode.setScale(new Vector(scale, scale, scale));

        objectNode.loadModel(mViroView.getViroContext(),
                Uri.parse("file:///android_asset/object_star_anim.vrx"),
                Object3D.Type.FBX, new AsyncObject3DListener() {
                    @Override
                    public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                        object.setPosition(position);
                        object.setScale(new Vector(scale, scale, scale));
                        object.setLightReceivingBitMask(bitmask);
                        object.setShadowCastingBitMask(bitmask);
                        object.getMaterials().get(0).setDiffuseColor(color);
                    }

                    @Override
                    public void onObject3DFailed(final String error) {

                    }
                });
        return objectNode;
    }
}
