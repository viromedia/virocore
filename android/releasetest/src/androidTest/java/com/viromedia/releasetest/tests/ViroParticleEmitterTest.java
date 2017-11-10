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
import com.viro.core.Node;
import com.viro.core.ParticleEmitter;
import com.viro.core.Surface;
import com.viro.core.Texture;
import com.viro.core.Vector;
import com.viro.core.ViroContext;
import org.junit.Test;

public class ViroParticleEmitterTest extends ViroBaseTest {
    private ParticleEmitter mEmitter;
    private Node mEmitterNode;

    public ViroParticleEmitterTest() {
        super();
    }

    private ParticleEmitter createBaseEmitter(){
        final ViroContext context = mViroView.getViroContext();
        float width = 2;
        float height = 2;

        // Construct particle Geometry.
        Surface nativeSurface = new Surface(1,1, 0, 0, 1, 1);
        nativeSurface.setWidth(width);
        nativeSurface.setHeight(height);

        // Construct Particle Image
        final Bitmap result = this.getBitmapFromAssets(mActivity, "particle_snow.png");
        final Texture snowTexture = new Texture(result, Texture.Format.RGBA8, true, true);
        nativeSurface.setImageTexture(snowTexture);

        // Finally construct the particle with the geometry and node.
        return new ParticleEmitter(context, nativeSurface);
    }

    private
    void runTest(Runnable test){
        prepareTest();
        test.run();
        cleanupTest();
    }

    /**
     * Constructs a new particle emitter and node and attaches it to the scene.
     */
    private void prepareTest(){
        if (mEmitter != null){
            mEmitter.dispose();
        }

        mEmitter = createBaseEmitter();

        // Prepare the node
        mEmitterNode = new Node();

        // Attach the emitter to the node.
        mEmitterNode.setPosition(new Vector(0, -10, -15));
        mEmitterNode.setParticleEmitter(mEmitter);

        // Attach the node to the scene.
        mScene.getRootNode().addChildNode(mEmitterNode);

        basicEmitterSetup();
    }

    /**
     * Detaches particle node from the scene and destroys it.
     */
    private void cleanupTest(){
        // Remove particle node from scene.
        mEmitterNode.removeFromParentNode();
        mEmitterNode.setParticleEmitter(null);
        mEmitter.dispose();
        mEmitterNode.dispose();
    }

    private void basicEmitterSetup(){
        mEmitter.setParticleLifetime(2000, 2000);
        mEmitter.setDuration(2000);
        mEmitter.setLoop(true);
        mEmitter.setDelay(0);
        mEmitter.setMaxParticles(200);
        mEmitter.setEmissionRatePerSecond(10, 10);
    }

    @Test
    public void particleEmitterTests() {
        runTest(()->{singleParticleTest();});
        runTest(()->{emitterColorTest();});
        runTest(()->{emitterScaleTest();});
        runTest(()->{emitterRotationTest();});
        runTest(()->{emitterOpacityTest();});
        runTest(()->{randomizedPropertiesTest();});
        runTest(()->{multipleIntervalTest();});
        runTest(()->{emitterDistanceTest();});
    }

    public void singleParticleTest() {
        mEmitter.setEmissionRatePerSecond(1, 1);
        mEmitter.run();
        assertPass("You should see 1 particle emit per second.");
    }

    public void emitterColorTest() {
        int colorStart = Color.RED;
        int colorEnd = Color.BLUE;

        ParticleEmitter.ParticleModifierColor mod
                = new ParticleEmitter.ParticleModifierColor(colorStart, colorStart);
        mod.addInterval(1000, colorEnd);
        mEmitter.setColorModifier(mod);
        mEmitter.run();

        assertPass("You should see the particle emit with red color and change to blue.");
    }


    public void emitterScaleTest() {
        Vector startSize = new Vector(1,1,0);
        Vector endSize = new Vector(3,3,0);

        ParticleEmitter.ParticleModifierVector mod
                = new ParticleEmitter.ParticleModifierVector(startSize, startSize);
        mod.addInterval(1000, endSize);
        mEmitter.setScaleModifier(mod);
        mEmitter.run();

        assertPass("Particles should scale.");
    }

    public void emitterRotationTest() {
        Vector startSize = new Vector(0,0,0);
        Vector endSize = new Vector(0,0,3.14);

        ParticleEmitter.ParticleModifierVector mod
                = new ParticleEmitter.ParticleModifierVector(startSize, startSize);
        mod.addInterval(1000, endSize);
        mEmitter.setRotationModifier(mod);
        mEmitter.run();

        assertPass("Particles should rotate.");
    }

    public void emitterOpacityTest() {
        float opacityStart = 1.0f;
        float opacityEnd = 0.0f;

        ParticleEmitter.ParticleModifierFloat mod
                = new ParticleEmitter.ParticleModifierFloat(opacityStart, opacityStart);
        mod.addInterval(1000, opacityEnd);
        mEmitter.setOpacityModifier(mod);
        mEmitter.run();

        assertPass("Particles should fade out.");
    }

    public void randomizedPropertiesTest(){
        float opacityStart = 1.0f;
        float opacityEnd = 0.0f;
        Vector startRot = new Vector(0,0,0);
        Vector endRot = new Vector(0,0,3.14);
        Vector startScale = new Vector(1,1,0);
        Vector endScale = new Vector(3,3,0);
        int colorStart = Color.RED;
        int colorEnd = Color.BLUE;


        ParticleEmitter.ParticleModifierVector mod1
                = new ParticleEmitter.ParticleModifierVector(startRot, endRot);
        mod1.addInterval(1000, endRot);
        mEmitter.setRotationModifier(mod1);

        ParticleEmitter.ParticleModifierVector mod2
                = new ParticleEmitter.ParticleModifierVector(startScale, endScale);
        mod2.addInterval(1000, endScale);
        mEmitter.setScaleModifier(mod2);

        ParticleEmitter.ParticleModifierColor mod3
                = new ParticleEmitter.ParticleModifierColor(colorStart, colorEnd);
        mod3.addInterval(1000, colorEnd);
        mEmitter.setColorModifier(mod3);

        ParticleEmitter.ParticleModifierFloat mod4
                = new ParticleEmitter.ParticleModifierFloat(opacityStart, opacityEnd);
        mod4.addInterval(1000, opacityEnd);
        mEmitter.setOpacityModifier(mod4);
        mEmitter.run();

        assertPass("Particles should change color, grow, fade out, rotate, non-uniformly.");
    }

    public void multipleIntervalTest(){
        int colorStart = Color.RED;
        int colorMid = Color.YELLOW;
        int ColorMid2 = Color.GREEN;
        int colorEnd = Color.BLUE;

        mEmitter.setParticleLifetime(3000,3000);
        ParticleEmitter.ParticleModifierColor mod
                = new ParticleEmitter.ParticleModifierColor(colorStart, colorStart);
        mod.addInterval(400, colorMid);
        mod.addInterval(1300, ColorMid2);
        mod.addInterval(2000, colorEnd);
        mEmitter.setColorModifier(mod);
        mEmitter.run();

        assertPass("Particles should turn into yellow, green, blue color.");
    }

    public void emitterDistanceTest(){
        int colorStart = Color.RED;
        int colorEnd = Color.BLUE;
        Vector startSize = new Vector(1,1,0);
        Vector endSize = new Vector(0,0,0);

        ParticleEmitter.ParticleModifierVector modScale
                = new ParticleEmitter.ParticleModifierVector(startSize, startSize);
        modScale.setFactor(ParticleEmitter.Factor.DISTANCE);
        modScale.addInterval(5, endSize);
        mEmitter.setScaleModifier(modScale);

        ParticleEmitter.ParticleModifierColor mod
                = new ParticleEmitter.ParticleModifierColor(colorStart, colorStart);
        mod.setFactor(ParticleEmitter.Factor.DISTANCE);
        mod.addInterval(2, colorEnd);
        mEmitter.setColorModifier(mod);

        mEmitter.run();

        assertPass("Distance Test: Particles should turn blue and shrink the further away it is from the spawn point.");
    }

    @Override
    void configureTestScene() {
        // No-op.
    }
}
