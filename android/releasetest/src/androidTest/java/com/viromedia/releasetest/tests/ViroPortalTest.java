//
//  Copyright (c) 2017-present, ViroMedia, Inc.
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

import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.AmbientLight;
import com.viro.core.Camera;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Portal;
import com.viro.core.PortalScene;
import com.viro.core.Renderer;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.VideoTexture;
import com.viromedia.releasetest.ViroReleaseTestActivity;

import org.junit.Test;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class ViroPortalTest extends ViroBaseTest  {

    private PortalScene mPortalScene;
    private Portal mPortal;
    private Camera mCamera;

    @Override
    void configureTestScene() {
        AmbientLight light = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(light);
        mPortal = new Portal();
        mPortal.setIgnoreEventHandling(true);
        mPortalScene = new PortalScene();
        mPortalScene.setPosition(new Vector(0, 0, -6));
        mScene.getRootNode().addChildNode(mPortalScene);

        Object3D object3DArchway = new Object3D();
        object3DArchway.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/portal_archway.vrx"), Object3D.Type.FBX, null);
        mPortal.addChildNode(object3DArchway);
        mPortalScene.setPortalEntrance(mPortal);

        ViroReleaseTestActivity activity = (ViroReleaseTestActivity)mActivity;
        Renderer renderer = activity.getViroView().getRenderer();
        renderer.setPointOfView(mScene.getRootNode());
        mMutableTestMethod = null;
        if (mCamera != null) {
            mCamera.setPosition(new Vector(0, 0, -4));
        }
    }

    @Test
    public void testPortal() {
        runUITest(() -> testAddPortalEntrance());
        runUITest(() -> testPortalSceneBackgroundTexture());
        runUITest(() -> testPortalSceneBackgroundVideoTexture());
        runUITest(() -> testPortalScenesBackgroundRotation());
        runUITest(() -> testPortalSceneBackgroundColor());
        runUITest(() -> testPortalSceneBackgroundCube());
        runUITest(() -> testPortalSceneIsPassableOn());
        runUITest(() -> testPortalSceneDelegate());
        runUITest(() -> testPortalSceneIsPassableOff());
    }

    private void testAddPortalEntrance() {
        Object3D object3DArchway = new Object3D();
        object3DArchway.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/portal_archway.vrx"), Object3D.Type.FBX, null);

        Object3D object3DShip = new Object3D();
        object3DShip.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/portal_ship.vrx"), Object3D.Type.FBX, null);

        mPortal.addChildNode(object3DArchway);
        mPortalScene.setPortalEntrance(mPortal);
        mMutableTestMethod = () -> {
            if(mPortal.getChildNodes().get(0) == object3DShip) {
                mPortal.removeAllChildNodes();
                mPortal.addChildNode(object3DArchway);
                object3DArchway.setIgnoreEventHandling(true);
            } else {
                mPortal.removeAllChildNodes();
                mPortal.addChildNode(object3DShip);
                object3DShip.setIgnoreEventHandling(true);
            }
        };

        assertPass("The portal entrance should switch between door archway and ship");
    }

    private void testPortalSceneBackgroundTexture() {

        Bitmap background = this.getBitmapFromAssets(mActivity, "360_westlake.jpg");
        final Texture backgroundTexture = new Texture(background, Texture.Format.RGBA8, true, true);
        mPortalScene.setBackgroundTexture(backgroundTexture);
        assertPass("The portal background should display a texture of westlake.");
    }

    private void testPortalSceneBackgroundVideoTexture() {

        final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(),
                Uri.parse("https://s3.amazonaws.com/viro.video/Climber2Top.mp4"));
        mPortalScene.setBackgroundTexture(videoTexture);
        videoTexture.setLoop(true);
        videoTexture.play();

        assertPass("The portal background should display a video of a climber.");
    }

    private void testPortalScenesBackgroundRotation() {
        Bitmap background = this.getBitmapFromAssets(mActivity, "360_westlake.jpg");
        final Texture backgroundTexture = new Texture(background, Texture.Format.RGBA8, true, true);
        mPortalScene.setBackgroundTexture(backgroundTexture);

        final List<Float> rotations = Arrays.asList(0f, 45f, 90f, 135f, 180f, 225f, 270f);
        final Iterator<Float> itr = Iterables.cycle(rotations).iterator();
        mMutableTestMethod = () -> {
            mPortalScene.setBackgroundRotation(new Vector(0, 0, itr.next()));
        };
        assertPass("The portal background should rotate.");
    }

    private void testPortalSceneBackgroundColor() {
        final List<Integer> bgColors = Arrays.asList(Color.WHITE, Color.YELLOW, Color.BLUE, Color.RED);
        final Iterator<Integer> itr = Iterables.cycle(bgColors).iterator();
        mMutableTestMethod = () -> {
            mPortalScene.setBackgroundCubeWithColor(itr.next());
        };
        assertPass("The portal background should rotate thru colors.");
    }

    private void testPortalSceneBackgroundCube() {
        mMutableTestMethod = null;
        final Bitmap px = this.getBitmapFromAssets(mActivity,"px.png");
        final Bitmap nx = this.getBitmapFromAssets(mActivity, "nx.png");
        final Bitmap py = this.getBitmapFromAssets(mActivity,"py.png");
        final Bitmap ny = this.getBitmapFromAssets(mActivity, "ny.png");
        final Bitmap pz = this.getBitmapFromAssets(mActivity, "pz.png");
        final Bitmap nz = this.getBitmapFromAssets(mActivity, "nz.png");

        final Texture cubeTexture = new Texture(px, nx, py, ny,
                pz, nz, Texture.Format.RGBA8);

        mPortalScene.setBackgroundCubeTexture(cubeTexture);

        assertPass("The portal background should be a cube map.");
    }

    private void testPortalSceneIsPassableOn() {
        mCamera = new Camera();
        mScene.getRootNode().setCamera(mCamera);
        mPortalScene.setPassable(true);
        mCamera.setPosition(new Vector(0, 0, -4));
        Bitmap background = this.getBitmapFromAssets(mActivity, "360_westlake.jpg");
        final Texture backgroundTexture = new Texture(background, Texture.Format.RGBA8, true, true);
        mPortalScene.setBackgroundTexture(backgroundTexture);

        final List<Float> rotations = Arrays.asList(-4.8f, -5.2f,  -5.9f, -6.1f, -6.8f);
        final Iterator<Float> itr = Iterables.cycle(rotations).iterator();
        mMutableTestMethod = () -> {
            mCamera.setPosition(new Vector(0, 0, itr.next()));
        };

        assertPass("The user should traverse inside the portal");
    }

    private void testPortalSceneDelegate() {
        mPortalScene.setPassable(true);
        Bitmap background = this.getBitmapFromAssets(mActivity, "360_westlake.jpg");
        final Texture backgroundTexture = new Texture(background, Texture.Format.RGBA8, true, true);
        mPortalScene.setBackgroundTexture(backgroundTexture);

        mScene.getRootNode().setCamera(mCamera);
        final Text text = new Text(mViroView.getViroContext(), getClass().getSimpleName(),
                "Roboto", 25,
                Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);

        final Node textNode = new Node();
        textNode.setPosition(new Vector(0, -1, -2f));
        textNode.setGeometry(text);
        text.setText("Portal Delegate:Haven't entered portal.");
        mScene.getRootNode().addChildNode(textNode);

        mPortalScene.setPassable(true);
        mPortalScene.setEntryListener(new PortalScene.EntryListener() {
            @Override
            public void onPortalEnter(PortalScene portalScene) {
                text.setText("Entered portal!");
            }

            @Override
            public void onPortalExit(PortalScene portalScene) {
                text.setText("Exited portal!!");
            }
        });

        mCamera.setPosition(new Vector(0, 0, -4));
        final List<Float> rotations = Arrays.asList(-4.8f, -5.2f,  -5.9f, -6.1f, -6.8f);
        final Iterator<Float> itr = Iterables.cycle(rotations).iterator();
        mMutableTestMethod = () -> {
            mCamera.setPosition(new Vector(0, 0, itr.next()));
        };

        assertPass("'Portal Delegate' text should change when entering/exiting the portal");
    }


    private void testPortalSceneIsPassableOff() {
        Bitmap background = this.getBitmapFromAssets(mActivity, "360_westlake.jpg");
        final Texture backgroundTexture = new Texture(background, Texture.Format.RGBA8, true, true);
        mPortalScene.setBackgroundTexture(backgroundTexture);

        mPortalScene.setPassable(false);
        mCamera = new Camera();
        mScene.getRootNode().setCamera(mCamera);
        mCamera.setPosition(new Vector(0, 0, -4));
        final List<Float> rotations = Arrays.asList(-4.8f, -5.2f,  -5.9f, -6.1f, -6.8f);
        final Iterator<Float> itr = Iterables.cycle(rotations).iterator();
        mMutableTestMethod = () -> {
            mCamera.setPosition(new Vector(0, 0, itr.next()));
        };

        assertPass("The camera should traverse pass the portal, not entering it.");
    }

}
