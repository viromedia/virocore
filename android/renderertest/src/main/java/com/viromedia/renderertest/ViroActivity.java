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
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.ImageView;

import com.viro.renderer.ARAnchor;
import com.viro.renderer.ARHitTestResult;
import com.viro.renderer.jni.ARNode;
import com.viro.renderer.jni.ARPlane;
import com.viro.renderer.jni.ARScene;
import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.AnimationTimingFunction;
import com.viro.renderer.jni.AnimationTransaction;
import com.viro.renderer.jni.AsyncObject3DListener;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.Controller;
import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.EventDelegate;
import com.viro.renderer.jni.GLListener;
import com.viro.renderer.jni.Geometry;
import com.viro.renderer.jni.Image;
import com.viro.renderer.jni.ImageTracker;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Object3D;
import com.viro.renderer.jni.OmniLight;
import com.viro.renderer.jni.OpenCV;
import com.viro.renderer.jni.Polyline;
import com.viro.renderer.jni.Scene;
import com.viro.renderer.jni.Sound;
import com.viro.renderer.jni.SoundData;
import com.viro.renderer.jni.SoundField;
import com.viro.renderer.jni.SpatialSound;
import com.viro.renderer.jni.Sphere;
import com.viro.renderer.jni.Spotlight;
import com.viro.renderer.jni.Surface;
import com.viro.renderer.jni.Text;
import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.Texture.TextureFormat;
import com.viro.renderer.jni.Vector;
import com.viro.renderer.jni.VideoTexture;
import com.viro.renderer.jni.ViroContext;
import com.viro.renderer.jni.ViroGvrLayout;
import com.viro.renderer.jni.ViroOvrView;
import com.viro.renderer.jni.ViroView;
import com.viro.renderer.jni.ViroViewARCore;
import com.viro.renderer.jni.event.ClickState;
import com.viro.renderer.jni.event.ControllerStatus;
import com.viro.renderer.jni.event.PinchState;
import com.viro.renderer.jni.event.RotateState;
import com.viro.renderer.jni.event.SwipeState;
import com.viro.renderer.jni.event.TouchState;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ViroActivity extends AppCompatActivity implements GLListener {
    private static final String TAG = ViroActivity.class.getSimpleName();
    private static int SOUND_COUNT = 0;
    private final Map<String, Sound> mSoundMap = new HashMap<>();
    private final Map<String, SoundField> mSoundFieldMap = new HashMap();
    private final Map<String, SpatialSound> mSpatialSoundMap = new HashMap<>();
    private ViroView mViroView;
    private ARNode.ARNodeDelegate mARNodeDelegate;
    private Handler mHandler;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = new ViroGvrLayout(this, this, new Runnable(){
                @Override
                public void run() {
                    Log.e(TAG, "On GVR userRequested exit");
                }
            });
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = new ViroOvrView(this, this);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = new ViroViewARCore(this, this);
        }

        mViroView.setVrModeEnabled(true);
        mViroView.validateApiKey("7EEDCB99-2C3B-4681-AE17-17BC165BF792");
        setContentView(mViroView.getContentView());

        mHandler = new Handler(getMainLooper());
        // uncomment the below line to test AR.
        //testEdgeDetect();
        //testFindTarget();
    }

    @Override
    protected void onStart(){
        super.onStart();
        mViroView.onActivityStarted(this);
    }

    @Override
    protected void onResume(){
        super.onResume();
        mViroView.onActivityResumed(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        mViroView.onActivityPaused(this);
    }

    @Override
    protected void onStop(){
        super.onStop();
        mViroView.onActivityStopped(this);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mViroView.onActivityDestroyed(this);
    }

    @Override
    public void onGlInitialized() {
        Log.e("ViroActivity", "onGlInitialized called");
        initializeVrScene();
//        initializeArScene();
    }

    private void initializeVrScene() {
        // Creation of SceneControllerJni within scene navigator
        final Scene scene = new Scene();
        final Node rootNode = scene.getRootNode();
        List<Node> nodes = new ArrayList<>();
        //nodes = testSurfaceVideo(this);
       // nodes = testSphereVideo(this);
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

        //addNormalSound("http://www.kozco.com/tech/32.mp3");
        //addNormalSound("http://www.bensound.com/royalty-free-music?download=dubstep");
        //addSoundField("file:///android_asset/thelin.wav");
        //addSpatialSound("http://www.kozco.com/tech/32.mp3");

        //ByteBuffer hdrImage = getByteBufferFromAssets("wooden.vhd");
        //Texture background = new Texture(hdrImage, null);
        //scene.setBackgroundTexture(background);

        //final SoundData data = new SoundData("http://www.kozco.com/tech/32.mp3", false);
        //addSpatialSound(data);
        //addNormalSound(data);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                setSoundRoom(scene, mViroView.getViroContext());
            }
        }, 5000);
        //setSoundRoom(scene, mViroView.getViroContext());

        for (final Node node : nodes) {
            rootNode.addChildNode(node);
        }
        testSceneLighting(rootNode);

        // Updating the scene.
        mViroView.setScene(scene);
        final Controller nativeController = new Controller(mViroView.getViroContext());
        //nativeController.setReticleVisibility(false);
    }

    /*
     Used to initialize the AR Scene, should also change the mVrView to the AR one...
     */
    private void initializeArScene() {
        final ARScene scene = new ARScene();
        final Node rootNode = scene.getRootNode();

        final List<Node> nodes = new ArrayList<>();
        nodes.add(testLine(this));

        //testBackgroundImage(scene);

        nodes.addAll(testARDrag());

        for (final Node node : nodes) {
            rootNode.addChildNode(node);
        }
        testSceneLighting(rootNode);

        // Updating the scene.
        mViroView.setScene(scene);
    }

    private void testEdgeDetect() {
        final OpenCV cv = new OpenCV(this);
        final Bitmap bm = cv.edgeDetectImage("boba.png");
        final ImageView image = new ImageView(this);
        image.setImageBitmap(bm);
        setContentView(image);
    }

    private void testFindTarget() {
        final Bitmap targetImage = getBitmapFromAssets("ben.jpg");
        final Bitmap screenshot = getBitmapFromAssets("screenshot.png");
        if (targetImage != null && screenshot != null) {
            final ImageTracker tracker = new ImageTracker(this, targetImage);
            tracker.findTarget(screenshot);
        }
    }

    private ByteBuffer getByteBufferFromAssets(final String assetName) {
        final InputStream istr;

        final byte[] buffer = new byte[1024];
        final ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        try {
            istr = getAssets().open(assetName);

            while (true) {
                final int read = istr.read(buffer);
                if (read == -1) {
                    break;
                }
                outStream.write(buffer, 0, read);
            }
            outStream.flush();
        } catch (final IOException e) {
            e.printStackTrace();
        }

        final ByteBuffer direct = ByteBuffer.allocateDirect(outStream.size());
        direct.put(outStream.toByteArray());
        direct.rewind();
        return direct;
    }

    private Bitmap getBitmapFromAssets(final String assetName) {
        final InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = getAssets().open(assetName);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (final IOException e) {
            return null;
        }
        return bitmap;
    }

    private List<Node> testText(final Context context) {
        final Node node = new Node();
        // Create text
        final Text text = new Text(mViroView.getViroContext(),
                "Test Text Here", "Roboto", 25, Color.WHITE, 1f,
                1f, Text.HorizontalAlignment.LEFT, Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final float[] position = {0, -0.5f, -0.5f};
        node.setPosition(new Vector(position));
        node.setGeometry(text);
        return Arrays.asList(node);
    }

    private void testSceneLighting(final Node node) {
        final float[] lightDirection = {0, 0, -1};
        final AmbientLight ambientLightJni = new AmbientLight(Color.BLACK, 1000.0f);
        node.addLight(ambientLightJni);

        final DirectionalLight directionalLightJni = new DirectionalLight(Color.BLUE, 1000.0f, new Vector(lightDirection));
        node.addLight(directionalLightJni);

        final float[] omniLightPosition = {1, 0, 0};
        final OmniLight omniLightJni = new OmniLight(Color.RED, 1000.0f, 1, 10, new Vector(omniLightPosition));
        node.addLight(omniLightJni);

        final float[] spotLightPosition = {-2, 0, 3};
        final Spotlight spotLightJni = new Spotlight(Color.YELLOW, 1000.0f, 1, 10, new Vector(spotLightPosition),
                new Vector(lightDirection), 2, 10);
        node.addLight(spotLightJni);
    }

    private List<Node> testSurfaceVideo(final Context context) {
        final Node node = new Node();
        final Surface surface = new Surface(4, 4, 0, 0, 1, 1);
        final float[] position = {0, 0, -3};
        node.setPosition(new Vector(position));
        final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(), Uri.parse("https://s3.amazonaws.com/viro.video/Climber2Top.mp4"));
        final Material material = new Material();
        material.setDiffuseTexture(videoTexture);
        surface.setMaterials(Arrays.asList(material));
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(true);
        videoTexture.play();

        node.setGeometry(surface);
        return Arrays.asList(node);
    }

    private List<Node> testSphereVideo(final Context context) {
        final Node node = new Node();
        final Sphere sphere = new Sphere(2, 20, 20, false);
        final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(),
                Uri.parse("https://s3.amazonaws.com/viro.video/Climber2Top.mp4"));
        final Material material = new Material();
        material.setDiffuseTexture(videoTexture);
        sphere.setMaterials(Arrays.asList(material));
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(false);
        videoTexture.play();

        node.setGeometry(sphere);
        return Arrays.asList(node);
    }

    private void testBackgroundVideo(final Scene scene) {
        final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(),
                Uri.parse("https://s3.amazonaws.com/viro.video/Climber2Top.mp4"));
        scene.setBackgroundTexture(videoTexture);

        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(false);
        videoTexture.play();

    }

    private void testBackgroundImage(final Scene scene) {
        final Image imageJni = new Image("boba.png", TextureFormat.RGBA8);
        final Texture videoTexture = new Texture(imageJni, TextureFormat.RGBA8, true, false);
        scene.setBackgroundTexture(videoTexture);
        final float[] rotation = {90, 0, 0};
        scene.setBackgroundRotation(new Vector(rotation));
    }

    private void testSkyBoxImage(final Scene scene) {
        final TextureFormat format = TextureFormat.RGBA8;

        final Image pximageJni = new Image("px.png", format);
        final Image nximageJni = new Image("nx.png", format);
        final Image pyimageJni = new Image("py.png", format);
        final Image nyimageJni = new Image("ny.png", format);
        final Image pzimageJni = new Image("pz.png", format);
        final Image nzimageJni = new Image("nz.png", format);

        final Texture cubeTexture = new Texture(pximageJni, nximageJni, pyimageJni, nyimageJni,
                pzimageJni, nzimageJni, format);

        scene.setBackgroundCubeTexture(cubeTexture);
    }

    private List<Node> testBox(final Context context) {
        final Node node1 = new Node();
        final Node node2 = new Node();

        final Node node3 = new Node();
        final Text textJni = new Text(mViroView.getViroContext(), "Test text 1 2 3", "Roboto", 24,
                Color.WHITE, 10, 4, Text.HorizontalAlignment.CENTER, Text.VerticalAlignment.CENTER, Text.LineBreakMode.NONE,
                Text.ClipMode.CLIP_TO_BOUNDS, 1);

        final float[] position = {0, -1, -2};
        node3.setPosition(new Vector(position));
        node3.setGeometry(textJni);
        //node3.setEventDelegate(getGenericDelegate("Text"));

        final Bitmap bobaBitmap = getBitmapFromAssets("boba.png");
        final Bitmap specBitmap = getBitmapFromAssets("specular.png");

        final Texture bobaTexture = new Texture(bobaBitmap, TextureFormat.RGBA8, true, true);
        final Texture specTexture = new Texture(specBitmap, TextureFormat.RGBA8, true, true);

        final Material material = new Material();
        material.setDiffuseTexture(bobaTexture);
        material.setDiffuseColor(Color.BLUE);
        material.setSpecularTexture(specTexture);
        material.setLightingModel(Material.LightingModel.LAMBERT);

        // Creation of ViroBox to the right and billboarded
        final Box boxGeometry = new Box(2, 4, 2);
        node1.setGeometry(boxGeometry);
        final Vector boxPosition = new Vector(5, 0, -3);
        node1.setPosition(boxPosition);
        boxGeometry.setMaterials(Arrays.asList(material));
        final EnumSet<Node.TransformBehavior> behaviors = EnumSet.of(Node.TransformBehavior.BILLBOARD);
        //node1.setTransformBehaviors(behaviors);
        node1.setEventDelegate(getGenericDelegate("Box"));

        final Box boxGeometry2 = new Box(2, 2, 2);
        node2.setGeometry(boxGeometry2);
        final Vector boxPosition2 = new Vector(-2, 0, -3);
        node2.setPosition(boxPosition2);
        boxGeometry2.setMaterials(Arrays.asList(material));
        node2.setEventDelegate(getGenericDelegate("Box2"));

        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDelay(2);
        AnimationTransaction.setAnimationDuration(5);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.Bounce);
        AnimationTransaction.setFinishCallback(new AnimationTransaction.FinishedCallback() {
            @Override
            public void onFinished(final AnimationTransaction transaction) {
                Log.i("Viro", "Animation finished");
            }
        });
        node2.setPosition(new Vector(-2, 2.5f, -3));

        final AnimationTransaction transaction = AnimationTransaction.commit();



        return Arrays.asList(node1, node2, node3);
    }

    private List<Node> test3dObjectLoading(final Context context) {
        final Node node1 = new Node();

        // Creation of ObjectJni to the right
        final Object3D objectJni = new Object3D();
        objectJni.loadModel(Uri.parse("file:///android_asset/heart.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                // Create a new material with a diffuseTexture set to the image "heart_d.jpg"
                final Image heartImage = new Image("heart_d.jpg", TextureFormat.RGBA8);
                final Texture heartTexture = new Texture(heartImage, TextureFormat.RGBA8, true, true);
                final Material material = new Material();
                material.setDiffuseTexture(heartTexture);
                material.setLightingModel(Material.LightingModel.CONSTANT);

                final Geometry geometry = object.getGeometry();
                if (geometry != null) {
               //     geometry.setMaterials(Arrays.asList(material));
                }

                Log.w("Viro", "OBJECT WAS LOADED!");
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });
        node1.addChildNode(objectJni);

        final float[] heartPosition = {-0, -5.5f, -1.15f};
        node1.setPosition(new Vector(heartPosition));
        return Arrays.asList(node1);
    }

    private List<Node> testImageSurface(final Context context) {
        final Node node = new Node();
        final Image bobaImage = new Image("boba.png", TextureFormat.RGBA8);

        final Texture bobaTexture = new Texture(bobaImage, TextureFormat.RGBA8, true, true);
        final Material material = new Material();

        final Surface surface = new Surface(1, 1, 0, 0, 1, 1);
        surface.setMaterial(material);
        surface.setImageTexture(bobaTexture);

        node.setGeometry(surface);
        final float[] position = {0, 0, -2};
        node.setPosition(new Vector(position));
        return Arrays.asList(node);
    }

    private List<Node> testStereoImageSurface(final Context context) {
        final float[] position1 = {-1f, 1f, -3.3f};
        final Node imageNode1 = getStereoImage(position1, "stereo1.jpg");
        final float[] position2 = {0, 1f, -3.3f};
        final Node imageNode2 = getStereoImage(position2, "stereo2.jpg");

        return Arrays.asList(imageNode1,
                imageNode2);
    }

    private Node getStereoImage(final float[] pos, final String img) {
        final Node node = new Node();
        final Image bobaImage = new Image(img, TextureFormat.RGBA8);
        final Texture bobaTexture = new Texture(bobaImage,
                TextureFormat.RGBA8, true, true, "LeftRight");
        final Material material = new Material();
        final Surface surface = new Surface(1, 1, 0, 0, 1, 1);
        surface.setMaterial(material);
        surface.setImageTexture(bobaTexture);
        node.setGeometry(surface);
        node.setPosition(new Vector(pos));
        return node;
    }

    private void testStereoBackgroundImage(final Scene scene) {
        final Image imageJni = new Image("stereo3601.jpg", TextureFormat.RGBA8);
        final Texture videoTexture = new Texture(imageJni,
                TextureFormat.RGBA8, true, false, "TopBottom");
        scene.setBackgroundTexture(videoTexture);
        final float[] rotation = {0, 0, 0};
        scene.setBackgroundRotation(new Vector(rotation));
    }


    private List<Node> testStereoSurfaceVideo(final Context context) {
        final Node node = new Node();
        final Surface surface = new Surface(4, 4, 0, 0, 1, 1);
        final float[] position = {0, 0, -5};
        node.setPosition(new Vector(position));

        final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(),
                Uri.parse("file:///android_asset/stereoVid.mp4"), null, Texture.StereoMode.LEFT_RIGHT);
        videoTexture.setVolume(0.1f);
        videoTexture.setLoop(true);
        videoTexture.play();

        node.setGeometry(surface);
        return Arrays.asList(node);
    }

    private void testStereoBackgroundVideo(final Scene scene) {
        final VideoTexture.Delegate delegate = new VideoTexture.Delegate() {
            @Override
            public void onVideoBufferStart(final VideoTexture video) {

            }

            @Override
            public void onVideoBufferEnd(final VideoTexture video) {

            }

            @Override
            public void onVideoFinish(final VideoTexture video) {
            }

            @Override
            public void onVideoFailed(final String error) {
            }

            @Override
            public void onReady(final VideoTexture video) {
                scene.setBackgroundTexture(video);
                video.setVolume(0.1f);
                video.setLoop(false);
                video.play();
            }

            @Override
            public void onVideoUpdatedTime(final VideoTexture video, final float seconds, final float duration) {
                Log.e(TAG,"onVideoUpdatedTime for Background within ViroActivity:" + seconds);
            }
        };
        final VideoTexture videoTexture = new VideoTexture(mViroView.getViroContext(), Uri.parse("file:///android_asset/stereoVid360.mp4"),
                delegate, Texture.StereoMode.TOP_BOTTOM);
    }

    private List<Node> testARPlane(final ARScene arScene) {
        final ARPlane arPlane = new ARPlane(0, 0);
        final Node node = new Node();
        final Surface surface = new Surface(.5f, .5f, 0, 0, 1, 1);

        final float[] rotation = {-90, 0, 0};
        node.setRotation(new Vector(rotation));
        mARNodeDelegate = new ARNode.ARNodeDelegate() {
            @Override
            public void onAnchorFound(final ARAnchor anchor) {
                Log.i("ViroActivity", "onAnchorFound");
            }

            @Override
            public void onAnchorUpdated(final ARAnchor anchor) {
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

        node.setGeometry(surface);
        arPlane.addChildNode(node);
        arScene.addARPlane(arPlane);

        final ArrayList<Node> list = new ArrayList<>();
        list.add(arPlane);
        return list;

    }

    private List<Node> testARDrag() {
        final Node node = new Node();
        final Node boxNode = new Node();
        final Box box = new Box(.15f, .15f, .15f);

        final EventDelegate delegate = getGenericDelegate("boxNode");

        delegate.setEventEnabled(EventDelegate.EventAction.ON_DRAG, true);
        delegate.setEventEnabled(EventDelegate.EventAction.ON_ROTATE, true);
        delegate.setEventEnabled(EventDelegate.EventAction.ON_PINCH, true);
        delegate.setEventDelegateCallback(new ARDragDelegateCallback("boxNode", boxNode));
        node.setEventDelegate(delegate);
        node.setDragType(Node.DragType.FIXED_TO_WORLD);

        final float[] nodePos = {0, 0, -1};
        node.setPosition(new Vector(nodePos));

        final float[] boxPos = {0, .075f, 0};
        boxNode.setPosition(new Vector(boxPos));

        boxNode.setGeometry(box);
        node.addChildNode(boxNode);

        return Arrays.asList(node);
    }

    private void addNormalSound(final String path) {
        final String key = path + SOUND_COUNT++;
        mSoundMap.put(key, new Sound(mViroView.getViroContext(), Uri.parse(path), new Sound.Delegate() {
            @Override
            public void onSoundReady(final Sound sound) {
                Log.i("NormalSound", "ViroActivity sound is ready! is paused? " + sound.isPaused());
                sound.setLoop(true);
                sound.play();
            }

            @Override
            public void onSoundFinish(final Sound sound) {
                Log.i("Viro", "ViroActivity sound has finished!");
            }

            @Override
            public void onSoundFail(final String error) {
                Log.i("Viro", "Sound fail with error " + error);
            }
        }));
    }

    private void addNormalSound(final SoundData data) {
        final String key = "" + SOUND_COUNT++;
        mSoundMap.put(key, new Sound(data,
                mViroView.getViroContext(), new Sound.Delegate() {
            @Override
            public void onSoundReady(final Sound sound) {
                Log.i("NormalSound", "ViroActivity sound is ready! is paused? " + sound.isPaused());
                sound.play();
            }

            @Override
            public void onSoundFinish(final Sound sound) {
                Log.i("NormalSound", "ViroActivity sound has finished!");
            }

            @Override
            public void onSoundFail(final String error) {

            }
        }));
    }

    private void addSoundField(final String path) {
        final String key = path + SOUND_COUNT++;
        mSoundFieldMap.put(key, new SoundField(
                mViroView.getViroContext(), Uri.parse(path), new SoundField.Delegate() {
            @Override
            public void onSoundReady(final SoundField sound) {
                Log.i("SoundField", "ViroActivity sound is ready!");
                sound.play();
                sound.setLoop(true);
            }

            @Override
            public void onSoundFail(final String error) {
                Log.i("SoundField", "Failed to load sound: " + error);
            }
        }));

        final float[] rotation = {0, 0, 90};
        mSoundFieldMap.get(key).setRotation(new Vector(rotation));
    }

    private void addSpatialSound(final String path) {
        final String key = path + SOUND_COUNT++;
        mSpatialSoundMap.put(key, new SpatialSound(mViroView.getViroContext(), Uri.parse(path), new SpatialSound.Delegate() {
            @Override
            public void onSoundReady(final SpatialSound sound) {
                Log.i("SpatialSound", "ViroActivity sound is ready!");

                final float[] position = {5, 0, 0};
                sound.setPosition(new Vector(position));
                sound.play();
            }

            @Override
            public void onSoundFail(final String error) {

            }
        }));
    }

    private void addSpatialSound(final SoundData data) {
        final String key = "" + SOUND_COUNT++;
        mSpatialSoundMap.put(key, new SpatialSound(data,
                mViroView.getViroContext(), new SpatialSound.Delegate() {
            @Override
            public void onSoundReady(final SpatialSound sound) {
                final float[] position = {5, 0, 0};
                sound.setPosition(new Vector(position));
                sound.play();
            }

            @Override
            public void onSoundFail(final String error) {

            }
        }));
    }

    private void setSoundRoom(final Scene scene, final ViroContext viroContextJni) {
        final float[] size = {15, 15, 15};
        scene.setSoundRoom(viroContextJni, new Vector(size), Scene.AudioMaterial.BRICK_BARE, Scene.AudioMaterial.MARBLE,
                Scene.AudioMaterial.BRICK_BARE);
    }

    private Node testLine(final Context scene) {
        final Material material = new Material();
        material.setDiffuseColor(Color.RED);
        material.setLightingModel(Material.LightingModel.CONSTANT);
        material.setCullMode(Material.CullMode.NONE);

        final float[] linePos = {0, 0, -2};
        final float[][] points = {{0, 0}, {1, 0}, {1, 1}};

        final Polyline polyline = new Polyline(points, 0.1f);
        final Node node1 = new Node();

        node1.setPosition(new Vector(linePos));
        node1.setGeometry(polyline);
        polyline.setMaterials(Arrays.asList(material));
        node1.setEventDelegate(getGenericDelegate("Line"));

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                polyline.appendPoint(new Vector(-1, 1, 0));
            }
        }, 2000);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                final List<Vector> newPoints = new ArrayList<>();
                newPoints.add(new Vector(0, 0, 0));
                newPoints.add(new Vector(1, 1, 0));

                polyline.setPoints(newPoints);
            }
        }, 4000);

        return node1;
    }

    private EventDelegate getGenericDelegate(final String delegateTag){
        final EventDelegate delegateJni = new EventDelegate();
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_HOVER, false);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_FUSE, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_DRAG, true);
        delegateJni.setEventEnabled(EventDelegate.EventAction.ON_CLICK, true);
        delegateJni.setEventDelegateCallback(new GenericEventCallback(delegateTag));

        return delegateJni;
    }

    private class ARDragDelegateCallback extends GenericEventCallback {

        private final Node mNode;

        private float[] mStartScale;

        private float mYRotation = 0;

        public ARDragDelegateCallback(final String tag, final Node node) {
            super(tag);
            mNode = node;
        }

        @Override
        public void onPinch(final int source, final Node node, final float scaleFactor, final PinchState pinchState) {
            if (pinchState == PinchState.PINCH_START) {
                if (mStartScale == null) {
                    final float[] scale = {1, 1, 1};
                    mStartScale = scale;
                }
            } else if (pinchState == PinchState.PINCH_END) {
                for (int i = 0; i < 3; i++) {
                    mStartScale[i] = mStartScale[i] * scaleFactor;
                }
            } else {
                final float[] newScale = {0, 0, 0};
                for (int i = 0; i < 3; i++) {
                    newScale[i] = mStartScale[i] * scaleFactor;
                }
                mNode.setScale(new Vector(newScale));
            }
        }

        @Override
        public void onRotate(final int source, final Node node, final float rotateFactor, final RotateState rotateState) {
            if (rotateState == RotateState.ROTATE_MOVE) {
                final float[] newRotation = {0, mYRotation - rotateFactor, 0};
                mNode.setRotation(new Vector(newRotation));
            } else if (rotateState == RotateState.ROTATE_END) {
                mYRotation = mYRotation - rotateFactor;
            }
        }
    }

    private class GenericEventCallback implements EventDelegate.EventDelegateCallback {
        protected final String delegateTag;

        public GenericEventCallback(final String tag) {
            delegateTag = tag;
        }

        @Override
        public void onHover(final int source, final Node node, final boolean isHovering, final float[] hitLoc) {
            Log.e(TAG, delegateTag + " onHover " + isHovering);
        }

        @Override
        public void onClick(final int source, final Node node, final ClickState clickState, final float[] hitLoc) {
            Log.e(TAG, delegateTag + " onClick " + clickState.toString() + " location " +
                    hitLoc[0] + ", " + hitLoc[1] + ", " + hitLoc[2]);
        }

        @Override
        public void onTouch(final int source, final Node node, final TouchState touchState, final float[] touchPadPos) {
            Log.e(TAG, delegateTag + "onTouch " + touchPadPos[0] + "," + touchPadPos[1]);
        }

        @Override
        public void onControllerStatus(final int source, final ControllerStatus status) {

        }

        @Override
        public void onSwipe(final int source, final Node node, final SwipeState swipeState) {
            Log.e(TAG, delegateTag + " onSwipe " + swipeState.toString());
        }

        @Override
        public void onScroll(final int source, final Node node, final float x, final float y) {
            Log.e(TAG, delegateTag + " onScroll " + x + "," +y);

        }

        @Override
        public void onDrag(final int source, final Node node, final float x, final float y, final float z) {
            Log.e(TAG, delegateTag +" On drag: " + x + ", " + y + ", " + z);

            Vector converted = node.convertLocalPositionToWorldSpace(new Vector(x, y, z));
            if (node.getParentNode() != null) {
                converted = node.getParentNode().convertLocalPositionToWorldSpace(new Vector(x, y, z));
                Log.e(TAG, delegateTag + " On CONV: " + converted.x + ", " + converted.y + ", " + converted.z);
            }
        }

        @Override
        public void onFuse(final int source, final Node node) {
            Log.e(TAG, delegateTag + " On fuse");
        }

        @Override
        public void onPinch(final int source, final Node node, final float scaleFactor, final PinchState pinchState) {
            Log.e(TAG, delegateTag + " On pinch");
        }

        @Override
        public void onRotate(final int source, final Node node, final float rotateFactor, final RotateState rotateState) {
            Log.e(TAG, delegateTag + " On rotate");
        }

        @Override
        public void onCameraARHitTest(final int source, final ARHitTestResult[] results) {
            Log.e(TAG, delegateTag + " On Camera AR Hit Test");
        }
    }
}
