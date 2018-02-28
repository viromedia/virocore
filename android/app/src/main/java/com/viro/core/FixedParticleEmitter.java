/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.FixedParticleEmitter.java
 * Java JNI Wrapper     : com.viro.renderer.ParticleEmitterJni.java
 * Cpp JNI wrapper      : FixedParticleEmitter_JNI.cpp
 * Cpp Object           : VROFixedParticleEmitter.cpp
 */
package com.viro.core;
import java.util.List;

/**
 * FixedParticleEmitter enables developers to create easily and configure a group of static
 * particle effects, where the particles themselves are fixed in world space.
 */
public class FixedParticleEmitter {
    long mNativeRef;

    /**
     * Create a new FixedParticleEmitter with individual particles that resemble the provided {@link
     * Surface}. To be used, the FixedParticleEmitter needs to be added to a {@link Node} through {@link
     * Node#setFixedParticleEmitter(FixedParticleEmitter)}. Note that a node can only contain either
     * a FixedParticleEmitter or a ParticleEmitter at any given point in time, not both (any existing
     * emitters will be unset before a new emitter is set on the node). Finally, the FixedParticleEmitter
     * will also conform to all the transforms of its parent Node, meaning it can be moved and oriented
     * with the scene graph.
     *
     * @param viroContext The ViroContext is required to generate particles.
     * @param surface     {@link Surface} representing how each individual particle should appear.
     *                    The Surface may be textured as well. If none is provided, a default
     *                    surface is used.
     */
    public FixedParticleEmitter(ViroContext viroContext, Surface surface) {
        long surfaceRef = 0;
        if (surface != null){
            surfaceRef = surface.mNativeRef;
        }
        mNativeRef = nativeCreateEmitter(viroContext.mNativeRef, surfaceRef);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this ParticleEmitter.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyEmitter(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Set a new surface to be associated and spawned with this particle emitter.
     */
    public void setSurface(Surface surface){
        if (surface == null || surface.mNativeRef == 0){
            return;
        }

        nativeSetEmitterSurface(mNativeRef, surface.mNativeRef);
    }

    /**
     * Sets a vector list of positional values representing the particles to be rendered
     * by this emitter in world space.
     */
    public void setParticles(List<Vector> positions){
        if (positions == null){
            return;
        }

        float[][] nativePosition = new float[positions.size()][3];
        for (int i = 0; i < positions.size(); i ++ ) {
            float[] vec = new float[3];
            Vector pos = positions.get(i);
            vec[0] = pos.x;
            vec[1] = pos.y;
            vec[2] = pos.z;
            nativePosition[i] = vec;
        }

        nativeSetParticles(mNativeRef, nativePosition);
    }

    /**
     * Clears all particles currently being rendered by this emitter.
     */
    public void clearParticles(){
        nativeClearParticles(mNativeRef);
    }

    private native long nativeCreateEmitter(long contextRef, long surfaceRef);
    private native void nativeDestroyEmitter(long ref);
    private native void nativeSetParticles(long ref, float[][] positions);
    private native void nativeClearParticles(long ref);
    private native void nativeSetEmitterSurface(long ref, long surfaceRef);
}
