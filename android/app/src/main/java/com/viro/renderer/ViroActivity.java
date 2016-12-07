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

import com.viro.renderer.jni.BoxJni;
import com.viro.renderer.jni.ImageJni;
import com.viro.renderer.jni.MaterialJni;
import com.viro.renderer.jni.NodeJni;
import com.viro.renderer.jni.SceneJni;
import com.viro.renderer.jni.SphereJni;
import com.viro.renderer.jni.SurfaceJni;
import com.viro.renderer.jni.TextureJni;
import com.viro.renderer.jni.VideoTextureJni;
import com.viro.renderer.jni.ViroGvrLayout;

import java.util.Arrays;

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
        NodeJni videoSurfaceNode = new NodeJni(this);
        //testSurfaceVideo(node);
        //testSphereVideo(node);
        //testBackgroundVideo(scene);
        //testBox(node);

        float[] position = {0F,0F,-5F};
        videoSurfaceNode.setPosition(position);

        // Create a new material with a diffuseTexture set to the image "boba.png"
        ImageJni bobaImage = new ImageJni("boba.png");

        TextureJni bobaTexture = new TextureJni(bobaImage);
        MaterialJni material = new MaterialJni();
        material.setTexture(bobaTexture, "diffuseTexture");

        // Creation of ViroBox
        BoxJni boxGeometry = new BoxJni(2,4,2);

        NodeJni boxNode = new NodeJni(this);
        boxNode.setGeometry(boxGeometry);
        float[] boxPosition = {5,0,0};
        boxNode.setPosition(boxPosition);
        boxNode.setMaterials(Arrays.asList(material));

        // add Video and Box to scene
        rootNode.addChildNode(videoSurfaceNode);
        rootNode.addChildNode(boxNode);

        // Updating the scene.
        mViroGvrLayout.setScene(scene);
    }

    private void testSurfaceVideo(NodeJni node){
        SurfaceJni surface = new SurfaceJni(40,40);
        VideoTextureJni videoTexture = new VideoTextureJni();
        videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mViroGvrLayout.getRenderContextRef());
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(false);
        videoTexture.play();
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoFinish() {
                Log.e(TAG,"onVideoFinished for Surface within ViroActivity");
            }
        });
        surface.setVideoTexture(videoTexture);
        node.setGeometry(surface);
    }

    private void testSphereVideo(NodeJni node){
        SphereJni sphere = new SphereJni(2, 20, 20, false);
        VideoTextureJni videoTexture = new VideoTextureJni();
        videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mViroGvrLayout.getRenderContextRef());
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(false);
        videoTexture.play();
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoFinish() {
                Log.e(TAG,"onVideoFinished for Sphere within ViroActivity");
            }
        });
        sphere.setVideoTexture(videoTexture);
        node.setGeometry(sphere);
    }

    private void testBackgroundVideo(SceneJni scene){
        VideoTextureJni videoTexture = new VideoTextureJni();
        videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mViroGvrLayout.getRenderContextRef());
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(false);
        videoTexture.play();
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoFinish() {
                Log.e(TAG,"onVideoFinished for Background within ViroActivity");
            }
        });
        scene.setBackgroundVideoTexture(videoTexture);
    }

    private void testBox(NodeJni node){
        //Creation of ViroBox
        BoxJni boxGeometry = new BoxJni(2,4,2);
        node.setGeometry(boxGeometry);
    }
}
