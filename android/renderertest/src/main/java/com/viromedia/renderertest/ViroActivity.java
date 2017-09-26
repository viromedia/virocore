/**
 * Copyright (c) 2015-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
package com.viromedia.renderertest;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;

import com.viro.renderer.jni.ARSceneControllerJni;
import com.viro.renderer.jni.AmbientLightJni;
import com.viro.renderer.jni.AsyncObjListener;
import com.viro.renderer.jni.BoxJni;
import com.viro.renderer.jni.ControllerJni;
import com.viro.renderer.jni.DirectionalLightJni;
import com.viro.renderer.jni.EventDelegateJni;
import com.viro.renderer.jni.GlListener;
import com.viro.renderer.jni.ImageJni;
import com.viro.renderer.jni.ImageTrackerJni;
import com.viro.renderer.jni.MaterialJni;
import com.viro.renderer.jni.NodeJni;
import com.viro.renderer.jni.ObjectJni;
import com.viro.renderer.jni.OmniLightJni;
import com.viro.renderer.jni.OpenCVJni;
import com.viro.renderer.jni.PolylineJni;
import com.viro.renderer.jni.RenderContextJni;
import com.viro.renderer.jni.SceneJni;
import com.viro.renderer.jni.SoundDataJni;
import com.viro.renderer.jni.SoundDelegate;
import com.viro.renderer.jni.SoundFieldJni;
import com.viro.renderer.jni.SoundJni;
import com.viro.renderer.jni.SpatialSoundJni;
import com.viro.renderer.jni.SphereJni;
import com.viro.renderer.jni.SpotLightJni;
import com.viro.renderer.jni.SurfaceJni;
import com.viro.renderer.jni.TextJni;
import com.viro.renderer.jni.TextureFormat;
import com.viro.renderer.jni.TextureJni;
import com.viro.renderer.jni.VideoTextureJni;
import com.viro.renderer.jni.ViroGvrLayout;
import com.viro.renderer.jni.ViroOvrView;
import com.viro.renderer.jni.VrView;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ViroActivity extends AppCompatActivity implements GlListener {
    private static int SOUND_COUNT = 0;
    private VrView mVrView;
    private final Map<String, SoundJni> mSoundMap = new HashMap<>();
    private final Map<String, SoundFieldJni> mSoundFieldMap = new HashMap();
    private final Map<String, SpatialSoundJni> mSpatialSoundMap = new HashMap<>();
    private static String TAG = ViroActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            mVrView = new ViroGvrLayout(this, this, new Runnable(){
                @Override
                public void run() {
                    Log.e(TAG, "On GVR userRequested exit");
                }
            });
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            mVrView = new ViroOvrView(this, this);
        }

        mVrView.setVrModeEnabled(true);
        mVrView.validateApiKey("7EEDCB99-2C3B-4681-AE17-17BC165BF792");
        setContentView(mVrView.getContentView());

        // uncomment the below line to test AR.
        //testEdgeDetect();
        testFindTarget();
    }

    @Override
    protected void onStart(){
        super.onStart();
        mVrView.onActivityStarted(this);
    }

    @Override
    protected void onResume(){
        super.onResume();
        mVrView.onActivityResumed(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        mVrView.onActivityPaused(this);
    }

    @Override
    protected void onStop(){
        super.onStop();
        mVrView.onActivityStopped(this);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mVrView.onActivityDestroyed(this);
    }

    @Override
    public void onGlInitialized() {
        Log.e("ViroActivity", "onGlInitialized called");
        initializeVrScene();
        //initializeArScene();
    }

    private void initializeVrScene() {
        // Creation of SceneJni within scene navigator
        NodeJni rootNode = new NodeJni();
        SceneJni scene = new SceneJni(rootNode);
        List<NodeJni> nodes = new ArrayList<>();
        //nodes = testSurfaceVideo(this);
        //nodes = testSphereVideo(this);
        //nodes = testBox(getApplicationContext());
        //nodes = test3dObjectLoading(getApplicationContext());

        //nodes = testImageSurface(this);
        //nodes = testText(this);

        //testBackgroundVideo(scene);
        //testBackgroundImage(scene);
        //testSkyBoxImage(scene);

        //nodes = testStereoSurfaceVideo(this);
        //nodes = testStereoImageSurface(this);
        nodes.add(testLine(this));
        //testStereoBackgroundVideo(scene);
        //testStereoBackgroundImage(scene);

        // addNormalSound("http://www.kozco.com/tech/32.mp3");
        // addNormalSound("http://www.bensound.com/royalty-free-music?download=dubstep");
        // addSoundField("http://ambisonics.dreamhosters.com/AMB/pink_pan_H.amb");
        // addSpatialSound("http://www.kozco.com/tech/32.mp3");

        final SoundDataJni data = new SoundDataJni("http://www.kozco.com/tech/32.mp3", false);
        //addSpatialSound(data);
        //addNormalSound(data);

        setSoundRoom(scene, mVrView.getRenderContextRef());

        for (NodeJni node: nodes) {
            rootNode.addChildNode(node);
        }
        testSceneLighting(rootNode);

        // Updating the scene.
        mVrView.setScene(scene);
        ControllerJni nativeController = new ControllerJni(mVrView.getRenderContextRef());
        //nativeController.setReticleVisibility(false);
    }

    /*
     Used to initialize the AR Scene, should also change the mVrView to the AR one...
     */
    private void initializeArScene() {
        NodeJni rootNode = new NodeJni();
        ARSceneControllerJni scene = new ARSceneControllerJni(rootNode);

        List<NodeJni> nodes = new ArrayList<>();
        nodes.add(testLine(this));

        testBackgroundImage(scene);

        for (NodeJni node: nodes) {
            rootNode.addChildNode(node);
        }
        testSceneLighting(rootNode);

        // Updating the scene.
        mVrView.setScene(scene);
        ControllerJni nativeController = new ControllerJni(mVrView.getRenderContextRef());
    }

    private void testEdgeDetect() {
        OpenCVJni cv = new OpenCVJni(this);
        Bitmap bm = cv.edgeDetectImage("boba.png");
        ImageView image = new ImageView(this);
        image.setImageBitmap(bm);
        setContentView(image);
    }

    private void testFindTarget() {
        Bitmap targetImage = getBitmapFromAssets("ben.jpg");
        Bitmap screenshot = getBitmapFromAssets("screenshot.png");
        if (targetImage != null && screenshot != null) {
            ImageTrackerJni tracker = new ImageTrackerJni(this, targetImage);
            tracker.findTarget(screenshot);
        }
    }

    private Bitmap getBitmapFromAssets(String assetName) {
        InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = getAssets().open(assetName);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (IOException e) {
            return null;
        }
        return bitmap;
    }

    private List<NodeJni> testText(Context context) {
        NodeJni node = new NodeJni();
        // Create text
        TextJni text = new TextJni(mVrView.getRenderContextRef(),
                "Test Text Here", "Roboto", 25, Color.WHITE, 1f,
                1f, "Left", "Top", "WordWrap", "None", 0);
        float[] position = {0, -0.5f, -0.5f};
        node.setPosition(position);
        node.setGeometry(text);
        return Arrays.asList(node);
    }

    private void testSceneLighting(NodeJni node) {
        float[] lightDirection = {0, 0, -1};
        AmbientLightJni ambientLightJni = new AmbientLightJni(Color.BLACK, 1000.0f);
        ambientLightJni.addToNode(node);

        DirectionalLightJni directionalLightJni = new DirectionalLightJni(Color.BLUE, 1000.0f, lightDirection);
        directionalLightJni.addToNode(node);

        float[] omniLightPosition = {1,0,0};
        OmniLightJni omniLightJni = new OmniLightJni(Color.RED, 1000.0f, 1, 10, omniLightPosition);
        omniLightJni.addToNode(node);

        float[] spotLightPosition = {-2, 0, 3};
        SpotLightJni spotLightJni = new SpotLightJni(Color.YELLOW, 1000.0f, 1, 10, spotLightPosition,
                lightDirection, 2, 10);
        spotLightJni.addToNode(node);
    }

    private List<NodeJni> testSurfaceVideo(Context context) {
        NodeJni node = new NodeJni();
        final SurfaceJni surface = new SurfaceJni(4, 4, 0, 0, 1, 1);
        float[] position = {0,0,-3};
        node.setPosition(position);
        final VideoTextureJni videoTexture = new VideoTextureJni(mVrView.getRenderContextRef());
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoBufferStart() {

            }

            @Override
            public void onVideoBufferEnd() {

            }

            @Override
            public void onVideoFinish() {
            }

            @Override
            public void onVideoFailed(String error) {

            }

            @Override
            public void onReady() {
                surface.setVideoTexture(videoTexture);
                videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mVrView.getRenderContextRef());
                videoTexture.setVolume(0.1f);
                videoTexture.setLoop(false);
                videoTexture.play();
            }

            @Override
            public void onVideoUpdatedTime(float seconds, float duration) {
                Log.e(TAG,"onVideoUpdatedTime for Surface within ViroActivity:" + seconds + "/" + duration);
            }
        });

        node.setGeometry(surface);
        return Arrays.asList(node);
    }

    private List<NodeJni> testSphereVideo(Context context) {
        NodeJni node = new NodeJni();
        final SphereJni sphere = new SphereJni(2, 20, 20, false);
        final VideoTextureJni videoTexture = new VideoTextureJni(mVrView.getRenderContextRef());
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoBufferStart() {

            }

            @Override
            public void onVideoBufferEnd() {

            }

            @Override
            public void onVideoFinish() {
            }

            @Override
            public void onVideoFailed(String error) {

            }

            @Override
            public void onReady() {
                sphere.setVideoTexture(videoTexture);
                videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mVrView.getRenderContextRef());
                videoTexture.setVolume(0.1f);
                videoTexture.setLoop(false);
                videoTexture.play();
            }

            @Override
            public void onVideoUpdatedTime(float seconds, float duration) {
                Log.e(TAG,"onVideoUpdatedTime for Sphere within ViroActivity:" + seconds);
            }
        });
        node.setGeometry(sphere);
        return Arrays.asList(node);
    }

    private void testBackgroundVideo(final SceneJni scene) {
        final VideoTextureJni videoTexture = new VideoTextureJni(mVrView.getRenderContextRef());
        videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mVrView.getRenderContextRef());
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(false);
        videoTexture.play();
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoBufferStart() {

            }

            @Override
            public void onVideoBufferEnd() {

            }

            @Override
            public void onVideoFinish() {
            }

            @Override
            public void onVideoFailed(String error) {

            }

            @Override
            public void onReady() {
                scene.setBackgroundVideoTexture(videoTexture);
                videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mVrView.getRenderContextRef());
                videoTexture.setVolume(0.1f);
                videoTexture.setLoop(false);
                videoTexture.play();
            }

            @Override
            public void onVideoUpdatedTime(float seconds, float duration) {
                Log.e(TAG,"onVideoUpdatedTime for Background within ViroActivity:" + seconds);
            }
        });
    }

    private void testBackgroundImage(SceneJni scene) {
        ImageJni imageJni = new ImageJni("boba.png", TextureFormat.RGBA8);
        TextureJni videoTexture = new TextureJni(imageJni, TextureFormat.RGBA8, true, false);
        scene.setBackgroundImageTexture(videoTexture);
        float[] rotation = {90, 0, 0};
        scene.setBackgroundRotation(rotation);
    }

    private void testSkyBoxImage(SceneJni scene) {
        TextureFormat format = TextureFormat.RGBA8;

        ImageJni pximageJni = new ImageJni("px.png", format);
        ImageJni nximageJni = new ImageJni("nx.png", format);
        ImageJni pyimageJni = new ImageJni("py.png", format);
        ImageJni nyimageJni = new ImageJni("ny.png", format);
        ImageJni pzimageJni = new ImageJni("pz.png", format);
        ImageJni nzimageJni = new ImageJni("nz.png", format);

        TextureJni cubeTexture = new TextureJni(pximageJni, nximageJni, pyimageJni, nyimageJni,
                pzimageJni, nzimageJni, format);

        scene.setBackgroundCubeImageTexture(cubeTexture);
    }

    private List<NodeJni> testBox(Context context) {
        NodeJni node1 = new NodeJni();
        NodeJni node2 = new NodeJni();

        NodeJni node3 = new NodeJni();
        TextJni textJni = new TextJni(mVrView.getRenderContextRef(), "Test text 1 2 3", "Roboto", 24,
                Color.WHITE, 10, 4, "Center", "Center", "None", "None", 1);

        float[] position = {0, -1, -2};
        node3.setPosition(position);
        node3.setGeometry(textJni);
        node3.setEventDelegateJni(getGenericDelegate("Text"));

        // Create a new material with a diffuseTexture set to the image "boba.png"
        ImageJni bobaImage = new ImageJni("boba.png", TextureFormat.RGBA8);

        TextureJni bobaTexture = new TextureJni(bobaImage, TextureFormat.RGBA8, true, true);
        MaterialJni material = new MaterialJni();
//        material.setTexture(bobaTexture, "diffuseTexture");
        material.setColor(Color.BLUE, "diffuseColor");
        material.setLightingModel("Blinn");

        // Creation of ViroBox to the right and billboarded
        BoxJni boxGeometry = new BoxJni(2,4,2);
        node1.setGeometry(boxGeometry);
        float[] boxPosition = {5,0,-3};
        node1.setPosition(boxPosition);
        node1.setMaterials(Arrays.asList(material));
        String[] behaviors = {"billboard"};
        node1.setTransformBehaviors(behaviors);
        node1.setEventDelegateJni(getGenericDelegate("Box"));

        BoxJni boxGeometry2 = new BoxJni(2, 2, 2);
        node2.setGeometry(boxGeometry2);
        float[] boxPosition2 = {-2, 0, -3};
        node2.setPosition(boxPosition2);
        node2.setMaterials(Arrays.asList(material));
        node2.setEventDelegateJni(getGenericDelegate("Box2"));
        return Arrays.asList(node1, node2, node3);
    }

    private List<NodeJni> test3dObjectLoading(Context context) {
        final NodeJni node1 = new NodeJni();

        // Creation of ObjectJni to the right
        ObjectJni objectJni = new ObjectJni(Uri.parse("heart.obj"), false, new AsyncObjListener() {
            @Override
            public void onObjLoaded() {
                // Create a new material with a diffuseTexture set to the image "heart_d.jpg"
                ImageJni heartImage = new ImageJni("heart_d.jpg", TextureFormat.RGBA8);
                TextureJni heartTexture = new TextureJni(heartImage, TextureFormat.RGBA8, true, true);
                MaterialJni material = new MaterialJni();
                material.setTexture(heartTexture, "diffuseTexture");
                material.setLightingModel("Constant");
                node1.setMaterials(Arrays.asList(material));
            }

            @Override
            public void onObjAttached() {

            }

            @Override
            public void onObjFailed(String error) {

            }
        });
        node1.setGeometry(objectJni);

        float[] heartPosition = {-0, -5.5f, -1.15f};
        node1.setPosition(heartPosition);
        return Arrays.asList(node1);
    }

    private EventDelegateJni getGenericDelegate(final String delegateTag){
        EventDelegateJni delegateJni = new EventDelegateJni();
        delegateJni.setEventEnabled(EventDelegateJni.EventAction.ON_HOVER, false);
        delegateJni.setEventEnabled(EventDelegateJni.EventAction.ON_FUSE, true);

        delegateJni.setEventDelegateCallback(new EventDelegateJni.EventDelegateCallback() {
            @Override
            public void onHover(int source, boolean isHovering, float[] hitLoc) {
                Log.e(TAG, delegateTag + " onHover " + isHovering);
            }

            @Override
            public void onClick(int source, EventDelegateJni.ClickState clickState, float[] hitLoc) {
                Log.e(TAG, delegateTag + " onClick " + clickState.toString());
            }

            @Override
            public void onTouch(int source, EventDelegateJni.TouchState touchState, float[] touchPadPos) {
                Log.e(TAG, delegateTag + "onTouch " + touchPadPos[0] + "," + touchPadPos[1]);
            }

            @Override
            public void onControllerStatus(int source, EventDelegateJni.ControllerStatus status) {

            }

            @Override
            public void onSwipe(int source, EventDelegateJni.SwipeState swipeState) {
                Log.e(TAG, delegateTag + " onSwipe " + swipeState.toString());
            }

            @Override
            public void onScroll(int source, float x, float y) {
                Log.e(TAG, delegateTag + " onScroll " + x + "," +y);

            }

            @Override
            public void onDrag(int source, float x, float y, float z) {
                Log.e(TAG, delegateTag +" On drag: " + x + ", " + y + ", " + z);
            }

            @Override
            public void onFuse(int source) {
                Log.e(TAG, delegateTag + " On fuse");
            }
        });

        return delegateJni;
    }

    private List<NodeJni> testImageSurface(Context context) {
        NodeJni node = new NodeJni();
        ImageJni bobaImage = new ImageJni("boba.png", TextureFormat.RGBA8);

        TextureJni bobaTexture = new TextureJni(bobaImage, TextureFormat.RGBA8, true, true);
        MaterialJni material = new MaterialJni();

        SurfaceJni surface = new SurfaceJni(1, 1, 0, 0, 1, 1);
        surface.setMaterial(material);
        surface.setImageTexture(bobaTexture);

        node.setGeometry(surface);
        float[] position = {0, 0, -2};
        node.setPosition(position);
        return Arrays.asList(node);
    }

    private List<NodeJni> testStereoImageSurface(Context context) {
        float[] position1 = {-1f, 1f, -3.3f};
        NodeJni imageNode1 = getStereoImage(position1, "stereo1.jpg");
        float[] position2 = {0, 1f, -3.3f};
        NodeJni imageNode2 = getStereoImage(position2, "stereo2.jpg");

        return Arrays.asList(imageNode1,
                imageNode2);
    }

    private NodeJni getStereoImage(float[] pos, String img){
        NodeJni node = new NodeJni();
        ImageJni bobaImage = new ImageJni(img, TextureFormat.RGBA8);
        TextureJni bobaTexture = new TextureJni(bobaImage,
                TextureFormat.RGBA8, true, true, "LeftRight");
        MaterialJni material = new MaterialJni();
        SurfaceJni surface = new SurfaceJni(1, 1, 0, 0, 1, 1);
        surface.setMaterial(material);
        surface.setImageTexture(bobaTexture);
        node.setGeometry(surface);
        node.setPosition(pos);
        return node;
    }

    private void testStereoBackgroundImage(SceneJni scene) {
        ImageJni imageJni = new ImageJni("stereo3601.jpg", TextureFormat.RGBA4);
        TextureJni videoTexture = new TextureJni(imageJni,
                TextureFormat.RGBA8, true, false, "TopBottom");
        scene.setBackgroundImageTexture(videoTexture);
        float[] rotation = {0, 0, 0};
        scene.setBackgroundRotation(rotation);
    }


    private List<NodeJni> testStereoSurfaceVideo(final Context context) {
        NodeJni node = new NodeJni();
        final SurfaceJni surface = new SurfaceJni(4, 4, 0, 0, 1, 1);
        float[] position = {0,0,-5};
        node.setPosition(position);
        final VideoTextureJni videoTexture = new VideoTextureJni(mVrView.getRenderContextRef(), "LeftRight");
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoBufferStart() {

            }

            @Override
            public void onVideoBufferEnd() {

            }

            @Override
            public void onVideoFinish() {}

            @Override
            public void onVideoFailed(String error) {}

            @Override
            public void onReady() {
                videoTexture.loadSource("file:///android_asset/stereoVid.mp4", mVrView.getRenderContextRef());
                videoTexture.setVolume(0.1f);
                videoTexture.setLoop(true);
                videoTexture.play();
                surface.setVideoTexture(videoTexture);
            }

            @Override
            public void onVideoUpdatedTime(float seconds, float duration) {
                Log.e(TAG,"onVideoUpdatedTime for Surface within ViroActivity:" + seconds);
            }
        });

        node.setGeometry(surface);
        return Arrays.asList(node);
    }

    private void testStereoBackgroundVideo(final SceneJni scene) {
        final VideoTextureJni videoTexture = new VideoTextureJni(mVrView.getRenderContextRef(), "TopBottom");
        videoTexture.setVideoDelegate(new VideoTextureJni.VideoDelegate() {
            @Override
            public void onVideoBufferStart() {

            }

            @Override
            public void onVideoBufferEnd() {

            }

            @Override
            public void onVideoFinish() {}

            @Override
            public void onVideoFailed(String error) {}

            @Override
            public void onReady() {
                scene.setBackgroundVideoTexture(videoTexture);
                videoTexture.loadSource("file:///android_asset/stereoVid360.mp4", mVrView.getRenderContextRef());
                videoTexture.setVolume(0.1f);
                videoTexture.setLoop(false);
                videoTexture.play();
            }

            @Override
            public void onVideoUpdatedTime(float seconds, float duration) {
                Log.e(TAG,"onVideoUpdatedTime for Background within ViroActivity:" + seconds);
            }
        });
    }

    private void addNormalSound(String path) {
        final String key = path + SOUND_COUNT++;
        mSoundMap.put(key, new SoundJni(path,
                mVrView.getRenderContextRef(), new SoundDelegate() {
            @Override
            public void onSoundReady() {
                Log.i("NormalSound", "ViroActivity sound is ready!");
                if (mSoundMap.get(key) != null) {
                    mSoundMap.get(key).play();
                }
            }

            @Override
            public void onSoundFinish() {
                Log.i("NormalSound", "ViroActivity sound has finished!");
            }

            @Override
            public void onSoundFail(String error) {

            }
        }, false));
    }

    private void addNormalSound(SoundDataJni data) {
        final String key = "" + SOUND_COUNT++;
        mSoundMap.put(key, new SoundJni(data,
                mVrView.getRenderContextRef(), new SoundDelegate() {
            @Override
            public void onSoundReady() {
                Log.i("NormalSound", "ViroActivity sound is ready!");
                if (mSoundMap.get(key) != null) {
                    mSoundMap.get(key).play();
                }
            }

            @Override
            public void onSoundFinish() {
                Log.i("NormalSound", "ViroActivity sound has finished!");
            }

            @Override
            public void onSoundFail(String error) {

            }
        }));
    }

    private void addSoundField(String path) {
        final String key = path + SOUND_COUNT++;
        mSoundFieldMap.put(key, new SoundFieldJni(path,
                mVrView.getRenderContextRef(), new SoundDelegate() {
            @Override
            public void onSoundReady() {
                Log.i("SoundField", "ViroActivity sound is ready!");
                if (mSoundFieldMap.get(key) != null) {
                    mSoundFieldMap.get(key).setMuted(false);
                    mSoundFieldMap.get(key).setVolume(1);
                    mSoundFieldMap.get(key).play();
                }
            }

            @Override
            public void onSoundFinish() {
                Log.i("SoundField", "ViroActivity sound has finished!");
            }

            @Override
            public void onSoundFail(String error) {

            }
        }, false));
        float[] rotation = {0,0,90};
        mSoundFieldMap.get(key).setRotation(rotation);
    }

    private void addSpatialSound(final String path) {
        final String key = path + SOUND_COUNT++;
        mSpatialSoundMap.put(key, new SpatialSoundJni(path,
                mVrView.getRenderContextRef(), new SoundDelegate() {
            @Override
            public void onSoundReady() {
                Log.i("SpatialSound", "ViroActivity sound is ready!");

                SpatialSoundJni sound = mSpatialSoundMap.get(key);
                if (sound != null) {
                    float[] position = {5, 0, 0};
                    sound.setPosition(position);
                    sound.setMuted(false);
                    sound.setVolume(1);
                    sound.setLoop(true);
                    sound.play();
                }
            }

            @Override
            public void onSoundFinish() {
                Log.i("SpatialSound", "ViroActivity sound has finished!");
            }

            @Override
            public void onSoundFail(String error) {

            }
        }, false));
    }

    private void addSpatialSound(final SoundDataJni data) {
        final String key = "" + SOUND_COUNT++;
        mSpatialSoundMap.put(key, new SpatialSoundJni(data,
                mVrView.getRenderContextRef(), new SoundDelegate() {
            @Override
            public void onSoundReady() {

                SpatialSoundJni sound = mSpatialSoundMap.get(key);
                if (sound != null) {
                    float[] position = {5, 0, 0};
                    sound.setPosition(position);
                    sound.setMuted(false);
                    sound.setVolume(1);
                    sound.setLoop(true);
                    sound.play();
                }
            }

            @Override
            public void onSoundFinish() {
                Log.i("SpatialSound", "ViroActivity sound has finished!");
            }

            @Override
            public void onSoundFail(String error) {

            }
        }));
    }

    private void setSoundRoom(SceneJni scene, RenderContextJni renderContextJni) {
        float[] size = {2, 2, 2};
        scene.setSoundRoom(renderContextJni, size, "transparent", "wood_panel", "thin_glass");
    }

    private NodeJni testLine(Context scene) {
        MaterialJni material = new MaterialJni();
        material.setColor(Color.RED, "diffuseColor");
        material.setLightingModel("Constant");
        material.setCullMode("None");

        float[] linePos = {0,0,-1};
        float[][] points = {{0,0}, {.5f,.5f}, {1,0}};
        PolylineJni polyline = new PolylineJni(points, 0.1f);
        NodeJni node1 = new NodeJni();

        node1.setPosition(linePos);
        node1.setGeometry(polyline);
        node1.setMaterials(Arrays.asList(material));
        node1.setEventDelegateJni(getGenericDelegate("Line"));
        return node1;
    }
}
