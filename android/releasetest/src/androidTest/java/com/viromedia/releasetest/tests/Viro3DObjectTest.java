package com.viromedia.releasetest.tests;

import android.graphics.Color;
import android.net.Uri;
import android.support.test.espresso.core.deps.guava.collect.Iterables;

import com.viro.core.AmbientLight;
import com.viro.core.Animation;
import com.viro.core.AsyncObject3DListener;
import com.viro.core.Material;
import com.viro.core.Node;
import com.viro.core.Object3D;
import com.viro.core.Scene;
import com.viro.core.Text;
import com.viro.core.Vector;


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
    private boolean mIsAnimPaused = false;

    @Override
    void configureTestScene() {
        // Creation of ObjectJni to the right
        mObject3D = new Object3D();
        mObject3D.setPosition(new Vector(0, 0, -5));
        AmbientLight ambientLight = new AmbientLight(Color.WHITE, 1000f);
        mScene.getRootNode().addLight(ambientLight);
        mScene.getRootNode().addChildNode(mObject3D);
    }

    @Test
    public void test(){
        stage1_testLoadModelFBX();
        stage2_testFBXAnimPause();
        stage3_testFBXAnimStop();
        stage4_testLoadModelOBJ();
        stage5_testLoadModelFBXError();
        stage5_testLoadModelError();
        stage6_testLoadModelOBJMaterials();
        stage7_testLoadModelVRXReplaceMaterial();
    }

    public void stage1_testLoadModelFBX() {
        mObject3D.loadModel(Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
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
        mMutableTestMethod = null;
        mAnimation.stop();
        assertPass("FBX animation stops.");
    }

    public void stage4_testLoadModelOBJ() {
        mObject3D.setPosition(new Vector(0, 0, -11));
        mObject3D.setScale(new Vector(0.04f, 0.04f, 0.04f));
        mObject3D.loadModel((Uri.parse("file:///android_asset/male02.obj")), Object3D.Type.OBJ,  new AsyncObject3DListener() {
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
                "Roboto", 25, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);

        final float[] classNamePosition = {0, -.5f, -3.3f};
        node.setPosition(new Vector(classNamePosition));
        node.setGeometry(text);
        mScene.getRootNode().addChildNode(node);

        mObject3D.loadModel((Uri.parse("file:///android_asset/momentslogo.fbx")), Object3D.Type.FBX,  new AsyncObject3DListener() {
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
                "Roboto", 25, Color.WHITE, 1f, 1f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.TOP, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 0);

        final float[] classNamePosition = {0, -.5f, -3.3f};
        node.setPosition(new Vector(classNamePosition));
        node.setGeometry(text);
        mScene.getRootNode().addChildNode(node);

        mObject3D.loadModel((Uri.parse("file:///android_asset/momentslogo.pong")), Object3D.Type.OBJ,  new AsyncObject3DListener() {
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
        });
    }

    public void stage6_testLoadModelOBJMaterials() {
        Node mTextNode1 = new Node();
        Text text1 = new Text(mViroView.getViroContext(), "Toggle BLoom",
                "Roboto", 15, Color.WHITE, 5f, 5f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.BOTTOM, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 50);
        mTextNode1.setGeometry(text1);
        mTextNode1.setPosition(new Vector(-2,-0,-4));

        Node mTextNode2 = new Node();
        Text text2 = new Text(mViroView.getViroContext(), "Toggle BLoom",
                "Roboto", 15, Color.WHITE, 5f, 5f, Text.HorizontalAlignment.LEFT,
                Text.VerticalAlignment.BOTTOM, Text.LineBreakMode.WORD_WRAP, Text.ClipMode.NONE, 50);
        mTextNode2.setGeometry(text2);
        mTextNode2.setPosition(new Vector(0.5,0.0,-4));

        Object3D object3D = new Object3D();
        mScene.getRootNode().addChildNode(mTextNode1);
        mScene.getRootNode().addChildNode(mTextNode2);
        mScene.getRootNode().addChildNode(object3D);

        String expectedValues= "Expected Values:\n" +
                "mName: object_star(some number)\n" +
                "mDiffuseIntensity: 0.8\n" +
                "mShininess: 6.311791\n" +
                "mFresnelExponent: 0.5\n" +
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
                "mNormalMap S: CLAMP\n" +
                "mSpecularTexture S: REPEAT\n" +
                "mMetalnessMap S: CLAMP\n" +
                "mRoughnessMap S: CLAMP\n" +
                "mAmbientOcclusionMap S: CLAMP";
        text1.setText(expectedValues);

        // Creation of ObjectJni to the right
        object3D.setPosition(new Vector(2, 0, -5));
        object3D.loadModel(Uri.parse("file:///android_asset/object_star_anim.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
            @Override
            public void onObject3DLoaded(final Object3D object, final Object3D.Type type) {
                Material mat = object.getMaterials().get(0);
                StringBuilder builder = new StringBuilder();
                builder.append("\nmAttained Values: ");
                builder.append("\nmName: ");
                builder.append(mat.getName());
                builder.append("\nmDiffuseIntensity: ");
                builder.append(mat.getDiffuseIntensity());
                builder.append("\nmShininess: ");
                builder.append(mat.getShininess());
                builder.append("\nmFresnelExponent: ");
                builder.append(mat.getFresnelExponent());
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
                builder.append("\nmNormalMap S: ");
                builder.append(mat.getNormalMap().getWrapS().toString());
                builder.append("\nmSpecularTexture S: ");
                builder.append(mat.getSpecularTexture().getWrapS().toString());
                builder.append("\nmMetalnessMap S: ");
                builder.append(mat.getMetalnessMap().getWrapS().toString());
                builder.append("\nmRoughnessMap S: ");
                builder.append(mat.getRoughnessMap().getWrapS().toString());
                builder.append("\nmAmbientOcclusionMap S: ");
                builder.append(mat.getAmbientOcclusionMap().getWrapS().toString());

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

        object3D.loadModel(Uri.parse("file:///android_asset/dragao_2018.vrx"), Object3D.Type.FBX, new AsyncObject3DListener() {
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

        assertPass("You should see dragon material change color over time from white, blue, green to black.",()->{
            object3D.removeFromParentNode();
        });

    }
}
