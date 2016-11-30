/**
 * Copyright (c) 2015-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
package com.viro.renderer;

import android.content.Context;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.viro.renderer.jni.BoxJni;
import com.viro.renderer.jni.NodeJni;
import com.viro.renderer.jni.SceneJni;
import com.viro.renderer.jni.ViroGvrLayout;

public class ViroActivity extends AppCompatActivity {
    private ViroGvrLayout mViroGvrLayout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mViroGvrLayout = new ViroGvrLayout(this, true);
        setContentView(mViroGvrLayout);

    }

    @Override
    protected void onStart(){
        super.onStart();

        // Creation of SceneJni within scene navigator
        NodeJni rootNode = new NodeJni(this);
        SceneJni scene = new SceneJni(rootNode);

        // Creation of ViroBox
        BoxJni boxGeometry = new BoxJni(2,4,2);
        NodeJni boxNode = new NodeJni(this);
        boxNode.setGeometry(boxGeometry);
        boxNode.setPosition(0,0,-5);

        // Assignment of ViroBox to scene
        rootNode.addChildNode(boxNode);

        // Updating the scene.
        mViroGvrLayout.setScene(scene);
    }
}
