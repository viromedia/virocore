/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.releasetest.tests;

import android.graphics.Bitmap;
import android.graphics.Color;

import com.viro.renderer.jni.AmbientLight;
import com.viro.renderer.jni.Box;
import com.viro.renderer.jni.DirectionalLight;
import com.viro.renderer.jni.Material;
import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.PhysicsBody;
import com.viro.renderer.jni.PhysicsShapeBox;
import com.viro.renderer.jni.PhysicsShapeSphere;
import com.viro.renderer.jni.PhysicsWorld;
import com.viro.renderer.jni.Sphere;
import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.Vector;

import org.junit.Test;

import java.util.Arrays;
import java.util.List;
import java.util.Random;

/**
 * Created by manish on 11/3/17.
 */

public class ViroPhysicsBodyTest extends ViroBaseTest {
    private static final int INFLUENCE_BITMASK = 1;
    private static final double TABLE_WIDTH = 4.5;
    private static final double TABLE_LENGTH = 9;
    private static final double BALL_RADIUS = 0.2;
    private static final double SCALE_FACTOR = 8;
    private static final double BALL_FRICTION = 0.08;
    private static final double TABLE_FRICTION = 0.2;
    private static final double BALL_RESTITUTION = 0.92;
    private static final double TABLE_RESTITUTION = 0.5;
    private static final String POOL_BALL_SPECULAR = "pool_ball_specular.png";

    private Box mTable;
    private PhysicsBody mTablePhysics;
    private Box mRail1;
    private PhysicsBody mRail1Physics;
    private Box mRail2;
    private PhysicsBody mRail2Physics;
    private Box mRail3;
    private PhysicsBody mRail3Physics;
    private Box mRail4;
    private PhysicsBody mRail4Physics;


    private Sphere mCueBall;
    private PhysicsBody mCueBallPhysics;
    private Sphere mBall1;
    private PhysicsBody mBall1Physics;
    private Sphere mBall2;
    private PhysicsBody mBall2Physics;
    private Sphere mBall3;
    private PhysicsBody mBall3Physics;
    private Sphere mBall4;
    private PhysicsBody mBall4Physics;
    private Sphere mBall5;
    private PhysicsBody mBall5Physics;
    private Sphere mBall6;
    private PhysicsBody mBall6Physics;
    private Sphere mBall7;
    private PhysicsBody mBall7Physics;
    private Sphere mBall8;
    private PhysicsBody mBall8Physics;
    private Sphere mBall9;
    private PhysicsBody mBall9Physics;
    private Sphere mBall10;
    private PhysicsBody mBall10Physics;
    private Sphere mBall11;
    private PhysicsBody mBall11Physics;
    private Sphere mBall12;
    private PhysicsBody mBall12Physics;
    private Sphere mBall13;
    private PhysicsBody mBall13Physics;
    private Sphere mBall14;
    private PhysicsBody mBall14Physics;
    private Sphere mBall15;
    private PhysicsBody mBall15Physics;

    private List<Sphere> mBalls;
    private List<PhysicsBody> mBallsPhysics;

    private PhysicsWorld mPhysicsWorld;

    @Override
    void configureTestScene() {
        final AmbientLight ambientLight = new AmbientLight(Color.WHITE, 200);
        mScene.getRootNode().addLight(ambientLight);

        final DirectionalLight directionalLight = new DirectionalLight(Color.WHITE, 1000, new Vector(1, -1, -1));
        directionalLight.setCastsShadow(true);
        directionalLight.setShadowOpacity(1);
        directionalLight.setInfluenceBitMask(3);
        directionalLight.setShadowFarZ(-36);
        directionalLight.setShadowNearZ(36);

//        mScene.getRootNode().addLight(spotlight);

        final Node playGroundNode = new Node();
        playGroundNode.setPosition(new Vector(0, -16, -48));
//        playGroundNode.setRotation(new Vector(0, -90, 0));
        final Material groundMaterial = new Material();
        groundMaterial.setDiffuseColor(Color.GREEN);
        groundMaterial.setLightingModel(Material.LightingModel.BLINN);

        final Node tableNode = new Node();
        mTable = new Box(scaleSize(TABLE_WIDTH), 1, scaleSize(TABLE_LENGTH));
        mTable.setMaterials(Arrays.asList(groundMaterial));
        tableNode.setGeometry(mTable);

        final PhysicsShapeBox physicsShapeBox = new PhysicsShapeBox(mTable.getWidth(),
                mTable.getHeight(), mTable.getLength());

        mTablePhysics = tableNode.initPhysicsBody(PhysicsBody.RigidBodyType.STATIC, 1, null);

        playGroundNode.addChildNode(tableNode);
        playGroundNode.addLight(directionalLight);
        // Rail1
        final Material railMaterial = new Material();
        railMaterial.setDiffuseColor(0xFFD2691E);
        railMaterial.setLightingModel(Material.LightingModel.BLINN);

        final Node rail1Node = new Node();
        mRail1 = new Box(1, 1, 1);
        mRail1.setWidth(scaleSize(TABLE_WIDTH));
        mRail1.setHeight(4);
        mRail1.setMaterials(Arrays.asList(railMaterial));
        rail1Node.setGeometry(mRail1);
        rail1Node.setPosition(new Vector(0, 2, 36));
        final PhysicsShapeBox physicsShapeRail1 = new PhysicsShapeBox(mRail1.getWidth(), mRail1.getHeight(), mRail1.getLength());
        mRail1Physics = rail1Node.initPhysicsBody(PhysicsBody.RigidBodyType.STATIC, 1, null);
        rail1Node.setLightReceivingBitMask(3);
        rail1Node.setShadowCastingBitMask(3);
        playGroundNode.addChildNode(rail1Node);

        // Rail2
        final Node rail2Node = new Node();
        mRail2 = new Box(1, 1, 1);
        mRail2.setLength(scaleSize(TABLE_LENGTH));
        mRail2.setHeight(4);
        mRail2.setMaterials(Arrays.asList(railMaterial));
        rail2Node.setGeometry(mRail2);
        rail2Node.setPosition(new Vector(17, 2, 0));
        mRail2Physics = rail2Node.initPhysicsBody(PhysicsBody.RigidBodyType.STATIC, 1, null);
        rail2Node.setLightReceivingBitMask(3);
        rail2Node.setShadowCastingBitMask(3);
        playGroundNode.addChildNode(rail2Node);


        // Rail3
        final Node rail3Node = new Node();
        mRail3 = new Box(1, 1, 1);
        mRail3.setWidth(scaleSize(TABLE_WIDTH));
        mRail3.setHeight(4);
        mRail3.setMaterials(Arrays.asList(railMaterial));
        rail3Node.setGeometry(mRail3);
        rail3Node.setPosition(new Vector(0, 2, -36));
        mRail3Physics = rail3Node.initPhysicsBody(PhysicsBody.RigidBodyType.STATIC, 1, null);
        rail3Node.setLightReceivingBitMask(3);
        rail3Node.setShadowCastingBitMask(3);
        playGroundNode.addChildNode(rail3Node);

        // Rail4
        final Node rail4Node = new Node();
        mRail4 = new Box(1, 1, 1);
        mRail4.setLength(scaleSize(TABLE_LENGTH));
        mRail4.setHeight(4);
        mRail4.setMaterials(Arrays.asList(railMaterial));
        rail4Node.setGeometry(mRail4);
        rail4Node.setPosition(new Vector(-17, 2, 0));
        mRail4Physics = rail4Node.initPhysicsBody(PhysicsBody.RigidBodyType.STATIC, 1, null);
        rail4Node.setLightReceivingBitMask(3);
        rail4Node.setShadowCastingBitMask(3);
        playGroundNode.addChildNode(rail4Node);

        final PhysicsShapeSphere physicsShapeSphere = new PhysicsShapeSphere(scaleSize(BALL_RADIUS));

        // Cue Ball
        final Material cueBallMaterial = new Material();
        final Bitmap cueBallBitmap = getBitmapFromAssets(mActivity, "BallCue.jpg");
        final Texture cueBallTexture = new Texture(cueBallBitmap, Texture.TextureFormat.RGBA8, true, true);
        cueBallMaterial.setDiffuseTexture(cueBallTexture);
        cueBallMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node cueBallNode = new Node();
        mCueBall = new Sphere(scaleSize(BALL_RADIUS));
        mCueBall.setMaterials(Arrays.asList(cueBallMaterial));
        cueBallNode.setGeometry(mCueBall);
        cueBallNode.setPosition(new Vector(0, 5, -30));
        cueBallNode.setLightReceivingBitMask(3);
        cueBallNode.setShadowCastingBitMask(3);
        mCueBallPhysics = cueBallNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(cueBallNode);

        final Bitmap ballSpecularBitmap = getBitmapFromAssets(mActivity, "pool_ball_specular.png");
        final Texture specularTexture = new Texture(ballSpecularBitmap,
                Texture.TextureFormat.RGBA8, true, true);
        // Ball 1
        final Material ballOneMaterial = new Material();
        final Bitmap ballOneBitmap = getBitmapFromAssets(mActivity, "Ball11.jpg");
        final Texture ballOneTexture = new Texture(ballOneBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballOneMaterial.setDiffuseTexture(ballOneTexture);
        ballOneMaterial.setSpecularTexture(specularTexture);
        ballOneMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballOneNode = new Node();
        mBall1 = new Sphere(scaleSize(BALL_RADIUS));
        mBall1.setMaterials(Arrays.asList(ballOneMaterial));
        ballOneNode.setGeometry(mBall1);
        ballOneNode.setPosition(new Vector(-7f, 5, 30));
        ballOneNode.setLightReceivingBitMask(3);
        ballOneNode.setShadowCastingBitMask(3);
        mBall1Physics = ballOneNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballOneNode);

        // Ball 2
        final Material ballTwoMaterial = new Material();
        final Bitmap ballTwoBitmap = getBitmapFromAssets(mActivity, "Ball5.jpg");
        final Texture ballTwoTexture = new Texture(ballTwoBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballTwoMaterial.setDiffuseTexture(ballTwoTexture);
        ballTwoMaterial.setSpecularTexture(specularTexture);
        ballTwoMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballTwoNode = new Node();
        mBall2 = new Sphere(scaleSize(BALL_RADIUS));
        mBall2.setMaterials(Arrays.asList(ballTwoMaterial));
        ballTwoNode.setGeometry(mBall2);
        ballTwoNode.setPosition(new Vector(7f, 5, 30));
        ballTwoNode.setLightReceivingBitMask(3);
        ballTwoNode.setShadowCastingBitMask(3);
        mBall2Physics = ballTwoNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballTwoNode);

        // Ball 3
        final Material ballThreeMaterial = new Material();
        final Bitmap ballThreeBitmap = getBitmapFromAssets(mActivity, "Ball2.jpg");
        final Texture ballThreeTexture = new Texture(ballThreeBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballThreeMaterial.setDiffuseTexture(ballThreeTexture);
        ballThreeMaterial.setSpecularTexture(specularTexture);
        ballThreeMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballThreeNode = new Node();
        mBall3 = new Sphere(scaleSize(BALL_RADIUS));
        mBall3.setMaterials(Arrays.asList(ballThreeMaterial));
        ballThreeNode.setGeometry(mBall3);
        ballThreeNode.setPosition(new Vector(-3.5f, 5, 30));
        ballThreeNode.setLightReceivingBitMask(3);
        ballThreeNode.setShadowCastingBitMask(3);
        mBall3Physics = ballThreeNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballThreeNode);

        // Ball 4
        final Material ballFourMaterial = new Material();
        final Bitmap ballFourBitmap = getBitmapFromAssets(mActivity, "Ball13.jpg");
        final Texture ballFourTexture = new Texture(ballFourBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballFourMaterial.setDiffuseTexture(ballFourTexture);
        ballFourMaterial.setSpecularTexture(specularTexture);
        ballFourMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballFourNode = new Node();
        mBall4 = new Sphere(scaleSize(BALL_RADIUS));
        mBall4.setMaterials(Arrays.asList(ballFourMaterial));
        ballFourNode.setGeometry(mBall4);
        ballFourNode.setPosition(new Vector(0, 5, 30));
        ballFourNode.setLightReceivingBitMask(3);
        ballFourNode.setShadowCastingBitMask(3);
        mBall4Physics = ballFourNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballFourNode);

        // Ball 5
        final Material ballFiveMaterial = new Material();
        final Bitmap ballFiveBitmap = getBitmapFromAssets(mActivity, "Ball4.jpg");
        final Texture ballFiveTexture = new Texture(ballFiveBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballFiveMaterial.setDiffuseTexture(ballFiveTexture);
        ballFiveMaterial.setSpecularTexture(specularTexture);
        ballFiveMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballFiveNode = new Node();
        mBall5 = new Sphere(scaleSize(BALL_RADIUS));
        mBall5.setMaterials(Arrays.asList(ballFiveMaterial));
        ballFiveNode.setGeometry(mBall5);
        ballFiveNode.setPosition(new Vector(3.5f, 5, 30));
        ballFiveNode.setLightReceivingBitMask(3);
        ballFiveNode.setShadowCastingBitMask(3);
        mBall5Physics = ballFiveNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballFiveNode);

        // Ball 6
        final Material ballSixMaterial = new Material();
        final Bitmap ballSixBitmap = getBitmapFromAssets(mActivity, "Ball3.jpg");
        final Texture ballSixTexture = new Texture(ballSixBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballSixMaterial.setDiffuseTexture(ballSixTexture);
        ballSixMaterial.setSpecularTexture(specularTexture);
        ballSixMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballSixNode = new Node();
        mBall6 = new Sphere(scaleSize(BALL_RADIUS));
        mBall6.setMaterials(Arrays.asList(ballSixMaterial));
        ballSixNode.setGeometry(mBall6);
        ballSixNode.setPosition(new Vector(1.75f, 5, 24));
        ballSixNode.setLightReceivingBitMask(3);
        ballSixNode.setShadowCastingBitMask(3);
        mBall6Physics = ballSixNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballSixNode);

        // Ball 7
        final Material ballSevenMaterial = new Material();
        final Bitmap ballSevenBitmap = getBitmapFromAssets(mActivity, "Ball10.jpg");
        final Texture ballSevenTexture = new Texture(ballSevenBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballSevenMaterial.setDiffuseTexture(ballSevenTexture);
        ballSevenMaterial.setSpecularTexture(specularTexture);
        ballSevenMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballSevenNode = new Node();
        mBall7 = new Sphere(scaleSize(BALL_RADIUS));
        mBall7.setMaterials(Arrays.asList(ballSevenMaterial));
        ballSevenNode.setGeometry(mBall7);
        ballSevenNode.setPosition(new Vector(-1.75f, 5, 24));
        ballSevenNode.setLightReceivingBitMask(3);
        ballSevenNode.setShadowCastingBitMask(3);
        mBall7Physics = ballSevenNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballSevenNode);

        // Ball 8
        final Material ballEightMaterial = new Material();
        final Bitmap ballEightBitmap = getBitmapFromAssets(mActivity, "Ball6.jpg");
        final Texture ballEightTexture = new Texture(ballEightBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballEightMaterial.setDiffuseTexture(ballEightTexture);
        ballEightMaterial.setSpecularTexture(specularTexture);
        ballEightMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballEightNode = new Node();
        mBall8 = new Sphere(scaleSize(BALL_RADIUS));
        mBall8.setMaterials(Arrays.asList(ballEightMaterial));
        ballEightNode.setGeometry(mBall8);
        ballEightNode.setPosition(new Vector(-5.25f, 5, 24));
        ballEightNode.setLightReceivingBitMask(3);
        ballEightNode.setShadowCastingBitMask(3);
        mBall8Physics = ballEightNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballEightNode);

        // Ball 9
        final Material ballNineMaterial = new Material();
        final Bitmap ballNineBitmap = getBitmapFromAssets(mActivity, "Ball14.jpg");
        final Texture ballNineTexture = new Texture(ballNineBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballNineMaterial.setDiffuseTexture(ballNineTexture);
        ballNineMaterial.setSpecularTexture(specularTexture);
        ballNineMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballNineNode = new Node();
        mBall9 = new Sphere(scaleSize(BALL_RADIUS));
        mBall9.setMaterials(Arrays.asList(ballNineMaterial));
        ballNineNode.setGeometry(mBall9);
        ballNineNode.setPosition(new Vector(5.25f, 5, 24));
        ballNineNode.setLightReceivingBitMask(3);
        ballNineNode.setShadowCastingBitMask(3);
        mBall9Physics = ballNineNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballNineNode);

        // Ball 10
        final Material ballTenMaterial = new Material();
        final Bitmap ballTenBitmap = getBitmapFromAssets(mActivity, "Ball8.jpg");
        final Texture ballTenTexture = new Texture(ballTenBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballTenMaterial.setDiffuseTexture(ballTenTexture);
        ballTenMaterial.setSpecularTexture(specularTexture);
        ballTenMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballTenNode = new Node();
        mBall10 = new Sphere(scaleSize(BALL_RADIUS));
        mBall10.setMaterials(Arrays.asList(ballTenMaterial));
        ballTenNode.setGeometry(mBall10);
        ballTenNode.setPosition(new Vector(0f, 5, 18));
        ballTenNode.setLightReceivingBitMask(3);
        ballTenNode.setShadowCastingBitMask(3);
        mBall10Physics = ballTenNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballTenNode);

        // Ball 11
        final Material ballElevenMaterial = new Material();
        final Bitmap ballElevenBitmap = getBitmapFromAssets(mActivity, "Ball1.jpg");
        final Texture ballElevenTexture = new Texture(ballElevenBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballElevenMaterial.setDiffuseTexture(ballElevenTexture);
        ballElevenMaterial.setSpecularTexture(specularTexture);
        ballElevenMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballElevenNode = new Node();
        mBall11 = new Sphere(scaleSize(BALL_RADIUS));
        mBall11.setMaterials(Arrays.asList(ballElevenMaterial));
        ballElevenNode.setGeometry(mBall11);
        ballElevenNode.setPosition(new Vector(3.5f, 5, 18));
        ballElevenNode.setLightReceivingBitMask(3);
        ballElevenNode.setShadowCastingBitMask(3);
        mBall11Physics = ballElevenNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballElevenNode);

        // Ball 12
        final Material ballTwelveMaterial = new Material();
        final Bitmap ballTwelveBitmap = getBitmapFromAssets(mActivity, "Ball12.jpg");
        final Texture ballTwelveTexture = new Texture(ballTwelveBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballTwelveMaterial.setDiffuseTexture(ballTwelveTexture);
        ballTwelveMaterial.setSpecularTexture(specularTexture);
        ballTwelveMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballTwelveNode = new Node();
        mBall12 = new Sphere(scaleSize(BALL_RADIUS));
        mBall12.setMaterials(Arrays.asList(ballTwelveMaterial));
        ballTwelveNode.setGeometry(mBall12);
        ballTwelveNode.setPosition(new Vector(1.75f, 5, 12));
        ballTwelveNode.setLightReceivingBitMask(3);
        ballTwelveNode.setShadowCastingBitMask(3);
        mBall12Physics = ballTwelveNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballTwelveNode);

        // Ball 13
        final Material ballThirteenMaterial = new Material();
        final Bitmap ballThirteenBitmap = getBitmapFromAssets(mActivity, "Ball7.jpg");
        final Texture ballThirteenTexture = new Texture(ballThirteenBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballThirteenMaterial.setDiffuseTexture(ballThirteenTexture);
        ballThirteenMaterial.setSpecularTexture(specularTexture);
        ballThirteenMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballThirteenNode = new Node();
        mBall13 = new Sphere(scaleSize(BALL_RADIUS));
        mBall13.setMaterials(Arrays.asList(ballThirteenMaterial));
        ballThirteenNode.setGeometry(mBall13);
        ballThirteenNode.setPosition(new Vector(-1.75f, 5, 12));
        ballThirteenNode.setLightReceivingBitMask(3);
        ballThirteenNode.setShadowCastingBitMask(3);
        mBall13Physics = ballThirteenNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballThirteenNode);

        // Ball 14
        final Material ballFourteenMaterial = new Material();
        final Bitmap ballFourteenBitmap = getBitmapFromAssets(mActivity, "Ball9.jpg");
        final Texture ballFourteenTexture = new Texture(ballFourteenBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballFourteenMaterial.setDiffuseTexture(ballFourteenTexture);
        ballFourteenMaterial.setSpecularTexture(specularTexture);
        ballFourteenMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballFourteenNode = new Node();
        mBall14 = new Sphere(scaleSize(BALL_RADIUS));
        mBall14.setMaterials(Arrays.asList(ballFourteenMaterial));
        ballFourteenNode.setGeometry(mBall14);
        ballFourteenNode.setPosition(new Vector(0f, 5, 6));
        ballFourteenNode.setLightReceivingBitMask(3);
        ballFourteenNode.setShadowCastingBitMask(3);
        mBall14Physics = ballFourteenNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballFourteenNode);

        // Ball 15
        final Material ballFifteenMaterial = new Material();
        final Bitmap ballFifteenBitmap = getBitmapFromAssets(mActivity, "Ball15.jpg");
        final Texture ballFifteenTexture = new Texture(ballFifteenBitmap, Texture.TextureFormat.RGBA8, true, true);
        ballFifteenMaterial.setDiffuseTexture(ballFifteenTexture);
        ballFifteenMaterial.setSpecularTexture(specularTexture);
        ballFifteenMaterial.setLightingModel(Material.LightingModel.PHONG);
        final Node ballFifteenNode = new Node();
        mBall15 = new Sphere(scaleSize(BALL_RADIUS));
        mBall15.setMaterials(Arrays.asList(ballFifteenMaterial));
        ballFifteenNode.setGeometry(mBall15);
        ballFifteenNode.setPosition(new Vector(-3.5f, 5, 18));
        ballFifteenNode.setLightReceivingBitMask(3);
        ballFifteenNode.setShadowCastingBitMask(3);
        mBall15Physics = ballFifteenNode.initPhysicsBody(PhysicsBody.RigidBodyType.DYNAMIC, 0.170f, physicsShapeSphere);
        playGroundNode.addChildNode(ballFifteenNode);

        mScene.getRootNode().addChildNode(playGroundNode);

        mBalls = Arrays.asList(mCueBall);
        mPhysicsWorld = mScene.getPhysicsWorld();
        // mPhysicsWorld.setDebugDraw(true);
        mBallsPhysics = Arrays.asList(mBall1Physics, mBall2Physics, mBall3Physics, mBall4Physics, mBall5Physics,
                mBall6Physics, mBall7Physics, mBall8Physics, mBall9Physics, mBall10Physics,
                mBall11Physics, mBall12Physics, mBall13Physics, mBall14Physics, mBall15Physics);
        setDefaults();
    }

    private void setDefaults() {
        mCueBallPhysics.setUseGravity(true);
        mBallsPhysics.forEach(p -> p.setUseGravity(true));
        mBallsPhysics.forEach(p -> p.setRestitution((float) BALL_RESTITUTION));
        mBallsPhysics.forEach(p -> p.setFriction((float) BALL_FRICTION));
        mTablePhysics.setRestitution((float) TABLE_RESTITUTION);
        mTablePhysics.setFriction((float) TABLE_FRICTION);
        mRail1Physics.setRestitution((float) TABLE_RESTITUTION);
        mRail1Physics.setFriction((float) TABLE_FRICTION);
        mRail2Physics.setRestitution((float) TABLE_RESTITUTION);
        mRail2Physics.setFriction((float) TABLE_FRICTION);
        mRail3Physics.setRestitution((float) TABLE_RESTITUTION);
        mRail3Physics.setFriction((float) TABLE_FRICTION);
        mRail4Physics.setRestitution((float) TABLE_RESTITUTION);
        mRail4Physics.setFriction((float) TABLE_FRICTION);

    }

    private float scaleSize(final double actualSize) {
        return (float) (actualSize * SCALE_FACTOR);
    }

    @Test
    public void physicsTest() {
        testApplyImpulse();
        testApplyForce();
        testSetUseGravity();
        testSetEnabled();
        testSetMass();
        testSetFriction();
        testSetRestitution();
    }

    private void testApplyImpulse() {
        final List<Vector> directions = Arrays.asList(new Vector(0, 0, 10), new Vector(0, 0, -10));
        mMutableTestMethod = () -> {
            final Random rand = new Random();
            mCueBallPhysics.applyImpulse(directions.get(rand.nextInt(directions.size())), new Vector(0, 0, 0));
        };
        assertPass("Applying impulse to cue ball every second");
    }


    private void testApplyForce() {
        final List<Vector> directions = Arrays.asList(new Vector(0, 0, 10), new Vector(0, 0, -10));
        mMutableTestMethod = () -> {
            final Random rand = new Random();
            mCueBallPhysics.applyForce(directions.get(rand.nextInt(directions.size())), new Vector(0, 0, 0));
        };
        assertPass("Applying force to cue ball every second", () -> {
            mCueBallPhysics.clearForce();
        });
    }

    // TODO This test doesn't work. Figure out why.
    private void testSetUseGravity() {
        mMutableTestMethod = () -> {
            mCueBallPhysics.clearForce();
            mCueBallPhysics.setUseGravity(!mCueBallPhysics.getUseGravity());
            if (!mCueBallPhysics.getUseGravity()) {
                mCueBallPhysics.applyForce(new Vector(0, 0.5f, 0), new Vector(0, 0, 0));
            }
        };

        assertPass("Hitting the cue ball to bounce and toggling gravity on/off ", () -> {
            mCueBallPhysics.clearForce();
            mCueBallPhysics.setUseGravity(true);
        });
    }


    private void testSetEnabled() {
        final List<Vector> directions = Arrays.asList(new Vector(0, 0, 10), new Vector(0, 0, -10));
        mMutableTestMethod = () -> {
            final Random rand = new Random();
            mCueBallPhysics.applyImpulse(directions.get(rand.nextInt(directions.size())), new Vector(0, 0, 0));
            mBallsPhysics.forEach(p -> p.setEnabled(!p.isEnabled()));
        };

        assertPass("Hitting the cue ball and toggling physics enabled on/off on rest of the balls", () -> {
            mBallsPhysics.forEach(p -> p.setEnabled(true));
        });
    }

    private void testSetMass() {

        final List<Vector> directions = Arrays.asList(new Vector(0, 0, 10), new Vector(0, 0, -10));
        final List<Float> mass = Arrays.asList(new Float(0.170), new Float(1f), new Float(5f), new Float(10f));
        mMutableTestMethod = () -> {
            final Random rand = new Random();
            mCueBallPhysics.applyImpulse(directions.get(rand.nextInt(directions.size())), new Vector(0, 0, 0));
            mBallsPhysics.forEach(p -> p.setMass(mass.get(rand.nextInt(mass.size()))));
        };

        assertPass("Hitting the cue ball and setting random mass for balls", () -> {
            mBallsPhysics.forEach(p -> p.setMass(0.170f));
        });
    }

    // TODO getMomentOfInertia not present. Don't know what the default moment of inertia is
    private void testSetMomentOfInertia() {
        assertPass("Increasing moment of inertia of the magenta box along x axis");
    }

    private void testSetFriction() {
        final List<Vector> directions = Arrays.asList(new Vector(0, 0, 10), new Vector(0, 0, -10));
        mMutableTestMethod = () -> {
            final Random rand = new Random();
            mTablePhysics.setFriction(mTablePhysics.getFriction() + 0.1f);
            mCueBallPhysics.applyImpulse(directions.get(rand.nextInt(directions.size())), new Vector(0, 0, 0));
        };
        assertPass("Applying impulse to cue ball, while increasing table friction per second", () -> {
            mTablePhysics.setFriction(((float) TABLE_FRICTION));
        });

    }

    private void testSetRestitution() {
        final List<Vector> directions = Arrays.asList(new Vector(0, 0, 10), new Vector(0, 0, -10));
        mMutableTestMethod = () -> {
            final Random rand = new Random();
            mBallsPhysics.forEach(p -> p.setRestitution((float) (p.getRestitution() - 0.1)));
            mCueBallPhysics.applyImpulse(directions.get(rand.nextInt(directions.size())), new Vector(0, 0, 0));
        };
        assertPass("Applying impulse to cue ball, while decreasing restitution", () -> {
            mTablePhysics.setRestitution(((float) BALL_RESTITUTION));
        });

    }
}
