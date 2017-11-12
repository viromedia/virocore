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

import com.viro.core.ARScene;
import com.viro.core.AmbientLight;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;
import com.viro.core.Box;
import com.viro.core.ClickListener;
import com.viro.core.ClickState;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.RendererStartListener;
import com.viro.core.Sphere;
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

public class MemoryLeakTest extends AppCompatActivity implements RendererStartListener {
    private static final String TAG = MemoryLeakTest.class.getSimpleName();

    private boolean mGLInitialized = false;
    private ViroView mViroView;
    private String mTestToRun;
    private Handler mHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        //new Exception().printStackTrace();
        super.onCreate(savedInstanceState);
        System.out.println("onCreate called");
        if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("GVR")) {
            mViroView = new ViroViewGVR(this, this, new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "On GVR userRequested exit");
                }
            });
            mViroView.setVRModeEnabled(true);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("OVR")) {
            mViroView = new ViroViewOVR(this, this);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("ARCore")) {
            mViroView = new ViroViewARCore(this, this);
        } else if (BuildConfig.VR_PLATFORM.equalsIgnoreCase("Scene")) {
            mViroView = new ViroViewScene(this, this);
        }

        Intent intent = getIntent();
        mTestToRun = intent.getStringExtra("TestToRun");

        mViroView.setVRModeEnabled(false);
        mViroView.validateAPIKey("7EEDCB99-2C3B-4681-AE17-17BC165BF792");
        setContentView(mViroView);

       // mHandler = new Handler(getMainLooper());
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
        super.onDestroy();
        mViroView.onActivityDestroyed(this);
    }


    @Override
    public void onRendererStart() {
        mGLInitialized = true;
        Log.e("ViroActivity", "onRendererStart called");
        //initializeVrScene();
        initializeArScene();
    }

    public void initializeArScene() {
        final ARScene scene = new ARScene();
        final Node rootNode = scene.getRootNode();
        List<Node> nodes = selectScene();
        for (final Node node : nodes) {
            rootNode.addChildNode(node);
        }


        final AmbientLight ambientLightJni = new AmbientLight(Color.WHITE, 300.0f);
        scene.getRootNode().addLight(ambientLightJni);
        mViroView.setScene(scene);
    }

    private List<Node> selectScene() {
        //boxTest, sphereTest, videoTest, objectTest
        if(mTestToRun.equalsIgnoreCase("boxTest")) {
            return testBox(mViroView.getContext());
        } else if(mTestToRun.equalsIgnoreCase("sphereTest")) {
            return testSphereVideo(mViroView.getContext());
        } else if(mTestToRun.equalsIgnoreCase("videoTest")) {

        } else if(mTestToRun.equalsIgnoreCase("objectTest")) {

        }
        return null;
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
