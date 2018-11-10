package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.net.Uri;
import android.support.test.espresso.core.deps.guava.collect.Iterables;
import android.util.Log;

import com.viro.core.AmbientLight;
import com.viro.core.AnimationTransaction;
import com.viro.core.DirectionalLight;
import com.viro.core.Animation;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Text;
import com.viro.core.Vector;
import com.viro.core.Surface;
import com.viro.core.AnimationTimingFunction;
import com.viro.core.AnimationTransaction;


import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Created by vadvani on 11/2/17.
 */

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class Viro3DObjectTest extends ViroBaseTest {
    private Object3D mObject3D;
    private Animation mAnimation;
    private Object3D.MorphMode mMorphMode;
    private boolean mIsAnimPaused = false;
    private AmbientLight mAmbientLight;

    @Override
    void configureTestScene() {
        // Creation of ObjectJni to the right
        mObject3D = new Object3D();
        mObject3D.setPosition(new Vector(0, 0, -5));
        mAmbientLight = new AmbientLight(Color.WHITE, 1000f);
        mScene.getRootNode().addLight(mAmbientLight);
        mScene.getRootNode().addChildNode(mObject3D);
    }

    @Test
    public void test() {
        runUITest(() -> stage0_testLoadModelGLTF());
        runUITest(() -> stage0_testLoadModelGLTFMorph());
        runUITest(() -> stage1_testLoadModelFBX());
        runUITest(() -> stage2_testFBXAnimPause());
        runUITest(() -> stage3_testFBXAnimStop());
        runUITest(() -> stage4_testLoadModelOBJ());
        runUITest(() -> stage5_testLoadModelFBXError());
        runUITest(() -> stage5_testLoadModelError());
        runUITest(() -> stage6_testLoadModelOBJMaterials());
        runUITest(() -> stage7_testLoadModelVRXReplaceMaterial());
        runUITest(() -> stage8_testLoadModelAnimateVRXWithShadow());
        runUITest(() -> stage8_testLoadModelAnimateVRXFreeze());
        runUITest(() -> stage8_testLoadModelAnimateVRXDifferentSpeeds());
        runUITest(() -> stage8_testLoadModelAnimateSlow());
    }

    public void stage0_testLoadModelGLTF() {
        Node node = new Node();
        node.setScale(new Vector(0.15, 0.15 , 0.15));
        node.setPosition(new Vector(0,-0.7, -1));

        Object3D gltfModel = new Object3D();
        Object3D gltfModelGLB = new Object3D();
        Object3D gltfModelBase64 = new Object3D();

        gltfModelGLB.setPosition(new Vector(-1.75,0,0));
        gltfModelBase64.setPosition(new Vector(1.75,0,0));

        node.addChildNode(gltfModel);
        node.addChildNode(gltfModelBase64);
        node.addChildNode(gltfModelGLB);
        mScene.getRootNode().addChildNode(node);

        gltfModelGLB.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/DuckGlb.glb"), Object3D.Type.GLB, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                Log.w("Viro", "GLTF load successful with bounds " + object.getBoundingBox() + ", type [" + type + "]");
            }

            @Override
            public void onObject3DFailed(final String error) {
                Log.w("Viro", "GLTF failed to load: " + error);
            }
        });

        gltfModel.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/Duck.gltf"), Object3D.Type.GLTF, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                Log.w("Viro", "GLTF load successful with bounds " + object.getBoundingBox() + ", type [" + type + "]");
            }

            @Override
            public void onObject3DFailed(final String error) {
                Log.w("Viro", "GLTF failed to load: " + error);
            }
        });

        gltfModelBase64.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/Duck64Encoded.gltf"), Object3D.Type.GLTF, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                Log.w("Viro", "GLTF load successful with bounds " + object.getBoundingBox() + ", type [" + type + "]");
            }

            @Override
            public void onObject3DFailed(final String error) {
                Log.w("Viro", "GLTF failed to load: " + error);
            }
        });

        assertPass("You should see 3 Ducks Render in the scene.", ()->{
            node.removeFromParentNode();
        });
    }

    public void stage0_testLoadModelGLTFMorph() {
        Node node = new Node();
        node.setScale(new Vector(0.15, 0.15 , 0.15));
        node.setPosition(new Vector(0,-0.7, -1));

        Object3D gltfModel = new Object3D();
        Object3D gltfModelGLB = new Object3D();
        Object3D gltfModelBase64 = new Object3D();

        gltfModelGLB.setPosition(new Vector(-1.75,0,0));
        gltfModelBase64.setPosition(new Vector(1.75,0,0));

        node.addChildNode(gltfModel);
        node.addChildNode(gltfModelBase64);
        node.addChildNode(gltfModelGLB);
        mScene.getRootNode().addChildNode(node);

        gltfModelGLB.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/AnimatedMorphCube.glb"), Object3D.Type.GLB, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                Log.w("Viro", "GLTF load successful with bounds " + object.getBoundingBox() + ", type [" + type + "]");

                boolean foundMorph = false;
                for (String s : object.getMorphTargetKeys()){
                    Log.e("Daniel"," Keys -> " + s);
                    if (s.equalsIgnoreCase("thin")){
                        foundMorph = true;
                        break;
                    }
                }

                if (!foundMorph) {
                    return;
                }

                AnimationTransaction.begin();
                AnimationTransaction.setAnimationDuration(2000);
                AnimationTransaction.setAnimationLoop(true);
                AnimationTransaction.setTimingFunction(AnimationTimingFunction.Linear);
                gltfModelGLB.setMorphTargetWeight("thin", 1.0f);
                AnimationTransaction.commit();

            }

            @Override
            public void onObject3DFailed(final String error) {
                Log.w("Viro", "GLTF failed to load: " + error);
            }
        });

        mMorphMode = Object3D.MorphMode.GPU;

        gltfModelBase64.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/AnimatedMorphCube.glb"), Object3D.Type.GLB, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                Log.w("Viro", "GLTF load successful with bounds " + object.getBoundingBox() + ", type [" + type + "]");
                mAnimation = object.getAnimation("Square");
                mAnimation.setLoop(true);
                mAnimation.play();
                mAnimation.setListener(new Animation.Listener() {
                    @Override
                    public void onAnimationStart(Animation animation) {
                    }

                    @Override
                    public void onAnimationFinish(Animation animation, boolean canceled) {
                        Log.e("Daniel"," Mode : " + mMorphMode.mStringValue);
                        switch (mMorphMode){
                            case CPU:
                                mMorphMode = Object3D.MorphMode.GPU;
                                break;
                            case GPU:
                                mMorphMode = Object3D.MorphMode.HYBRID;
                                break;
                            case HYBRID:
                                mMorphMode = Object3D.MorphMode.GPU;
                                break;
                        }
                        object.setMorphMode(mMorphMode);
                    }
                });
            }

            @Override
            public void onObject3DFailed(final String error) {
                Log.w("Viro", "GLTF failed to load: " + error);
            }
        });

        assertPass("Both GLTF Morph cubes SHOULD Animate (wait for 3 cycles).", ()->{
            node.removeFromParentNode();
        });
    }

    public void stage1_testLoadModelFBX() {
        mObject3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -3));
                object.setScale(new Vector(0.4f, 0.4f, 0.4f));
                mAnimation = object.getAnimation("02_spin");
                mAnimation.setDelay(2000);
                mAnimation.setLoop(true);
                mAnimation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        assertPass("Star model loads and begins to animate.");
    }

    public void stage2_testFBXAnimPause() {
        mObject3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -3));
                object.setScale(new Vector(0.4f, 0.4f, 0.4f));
                mAnimation = object.getAnimation("02_spin");
                mAnimation.setDelay(2000);
                mAnimation.setLoop(true);
                mAnimation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        mMutableTestMethod = () -> {
            if(!mIsAnimPaused) {
                mAnimation.pause();
                mIsAnimPaused = true;
            } else {
                mAnimation.play();
            }
        };

        assertPass("FBX rotates from pause to play.", ()-> {
            if(mIsAnimPaused) {
                mMutableTestMethod = null;
                if(mIsAnimPaused) {
                    mAnimation.play();
                }
            }
        });
    }

    public void stage3_testFBXAnimStop() {
        mObject3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -3));
                object.setScale(new Vector(0.4f, 0.4f, 0.4f));
                mAnimation = object.getAnimation("02_spin");
                mAnimation.setLoop(true);
                mAnimation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        mMutableTestMethod = () -> mAnimation.stop();
        assertPass("FBX animation stops mid way (after *maybe* one spin)");
    }

    public void stage4_testLoadModelOBJ() {
        mObject3D.setPosition(new Vector(0, 0, -11));
        mObject3D.setScale(new Vector(0.04f, 0.04f, 0.04f));
        mObject3D.loadModel(mViroView.getViroContext(), (Uri.parse("file:///android_asset/male02.obj")), Object3D.Type.OBJ,  new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {

            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        assertPass("Tom Cruise lookalike model loads and displays.");
    }

    public void stage5_testLoadModelFBXError() {
        Node node = new Node();
        final Text text = new Text(mViroView.getViroContext(), "Awaiting fbx load.....",
                "Roboto", 25,
                Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);

        final float[] classNamePosition = {0, -.5f, -3.3f};
        node.setPosition(new Vector(classNamePosition));
        node.setGeometry(text);
        mScene.getRootNode().addChildNode(node);

        mObject3D.loadModel(mViroView.getViroContext(), (Uri.parse("file:///android_asset/momentslogo.fbx")), Object3D.Type.FBX,  new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {

            }

            @Override
            public void onObject3DFailed(final String error) {
                text.setText("FBX failed to load as it should!");

            }
        });
        assertPass("Text should display saying FBX failed to load.",()->{
            node.removeFromParentNode();
        });
    }

    public void stage5_testLoadModelError() {
        Node node = new Node();
        final Text text = new Text(mViroView.getViroContext(), "Awaiting obj load.....",
                "Roboto", 25,
                Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);

        final float[] classNamePosition = {0, -.5f, -3.3f};
        node.setPosition(new Vector(classNamePosition));
        node.setGeometry(text);
        mScene.getRootNode().addChildNode(node);

        mObject3D.loadModel(mViroView.getViroContext(), (Uri.parse("file:///android_asset/momentslogo.pong")), Object3D.Type.OBJ,  new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {

            }

            @Override
            public void onObject3DFailed(final String error) {
                text.setText("Object failed to load as it should!");

            }
        });
        assertPass("Text should display saying object failed to load.",()->{
            node.removeFromParentNode();
            mObject3D.removeFromParentNode();
        });
    }

    public void stage6_testLoadModelOBJMaterials() {
        Node mTextNode1 = new Node();
        Text text1 = new Text(mViroView.getViroContext(), "Toggle BLoom",
                "Roboto", 15,
                Color.WHITE, 5f, 5f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.BOTTOM, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 50);
        mTextNode1.setGeometry(text1);
        mTextNode1.setPosition(new Vector(-2,-0,-4));

        Node mTextNode2 = new Node();
        Text text2 = new Text(mViroView.getViroContext(), "Toggle BLoom",
                "Roboto", 15,
                Color.WHITE, 5f, 5f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.BOTTOM, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 50);
        mTextNode2.setGeometry(text2);
        mTextNode2.setPosition(new Vector(0.5,0.0,-4));

        Object3D object3D = new Object3D();
        mScene.getRootNode().addChildNode(mTextNode1);
        mScene.getRootNode().addChildNode(mTextNode2);
        mScene.getRootNode().addChildNode(object3D);

        String expectedValues= "Expected Values:\n" +
                "mName: object_star(some number)\n" +
                "mShininess: 6.311791\n" +
                "mBloomThreshold: -1.0\n" +
                "mRoughness: 0.484529\n" +
                "mMetalness: 0.0\n" +
                "mDiffuseColor: ffffffff\n" +
                "mWritesToDepthBuffer: true\n" +
                "mReadsFromDepthBuffer: true\n" +
                "mLightingModel: PHONG\n" +
                "mCullMode: BACK\n" +
                "mTransparencyMode: A_ONE\n" +
                "mBlendMode: ALPHA\n" +
                "mShadowMode: NORMAL\n" +
                "mDiffuseTexture S: REPEAT\n" +
                "mSpecularTexture S: REPEAT\n";
        text1.setText(expectedValues);

        // Creation of ObjectJni to the right
        object3D.setPosition(new Vector(2, 0, -5));
        object3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                Material mat = object.getMaterials().get(0);
                StringBuilder builder = new StringBuilder();
                builder.append("\nmAttained Values: ");
                builder.append("\nmName: ");
                builder.append(mat.getName());
                builder.append("\nmShininess: ");
                builder.append(mat.getShininess());
                builder.append("\nmBloomThreshold: ");
                builder.append(mat.getBloomThreshold());
                builder.append("\nmRoughness: ");
                builder.append(mat.getRoughness());
                builder.append("\nmMetalness: ");
                builder.append(mat.getMetalness());
                builder.append("\nmDiffuseColor: ");
                builder.append(Integer.toHexString(mat.getDiffuseColor()));
                builder.append("\nmWritesToDepthBuffer: ");
                builder.append(mat.getWritesToDepthBuffer());
                builder.append("\nmReadsFromDepthBuffer: ");
                builder.append(mat.getReadsFromDepthBuffer());
                builder.append("\nmLightingModel: ");
                builder.append(mat.getLightingModel().toString());
                builder.append("\nmCullMode: ");
                builder.append(mat.getCullMode().toString());
                builder.append("\nmTransparencyMode: ");
                builder.append(mat.getTransparencyMode().toString());
                builder.append("\nmBlendMode: ");
                builder.append(mat.getBlendMode().toString());
                builder.append("\nmShadowMode: ");
                builder.append(mat.getShadowMode().toString());
                builder.append("\nmDiffuseTexture S: ");
                builder.append(mat.getDiffuseTexture().getWrapS().toString());
                builder.append("\nmSpecularTexture S: ");
                builder.append(mat.getSpecularTexture().getWrapS().toString());
                text2.setText(builder.toString());
            }

            @Override
            public void onObject3DFailed(final String error) {
            }
        });


        assertPass("You should see matching values on the left and right text, " +
                "indicating a sucessful jMaterial Construction.",()->{
            mTextNode1.removeFromParentNode();
            mTextNode2.removeFromParentNode();
            object3D.removeFromParentNode();
        });
    }

    public void stage7_testLoadModelVRXReplaceMaterial() {
        final Object3D object3D = new Object3D();
        mScene.getRootNode().addChildNode(object3D);
        object3D.setPosition(new Vector(2, 0, -5));

        object3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/dragao_2018.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
            }

            @Override
            public void onObject3DFailed(final String error) {
            }
        });

        final List<Integer> materialColors= Arrays.asList(Color.WHITE, Color.BLUE, Color.YELLOW, Color.CYAN);
        final Iterator<Integer> iterator = Iterables.cycle(materialColors).iterator();
        mMutableTestMethod = () -> {
            if(object3D.getMaterials() != null && object3D.getMaterials().get(0) != null) {
                object3D.getMaterials().get(0).setDiffuseColor(iterator.next());
            }
        };

        assertPass("You should see dragon material change color over time from white, blue, yellow to cyan.",()->{
            object3D.removeFromParentNode();
        });

    }

    public void stage8_testLoadModelAnimateVRXWithShadow() {
        DirectionalLight light = new DirectionalLight();
        light.setColor(Color.WHITE);
        light.setDirection(new Vector(0, -1, 0));
        light.setShadowOrthographicPosition(new Vector(0, 20, -9));
        light.setShadowOrthographicSize(60);
        light.setShadowNearZ(1);
        light.setShadowFarZ(60);
        light.setShadowOpacity(1.0f);
        light.setCastsShadow(true);
        mScene.getRootNode().removeLight(mAmbientLight);
        mScene.getRootNode().addLight(light);

        Material coloredMaterial = new Material();
        coloredMaterial.setDiffuseColor(Color.RED);
        coloredMaterial.setLightingModel(Material.LightingModel.BLINN);
        //used to be 60 for width and height
        Surface surface = new Surface(60, 60);
        surface.setMaterials(Arrays.asList(coloredMaterial));
        Node surfaceNode = new Node();
        surfaceNode.setGeometry(surface);
        surfaceNode.setRotation(new Vector((float) -Math.PI / 2.0f, 0, 0));
        surfaceNode.setPosition(new Vector(0, -5, -9));
        mScene.getRootNode().addChildNode(surfaceNode);

        final Object3D object3D = new Object3D();
        mScene.getRootNode().addChildNode(object3D);
        object3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/dragao.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -9));
                object.setScale(new Vector(0.2f, 0.2f, 0.2f));
                mAnimation = object.getAnimation("01");
                mAnimation.setDelay(1000);
                mAnimation.setLoop(true);
                mAnimation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        assertPass("You should see an animated dragon with it's shadow moving.",()->{
            object3D.removeFromParentNode();
        });
    }

    public void stage8_testLoadModelAnimateVRXFreeze() {
        DirectionalLight light = new DirectionalLight();
        light.setColor(Color.WHITE);
        light.setDirection(new Vector(0, -1, 0));
        light.setShadowOrthographicPosition(new Vector(0, 20, -9));
        light.setShadowOrthographicSize(60);
        light.setShadowNearZ(1);
        light.setShadowFarZ(60);
        light.setShadowOpacity(1.0f);
        light.setCastsShadow(true);
        mScene.getRootNode().removeLight(mAmbientLight);
        mScene.getRootNode().addLight(light);

        Material coloredMaterial = new Material();
        coloredMaterial.setDiffuseColor(Color.RED);
        coloredMaterial.setLightingModel(Material.LightingModel.BLINN);
        //used to be 60 for width and height
        Surface surface = new Surface(60, 60);
        surface.setMaterials(Arrays.asList(coloredMaterial));
        Node surfaceNode = new Node();
        surfaceNode.setGeometry(surface);
        surfaceNode.setRotation(new Vector((float) -Math.PI / 2.0f, 0, 0));
        surfaceNode.setPosition(new Vector(0, -5, -9));
        mScene.getRootNode().addChildNode(surfaceNode);

        final Object3D object3D = new Object3D();
        mScene.getRootNode().addChildNode(object3D);
        object3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/dragao.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -9));
                object.setScale(new Vector(0.2f, 0.2f, 0.2f));
                mAnimation = object.getAnimation("01");
                mAnimation.setDelay(0);
                mAnimation.setTimeOffset(4500);
                mAnimation.setSpeed(0);
                mAnimation.setLoop(true);
                mAnimation.play();

            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        final List<Integer> timeOffsetList = Arrays.asList(4500, 6000, 1000, 0);
        final Iterator<Integer> itr = Iterables.cycle(timeOffsetList).iterator();
        mMutableTestMethod = () -> {
            if (mAnimation != null) {
                mAnimation.stop();
                mAnimation.setTimeOffset(itr.next());
                mAnimation.play();
            }
        };

        assertPass("You should see an animated dragon frozen at timestamp 4.5 seconds, 6s, 1s, 0s",()->{
            object3D.removeFromParentNode();
        });
    }

    public void stage8_testLoadModelAnimateVRXDifferentSpeeds() {
        DirectionalLight light = new DirectionalLight();
        light.setColor(Color.WHITE);
        light.setDirection(new Vector(0, -1, 0));
        light.setShadowOrthographicPosition(new Vector(0, 20, -9));
        light.setShadowOrthographicSize(60);
        light.setShadowNearZ(1);
        light.setShadowFarZ(60);
        light.setShadowOpacity(1.0f);
        light.setCastsShadow(true);
        mScene.getRootNode().removeLight(mAmbientLight);
        mScene.getRootNode().addLight(light);

        Material coloredMaterial = new Material();
        coloredMaterial.setDiffuseColor(Color.RED);
        coloredMaterial.setLightingModel(Material.LightingModel.BLINN);
        //used to be 60 for width and height
        Surface surface = new Surface(60, 60);
        surface.setMaterials(Arrays.asList(coloredMaterial));
        Node surfaceNode = new Node();
        surfaceNode.setGeometry(surface);
        surfaceNode.setRotation(new Vector((float) -Math.PI / 2.0f, 0, 0));
        surfaceNode.setPosition(new Vector(0, -5, -9));
        mScene.getRootNode().addChildNode(surfaceNode);

        final Object3D object3D = new Object3D();
        mScene.getRootNode().addChildNode(object3D);
        object3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/dragao.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -9));
                object.setScale(new Vector(0.2f, 0.2f, 0.2f));
                mAnimation = object.getAnimation("01");
                mAnimation.setDelay(0);
                mAnimation.setTimeOffset(0);
                mAnimation.setSpeed(1);
                mAnimation.setLoop(true);
                mAnimation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        final List<Float> speedList = Arrays.asList(1f, .2f, .1f, 1f);
        final Iterator<Float> itr = Iterables.cycle(speedList).iterator();
        mMutableTestMethod = () -> {
            if (mAnimation != null) {
               mAnimation.setSpeed(itr.next());
            }
        };
        assertPass("You should see an animated dragon move at different speeds: normal, slow, slower, then normal.",()->{
            object3D.removeFromParentNode();
        });
    }

    public void stage8_testLoadModelAnimateSlow() {
        DirectionalLight light = new DirectionalLight();
        light.setColor(Color.WHITE);
        light.setDirection(new Vector(0, -1, 0));
        light.setShadowOrthographicPosition(new Vector(0, 20, -9));
        light.setShadowOrthographicSize(60);
        light.setShadowNearZ(1);
        light.setShadowFarZ(60);
        light.setShadowOpacity(1.0f);
        light.setCastsShadow(true);
        mScene.getRootNode().removeLight(mAmbientLight);
        mScene.getRootNode().addLight(light);

        Material coloredMaterial = new Material();
        coloredMaterial.setDiffuseColor(Color.RED);
        coloredMaterial.setLightingModel(Material.LightingModel.BLINN);
        //used to be 60 for width and height
        Surface surface = new Surface(60, 60);
        surface.setMaterials(Arrays.asList(coloredMaterial));
        Node surfaceNode = new Node();
        surfaceNode.setGeometry(surface);
        surfaceNode.setRotation(new Vector((float) -Math.PI / 2.0f, 0, 0));
        surfaceNode.setPosition(new Vector(0, -5, -9));
        mScene.getRootNode().addChildNode(surfaceNode);

        final Object3D object3D = new Object3D();
        mScene.getRootNode().addChildNode(object3D);
        object3D.loadModel(mViroView.getViroContext(), Uri.parse("file:///android_asset/dragao.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                object.setPosition(new Vector(0, 0, -9));
                object.setScale(new Vector(0.2f, 0.2f, 0.2f));
                mAnimation = object.getAnimation("01");
                mAnimation.setDelay(1000);
                mAnimation.setLoop(true);

                mAnimation.setDuration(mAnimation.getDuration() * 4);
                mAnimation.play();
            }

            @Override
            public void onObject3DFailed(final String error) {

            }
        });

        assertPass("Animated dragon running 4x slower than usual",()->{
            object3D.removeFromParentNode();
        });
    }
}
