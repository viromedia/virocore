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
 * FixedParticleEmitter enables you to render groups of fixed particles. The particles do not
 * naturally move: they stay fixed in place until you manually change their position. This is useful
 * for rendering large numbers of identical small objects, like point clouds.
 */
public class FixedParticleEmitter {
    long mNativeRef;

    /**
     * Create a new FixedParticleEmitter where each particle is modeled after the provided {@link
     * Surface}. To be used, the positions of each particle must be set with {@link
     * #setParticles(List)}, and the FixedParticleEmitter must be added to a {@link Node} through
     * {@link Node#setFixedParticleEmitter(FixedParticleEmitter)}.
     * <p>
     * <p>
     * Note that a node can only contain either a FixedParticleEmitter or a ParticleEmitter at any
     * given point in time, not both (any existing emitters will be unset before a new emitter
     * becomes active).
     * <p>
     * The FixedParticleEmitter will conform to all the transforms of its parent Node, meaning it
     * can be moved and oriented with the scene graph.
     * <p>
     *
     * @param viroContext The ViroContext is required to generate particles.
     * @param surface     {@link Surface} representing how each individual particle should appear.
     *                    The Surface may be textured as well. If none is provided, a default
     *                    surface is used.
     */
    public FixedParticleEmitter(ViroContext viroContext, Surface surface) {
        long surfaceRef = 0;
        if (surface != null) {
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
     * Release native resources associated with this FixedParticleEmitter.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyEmitter(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Set a new {@link Surface} to redefine the appearance of each particle in this emitter.
     */
    public void setSurface(Surface surface){
        if (surface == null || surface.mNativeRef == 0){
            return;
        }

        nativeSetEmitterSurface(mNativeRef, surface.mNativeRef);
    }

    /**
     * Set the list of positions at which to render particles. Each particle will appear identical
     * to the {@link Surface} used by this FixedParticleEmitter. Any existing particles rendered by
     * this FixedParticleEmitter will be removed and replaced by these new particles.
     *
     * @param positions {@link List} of positions at which to render particles.
     */
    public void setParticles(List<Vector> positions) {
        if (positions == null) {
            return;
        }

        float[][] nativePosition = new float[positions.size()][3];
        for (int i = 0; i < positions.size(); i++) {
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
