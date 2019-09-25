//
//  Copyright (c) 2018-present, ViroMedia, Inc.
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
    private List<String> mHostedAnchors = new ArrayList<>();
    private List<ARNode> mAnchoredNodes = new ArrayList<>();

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

    @Override
    void resetTestState() {
        for (ARNode node : mAnchoredNodes) {
            node.detach();
        }
        mAnchoredNodes.clear();
        super.resetTestState();
    }

    @Test
    public void testARHitTest() {
        runUITest(() -> testHitResultAnchors());
        runUITest(() -> testAddArbitraryAnchors());
        runUITest(() -> testHostCloudAnchors());
        runUITest(() -> testResolveCloudAnchors());
    }

    private void testHitResultAnchors() {
        final ViroViewARCore view = (ViroViewARCore) mViroView;
        Point point = new Point((int) (view.getWidth() / 2.0), (int) (view.getHeight() / 2.0f));

        final int total = 15;
        final int[] count = new int[1];
        count[0] = 0;

        mMutableTestMethod = () -> {
            if (count[0] >= total) {
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
                                        mAnchoredNodes.add(node);
                                    }

                                } else if (result.getType() == ARHitTestResult.Type.PLANE) {
                                    ARNode node = result.createAnchoredNode();
                                    if (node != null) {
                                        node.addChildNode(loadObjectNode(1, .1f, new Vector(0, 0, 0), Color.RED));
                                        mAnchoredNodes.add(node);
                                    }
                                }
                            }
                        }
                    });
            count[0]++;
        };

        assertPass(total + " hit tests are performed: a yellow star is created for point intersections, a reddish star for plane intersections.");
    }

    private void testAddArbitraryAnchors() {
        final ARScene scene = (ARScene) mScene;

        final int total = 10;
        final int[] count = new int[1];
        count[0] = 0;

        final Vector anchorPosition = new Vector(0, -0.5, -0.5);
        final Vector anchorRotation = new Vector(0, 0, 0);

        mMutableTestMethod = () -> {
            if (count[0] >= total) {
                return;
            }

            ARNode node = scene.createAnchoredNode(anchorPosition, new Quaternion(anchorRotation));
            if (node != null) {
                node.addChildNode(loadObjectNode(1, .075f, new Vector(0, 0, 0), Color.WHITE));
                mAnchoredNodes.add(node);

                anchorPosition.y += 0.10f;
                anchorPosition.z -= 0.10f;
                anchorRotation.y += (Math.toRadians(10));
                count[0]++;
            }
        };

        assertPass("10 anchors are created, one per second, with a star. Each successive anchor is rotated more about the Y axis, and pushed further up and further away.");
    }

    private void testHostCloudAnchors() {
        final ViroViewARCore view = (ViroViewARCore) mViroView;
        final ARScene scene = (ARScene) mScene;

        Point point = new Point((int) (view.getWidth() / 2.0), (int) (view.getHeight() / 2.0f));
        mMutableTestMethod = () -> {
            if (mAnchoredNodes.size() >= 2) {
                return;
            }

            view.performARHitTest(point,
                    new ARHitTestListener() {
                        @Override
                        public void onHitTestFinished(ARHitTestResult[] results) {
                            Log.i("Viro", "Hit test received " + results.length + " results");
                            for (ARHitTestResult result : results) {
                                ARNode node = result.createAnchoredNode();
                                if (node == null) {
                                    continue;
                                }

                                final Object3D star = loadObjectNode(1, .1f, new Vector(0, 0, 0), Color.WHITE);
                                node.addChildNode(star);
                                mAnchoredNodes.add(node);

                                scene.hostCloudAnchor(node.getAnchor(), new ARScene.CloudAnchorHostListener() {
                                    @Override
                                    public void onSuccess(ARAnchor anchor, ARNode arNode) {
                                        star.getMaterials().get(0).setDiffuseColor(Color.RED);
                                        mHostedAnchors.add(anchor.getCloudAnchorId());
                                    }

                                    @Override
                                    public void onFailure(String error) {
                                        star.getMaterials().get(0).setDiffuseColor(Color.BLACK);
                                    }
                                });
                                // We only care about hosting the first result per hit-test
                                break;
                            }
                        }
                    });
        };
        assertPass("Hit tests are performed until two stars are created. The stars turn red when they are hosted, black if hosting fails.");
    }

    private void testResolveCloudAnchors() {
        if (mHostedAnchors.size() < 2) {
            assertPass("This test cannot be run unless testHostCloudAnchors succeeds");
            return;
        }

        final ARScene scene = (ARScene) mScene;
        for (int i = 0; i < mHostedAnchors.size(); i++) {
            String cloudAnchorId = mHostedAnchors.get(i);
            scene.resolveCloudAnchor(cloudAnchorId, new ARScene.CloudAnchorResolveListener() {
                @Override
                public void onSuccess(final ARAnchor anchor, final ARNode arNode) {
                    Log.i("Viro", "Resolve success");
                    arNode.addChildNode(loadObjectNode(1, 0.1f, new Vector(0, 0, 0), Color.WHITE));
                    mAnchoredNodes.add(arNode);
                }

                @Override
                public void onFailure(String error) {
                    Log.i("Viro", "Resolve failure: " + error);
                }
            });
        }
        assertPass("The anchors hosted in the last test are resolved: they appear as yellow stars");
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
