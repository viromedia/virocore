/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;
import java.util.ArrayList;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.ParticleEmitter.java
 * Java JNI Wrapper     : com.viro.renderer.ParticleEmitterJni.java
 * Cpp JNI wrapper      : ParticleEmitter_JNI.cpp
 * Cpp Object           : VROParticleEmitter.cpp
 */
public class ParticleEmitterJni {
    final protected long mNativeRef;

    public ParticleEmitterJni(RenderContextJni renderContext, NodeJni node, SurfaceJni surface) {
        mNativeRef = nativeCreateEmitter(renderContext.mNativeRef, node.mNativeRef, surface.mNativeRef);
    }

    public void destroy() {
        nativeDestroyEmitter(mNativeRef);
    }

    /*
     Basic emitter properties
     */
    public void setDelay(float value) {
        nativeSetDelay(mNativeRef, value);
    }

    public void setDuration(float value) {
        nativeSetDuration(mNativeRef, value);
    }

    public void setLoop(boolean value){
        nativeSetLoop(mNativeRef, value);
    }

    public void setRun(boolean value){
        nativeSetRun(mNativeRef, value);
    }

    public void setFixedToEmitter(boolean value){
        nativeSetFixedToEmitter(mNativeRef, value);
    }

    public void resetEmissionCycle(){
        nativeResetEmissionCycle(mNativeRef);
    }

    /*
     Spawn modifier properties
     */
    public void setEmissionRatePerSecond(int range[]) {
        nativeSetEmissionRatePerSecond(mNativeRef, range[0], range[1]);
    }

    public void setEmissionRatePerMeter(int range[]) {
        nativeSetEmissionRatePerMeter(mNativeRef, range[0], range[1]);
    }

    public void setParticleLifetime(int range[]) {
        nativeSetParticleLifetime(mNativeRef, range[0], range[1]);
    }

    public void setMaxParticles(int maxParticles) {
        nativeSetMaxParticles(mNativeRef, maxParticles);
    }

    public void setParticleBursts(ArrayList<ParticleBursts> bursts) {
        if (bursts.size() == 0){
            nativeSetParticleBursts(mNativeRef, null);
            return;
        }

        double burstsData[][] = new double[bursts.size()][6];
        for (int i = 0; i < bursts.size(); i ++){
            ParticleBursts burst = bursts.get(i);
            burstsData[i][1] = burst.burstStart;
            burstsData[i][2] = burst.burstMin;
            burstsData[i][3] = burst.burstMax;
            burstsData[i][4] = burst.burstPeriod;
            burstsData[i][5] = burst.burstCycles;

            String burstFactor = burst.burstFactor.toLowerCase();
            if (burstFactor.equals("time")){
                burstsData[i][0] = 1;
            } else {
                burstsData[i][0] = 0;
            }
        }

        nativeSetParticleBursts(mNativeRef, burstsData);
    }

    public void setSpawnVolume(String shape, float shapeParams[], boolean spawnOnSurface){
        nativeSetSpawnVolume(mNativeRef, shape, shapeParams, spawnOnSurface);
    }

    /*
     Appearance Modifier properties
     */
    public void setOpacityModifier(ParticleModifier modifier) {
        setModifier("opacity", modifier);
    }

    public void setScaleModifier(ParticleModifier modifier) {
        setModifier("scale", modifier);
    }

    public void setRotationModifier(ParticleModifier modifier) {
        setModifier("rotation", modifier);
    }

    public void setColorModifier(ParticleModifier modifier) {
        setModifier("color", modifier);
    }

    public boolean setBlendMode(String mode){
        return nativeSetParticleBlendMode(mNativeRef, mode);
    }

    public void setParticleBloomThreshold(float threshold){
        nativeSetBloomThreshold(mNativeRef, threshold);
    }

    /*
     Physics Modifier properties
     */
    public void setVelocityModifier(ParticleModifier modifier) {
        setModifier("velocity", modifier);
    }

    public void setAccelerationModifier(ParticleModifier modifier){
        setModifier("acceleration", modifier);
    }

    public void setExplosiveImpulse(float impluse, float position[], float deccelPeriod){
        nativeSetExplosiveImpulse(mNativeRef, impluse, position, deccelPeriod);
    }

    private void setModifier(String targetedModifier, ParticleModifier modifier){
        nativeSetParticleModifier(
                mNativeRef,
                targetedModifier,
                modifier.intialFactor,
                modifier.initialRange,
                modifier.interpolatedIntervals,
                modifier.interpolatedPoints);
    }

    public static final class ParticleBursts{
        final protected String burstFactor;
        final protected double burstStart;
        final protected double burstPeriod;
        final protected double burstMin;
        final protected double burstMax;
        final protected double burstCycles;

        public ParticleBursts(String factor, double startfactorValue, double period, double min, double max, double cycles) {
            burstFactor = factor;
            burstStart = startfactorValue;
            burstPeriod = period;
            burstMax = max;
            burstMin = min;
            burstCycles = cycles;
        }
    }

    public static final class ParticleModifier {
        final protected String intialFactor;
        final protected float initialRange[][];
        final protected float interpolatedIntervals[][];
        final protected float interpolatedPoints[][];

        public ParticleModifier(float[] initial){
            this(initial, initial);
        }

        public ParticleModifier(float[] min, float[] max){
            initialRange = new float[2][min.length];
            interpolatedIntervals = null;
            interpolatedPoints = null;
            initialRange[0] = min;
            initialRange[1] = max;
            intialFactor = "time";
        }

        public ParticleModifier(String factor, float range[][], float interval[][], float points[][]){
            intialFactor = factor;
            initialRange = range;
            interpolatedIntervals = interval;
            interpolatedPoints = points;
        }
    }

    /*
     Native Viro Particle Emitter JNI Functions
     */
    private native long nativeCreateEmitter(long contextRef, long nodeRef, long surfaceRef);
    private native void nativeDestroyEmitter(long ref);
    private native void nativeSetDelay(long ref, float value);
    private native void nativeSetDuration(long ref, float value);
    private native void nativeSetLoop(long ref, boolean value);
    private native void nativeSetRun(long ref, boolean value);
    private native void nativeSetFixedToEmitter(long ref, boolean value);
    private native void nativeResetEmissionCycle(long ref);
    private native void nativeSetEmissionRatePerSecond(long ref, int min, int max);
    private native void nativeSetEmissionRatePerMeter(long ref, int min, int max);
    private native void nativeSetParticleLifetime(long ref, int min, int max);
    private native void nativeSetSpawnVolume(long ref, String shape, float params[], boolean spawnOnSurface);
    private native void nativeSetMaxParticles(long ref, int value);
    private native void nativeSetParticleBursts(long ref, double bursts[][]);
    private native void nativeSetExplosiveImpulse(long ref, float impulse, float pos[], float deccelPeriod);
    private native void nativeSetParticleModifier(long ref, String modifier, String factor,
                                                  float range[][], float interval[][], float points[][]);
    private native boolean nativeSetParticleBlendMode(long ref, String blendMode);
    private native void nativeSetBloomThreshold(long ref, float threshold);
}
