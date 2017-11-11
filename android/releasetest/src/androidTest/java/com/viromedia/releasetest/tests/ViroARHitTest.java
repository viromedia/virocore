package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.graphics.Point;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import com.viro.core.ARHitTestListener;
import com.viro.core.ARHitTestResult;
import com.viro.core.AmbientLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Sphere;
import com.viro.core.Vector;
import com.viro.core.ViroViewARCore;

import org.junit.Test;

import java.util.Arrays;

/**
 * Created by vadvani on 11/8/17.
 */

public class ViroARHitTest extends ViroBaseTest {

    private ViroViewARCore mViewARCore;

    @Override
    void configureTestScene() {
        AmbientLight light = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(light);
        mViewARCore = (ViroViewARCore)mViroView;


    }

    @Test
    public void testARHitTest() {
        testPerformARHitTestWithRay();
        testPerformARHitTestWithPosition();
        testPerformARHitTest();
    }

    private void testPerformARHitTestWithRay() {
        mMutableTestMethod = () -> {
            Vector forward = mViroView.getLastCameraForwardRealtime();
            Vector fowardWithDistance = new Vector(forward.x * 2.f, forward.y * 2.f, forward.z * 2.f);

            mViewARCore.performARHitTestWithRay(fowardWithDistance, new ARHitTestListener() {
                public void onHitTestFinished(ARHitTestResult[] results) {
                    for (ARHitTestResult result : results) {
                        Sphere sphere = new Sphere(.1f);
                        Material material = new Material();
                        material.setLightingModel(Material.LightingModel.BLINN);
                        material.setDiffuseColor(Color.BLUE);
                        sphere.setMaterials(Arrays.asList(material));
                        Node nodeSphere = new Node();
                        nodeSphere.setPosition(result.getPosition());
                        nodeSphere.setGeometry(sphere);
                        mScene.getRootNode().addChildNode(nodeSphere);
                    }
                }
            });
        };

        assertPass("Should see rendered results from AR HIT with ray (blue balls).");
    }

    private void testPerformARHitTestWithPosition() {
        mViewARCore.performARHitTestWithPosition(new Vector(0, 0, -3), new ARHitTestListener() {
            @Override
            public void onHitTestFinished(ARHitTestResult[] results) {
                for(ARHitTestResult result: results) {
                    Sphere sphere = new Sphere(.1f);
                    Material material = new Material();
                    material.setLightingModel(Material.LightingModel.BLINN);
                    material.setDiffuseColor(Color.BLUE);
                    sphere.setMaterials(Arrays.asList(material));
                    Node nodeSphere = new Node();
                    nodeSphere.setPosition(result.getPosition());
                    nodeSphere.setGeometry(sphere);
                    mScene.getRootNode().addChildNode(nodeSphere);
                }
            }
        });

        assertPass("Should see rendered results from AR HIT with position.");
    }

    private void testPerformARHitTest() {
        mViewARCore.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN) {
                    event.getX();
                    event.getY();
                    mViewARCore.performARHitTest(new Point((int)event.getX(), (int)event.getY()), new ARHitTestListener() {
                        @Override
                        public void onHitTestFinished(ARHitTestResult[] results) {
                            for(ARHitTestResult result: results) {
                                Sphere sphere = new Sphere(.1f);
                                Material material = new Material();
                                material.setLightingModel(Material.LightingModel.BLINN);
                                material.setDiffuseColor(Color.BLUE);
                                sphere.setMaterials(Arrays.asList(material));
                                Node nodeSphere = new Node();
                                nodeSphere.setPosition(result.getPosition());
                                nodeSphere.setGeometry(sphere);
                                mScene.getRootNode().addChildNode(nodeSphere);
                            }
                        }
                    });
                }
                return false;
            }
        });

        assertPass("Should see rendered results from touching screen.");

    }

}
