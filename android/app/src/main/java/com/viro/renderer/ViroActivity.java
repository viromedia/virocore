/**
 * Copyright (c) 2015-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
package com.viro.renderer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import com.viro.renderer.jni.ImageJni;
import com.viro.renderer.jni.MaterialJni;
import com.viro.renderer.jni.NodeJni;
import com.viro.renderer.jni.SceneJni;
import com.viro.renderer.jni.TextureJni;
import com.viro.renderer.jni.VideoSurfaceJni;
import com.viro.renderer.jni.ViroGvrLayout;

public class ViroActivity extends AppCompatActivity {
    private ViroGvrLayout mViroGvrLayout;
    private static String TAG = ViroActivity.class.getSimpleName();

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
        NodeJni node = new NodeJni(this);

        // Testing video
        String url = "https://s3.amazonaws.com/viro.video/Climber2Top.mp4";
        VideoSurfaceJni video = new VideoSurfaceJni(40,40, url, mViroGvrLayout.getRenderContextRef());
        node.setGeometry(video);
        video.setVolume(0.1f);
        video.play();
        video.setLoop(false);
        video.setVideoDelegate(new VideoSurfaceJni.VideoDelegate() {
            @Override
            public void onVideoFinish() {
                Log.e(TAG,"onVideoFinished within ViroActivity");
            }
        });

        float[] position = {0F,0F,-5F};
        node.setPosition(position);

        // Creation of ViroBox
        // BoxJni boxGeometry = new BoxJni(2,4,2);
        // boxNode.setGeometry(boxGeometry);
        // float[] boxPosition = {0,0,-5};
        // node.setPosition(boxPosition);

        // Testing random material creation stuff, should have no effect for now.
        MaterialJni material = new MaterialJni();
        ImageJni image = new ImageJni("boba.png");

        TextureJni texture = new TextureJni(image);
        // TextureJni texture2 = new TextureJni(image, image, image,
        //        image, image, image);

        material.setTexture(texture, "diffuseTexture");
        material.setColor(0xff00ff, "diffuseColor");

        // Assignment of ViroBox to scene
        rootNode.addChildNode(node);

        // Updating the scene.
        mViroGvrLayout.setScene(scene);
    }
}
