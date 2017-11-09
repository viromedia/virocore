package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.net.Uri;

import com.viro.core.AmbientLight;
import com.viro.core.Animation;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Text;
import com.viro.core.Vector;


import org.junit.Test;

/**
 * Created by vadvani on 11/2/17.
 */

public class Viro3DObjectTest extends ViroBaseTest {
    private Object3D mObject3D;

    @Override
    void configureTestScene() {
        // Creation of ObjectJni to the right
        mObject3D = new Object3D();
        mObject3D.setPosition(new Vector(0, 0, -5));
        AmbientLight ambientLight = new AmbientLight(Color.WHITE, 1000f);
        mScene.getRootNode().addLight(ambientLight);
        mScene.getRootNode().addChildNode(mObject3D);
    }

    @Test
    public void test3DObject() {
        testLoadModelFBX();
        testLoadModelOBJ();
        testLoadModelError();
    }

    private void testLoadModelFBX() {
        mObject3D.loadModel(Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -3));
                object.setScale(new Vector(0.4f, 0.4f, 0.4f));

                Animation animation = object.getAnimation("02_spin");
                animation.setDelay(5000);
                animation.setLoop(true);
                animation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        assertPass("Star model loads and begins to animate.");
    }

    private void testLoadModelOBJ() {
        mObject3D.setPosition(new Vector(0, 0, -20));
        mObject3D.setScale(new Vector(0.2f, 0.2f, 0.2f));
        mObject3D.loadModel((Uri.parse("file:///android_asset/male02.obj")), Object3D.Type.OBJ,  new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {

            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        assertPass("Tom Cruise look alike model loads and displays.");
    }

    private void testLoadModelError() {
        mObject3D.loadModel((Uri.parse("file:///android_asset/momentslogo.pong")), Object3D.Type.OBJ,  new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {

            }

            @Override
            public void onObject3DFailed(final String error) {
                Node node = new Node();
               final Text text = new Text(mViroView.getViroContext(), "Object failed to load as it should!",
                        "Roboto", 25, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                        Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);

                final float[] classNamePosition = {0, -.5f, -3.3f};
                node.setPosition(new Vector(classNamePosition));
                node.setGeometry(text);
                mScene.getRootNode().addChildNode(node);

            }
        });
        assertPass("Text should display saying object failed to load.");
    }
}
