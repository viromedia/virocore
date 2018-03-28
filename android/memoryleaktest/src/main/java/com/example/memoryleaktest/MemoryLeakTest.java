package com.example.memoryleaktest;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.content.Intent;

import com.viro.core.ARAnchor;
import com.viro.core.ARNode;
import com.viro.core.ARPlaneAnchor;
import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.Animation;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.Box;
import com.viro.core.ClickListener;
import com.viro.core.ClickState;
import com.viro.core.DirectionalLight;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.OmniLight;
import com.viro.core.ParticleEmitter;
import com.viro.core.Scene;
import com.viro.core.SpatialSound;
import com.viro.core.Sphere;
import com.viro.core.Spotlight;
import com.viro.core.Surface;
import com.viro.core.Text;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.VideoTexture;
import com.viro.core.ViroView;
import com.viro.core.ViroViewARCore;
import com.viro.core.ViroViewGVR;
import com.viro.core.ViroViewOVR;
import com.viro.core.ViroViewScene;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.List;

public class MemoryLeakTest extends AppCompatActivity {
    private static final String TAG = MemoryLeakTest.class.getSimpleName();

    private boolean mGLInitialized = false;
    private ViroView mViroView;
    private String mTestToRun;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        //new Exception().printStackTrace();
        super.onCreate(savedInstanceState);
        System.out.println("onCreate called");
        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = new ViroViewGVR(this, new ViroViewGVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewGVR.StartupError error, String errorMessage) {

                }
            }, new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "On GVR userRequested exit");
                }
            });
            mViroView.setVRModeEnabled(true);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = new ViroViewOVR(this, new ViroViewOVR.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewOVR.StartupError error, String errorMessage) {

                }
            });
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = new ViroViewARCore(this, new ViroViewARCore.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewARCore.StartupError error, String errorMessage) {

                }
            });
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
            mViroView = new ViroViewScene(this, new ViroViewScene.StartupListener() {
                @Override
                public void onSuccess() {
                    onRendererStart();
                }

                @Override
                public void onFailure(ViroViewScene.StartupError error, String errorMessage) {

                }
            });
        }

        Intent intent = getIntent();
        mTestToRun = intent.getStringExtra("TestToRun");
        mViroView.setVRModeEnabled(false);
        setContentView(mViroView);
        Log.i("MemoryLeakTest", "ViroViewGVR addr onCreate:" + mViroView.hashCode());
    }

    @Override
    protected void onStart() {
        super.onStart();
        mViroView.onActivityStarted(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mViroView.onActivityResumed(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        mViroView.onActivityPaused(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        mViroView.onActivityStopped(this);
    }

    @Override
    protected void onDestroy(){
        Log.i("MemoryLeakTest", "ViroViewGVR addr onDestroy:" + mViroView.hashCode());
        super.onDestroy();
        mViroView.onActivityDestroyed(this);
        Log.i(TAG, "MemoryLeakTest onDestroy called.");
    }


    public void onRendererStart() {
        mGLInitialized = true;
        Log.e("ViroActivity", "onRendererStart called");
        //initializeVrScene();
        initializeArScene();
    }

    public void initializeVrScene() {
        final Scene scene = new Scene();
        final Node rootNode = scene.getRootNode();
        List<Node> nodes = selectScene(scene);
        if (nodes != null){
            for (final Node node : nodes) {
                rootNode.addChildNode(node);
            }
        }

        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        scene.getRootNode().addLight(ambientLightJni);
        mViroView.setScene(scene);
    }

    public void initializeArScene() {
        final ARScene scene = new ARScene();
        final Node rootNode = scene.getRootNode();
        List<Node> nodes = selectScene(scene);
        if(nodes != null) {
            for (final Node node : nodes) {
                rootNode.addChildNode(node);
            }
        }

        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 1000.0f);
        scene.getRootNode().addLight(ambientLightJni);
        mViroView.setScene(scene);
    }

    private List<Node> selectScene(Scene scene) {
        //boxTest, sphereTest, videoTest, objectTest
        if(mTestToRun.equalsIgnoreCase("boxTest")) {
            return testBox(mViroView.getContext());
        } else if(mTestToRun.equalsIgnoreCase("sphereTest")) {
            return testSphereVideo(mViroView.getContext());
        } else if(mTestToRun.equalsIgnoreCase("videoTest")) {
            return testSurfaceVideo(mViroView.getContext());
        } else if(mTestToRun.equalsIgnoreCase("objectTest")) {
            return test3dObjectLoading(mViroView.getContext());
        } else if(mTestToRun.equalsIgnoreCase("particleTest")) {
            return testParticles();
        } else if(mTestToRun.equalsIgnoreCase("bgStereoVideoTest")) {
            testStereoBackgroundVideo(scene);
        } else if(mTestToRun.equalsIgnoreCase("lightTest")) {
             testSceneLighting(scene.getRootNode());
        } else if(mTestToRun.equalsIgnoreCase("eventTest")) {
            return testEventsClickListener();
        } else if(mTestToRun.equalsIgnoreCase("imageTest")) {
            return testSurfaceImage();
        } else if(mTestToRun.equalsIgnoreCase("audioTest")) {
            return testAudio();
        } else if(mTestToRun.equalsIgnoreCase("animTest")) {
            return testAnim();
        } else if(mTestToRun.equalsIgnoreCase("arSceneTest")) {
            testARScene((ARScene)scene);
        }

        return null;
    }

    private List<Node> testBox(final Context context) {
        final Node node1 = new Node();
        final Node node2 = new Node();

        final Node node3 = new Node();
        final Text textJni = new Text.TextBuilder()
                .viroContext(mViroView.getViroContext())
                .textString("Test text 1 2 3")
                .fontFamilyName("Roboto")
                .fontSize(24)
                .color(Color.WHITE)
                .width(10)
                .height(4)
                .horizontalAlignment(Text.HorizontalAlignment.CENTER)
                .verticalAlignment(Text.VerticalAlignment.CENTER)
                .clipMode(Text.ClipMode.CLIP_TO_BOUNDS)
                .maxLines(1)
                .build();

        final float[] position = {0, -1, -2};
        node3.setPosition(new Vector(position));
        node3.setGeometry(textJni);
        //node3.setEventDelegate(getGenericDelegate("Text"));
        final Bitmap bobaBitmap = getBitmapFromAssets("boba.png");
        final Bitmap specBitmap = getBitmapFromAssets("specular.png");

        final Texture bobaTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);
        final Texture specTexture = new Texture(specBitmap, Texture.Format.RGBA8, true, true);

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
        //node1.setEventDelegate(getGenericDelegate("Box"));

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
        Material mat = node2.getGeometry().getMaterials().get(0);
        mat.setDiffuseColor(Color.RED);

        final AnimationTransaction transaction = AnimationTransaction.commit();

        return Arrays.asList(node1, node2, node3);
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

    private List<Node> testSurfaceImage() {
        final Node node = new Node();
        final Surface surface = new Surface(4, 4, 0, 0, 1, 1);
        final float[] position = {0, 0, -3};
        node.setPosition(new Vector(position));
        final Bitmap bobaBitmap = getBitmapFromAssets("boba.png");
        final Texture bobaTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);
        final Material material = new Material();
        material.setDiffuseTexture(bobaTexture);
        surface.setMaterials(Arrays.asList(material));
        node.setGeometry(surface);
        return Arrays.asList(node);
    }

    private List<Node> test3dObjectLoading(final Context context) {
        final Node node1 = new Node();

        // Creation of ObjectJni to the right
        final Object3D objectJni = new Object3D();
        objectJni.loadModel(Uri.parse("file:///android_asset/dragao_2018.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -6));
                object.setScale(new Vector(0.4f, 0.4f, 0.4f));

                final Animation animation = object.getAnimation("01");
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


    private void testSceneLighting(final Node node) {
        final float[] lightDirection = {0, 0, -1};
        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 300.0f);
        node.addLight(ambientLightJni);

        final DirectionalLight directionalLightJni = new DirectionalLight(Color.BLUE, 1000.0f, new Vector(lightDirection));
        node.addLight(directionalLightJni);

        final float[] omniLightPosition = {1, 0, 0};
        final OmniLight omniLightJni = new OmniLight();
        omniLightJni.setColor(Color.RED);
        omniLightJni.setIntensity(1000.0f);
        omniLightJni.setAttenuationStartDistance(1);
        omniLightJni.setAttenuationEndDistance(10);
        omniLightJni.setPosition(new Vector(omniLightPosition));
        node.addLight(omniLightJni);

        final float[] spotLightPosition = {-2, 0, 3};

        final Spotlight spotLightJni = new Spotlight();
        spotLightJni.setColor(Color.YELLOW);
        spotLightJni.setIntensity(1000.0f);
        spotLightJni.setAttenuationStartDistance(1);
        spotLightJni.setAttenuationEndDistance(10);
        spotLightJni.setPosition(new Vector(spotLightPosition));
        spotLightJni.setDirection(new Vector(lightDirection));
        spotLightJni.setInnerAngle((float) Math.toRadians(2));
        spotLightJni.setOuterAngle((float) Math.toRadians(10));
        node.addLight(spotLightJni);

        Node nodeBox = new Node();
        Box box = new Box(1,1,1);

        Material material = new Material();
        final Bitmap bobaBitmap = getBitmapFromAssets("boba.png");
        final Texture bobaTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);
        material.setDiffuseTexture(bobaTexture);
        material.setLightingModel(Material.LightingModel.BLINN);
        box.setMaterials(Arrays.asList(material));
        nodeBox.setPosition(new Vector(0, 0, -3));
        nodeBox.setGeometry(box);
        node.addChildNode(nodeBox);
    }


    private List<Node> testParticles() {
        final Bitmap bobaBitmap = getBitmapFromAssets("boba.png");
        final Bitmap specBitmap = getBitmapFromAssets("specular.png");
        final Texture bobaTexture = new Texture(bobaBitmap, Texture.Format.RGBA8, true, true);

        final Node particleNode = new Node();
        particleNode.setPosition(new Vector(0, -1, -6));

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

    private List<Node> testAudio() {
        SpatialSound sound = new SpatialSound(mViroView.getViroContext(), Uri.parse("file:///android_asset/flies_mono.wav"), null);
        sound.setPosition(new Vector(-1, 0, 0));
        sound.setVolume(1.0f);
        sound.setDistanceRolloff(SpatialSound.Rolloff.LINEAR, 3, 5);
        sound.setLoop(true);

        sound.setPlaybackListener(new SpatialSound.PlaybackListener() {
            @Override
            public void onSoundReady(final SpatialSound sound) {
                sound.play();
            }

            @Override
            public void onSoundFail(final String error) {
            }
        });

        Node soundNode = new Node();
        soundNode.addSound(sound);
        return  Arrays.asList(soundNode);
    }

    private List<Node> testAnim() {
        Node boxNode = new Node();
        boxNode.setGeometry(new Box(1, 1, 1));
        boxNode.setPosition(new Vector(0, 0, -3));
        AnimationTransaction.begin();
        AnimationTransaction.setAnimationDuration(4500);
        AnimationTransaction.setTimingFunction(AnimationTimingFunction.EaseOut);
            boxNode.setRotation(new Vector(0, 0.78, 0.78));
        AnimationTransaction.commit();
        return Arrays.asList(boxNode);
    }


    private List<Node> testEventsClickListener() {
        Log.i("ViroEventsTest", "in testEventsClickListener()");

        Node boxNode = new Node();
        boxNode.setPosition(new Vector(2, 0, -3));
        boxNode.setGeometry(new Box(1, 1, 1));

        Node sphereNode = new Node();
        sphereNode.setPosition(new Vector(0, 0, -3));
        sphereNode.setGeometry(new Sphere(.5f));

        Object3D objectNode = new Object3D();
        objectNode.setPosition(new Vector(-2, 0, -3));
        objectNode.loadModel(Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, null);

        Node textNode = new Node();
        textNode.setPosition(new Vector(0, -1, -3));
        final Text eventText = new Text(mViroView.getViroContext(), "No event tapped.",
                "Roboto", 25, Color.WHITE, 5f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        textNode.setGeometry(eventText);

        boxNode.setClickListener(new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                eventText.setText("Clicked on box node.");
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {

            }
        });

        ClickListener boxClickListener = new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                eventText.setText("Clicked on 3d object.");
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {

            }
        };

        objectNode.setClickListener(boxClickListener);

        sphereNode.setClickListener(new ClickListener() {
            @Override
            public void onClick(int source, Node node, Vector location) {
                eventText.setText("Clicked on sphere object.");
            }

            @Override
            public void onClickState(int source, Node node, ClickState clickState, Vector location) {

            }
        });

        return Arrays.asList(boxNode, sphereNode, objectNode, textNode);
    }


    private void testARScene(ARScene scene) {

        final Text arSceneText = new Text(mViroView.getViroContext(), "AR Text not initialized.",
                "Roboto", 12, Color.WHITE, .5f, .5f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);
        Node arSceneTextNode = new Node();
        arSceneTextNode.setGeometry(arSceneText);
        arSceneTextNode.setPosition(new Vector(0, -1, -.5));
        scene.getRootNode().addChildNode(arSceneTextNode);
        scene.setListener(new ARScene.Listener() {
            @Override
            public void onTrackingInitialized() {
                arSceneText.setText("AR Initialized callback received!");
            }

            @Override
            public void onTrackingUpdated(ARScene.TrackingState state, ARScene.TrackingStateReason reason) {

            }

            @Override
            public void onAmbientLightUpdate(float lightIntensity, float colorTemperature) {

            }

            @Override
            public void onAnchorFound(ARAnchor anchor, ARNode arNode) {
                Material material = new Material();
                material.setDiffuseColor(Color.RED);
                material.setLightingModel(Material.LightingModel.BLINN);
                Log.i(TAG, "MemoryLeakTest: onAnchorFound invoked!");
                if(anchor instanceof ARPlaneAnchor) {
                    ARPlaneAnchor arPlaneAnchor  = (ARPlaneAnchor)anchor;
                    Surface surface = new Surface(arPlaneAnchor.getExtent().x, arPlaneAnchor.getExtent().z);
                    surface.setMaterials(Arrays.asList(material));

                    Node surfaceNode = new Node();
                    surfaceNode.setGeometry(surface);
                    surfaceNode.setPosition(new Vector(0, 0, 0));
                    surfaceNode.setRotation(new Vector(-Math.toRadians(90.0), 0, 0));
                    arNode.addChildNode(surfaceNode);
                    Log.i(TAG, "MemoryLeakTest: onAnchorFound invoked!");
                }
            }

            @Override
            public void onAnchorUpdated(ARAnchor anchor, ARNode arNode) {
                    List<Node> childNodes = arNode.getChildNodes();
                    for (Node childNode : childNodes) {

                        if(childNode.getGeometry() instanceof Surface) {
                            ARPlaneAnchor planeAnchor = (ARPlaneAnchor)anchor;
                            Surface surface = (Surface)childNode.getGeometry();
                            surface.setWidth(planeAnchor.getExtent().x);
                            surface.setHeight(planeAnchor.getExtent().z);
                            childNode.setScale(planeAnchor.getScale());
                        }

                }

            }

            @Override
            public void onAnchorRemoved(ARAnchor anchor, ARNode arNode) {

            }
        });
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


    public Boolean isGlInitialized() {
        return mGLInitialized;
    }
}
