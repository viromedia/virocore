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

import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.viro.core.AmbientLight;
import com.viro.core.AndroidViewTexture;
import com.viro.core.ClickListener;
import com.viro.core.ClickState;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Quad;
import com.viro.core.Vector;
import com.viromedia.releasetest.R;
import org.junit.Test;

import java.io.IOException;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.concurrent.atomic.AtomicBoolean;

public class ViroAndroidViewTest extends ViroBaseTest {
    private Quad mSurface;
    private Node mNode;
    private AndroidViewTexture mRenderTexture;
    private View mAttachedView;
    private AtomicBoolean mForceHardwareAccelerated = new AtomicBoolean(false);

    @Override
    void configureTestScene() {
        // Setup the scene's lights
        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        mScene.getRootNode().addLight(ambientLightJni);

        boolean isAccelerated = mForceHardwareAccelerated.get();

        int width = 1000;
        int height = 1000;
        float distance = -1.75f;
        if (isAccelerated) {
            width = 3000;
            height = 3000;
            distance = -1.75f;
        }

        // Construct our renderAndroidView texture to be tested.
        mAttachedView = createLayoutDynamically();
        mRenderTexture = new AndroidViewTexture(mViroView,width, height, isAccelerated);
        mRenderTexture.attachView(mAttachedView);

        // Set the texture on the material and then the quad
        final Material material = new Material();
        material.setDiffuseTexture(mRenderTexture);
        material.setLightingModel(Material.LightingModel.BLINN);

        mSurface = new Quad(1, 1);
        mNode = new Node();
        mNode.setGeometry(mSurface);
        mNode.setClickListener(mRenderTexture.getClickListenerWithQuad(mSurface));
        final float[] position = {0, 0, distance};
        mNode.setPosition(new Vector(position));
        mSurface.setMaterials(Arrays.asList(material));
        mScene.getRootNode().addChildNode(mNode);
    }

    private Node createMoreSurfaces(){
        Node n = new Node();
        View v = createLayoutDynamically();
        AndroidViewTexture a = new AndroidViewTexture(mViroView, 1000, 1000, false);
        a.attachView(v);
        Material m = new Material();
        m.setDiffuseTexture(a);

        Quad q = new Quad(1,1);
        q.setMaterials(Arrays.asList(m));
        n.setGeometry(q);

        n.setClickListener(a.getClickListenerWithQuad(q));
        return n;
    }

    private View createLayoutDynamically() {
        Context c = mViroView.getContext();
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT);
        LinearLayout frame = new LinearLayout(c);
        frame.setOrientation(LinearLayout.VERTICAL);
        frame.setLayoutParams(params);
        frame.setBackgroundColor(Color.parseColor("#96bfefff"));

        Button btn = new Button(c);
        btn.setText("Manual Add 1");
        btn.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        frame.addView(btn);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                v.setBackgroundColor(Color.parseColor("#00FF00"));
                btn.setText("You clicked me!");
            }
        });


        Button btn2 = new Button(c);
        btn2.setText("Manual Add 2");
        btn2.setLayoutParams(new LinearLayout.LayoutParams(500,500));
        frame.addView(btn2);
        btn2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                v.setBackgroundColor(Color.parseColor("#FF0000"));
                btn2.setText("You clicked me!");
            }
        });

        ImageView image = new ImageView(c);
        try {
            Drawable d = Drawable.createFromStream(c.getAssets().open("boba.png"), null);
            image.setBackground(d);
        } catch (IOException e) {
            e.printStackTrace();
        }
        frame.addView(image);
        return frame;
    }

    @Test
    public void surfaceTest() {
        runUITest(() -> verifyBasicLayout());
        runUITest(() -> verifyRotatedLayout());
        runUITest(() -> verifyDynamicAttachViews());

        // Make our tests more flexible by dynamically tweaking the hardware acceleration flag.
        mForceHardwareAccelerated.set(true);
        runUITest(() -> verifyMultipleSoftwareSurfaces());
        runUITest(() -> verifyHardwareAcceleratedView());
    }

    public void verifyBasicLayout() {
        assertPass("You should see 2 clickable buttons and an Android Boba Image " +
                "View on quad. Buttons should be clickable.");
    }

    public void verifyRotatedLayout() {
        mNode.setRotation(new Vector(0,0,Math.toRadians(120)));
        assertPass("You should the same layout, but rotated. Buttons should be " +
                "clickable, even when rotated.");
    }

    public void verifyDynamicAttachViews() {
        mNode.setClickListener(new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                if (mAttachedView == null) {
                    mAttachedView = createLayoutDynamically();
                    mRenderTexture.attachView(mAttachedView);
                } else {
                    mAttachedView = null;
                    mRenderTexture.detachView();
                }
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {
                // No-op
            }
        });
        assertPass("Clicking on quad should attach / detach android views " +
                "(you should see the view, then black).");
    }

    public void verifyMultipleSoftwareSurfaces() {
        Node n1 = createMoreSurfaces();
        Node n2 = createMoreSurfaces();
        Node n3 = createMoreSurfaces();

        n1.setPosition(new Vector(0,1.5,-1.75f));
        n2.setPosition(new Vector(-1.5,0,-1.75f));
        n2.setTransformBehaviors((EnumSet.of(Node.TransformBehavior.BILLBOARD)));
        n3.setPosition(new Vector(1.5,0,-1.75f));
        n3.setTransformBehaviors((EnumSet.of(Node.TransformBehavior.BILLBOARD)));
        assertPass("You should see multiple interactible surfaces - left, center, " +
                "right, top. Click all the buttons, they should all be clickable.");

        mScene.getRootNode().addChildNode(n1);
        mScene.getRootNode().addChildNode(n2);
        mScene.getRootNode().addChildNode(n3);
    }

    public void verifyHardwareAcceleratedView() {
        assertPass("Same view is seen but is squished smaller towards the top" +
                " due to more pixels on the screen. The second button is also much skinner " +
                "compared to the first. All buttons should still be clickable.");
    }
}
