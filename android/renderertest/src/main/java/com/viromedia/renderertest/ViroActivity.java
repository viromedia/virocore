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

import com.viro.renderer.ARAnchor;
import com.viro.renderer.jni.ARNode;
import com.viro.renderer.jni.ARPlane;
import com.viro.renderer.jni.ARSceneController;
import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.AsyncObjListener;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.Controller;
import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.EventDelegate;
import com.viro.renderer.jni.GLListener;
import com.viro.renderer.jni.Image;
import com.viro.renderer.jni.ImageTracker;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Object3D;
import com.viro.renderer.jni.OmniLight;
import com.viro.renderer.jni.OpenCV;
import com.viro.renderer.jni.Polyline;
import com.viro.renderer.jni.RenderContext;
import com.viro.renderer.jni.SceneController;
import com.viro.renderer.jni.SoundData;
import com.viro.renderer.jni.SoundDelegate;
import com.viro.renderer.jni.SoundField;
import com.viro.renderer.jni.Sound;
import com.viro.renderer.jni.SpatialSound;
import com.viro.renderer.jni.Sphere;
import com.viro.renderer.jni.Spotlight;
import com.viro.renderer.jni.Surface;
import com.viro.renderer.jni.Text;
import com.viro.renderer.jni.TextureFormat;
import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.VideoTexture;
import com.viro.renderer.jni.ViroViewARCore;
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

public class ViroActivity extends AppCompatActivity implements GLListener {
    private static int SOUND_COUNT = 0;
    private VrView mVrView;
    private final Map<String, Sound> mSoundMap = new HashMap<>();
    private final Map<String, SoundField> mSoundFieldMap = new HashMap();
    private final Map<String, SpatialSound> mSpatialSoundMap = new HashMap<>();
    private static String TAG = ViroActivity.class.getSimpleName();
    private ARNode.ARNodeDelegate mARNodeDelegate;

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
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mVrView = new ViroViewARCore(this, this);
        }

        mVrView.setVrModeEnabled(true);
        mVrView.validateApiKey("7EEDCB99-2C3B-4681-AE17-17BC165BF792");
        setContentView(mVrView.getContentView());

        // uncomment the below line to test AR.
        //testEdgeDetect();
        //testFindTarget();
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
        //initializeVrScene();
        initializeArScene();
    }

    private void initializeVrScene() {
        // Creation of SceneControllerJni within scene navigator
        SceneController scene = new SceneController();
        Node rootNode = scene.getSceneNode();
        List<Node> nodes = new ArrayList<>();
        //nodes = testSurfaceVideo(this);
        //nodes = testSphereVideo(this);
        nodes = testBox(getApplicationContext());
        //nodes = test3dObjectLoading(getApplicationContext());

        //nodes = testImageSurface(this);
        //nodes = testText(this);

        //testBackgroundVideo(scene);
        //testBackgroundImage(scene);
        //testSkyBoxImage(scene);

        //nodes = testStereoSurfaceVideo(this);
        //nodes = testStereoImageSurface(this);
        //nodes.add(testLine(this));
        //testStereoBackgroundVideo(scene);
        //testStereoBackgroundImage(scene);

        // addNormalSound("http://www.kozco.com/tech/32.mp3");
        // addNormalSound("http://www.bensound.com/royalty-free-music?download=dubstep");
        // addSoundField("http://ambisonics.dreamhosters.com/AMB/pink_pan_H.amb");
        // addSpatialSound("http://www.kozco.com/tech/32.mp3");

        final SoundData data = new SoundData("http://www.kozco.com/tech/32.mp3", false);
        //addSpatialSound(data);
        //addNormalSound(data);

        setSoundRoom(scene, mVrView.getRenderContextRef());

        for (Node node: nodes) {
            rootNode.addChildNode(node);
        }
        testSceneLighting(rootNode);

        // Updating the scene.
        mVrView.setSceneController(scene);
        Controller nativeController = new Controller(mVrView.getRenderContextRef());
        //nativeController.setReticleVisibility(false);
    }

    /*
     Used to initialize the AR Scene, should also change the mVrView to the AR one...
     */
    private void initializeArScene() {
        Node rootNode = new Node();
        ARSceneController scene = new ARSceneController();

        List<Node> nodes = new ArrayList<>();
        nodes.add(testLine(this));

        //testBackgroundImage(scene);

        nodes.addAll(testARPlane(scene));

        for (Node node: nodes) {
            rootNode.addChildNode(node);
        }
        testSceneLighting(rootNode);

        // Updating the scene.
        mVrView.setSceneController(scene);
    }

    private void testEdgeDetect() {
        OpenCV cv = new OpenCV(this);
        Bitmap bm = cv.edgeDetectImage("boba.png");
        ImageView image = new ImageView(this);
        image.setImageBitmap(bm);
        setContentView(image);
    }

    private void testFindTarget() {
        Bitmap targetImage = getBitmapFromAssets("ben.jpg");
        Bitmap screenshot = getBitmapFromAssets("screenshot.png");
        if (targetImage != null && screenshot != null) {
            ImageTracker tracker = new ImageTracker(this, targetImage);
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

    private List<Node> testText(Context context) {
        Node node = new Node();
        // Create text
        Text text = new Text(mVrView.getRenderContextRef(),
                "Test Text Here", "Roboto", 25, Color.WHITE, 1f,
                1f, "Left", "Top", "WordWrap", "None", 0);
        float[] position = {0, -0.5f, -0.5f};
        node.setPosition(position);
        node.setGeometry(text);
        return Arrays.asList(node);
    }

    private void testSceneLighting(Node node) {
        float[] lightDirection = {0, 0, -1};
        AmbientLight ambientLightJni = new AmbientLight(Color.BLACK, 1000.0f);
        ambientLightJni.addToNode(node);

        DirectionalLight directionalLightJni = new DirectionalLight(Color.BLUE, 1000.0f, lightDirection);
        directionalLightJni.addToNode(node);

        float[] omniLightPosition = {1,0,0};
        OmniLight omniLightJni = new OmniLight(Color.RED, 1000.0f, 1, 10, omniLightPosition);
        omniLightJni.addToNode(node);

        float[] spotLightPosition = {-2, 0, 3};
        Spotlight spotLightJni = new Spotlight(Color.YELLOW, 1000.0f, 1, 10, spotLightPosition,
                lightDirection, 2, 10);
        spotLightJni.addToNode(node);
    }

    private List<Node> testSurfaceVideo(Context context) {
        Node node = new Node();
        final Surface surface = new Surface(4, 4, 0, 0, 1, 1);
        float[] position = {0,0,-3};
        node.setPosition(position);
        final VideoTexture videoTexture = new VideoTexture(mVrView.getRenderContextRef());
        videoTexture.setVideoDelegate(new VideoTexture.VideoDelegate() {
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

    private List<Node> testSphereVideo(Context context) {
        Node node = new Node();
        final Sphere sphere = new Sphere(2, 20, 20, false);
        final VideoTexture videoTexture = new VideoTexture(mVrView.getRenderContextRef());
        videoTexture.setVideoDelegate(new VideoTexture.VideoDelegate() {
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

    private void testBackgroundVideo(final SceneController scene) {
        final VideoTexture videoTexture = new VideoTexture(mVrView.getRenderContextRef());
        videoTexture.loadSource("https://s3.amazonaws.com/viro.video/Climber2Top.mp4", mVrView.getRenderContextRef());
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(false);
        videoTexture.play();
        videoTexture.setVideoDelegate(new VideoTexture.VideoDelegate() {
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

    private void testBackgroundImage(SceneController scene) {
        Image imageJni = new Image("boba.png", TextureFormat.RGBA8);
        Texture videoTexture = new Texture(imageJni, TextureFormat.RGBA8, true, false);
        scene.setBackgroundImageTexture(videoTexture);
        float[] rotation = {90, 0, 0};
        scene.setBackgroundRotation(rotation);
    }

    private void testSkyBoxImage(SceneController scene) {
        TextureFormat format = TextureFormat.RGBA8;

        Image pximageJni = new Image("px.png", format);
        Image nximageJni = new Image("nx.png", format);
        Image pyimageJni = new Image("py.png", format);
        Image nyimageJni = new Image("ny.png", format);
        Image pzimageJni = new Image("pz.png", format);
        Image nzimageJni = new Image("nz.png", format);

        Texture cubeTexture = new Texture(pximageJni, nximageJni, pyimageJni, nyimageJni,
                pzimageJni, nzimageJni, format);

        scene.setBackgroundCubeImageTexture(cubeTexture);
    }

    private List<Node> testBox(Context context) {
        Node node1 = new Node();
        Node node2 = new Node();

        Node node3 = new Node();
        Text textJni = new Text(mVrView.getRenderContextRef(), "Test text 1 2 3", "Roboto", 24,
                Color.WHITE, 10, 4, "Center", "Center", "None", "None", 1);

        float[] position = {0, -1, -2};
        node3.setPosition(position);
        node3.setGeometry(textJni);
        node3.setEventDelegateJni(getGenericDelegate("Text"));

        // Create a new material with a diffuseTexture set to the image "boba.png"
        Image bobaImage = new Image("boba.png", TextureFormat.RGBA8);

        Texture bobaTexture = new Texture(bobaImage, TextureFormat.RGBA8, true, true);
        Material material = new Material();
//        material.setTexture(bobaTexture, "diffuseTexture");
        material.setColor(Color.BLUE, "diffuseColor");
        material.setLightingModel("Blinn");

        // Creation of ViroBox to the right and billboarded
        Box boxGeometry = new Box(2,4,2);
        node1.setGeometry(boxGeometry);
        float[] boxPosition = {5,0,-3};
        node1.setPosition(boxPosition);
        node1.setMaterials(Arrays.asList(material));
        String[] behaviors = {"billboard"};
        node1.setTransformBehaviors(behaviors);
        node1.setEventDelegateJni(getGenericDelegate("Box"));

        Box boxGeometry2 = new Box(2, 2, 2);
        node2.setGeometry(boxGeometry2);
        float[] boxPosition2 = {-2, 0, -3};
        node2.setPosition(boxPosition2);
        node2.setMaterials(Arrays.asList(material));
        node2.setEventDelegateJni(getGenericDelegate("Box2"));
        return Arrays.asList(node1, node2, node3);
    }

    private List<Node> test3dObjectLoading(Context context) {
        final Node node1 = new Node();

        // Creation of ObjectJni to the right
        Object3D objectJni = new Object3D(Uri.parse("heart.obj"), false, new AsyncObjListener() {
            @Override
            public void onObjLoaded() {
                // Create a new material with a diffuseTexture set to the image "heart_d.jpg"
                Image heartImage = new Image("heart_d.jpg", TextureFormat.RGBA8);
                Texture heartTexture = new Texture(heartImage, TextureFormat.RGBA8, true, true);
                Material material = new Material();
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

    private EventDelegate getGenericDelegate(final String delegateTag){
        EventDelegate delegateJni = new EventDelegate();
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_HOVER, false);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_FUSE, true);

        delegateJni.setEventDelegateCallback(new EventDelegate.EventDelegateCallback() {
            @Override
            public void onHover(int source, boolean isHovering, float[] hitLoc) {
                Log.e(TAG, delegateTag + " onHover " + isHovering);
            }

            @Override
            public void onClick(int source, EventDelegate.ClickState clickState, float[] hitLoc) {
                Log.e(TAG, delegateTag + " onClick " + clickState.toString());
            }

            @Override
            public void onTouch(int source, EventDelegate.TouchState touchState, float[] touchPadPos) {
                Log.e(TAG, delegateTag + "onTouch " + touchPadPos[0] + "," + touchPadPos[1]);
            }

            @Override
            public void onControllerStatus(int source, EventDelegate.ControllerStatus status) {

            }

            @Override
            public void onSwipe(int source, EventDelegate.SwipeState swipeState) {
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

    private List<Node> testImageSurface(Context context) {
        Node node = new Node();
        Image bobaImage = new Image("boba.png", TextureFormat.RGBA8);

        Texture bobaTexture = new Texture(bobaImage, TextureFormat.RGBA8, true, true);
        Material material = new Material();

        Surface surface = new Surface(1, 1, 0, 0, 1, 1);
        surface.setMaterial(material);
        surface.setImageTexture(bobaTexture);

        node.setGeometry(surface);
        float[] position = {0, 0, -2};
        node.setPosition(position);
        return Arrays.asList(node);
    }

    private List<Node> testStereoImageSurface(Context context) {
        float[] position1 = {-1f, 1f, -3.3f};
        Node imageNode1 = getStereoImage(position1, "stereo1.jpg");
        float[] position2 = {0, 1f, -3.3f};
        Node imageNode2 = getStereoImage(position2, "stereo2.jpg");

        return Arrays.asList(imageNode1,
                imageNode2);
    }

    private Node getStereoImage(float[] pos, String img){
        Node node = new Node();
        Image bobaImage = new Image(img, TextureFormat.RGBA8);
        Texture bobaTexture = new Texture(bobaImage,
                TextureFormat.RGBA8, true, true, "LeftRight");
        Material material = new Material();
        Surface surface = new Surface(1, 1, 0, 0, 1, 1);
        surface.setMaterial(material);
        surface.setImageTexture(bobaTexture);
        node.setGeometry(surface);
        node.setPosition(pos);
        return node;
    }

    private void testStereoBackgroundImage(SceneController scene) {
        Image imageJni = new Image("stereo3601.jpg", TextureFormat.RGBA4);
        Texture videoTexture = new Texture(imageJni,
                TextureFormat.RGBA8, true, false, "TopBottom");
        scene.setBackgroundImageTexture(videoTexture);
        float[] rotation = {0, 0, 0};
        scene.setBackgroundRotation(rotation);
    }


    private List<Node> testStereoSurfaceVideo(final Context context) {
        Node node = new Node();
        final Surface surface = new Surface(4, 4, 0, 0, 1, 1);
        float[] position = {0,0,-5};
        node.setPosition(position);
        final VideoTexture videoTexture = new VideoTexture(mVrView.getRenderContextRef(), "LeftRight");
        videoTexture.setVideoDelegate(new VideoTexture.VideoDelegate() {
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

    private void testStereoBackgroundVideo(final SceneController scene) {
        final VideoTexture videoTexture = new VideoTexture(mVrView.getRenderContextRef(), "TopBottom");
        videoTexture.setVideoDelegate(new VideoTexture.VideoDelegate() {
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

    private List<Node> testARPlane(ARSceneController arScene) {
        ARPlane arPlane = new ARPlane(0, 0);
        Node node = new Node();
        final Surface surface = new Surface(.5f, .5f, 0, 0, 1, 1);

        float[] rotation = {-90, 0, 0};
        node.setRotation(rotation);
        mARNodeDelegate = new ARNode.ARNodeDelegate() {
            @Override
            public void onAnchorFound(ARAnchor anchor) {
                Log.i("ViroActivity", "onAnchorFound");
            }

            @Override
            public void onAnchorUpdated(ARAnchor anchor) {
                Log.i("ViroActivity", "onAnchorUpdated");
                surface.setWidth(anchor.getExtent()[0]);
                surface.setHeight(anchor.getExtent()[2]);
            }

            @Override
            public void onAnchorRemoved() {
                Log.i("ViroActivity", "onAnchorRemoved");
            }
        };
        arPlane.registerARNodeDelegate(mARNodeDelegate);

        surface.attachToNode(node);
        arPlane.addChildNode(node);
        arScene.addARPlane(arPlane);

        ArrayList<Node> list = new ArrayList<Node>();
        list.add(arPlane);
        return list;

    }

    private void addNormalSound(String path) {
        final String key = path + SOUND_COUNT++;
        mSoundMap.put(key, new Sound(path,
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

    private void addNormalSound(SoundData data) {
        final String key = "" + SOUND_COUNT++;
        mSoundMap.put(key, new Sound(data,
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
        mSoundFieldMap.put(key, new SoundField(path,
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
        mSpatialSoundMap.put(key, new SpatialSound(path,
                mVrView.getRenderContextRef(), new SoundDelegate() {
            @Override
            public void onSoundReady() {
                Log.i("SpatialSound", "ViroActivity sound is ready!");

                SpatialSound sound = mSpatialSoundMap.get(key);
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

    private void addSpatialSound(final SoundData data) {
        final String key = "" + SOUND_COUNT++;
        mSpatialSoundMap.put(key, new SpatialSound(data,
                mVrView.getRenderContextRef(), new SoundDelegate() {
            @Override
            public void onSoundReady() {

                SpatialSound sound = mSpatialSoundMap.get(key);
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

    private void setSoundRoom(SceneController scene, RenderContext renderContextJni) {
        float[] size = {2, 2, 2};
        scene.setSoundRoom(renderContextJni, size, "transparent", "wood_panel", "thin_glass");
    }

    private Node testLine(Context scene) {
        Material material = new Material();
        material.setColor(Color.RED, "diffuseColor");
        material.setLightingModel("Constant");
        material.setCullMode("None");

        float[] linePos = {0,0,-1};
        float[][] points = {{0,0}, {.5f,.5f}, {1,0}};
        Polyline polyline = new Polyline(points, 0.1f);
        Node node1 = new Node();

        node1.setPosition(linePos);
        node1.setGeometry(polyline);
        node1.setMaterials(Arrays.asList(material));
        node1.setEventDelegateJni(getGenericDelegate("Line"));
        return node1;
    }
}
