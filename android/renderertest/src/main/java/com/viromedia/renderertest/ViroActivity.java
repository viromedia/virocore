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
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Point;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.Toast;

import com.viro.core.ARAnchor;
import com.viro.core.ARHitTestListener;
import com.viro.core.ARImageTarget;
import com.viro.core.ARPointCloud;
import com.viro.core.BoundingBox;
import com.viro.core.CameraImageListener;
import com.viro.core.CameraIntrinsics;
import com.viro.core.ClickListener;
import com.viro.core.DragListener;
import com.viro.core.Quaternion;
import com.viro.core.RendererConfiguration;
import com.viro.core.ViroMediaRecorder;
import com.viro.core.ViroViewScene;
import com.viro.core.internal.ARDeclarativeNode;
import com.viro.core.internal.ARDeclarativePlane;
import com.viro.core.ARHitTestResult;
import com.viro.core.ARNode;
import com.viro.core.ARPlaneAnchor;
import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.Animation;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.Box;
import com.viro.core.Camera;
import com.viro.core.Controller;
import com.viro.core.DirectionalLight;
import com.viro.core.EventDelegate;
import com.viro.core.internal.Image;
import com.viro.core.internal.ImageTracker;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.OmniLight;
import com.viro.core.internal.ImageTrackerOutput;
import com.viro.core.internal.OpenCV;
import com.viro.core.ParticleEmitter;
import com.viro.core.Polyline;
import com.viro.core.Scene;
import com.viro.core.Sound;
import com.viro.core.SoundData;
import com.viro.core.SoundField;
import com.viro.core.SpatialSound;
import com.viro.core.Sphere;
import com.viro.core.Spotlight;
import com.viro.core.Surface;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Texture.Format;
import com.viro.core.Vector;
import com.viro.core.VideoTexture;
import com.viro.core.ViroContext;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import com.viro.core.ViroViewGVR;
import com.viro.core.ViroViewOVR;
import com.viro.core.ClickState;
import com.viro.core.ControllerStatus;
import com.viro.core.PinchState;
import com.viro.core.RotateState;
import com.viro.core.SwipeState;
import com.viro.core.TouchState;

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
import com.crashlytics.android.Crashlytics;

import io.fabric.sdk.android.Fabric;

public class ViroActivity extends AppCompatActivity {
    private static final String TAG = ViroActivity.class.getSimpleName();
    private static int SOUND_COUNT = 0;
    private final Map<String, Sound> mSoundMap = new HashMap<>();
    private final Map<String, SoundField> mSoundFieldMap = new HashMap();
    private final Map<String, SpatialSound> mSpatialSoundMap = new HashMap<>();
    private ARDeclarativeNode.Delegate mARNodeDelegate;
    private ViroView mViroView = null;
    private Handler mHandler;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Fabric.with(this, new Crashlytics());
        mHandler = new Handler(getMainLooper());

        RendererConfiguration config = new RendererConfiguration();
        config.setShadowsEnabled(true);
        config.setBloomEnabled(true);
        config.setHDREnabled(true);
        config.setPBREnabled(true);

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = new ViroViewGVR(this, new ViroViewGVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewGVR.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, new Runnable() {
                @Override
                public void run() {
                    Log.e(TAG, "On GVR userRequested exit");
                }
            }, config);
            setContentView(mViroView);

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = new ViroViewOVR(this, new ViroViewOVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewOVR.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, config);
            setContentView(mViroView);

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
            mViroView = new ViroViewScene(this, new ViroViewScene.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewScene.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, config);


            mViroView.setPadding(60,60, 60,60);
            mViroView.setBackgroundColor(Color.argb(0, 0, 0, 0));

            FrameLayout frameLayout = new FrameLayout(this);
            frameLayout.addView(mViroView);
            frameLayout.setBackgroundColor(Color.BLUE);

            setContentView(frameLayout);

        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = new ViroViewARCore(this, new ViroViewARCore.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {
                    onRendererFailed(error.toString(), errorMessage);
                }
            }, config);
            setContentView(mViroView);

            ViroViewARCore arView = (ViroViewARCore) mViroView;
            Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
            arView.setCameraRotation(display.getRotation());
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (mViroView instanceof ViroViewARCore) {
            ViroViewARCore arView = (ViroViewARCore) mViroView;
            Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
            arView.setCameraRotation(display.getRotation());
        }
    }

    @Override
    protected void onStart(){
        super.onStart();
        if (mViroView != null){
            mViroView.onActivityStarted(this);
        }
    }

    @Override
    protected void onResume(){
        super.onResume();
        if (mViroView != null){
            mViroView.onActivityResumed(this);
        }
    }

    @Override
    protected void onPause(){
        super.onPause();
        if (mViroView != null){
            mViroView.onActivityPaused(this);
        }
    }

    @Override
    protected void onStop(){
        super.onStop();

        if (mViroView != null){
            mViroView.onActivityStopped(this);
        }
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();

        if (mViroView != null){
            mViroView.onActivityDestroyed(this);
        }
    }

    private void onRendererStart() {
        Log.e("ViroActivity", "onRendererStart called");

        mViroView.setVRModeEnabled(true);
        mViroView.setDebugHUDEnabled(true);
        mViroView.validateAPIKey("7EEDCB99-2C3B-4681-AE17-17BC165BF792");

        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            initializeArScene();
        }
        else {
            initializeVrScene();
        }

        // Uncomment to run a test screenshot through the OpenCV target tracking code
        //testFindInScreenshot();
    }

    private void onRendererFailed(String error, String errorMessage) {
        Log.e("ViroActivity", "onRendererFailed [error: " + error + "], message [" + errorMessage + "]");
        Toast.makeText(this, errorMessage, Toast.LENGTH_LONG).show();
    }

    private void initializeVrScene() {
        // Creation of SceneControllerJni within scene navigator
        final Scene scene = new Scene();
        final Node rootNode = scene.getRootNode();
        List<Node> nodes = new ArrayList<>();
        nodes = testBox(getApplicationContext());
        //testBackgroundVideo(scene);

        //nodes.add(testLine(this));
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
        final Controller nativeController = mViroView.getController();
        //nativeController.setReticleVisibility(false);
    }

    /*
     Used to initialize the AR Scene, should also change the mVrView to the AR one...
     */
    private void initializeArScene() {
        final ARScene scene = new ARScene();
        final Node rootNode = scene.getRootNode();

        ((ViroViewARCore)mViroView).setCameraAutoFocusEnabled(true);

        final List<Node> nodes = new ArrayList<>();
        //nodes.addAll(testBox(this));
        //testBackgroundImage(scene);
        testAugmentedImageDatabase(scene);

        // Add a box in front of the user

        Box box = new Box(.2f, .2f, .2f);
        box.setMaterials(Arrays.asList(new Material()));
        Node boxNode = new Node();
        boxNode.setGeometry(box);
        boxNode.setPosition(new Vector(0, 0, -1));
        nodes.add(boxNode);
        boxNode.setDragListener(new DragListener() {
            @Override
            public void onDrag(int source, Node node, Vector worldLocation, Vector localLocation) {

            }
        });
        boxNode.setDragType(Node.DragType.FIXED_TO_PLANE);
        boxNode.setDragPlanePoint(new Vector(0,-1,0));
        boxNode.setDragPlaneNormal(new Vector(0,1,0));
        boxNode.setDragMaxDistance(5);

        //nodes.addAll(testImperativePlane(scene));
        //testARHitTest(scene, 0, 5);
        //testAddArbitraryAnchors(scene, 0, 5);
        //testHostCloudAnchors(scene, 0, 2);
        //testResolveCloudAnchor(scene, "ua-ceb1cbf2825bf3545b4ef91650fbeff2");
        //nodes.addAll(testARImageTarget(scene));
        //testCameraImageCapture(scene);

        for (final Node node : nodes) {
            rootNode.addChildNode(node);
        }
        testSceneLighting(rootNode);

        // Updating the scene.
        mViroView.setScene(scene);

        //testVideoRecording();
        //scene.displayPointCloud(true);
    }

    private void testAugmentedImageDatabase(final ARScene scene) {
        try {
            Uri databaseUrl = Uri.parse("file:///android_asset/mi_posters.imgdb");

            scene.loadARImageDatabase(databaseUrl, new ARScene.LoadARImageDatabaseListener() {
                @Override
                public void onSuccess() {

                }

                @Override
                public void onFailure(String error) {

                }
            });
        } catch(Exception e) {
            // do nothing.
        }

        scene.setListener(new ARScene.Listener() {
            @Override
            public void onTrackingInitialized() {

            }

            @Override
            public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {

            }

            @Override
            public void onAmbientLightUpdate(float intensity, Vector color) {

            }

            @Override
            public void onAnchorFound(ARAnchor anchor, ARNode arNode) {
                if (anchor.getType() == ARAnchor.Type.IMAGE) {
                    Log.d("ViroActivity", "found image: " + anchor.getAnchorId());
                }
            }

            @Override
            public void onAnchorUpdated(ARAnchor anchor, ARNode arNode) {

            }

            @Override
            public void onAnchorRemoved(ARAnchor anchor, ARNode arNode) {

            }
        });

        (new Handler()).postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.d("ViroActivity"," posting delayed!");
                scene.unloadARImageDatabase();
            }
        }, 10000);

        Bitmap bitmap = null;
        try {
            InputStream istr = getAssets().open("boba.png");
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (IOException e) {
            // handle exception
        }

        ARImageTarget target = new ARImageTarget(bitmap, ARImageTarget.Orientation.Up, .1f);
        scene.addARImageTarget(target);
    }

    private void testEdgeDetect() {
        final OpenCV cv = new OpenCV(this);
        final Bitmap bm = cv.edgeDetectImage("boba.png");
        final ImageView image = new ImageView(this);
        image.setImageBitmap(bm);
        setContentView(image);
    }

    /*
     This function runs the same test as VROTrackingHelper's findInScreenshot on iOS
     */
    private void testFindInScreenshot() {
        final Bitmap targetImage = getBitmapFromAssets("ben.jpg");
        final Bitmap screenshot = getBitmapFromAssets("screenshot.png");
        if (targetImage != null && screenshot != null) {
            // width of a US bill is 6.14 inches ~= .156 meters
            final ARImageTarget arImageTarget = new ARImageTarget(targetImage, ARImageTarget.Orientation.Up, .156f);
            final ImageTracker tracker = new ImageTracker(arImageTarget);
            ImageTrackerOutput output = tracker.findTarget(screenshot);
            if (output.found()) {
                for (float f : output.corners()) {
                    Log.i("ImageTracker", "corners are: " + f);
                }
                float[] position = output.position();
                Log.i("ImageTracker", "position is: " + position[0] + ", " + position[1] + ", " + position[2]);
                float[] rotation = output.rotation();
                Log.i("ImageTracker", "rotation is: " + rotation[0] + ", " + rotation[1] + ", " + rotation[2]);
            }
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
            throw new IllegalArgumentException("Loading bitmap failed!", e);
        }
        return bitmap;
    }

    private List<Node> testText() {
        final Node node = new Node();
        // Create text
        final Text text = new Text(mViroView.getViroContext(),
                "Test Text Here", "Roboto", 25, Color.WHITE, 1f,
                1f, Text.HorizontalAlignment.LEFT, Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        final float[] position = {0, -0.5f, -0.5f};
        node.setPosition(new Vector(position));
        node.setGeometry(text);

        final Node pointOfView = new Node();
        final Camera camera = new Camera();
        camera.setPosition(new Vector(0, 0, 3));
        pointOfView.setCamera(camera);
        mViroView.setPointOfView(pointOfView);

        return Arrays.asList(node);
    }

    private void testSceneLighting(final Node node) {
        final float[] lightDirection = {0, -1, 0};
        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 300.0f);
        node.addLight(ambientLightJni);

        final DirectionalLight directional = new DirectionalLight(Color.BLUE, 1000.0f, new Vector(lightDirection));
        directional.setCastsShadow(true);
        directional.setShadowOrthographicSize(10);
        node.addLight(directional);

        final float[] omniLightPosition = {1, 0, 0};
        final OmniLight omni = new OmniLight(Color.WHITE, 1000.0f, 1, 10, new Vector(omniLightPosition));
        node.addLight(omni);
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

    static class TestCameraImageListener implements CameraImageListener {
        @Override
        public void onCameraImageUpdated(ByteBuffer buffer, int width, int height, CameraIntrinsics intrinsics) {
            Log.i("Viro", "Received image with buffer size " + buffer.capacity() + ", limit " + buffer.limit() + ", remaining " + buffer.remaining());
            Log.i("Viro", "Focal Length" + intrinsics.getFocalLength()[0] + ", " + intrinsics.getFocalLength()[1] + "; Principal Point " + intrinsics.getPrincipalPoint()[0] + ", " + intrinsics.getPrincipalPoint()[1]);

            Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            bitmap.copyPixelsFromBuffer(buffer);

            //ViroViewARCore.setBitmapOnTrackingImageView(bitmap);
        }
    }

    private void testCameraImageCapture(final ARScene scene) {
        ((ViroViewARCore) mViroView).setCameraImageListener(mViroView.getViroContext(), new TestCameraImageListener());
        //mHandler.postDelayed(new Runnable() {
        //    @Override
        //    public void run() {
        //         ((ViroViewARCore) mViroView).setCameraImageListener(mViroView.getViroContext(), null);
        //    }
        //}, 5000);
    }

    private void testBackgroundImage(final Scene scene) {
        final Image imageJni = new Image("boba.png", Format.RGBA8);
        final Texture videoTexture = new Texture(imageJni, true, false);
        scene.setBackgroundTexture(videoTexture);
        final float[] rotation = {90, 0, 0};
        scene.setBackgroundRotation(new Vector(rotation));
    }

    private void testSkyBoxImage(final Scene scene) {
        final Format format = Format.RGBA8;

        final Image pximageJni = new Image("px.png", format);
        final Image nximageJni = new Image("nx.png", format);
        final Image pyimageJni = new Image("py.png", format);
        final Image nyimageJni = new Image("ny.png", format);
        final Image pzimageJni = new Image("pz.png", format);
        final Image nzimageJni = new Image("nz.png", format);

        final Texture cubeTexture = new Texture(pximageJni, nximageJni, pyimageJni, nyimageJni,
                pzimageJni, nzimageJni);

        scene.setBackgroundCubeTexture(cubeTexture);
    }

    private List<Node> testParticles() {
        final Bitmap bobaBitmap = getBitmapFromAssets("boba.png");
        final Bitmap specBitmap = getBitmapFromAssets("specular.png");
        final Texture bobaTexture = new Texture(bobaBitmap, Format.RGBA8, true, true);

        final Node particleNode = new Node();
        particleNode.setPosition(new Vector(0, -10, -15));

        final Material material = new Material();
        material.setDiffuseTexture(bobaTexture);
        material.setDiffuseColor(Color.BLUE);
        material.setSpecularTexture(bobaTexture);
        material.setLightingModel(Material.LightingModel.LAMBERT);

        final Surface surface = new Surface(1, 1);
        surface.setMaterials(Arrays.asList(material));

        final ParticleEmitter particleEmitter = new ParticleEmitter(mViroView.getViroContext(), surface);

        final Vector sizeMinStart = new Vector(2, 2, 2);
        final Vector sizeMaxStart = new Vector(5, 5, 5);
        final ParticleEmitter.ParticleModifierVector modifier = new ParticleEmitter.ParticleModifierVector(sizeMinStart, sizeMaxStart);
        modifier.addInterval(1000, new Vector(0, 0, 0));

        particleEmitter.setScaleModifier(modifier);
        particleNode.setParticleEmitter(particleEmitter);
        particleEmitter.run();
        return Arrays.asList(particleNode);
    }

    private ByteBuffer getRBGAFromBitmap(Bitmap bitmap) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int componentsPerPixel = 4;
        int totalPixels = width * height;
        int totalBytes = totalPixels * componentsPerPixel;

        byte[] rgbValues = new byte[totalBytes];
        int[] argbPixels = new int[totalPixels];
        bitmap.getPixels(argbPixels, 0, width, 0, 0, width, height);
        for (int i = 0; i < totalPixels; i++) {
            int argbPixel = argbPixels[i];
            int red = Color.red(argbPixel);
            int green = Color.green(argbPixel);
            int blue = Color.blue(argbPixel);
            int alpha = Color.alpha(argbPixel);
            rgbValues[i * componentsPerPixel + 0] = (byte) red;
            rgbValues[i * componentsPerPixel + 1] = (byte) green;
            rgbValues[i * componentsPerPixel + 2] = (byte) blue;
            rgbValues[i * componentsPerPixel + 3] = (byte) alpha;
        }

        ByteBuffer buffer = ByteBuffer.allocateDirect(rgbValues.length);
        buffer.put(rgbValues);
        buffer.flip();
        return buffer;
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

        //final Texture bobaTexture = new Texture(bobaBitmap, TextureFormat.RGBA8, true, true);
        final Texture bobaTexture = new Texture(getRBGAFromBitmap(bobaBitmap), bobaBitmap.getWidth(), bobaBitmap.getHeight(), Format.RGBA8, Format.RGBA8,
        true, false, null);
        final Texture specTexture = new Texture(specBitmap, Format.RGBA8, true, true);

        final Material material = new Material();
        material.setDiffuseTexture(bobaTexture);
        material.setSpecularTexture(specTexture);
        material.setLightingModel(Material.LightingModel.PHYSICALLY_BASED);
        //material.setBloomThreshold(0.2f);

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
        node2.setClickListener(new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                Log.i("Viro", "ON CLICK");
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {
                Log.i("Viro", "on click state " + clickState.toString());
            }
        });

        // Light for the box
        Spotlight light = new Spotlight();
        light.setDirection(new Vector(0, 0, -1));
        light.setPosition(new Vector(0, 0, 5));
        light.setIntensity(1500);
        node1.addLight(light);

        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDelay(1000);
        AnimationTransaction.setAnimationDuration(5000);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.Bounce);
        AnimationTransaction.setListener(new AnimationTransaction.Listener() {
            @Override
            public void onFinish(final AnimationTransaction transaction) {
                Log.i("Viro", "Animation finished");
            }
        });
        node2.setPosition(new Vector(-2, 2.5f, -3));

        final AnimationTransaction transaction = AnimationTransaction.commit();

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                BoundingBox box = node1.getBoundingBox();
                Log.d("Viro", "bounding box for [2,4,2] box is: " + box.minX + ", " + box.maxX + ", " + box.minY
                        + ", " + box.maxY + ", " + box.minZ + ", " + box.maxZ);
            }
        }, 1000);

        return Arrays.asList(node1, node2, node3);
    }

    private List<Node> test3dObjectLoading(final Context context) {
        final Node node1 = new Node();

        // Creation of ObjectJni to the right
        final Object3D objectJni = new Object3D();
        objectJni.loadModel(mViroView.getViroContext(),
                Uri.parse("file:///android_asset/object_star_anim.vrx"),
                Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -3));
                object.setScale(new Vector(0.4f, 0.4f, 0.4f));

                final Animation animation = object.getAnimation("02_spin");
                animation.setDelay(5000);
                animation.setLoop(true);
                animation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });
        node1.addChildNode(objectJni);
        return Arrays.asList(node1);
    }

    private List<Node> testImageSurface(final Context context) {
        final Node node = new Node();
        final Image bobaImage = new Image("boba.png", Format.RGBA8);

        final Texture bobaTexture = new Texture(bobaImage, true, true);
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
        final Image bobaImage = new Image(img, Format.RGBA8);
        final Texture bobaTexture = new Texture(bobaImage, true, true, "LeftRight");
        final Material material = new Material();
        final Surface surface = new Surface(1, 1, 0, 0, 1, 1);
        surface.setMaterial(material);
        surface.setImageTexture(bobaTexture);
        node.setGeometry(surface);
        node.setPosition(new Vector(pos));
        return node;
    }

    private void testStereoBackgroundImage(final Scene scene) {
        final Image imageJni = new Image("stereo3601.jpg", Format.RGBA8);
        final Texture videoTexture = new Texture(imageJni, true, false, "TopBottom");
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
        final VideoTexture.PlaybackListener delegate = new VideoTexture.PlaybackListener() {
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
        final ARDeclarativePlane arPlane = new ARDeclarativePlane(0, 0, ARPlaneAnchor.Alignment.HORIZONTAL);
        final Node node = new Node();
        final Surface surface = new Surface(.5f, .5f, 0, 0, 1, 1);

        final float[] rotation = { (float) -Math.PI / 2, 0, 0};
        node.setRotation(new Vector(rotation));

        mARNodeDelegate = new ARDeclarativeNode.Delegate() {
            @Override
            public void onAnchorFound(final ARAnchor anchor) {
                Log.i("ViroActivity", "onAnchorFound");
            }

            @Override
            public void onAnchorUpdated(final ARAnchor anchor) {
                final ARPlaneAnchor planeAnchor = (ARPlaneAnchor) anchor;
                Log.i("ViroActivity", "onAnchorUpdated width " + planeAnchor.getExtent().x + ", " + planeAnchor.getExtent().z);
                surface.setWidth(planeAnchor.getExtent().x);
                surface.setHeight(planeAnchor.getExtent().z);
            }

            @Override
            public void onAnchorRemoved() {
                Log.i("ViroActivity", "onAnchorRemoved");
            }
        };
        arPlane.setDelegate(mARNodeDelegate);

        node.setGeometry(surface);
        arPlane.addChildNode(node);
        arScene.addARDeclarativeNode(arPlane);

        final ArrayList<Node> list = new ArrayList<>();
        list.add(arPlane);
        return list;
    }

    private Object3D loadObjectNode(final int bitmask, final float scale, final Vector position) {
        final Object3D objectNode = new Object3D();
        objectNode.loadModel(mViroView.getViroContext(),
                Uri.parse("file:///android_asset/object_star_anim.vrx"),
                Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(position);
                object.setScale(new Vector(scale, scale, scale));
                object.setLightReceivingBitMask(bitmask);
                object.setShadowCastingBitMask(bitmask);

                final Animation animation = object.getAnimation("02_spin");
                //animation.setDelay(5000);
                animation.setLoop(true);
                animation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });
        return objectNode;
    }

    private List<ARNode> anchoredNodes = new ArrayList<ARNode>();

    private void testARHitTest(final ARScene scene, final int count, final int total) {
        // Once count hits total, clear all the anchors we've found
        if (count >= total) {
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    Log.i("Viro", "Detaching all nodes");
                    for (ARNode node : anchoredNodes) {
                        node.detach();
                    }
                    anchoredNodes.clear();
                }}, 2000);
            return;
        }

        final ViroViewARCore view = (ViroViewARCore) mViroView;

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Point point = new Point((int) (view.getWidth() / 2.0), (int) (view.getHeight() / 2.0f));
                Log.i("Viro", "Performing AR hit test at point " + point.x + ", " + point.y);

                view.performARHitTest(point,
                        new ARHitTestListener() {
                            @Override
                            public void onHitTestFinished(ARHitTestResult[] results) {
                                Log.i("Viro", "Hit test complete with " + results.length + " results");

                                for (ARHitTestResult result : results) {
                                    if (result.getType() == ARHitTestResult.Type.FEATURE_POINT) {
                                        Log.i("Viro", "   Hit feature point, creating little star");
                                        ARNode node = result.createAnchoredNode();
                                        node.addChildNode(loadObjectNode(1, .1f, new Vector(0, 0, 0)));

                                        anchoredNodes.add(node);

                                    } else if (result.getType() == ARHitTestResult.Type.PLANE) {
                                        Log.i("Viro", "   Hit plane, creating big star");
                                        ARNode node = result.createAnchoredNode();
                                        node.addChildNode(loadObjectNode(1, .2f, new Vector(0, 0, 0)));

                                        anchoredNodes.add(node);
                                    }
                                }

                                testARHitTest(scene, count + 1, total);
                            }
                        });
            }
        }, 2000);
    }

    private Vector mAnchorPosition = new Vector(0, 0, -0.5);
    private Vector mAnchorRotation = new Vector(0, 0, 0);

    private void testAddArbitraryAnchors(final ARScene scene, final int count, final int total) {
        // Once count hits total, clear all the anchors we've found
        if (count >= total) {
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    Log.i("Viro", "Detaching all nodes");
                    for (ARNode node : anchoredNodes) {
                        node.detach();
                    }
                    anchoredNodes.clear();
                }}, 2000);
            return;
        }

        final ViroViewARCore view = (ViroViewARCore) mViroView;
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Point point = new Point((int) (view.getWidth() / 2.0), (int) (view.getHeight() / 2.0f));
                Log.i("Viro", "Adding arbitrary anchor at position " + mAnchorPosition + " with rotation "
                        + mAnchorRotation);

                    ARNode node = scene.createAnchoredNode(mAnchorPosition, new Quaternion(mAnchorRotation));
                    node.addChildNode(loadObjectNode(1, .1f, new Vector(0, 0, 0)));

                    anchoredNodes.add(node);

                    mAnchorPosition.x += 0.25f;
                    mAnchorPosition.z -= 0.25f;
                    mAnchorRotation.y += (Math.toRadians(30));

                    testAddArbitraryAnchors(scene, count + 1, total);
            }
        }, 2000);
    }

    private void testHostCloudAnchors(final ARScene scene, final int count, final int total) {
        if (count >= total) {
            return;
        }

        final ViroViewARCore view = (ViroViewARCore) mViroView;

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Point point = new Point((int) (view.getWidth() / 2.0), (int) (view.getHeight() / 2.0f));
                Log.i("Viro", "Performing AR hit test at point " + point.x + ", " + point.y);

                view.performARHitTest(point,
                        new ARHitTestListener() {
                            @Override
                            public void onHitTestFinished(ARHitTestResult[] results) {
                                Log.i("Viro", "Hit test complete with " + results.length + " results");

                                for (ARHitTestResult result : results) {
                                    Log.i("Viro", "   Successful hit, creating little star and hosting anchor");
                                    ARNode node = result.createAnchoredNode();
                                    node.addChildNode(loadObjectNode(1, .1f, new Vector(0, 0, 0)));

                                    scene.hostCloudAnchor(node.getAnchor(), new ARScene.CloudAnchorHostListener() {
                                        @Override
                                        public void onSuccess(ARAnchor anchor, ARNode arNode) {
                                            Log.i("Viro", "Host successful [Cloud ID: " + anchor.getCloudAnchorId() + "]: adding a big star to the right");
                                            arNode.addChildNode(loadObjectNode(1, 0.2f, new Vector(0.5f, 0, 0)));
                                        }

                                        @Override
                                        public void onFailure(String error) {
                                            Log.i("Viro", "Host failure: " + error);
                                        }
                                    });

                                    // We only care about hosting the first result's anchor for this test
                                    break;
                                }

                                testHostCloudAnchors(scene, count + 1, total);
                            }
                        });
            }
        }, 4000);
    }

    private void testResolveCloudAnchor(final ARScene scene, final String cloudAnchorId) {
        final ViroViewARCore view = (ViroViewARCore) mViroView;

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                scene.resolveCloudAnchor(cloudAnchorId, new ARScene.CloudAnchorResolveListener() {
                    @Override
                    public void onSuccess(final ARAnchor anchor, final ARNode arNode) {
                        Log.i("Viro", "Resolve successful [Cloud ID: " + anchor.getCloudAnchorId() + "]: adding star");
                        arNode.addChildNode(loadObjectNode(1, 0.1f, new Vector(0, 0, 0)));

                        mHandler.postDelayed(new Runnable() {
                            public void run() {
                                Log.i("Viro", "Detaching the resolved cloud anchor");
                                arNode.detach();
                            }
                        }, 16000);
                    }

                    @Override
                    public void onFailure(String error) {
                        Log.i("Viro", "Resolve failure: " + error);
                    }
                });
            }
        }, 4000);
    }

    private List<Node> testImperativePlane(final ARScene arScene) {
        EnumSet<ViroViewARCore.AnchorDetectionType> types = EnumSet.of(ViroViewARCore.AnchorDetectionType.PLANES_HORIZONTAL,
                ViroViewARCore.AnchorDetectionType.PLANES_VERTICAL);
        ((ViroViewARCore) mViroView).setAnchorDetectionTypes(types);

        arScene.setListener(new ARScene.Listener() {
            @Override
            public void onTrackingInitialized() {
            }

            @Override
            public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {
                Log.i("Viro","Tracking state " + state.toString() + " Reason : " + reason.toString());
            }

            @Override
            public void onAmbientLightUpdate(final float lightIntensity, Vector lightColor) {

            }

            @Override
            public void onAnchorFound(final ARAnchor anchor, final ARNode node) {
                Log.i("Viro", "ANCHOR FOUND [type: " + anchor.getType().getStringValue() + "]");

                if (anchor.getType() == ARAnchor.Type.PLANE) {
                    int bitmask = 4;
                    final Spotlight spot = new Spotlight(Color.RED, 1000.0f, 1,
                            10, new Vector(0, 4, 0),
                            new Vector(0, -1, 0), (float) Math.toRadians(2), (float) Math.toRadians(10));
                    spot.setInfluenceBitMask(bitmask);
                    spot.setCastsShadow(true);

                    node.addLight(spot);
                    node.addChildNode(loadObjectNode(bitmask | 1, .04f, new Vector(0, 0, 0)));
                    node.setDragListener(new DragListener() {
                        @Override
                        public void onDrag(int source, Node node, Vector worldLocation, Vector localLocation) {

                        }
                    });

                    Surface surface = new Surface(2, 2);
                    Material material = new Material();
                    material.setDiffuseColor(Color.WHITE);
                    material.setShadowMode(Material.ShadowMode.TRANSPARENT);
                    surface.setMaterials(Arrays.asList(material));

                    Node surfaceNode = new Node();
                    surfaceNode.setPosition(new Vector(0, -0.5f, 0));
                    surfaceNode.setRotation(new Vector((float) -Math.PI / 2, 0, 0));
                    surfaceNode.setGeometry(surface);
                    surfaceNode.setLightReceivingBitMask(bitmask | 1);
                    node.addChildNode(surfaceNode);
                }
            }

            @Override
            public void onAnchorUpdated(final ARAnchor anchor, final ARNode node) {
                //Log.i("Viro", "Node position is " + node.getPositionRealtime().x + ", " + node.getPositionRealtime().y + ", " + node.getPositionRealtime().z);

            }

            @Override
            public void onAnchorRemoved(final ARAnchor anchor, final ARNode node) {

            }
        });

        final ArrayList<Node> list = new ArrayList<>();
        return list;
    }

    private Node mImageMarkerTestNode;
    private ARImageTarget mARImageTarget;
    private Bitmap targetImage;

    private List<Node> testARImageTarget(final ARScene arScene) {
        Node boxNode = new Node();
        //Box box = new Box(.155956f, .001f, .066294f); // ben.jpg
        //Box box = new Box(.1905f, .001f, 0.244475f); // variety mag - tracker_assets/variety_magazine.jpg
        Box box = new Box(.19f, .001f, 0.28f); // black panther - tracker_assets/poster_computer.jpg
        boxNode.setGeometry(box);
        Material whiteTransparent = new Material();
        whiteTransparent.setDiffuseColor(0xaaffffff);
        box.setMaterials(Arrays.asList(whiteTransparent));

        targetImage = getBitmapFromAssets("tracker_assets/poster_computer.jpg");
        // store this in a field so it doesn't get destroyed before we can use it!
        mARImageTarget = new ARImageTarget(targetImage, ARImageTarget.Orientation.Up, .19f);
        arScene.addARImageTarget(mARImageTarget);

        // Add a 2nd tracker to test threading
        Bitmap tempImage = getBitmapFromAssets("tracker_assets/variety_magazine.jpg");
        final ARImageTarget varietyTarget = new ARImageTarget(tempImage, ARImageTarget.Orientation.Up, .19f);
//        arScene.addARImageTarget(varietyTarget);

        mImageMarkerTestNode = new Node();
        mImageMarkerTestNode.addChildNode(boxNode);
        mImageMarkerTestNode.setVisible(false);

        arScene.setListener(new ARScene.Listener() {
            @Override
            public void onTrackingInitialized() {

            }

            @Override
            public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {

            }

            @Override
            public void onAmbientLightUpdate(final float lightIntensity, Vector color) {

            }

            @Override
            public void onAnchorFound(final ARAnchor anchor, final ARNode node) {
                if (anchor.getAnchorId().equals(String.valueOf(mARImageTarget.hashCode()))) {
                    Log.d("ViroActivity", "onAnchorFound - testARImageTarget - id: " + anchor.getAnchorId() + ", " + anchor.getPosition());
                    mImageMarkerTestNode.setVisible(true);
                    mImageMarkerTestNode.setPosition(anchor.getPosition());
                    mImageMarkerTestNode.setRotation(anchor.getRotation());
                } else if (anchor.getAnchorId().equals(String.valueOf(varietyTarget.hashCode()))) {
                    Log.d("ViroActivity", "onAnchorFound the variety target!");
                }
            }

            @Override
            public void onAnchorUpdated(final ARAnchor anchor, final ARNode node) {
                if (anchor.getAnchorId().equals(String.valueOf(mARImageTarget.hashCode()))) {
                    Log.d("ViroActivity", "onAnchorUpdated - testARImageTarget - id: " + anchor.getAnchorId() + ", " + anchor.getPosition());
                    mImageMarkerTestNode.setPosition(anchor.getPosition());
                    mImageMarkerTestNode.setRotation(anchor.getRotation());
                } else if (anchor.getAnchorId().equals(String.valueOf(varietyTarget.hashCode()))) {
                    Log.d("ViroActivity", "onAnchorUpdated the variety target!");
                }
            }

            @Override
            public void onAnchorRemoved(final ARAnchor anchor, final ARNode node) {
                if (anchor.getAnchorId().equals(String.valueOf(mARImageTarget.hashCode()))) {
                    Log.d("ViroActivity", "onAnchorRemoved - testARImageTarget - id: " + anchor.getAnchorId() + ", " + anchor.getPosition());
                    mImageMarkerTestNode.setVisible(false);
                }
            }
        });

        final ArrayList<Node> list = new ArrayList<>();
        list.add(mImageMarkerTestNode);
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
        mSoundMap.put(key, new Sound(mViroView.getViroContext(), Uri.parse(path), new Sound.PlaybackListener() {
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
                mViroView.getViroContext(), new Sound.PlaybackListener() {
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
                mViroView.getViroContext(), Uri.parse(path), new SoundField.PlaybackListener() {
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

        final float[] rotation = {0, 0, (float) Math.PI / 2};
        mSoundFieldMap.get(key).setRotation(new Vector(rotation));
    }

    private void addSpatialSound(final String path) {
        final String key = path + SOUND_COUNT++;
        mSpatialSoundMap.put(key, new SpatialSound(mViroView.getViroContext(), Uri.parse(path), new SpatialSound.PlaybackListener() {
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
                mViroView.getViroContext(), new SpatialSound.PlaybackListener() {
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
        final float[][] points = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}};

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

    private void testVideoRecording() {
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.i("Viro", "STARTING RECORDING");
                mViroView.getRecorder().startRecordingAsync("file1", true, new ViroMediaRecorder.RecordingErrorListener() {
                    @Override
                    public void onRecordingFailed(ViroMediaRecorder.Error errorCode) {
                        Log.i("Viro", "Failed " + errorCode);
                    }
                });
            }
        }, 2000);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.i("Viro", "STOPPING RECORDING");

                mViroView.getRecorder().stopRecordingAsync(new ViroMediaRecorder.VideoRecordingFinishListener() {
                    @Override
                    public void onSuccess(String filePath) {
                        Log.i("Viro", "Wrote video to path " + filePath);
                    }

                    @Override
                    public void onError(ViroMediaRecorder.Error errorCode) {
                        Log.i("Viro", "Failed to stop video recorder (1): " + errorCode);
                    }
                });
            }
        }, 5000);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.i("Viro", "STARTING RECORDING (2)");

                mViroView.getRecorder().startRecordingAsync("file2", true, new ViroMediaRecorder.RecordingErrorListener() {
                    @Override
                    public void onRecordingFailed(ViroMediaRecorder.Error errorCode) {
                        Log.i("Viro", "Failed " + errorCode);
                    }
                });
            }
        }, 8000);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.i("Viro", "STOPPING RECORDING (2)");

                mViroView.getRecorder().stopRecordingAsync(new ViroMediaRecorder.VideoRecordingFinishListener() {
                    @Override
                    public void onSuccess(String filePath) {
                        Log.i("Viro", "Wrote video to path " + filePath);
                    }

                    @Override
                    public void onError(ViroMediaRecorder.Error errorCode) {
                        Log.i("Viro", "Failed to stop video recorder (2): " + errorCode);
                    }
                });
            }
        }, 11000);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.i("Viro", "TAKING SCREENSHOT");

                mViroView.getRecorder().takeScreenShotAsync("file3", true, new ViroMediaRecorder.ScreenshotFinishListener() {
                    @Override
                    public void onSuccess(Bitmap bitmap, String filePath) {
                        Log.i("Viro", "Wrote screenshot to path " + filePath);
                    }

                    @Override
                    public void onError(ViroMediaRecorder.Error errorCode) {
                        Log.i("Viro", "Failed to take screenshot: " + errorCode);
                    }
                });
            }
        }, 13000);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                Log.i("Viro", "TAKING SCREENSHOT (2)");

                mViroView.getRecorder().takeScreenShotAsync("file4", true, new ViroMediaRecorder.ScreenshotFinishListener() {
                    @Override
                    public void onSuccess(Bitmap bitmap, String filePath) {
                        Log.i("Viro", "Wrote screenshot (2) to path " + filePath);
                    }

                    @Override
                    public void onError(ViroMediaRecorder.Error errorCode) {
                        Log.i("Viro", "Failed to take screenshot (2): " + errorCode);
                    }
                });
            }
        }, 15000);
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
        public void onRotate(final int source, final Node node, final float rotationRadians, final RotateState rotateState) {
            if (rotateState == RotateState.ROTATE_MOVE) {
                final float[] newRotation = {0, mYRotation - rotationRadians, 0};
                mNode.setRotation(new Vector(newRotation));
            } else if (rotateState == RotateState.ROTATE_END) {
                mYRotation = mYRotation - rotationRadians;
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
        public void onRotate(final int source, final Node node, final float rotationRadians, final RotateState rotateState) {
            Log.e(TAG, delegateTag + " On rotate");
        }

        @Override
        public void onCameraARHitTest(ARHitTestResult[] results) {
            Log.e(TAG, delegateTag + " On CameraARHitTest");
        }

        @Override
        public void onARPointCloudUpdate(ARPointCloud pointCloud) {
            Log.e(TAG, delegateTag + " On ARPointCloudUpdate");
        }

        @Override
        public void onCameraTransformUpdate(float posX, float poxY, float posZ,
                                            float rotEulerX, float rotEulerY, float rotEulerZ,
                                            float forwardX, float forwardY, float forwardZ,
                                            float upX, float upY, float upZ) {
            Log.e(TAG, delegateTag + " On CameraTransformUpdate");
        }
    }
}
