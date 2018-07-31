/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.ParticleEmitter.java
 * Java JNI Wrapper     : com.viro.renderer.ParticleEmitterJni.java
 * Cpp JNI wrapper      : ParticleEmitter_JNI.cpp
 * Cpp Object           : VROParticleEmitter.cpp
 */
package com.viro.core;
import android.graphics.Color;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ParticleEmitter enables developers to create and configure quad emitters for building complex and
 * intricate particle effects. Some examples are smoke, rain, confetti, and fireworks.
 * <p>
 * ParticleEmitters are particle factories that create, contain and recycle a pool of particles.
 * Groups of particle systems can be used in conjunction with one another to create a composite
 * effect. For example, a campfire may include a flame emitter, a smoke emitter, and an ember or
 * sparks emitter.
 * <p>
 * For an extended discussion of particle systems in Viro, refer to the <a
 * href="https://virocore.viromedia.com/docs/particle-effects">Particle Effects Guide</a>.
 */
public class ParticleEmitter {

    /**
     * Interface for volumes that can be defined for spawning particles. By default, ParticleEmitter
     * spawns particles at the location of the emitter. This is useful for effects tied to a single
     * source, like smoke rising from a chimney. However, other effects may require more complex
     * spawn volumes; for example, spawning snow or rain over an area of land.
     * <p>
     * By setting a spawn volume via {@link #setSpawnVolume(SpawnVolume, boolean)}, you can specify
     * the shape within which to spawn particles.
     * <p>
     * All particles will spawn in a uniformly distributed pattern within the shape.
     * <p>
     */
    public interface SpawnVolume {
        /**
         * @hide
         */
        String getName();

        /**
         * @hide
         */
        float[] getParams();
    }

    /**
     * For spawning particles at a particular point in the parent Node's coordinate system.
     */
    public static class SpawnVolumePoint implements SpawnVolume {
        private final Vector mPoint;

        /**
         * Create a new SpawnVolumePoint.
         *
         * @param point The point in the parent Node's coordinate system at which particles will be
         *              spawned.
         */
        public SpawnVolumePoint(Vector point) {
            mPoint = point;
        }

        @Override
        public String getName() {
            return "Point";
        }

        @Override
        public float[] getParams() {
            return mPoint.toArray();
        }
    }

    /**
     * For spawning particles across or within a box.
     */
    public static class SpawnVolumeBox implements SpawnVolume {
        final float mWidth, mHeight, mLength;

        /**
         * Create a new SpawnVolumeBox.
         *
         * @param width  The width of the box (X dimension).
         * @param height The height of the box (Y dimension).
         * @param length The length of the box (Z dimension).
         */
        public SpawnVolumeBox(float width, float height, float length) {
            mWidth = width;
            mHeight = height;
            mLength = length;
        }

        @Override
        public String getName() {
            return "Box";
        }
        @Override
        public float[] getParams() {
            return new float[] { mWidth, mHeight, mLength };
        }
    }

    /**
     * For spawning particles aross or within a sphere.
     */
    public static class SpawnVolumeSphere implements SpawnVolume {
        final float mRadius;

        /**
         * Create a new SpawnVolumeSphere.
         *
         * @param radius The radius of the sphere.
         */
        public SpawnVolumeSphere(float radius) {
            mRadius = radius;
        }

        @Override
        public String getName() {
            return "Sphere";
        }
        @Override
        public float[] getParams() {
            return new float[] { mRadius };
        }
    }

    /**
     * For spawning particles across or within an ellipsoid.
     */
    public static class SpawnVolumeEllipsoid implements SpawnVolume {
        final float mX, mY, mZ;

        /**
         * Create a new SpawnVolumeEllipsoid.
         *
         * @param x The length of the X principal axis.
         * @param y The length of the Y principal axis.
         * @param z The length of the Z principal axis.
         */
        public SpawnVolumeEllipsoid(float x, float y, float z) {
            mX = x;
            mY = y;
            mZ = z;
        }

        @Override
        public String getName() {
            return "Sphere";
        }
        @Override
        public float[] getParams() {
            return new float[] { mX, mY, mZ };
        }
    }

    /**
     * Factor is used to distinguish between various particle emission properties that can either
     * be set as a rate per <tt>second</tt> or per <tt>meter</tt>.
     */
    public enum Factor {
        /**
         * The property will be based on units of time (seconds).
         */
        TIME("time"),

        /**
         * The property will be based on units of distance (meters).
         */
        DISTANCE("distance");

        private String mStringValue;
        private Factor(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, Factor> map = new HashMap<String, Factor>();
        static {
            for (Factor value : Factor.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         */
        public static Factor valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    /**
     * EmissionBurst defines moments of instantaneous particle creation. Certain particle effects
     * may require the ability to instantaneously spawn <tt>N</tt> number of particles all at once
     * in a single burst. Fireworks, sparks, and explosions are some examples. To create these kinds
     * of effects, use EmissionBurst.
     */
    public static final class EmissionBurst {
        final protected Factor burstFactor;
        final protected float burstStart;
        final protected float burstPeriod;
        final protected int burstMin;
        final protected int burstMax;
        final protected int burstCycles;

        /**
         * Create a new EmissionBurst to instantaneously create N particles, where N is randomly
         * chosen from the uniform distribution defined by <tt>min</tt> and <tt>max</tt>. The burst
         * will start at the provided time (or distance), and will loop for the given number of
         * cycles, with a cooldown period between each cycle.
         *
         * @param factor           The {@link Factor} indicating whether this EmissionBurst will
         *                         start after a certain time, or after a certain distance traveled
         *                         by the {@link ParticleEmitter}.
         * @param startFactorValue If <tt>factor</tt> is {@link Factor#TIME}, then this defines the
         *                         number of milliseconds after the start time of the last emission
         *                         cycle that this burst will start. If <tt>factor</tt> is {@link
         *                         Factor#DISTANCE}, then this defines the number of meters the
         *                         ParticleSystem must travel from the start position of the last
         *                         emission cycle for this burst to start.
         * @param min              The minimum number of particles to create in this EmissionBurst.
         * @param max              The maximum number of particles to create in this EmissionBurst.
         * @param cycles           The number of times to loop the EmissionBurst.
         * @param cooldownPeriod   The duration, in milliseconds, between each cycle.
         */
        public EmissionBurst(Factor factor, float startFactorValue, int min, int max, int cycles, float cooldownPeriod) {
            burstFactor = factor;
            burstStart = startFactorValue;
            burstPeriod = cooldownPeriod;
            burstMax = max;
            burstMin = min;
            burstCycles = cycles;
        }
    }

    /**
     * ParticleModifier is used by various {@link ParticleEmitter} properties to set not only how
     * particles will initially behave, but how they will behave over time or across distance. For
     * example, you can use ParticleModifier with {@link ParticleEmitter#setOpacityModifier(ParticleModifierFloat)}
     * to make particles initially appear with 50% to 75% opacity, and then have their opacity
     * decline to 20% over 4000 ms.
     * <p>
     * ParticleModifier is always constructed with the initial range for the property. You can then
     * add time or distance intervals specifying how the property should change over time for each
     * particle. See the ParticleModifier subclasses for details.
     */
    public interface ParticleModifier {
        /**
         * @hide
         */
        Factor getFactor();
        /**
         * @hide
         */
        float[][] getInitialRange();          // [x, y, z]
        /**
         * @hide
         */
        float[][] getInterpolatedIntervals(); // [start, end]
        /**
         * @hide
         */
        float[][] getInterpolatedPoints();     // [x, y, z]
    }

    /**
     * Used by the Bridge.
     * @hide
     */
    //#IFDEF 'viro_react'
    public static class ParticleModifierFloatArray implements ParticleModifier {
        private Factor initialFactor;
        private float initialRange[][];
        private float interpolatedIntervals[][];
        private float interpolatedPoints[][];

        public ParticleModifierFloatArray(float[] initial) {
            this(initial, initial);
        }

        public ParticleModifierFloatArray(float[] min, float[] max) {
            initialRange = new float[2][min.length];
            initialRange[0] = min;
            initialRange[1] = max;

            interpolatedIntervals = null;
            interpolatedPoints = null;
            initialFactor = Factor.TIME;
        }

        public ParticleModifierFloatArray(Factor factor, float range[][], float interval[][], float points[][]) {
            initialFactor = factor;
            initialRange = range;
            interpolatedIntervals = interval;
            interpolatedPoints = points;
        }


        @Override
        public Factor getFactor() {
            return initialFactor;
        }

        @Override
        public float[][] getInitialRange() {
            return initialRange;
        }

        @Override
        public float[][] getInterpolatedIntervals() {
            return interpolatedIntervals;
        }
        @Override
        public float[][] getInterpolatedPoints() {
            return interpolatedPoints;
        }
    }
    //#ENDIF
    /**
     * ParticleModifier for floating point properties.
     */
    public static class ParticleModifierFloat implements ParticleModifier {
        private Factor mFactor = Factor.TIME;
        private float mInitialMin;
        private float mInitialMax;
        private List<Float> mEndValues = new ArrayList<Float>();
        private List<Vector> mIntervals = new ArrayList<Vector>();

        /**
         * Construct a new ParticleModifier that initializes the property for each particle to
         * a random value chosen from the given uniform distribution.
         *
         * @param initialMin The minimum value of the uniform distribution.
         * @param initialMax The maximum value of the uniform distribution.
         */
        public ParticleModifierFloat(float initialMin, float initialMax) {
            mInitialMin = initialMin;
            mInitialMax = initialMax;
            mFactor = Factor.TIME;
        }

        /**
         * Set the {@link Factor} that defines the units used for the intervals. If set to
         * {@link Factor#TIME}, then each interval will be in milliseconds; if the factor
         * is set to {@link Factor#DISTANCE}, then the interval will be in meters that the
         * {@link ParticleEmitter} has traveled.
         * <p>
         * Default is {@link Factor#TIME}.
         *
         * @param factor The {@link Factor} to use for this modifier.
         */
        public void setFactor(Factor factor) {
            mFactor = factor;
        }

        /**
         * Add an interval to this {@link ParticleModifier} that will move all particles to the
         * given <tt>endValue</tt> at the end of the interval.
         *
         * @param intervalLength The length of the interval. If factor is set to {@link
         *                       Factor#TIME}, then this parameter defines the length of the
         *                       interval in milliseconds; if factor is set to {@link
         *                       Factor#DISTANCE}, then this parameter defines the length of the
         *                       interval in meters that the {@link ParticleEmitter} has traveled.
         * @param endValue       The value all particles will interpolate toward across the
         *                       interval. All particles will finish with this value.
         */
        public void addInterval(float intervalLength, float endValue) {
            float intervalStart = 0;
            if (mIntervals.size() > 0) {
                intervalStart = mIntervals.get(mIntervals.size() - 1).y;
            }
            mIntervals.add(new Vector(intervalStart, intervalStart + intervalLength, 0));
            mEndValues.add(endValue);
        }

        /**
         * @hide
         */
        @Override
        public Factor getFactor() {
            return mFactor;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInitialRange() {
            float[][] range = new float[2][3];
            range[0] = new float[] { mInitialMin, 0, 0 };
            range[1] = new float[] { mInitialMax, 0, 0 };
            return range;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInterpolatedIntervals() {
            if (mIntervals.size() == 0) {
                return null;
            }
            float[][] intervals = new float[mIntervals.size()][2];
            for (int i = 0; i < intervals.length; i++) {
                intervals[i][0] = mIntervals.get(i).x;
                intervals[i][1] = mIntervals.get(i).y;
            }
            return intervals;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInterpolatedPoints() {
            if (mEndValues.size() == 0) {
                return null;
            }
            float[][] points = new float[mEndValues.size()][3];
            for (int i = 0; i < points.length; i++) {
                points[i][0] = mEndValues.get(i);
                points[i][1] = 0;
                points[i][2] = 0;
            }
            return points;
        }
    }

    /**
     * ParticleModifier for color properties.
     */
    public static class ParticleModifierColor implements ParticleModifier {
        private Factor mFactor = Factor.TIME;
        private int mInitialMin;
        private int mInitialMax;
        private List<Integer> mEndValues = new ArrayList<Integer>();
        private List<Vector> mIntervals = new ArrayList<Vector>();

        /**
         * Construct a new ParticleModifier that initializes the property for each particle to
         * a random value chosen from the given uniform distribution. Each int is derived
         * from android.graphics.Color.
         *
         * @param initialMin The minimum value of the uniform distribution.
         * @param initialMax The maximum value of the uniform distribution.
         */
        public ParticleModifierColor(int initialMin, int initialMax) {
            mInitialMin = initialMin;
            mInitialMax = initialMax;
            mFactor = Factor.TIME;
        }

        /**
         * Set the {@link Factor} that defines the units used for the intervals. If set to
         * {@link Factor#TIME}, then each interval will be in milliseconds; if the factor
         * is set to {@link Factor#DISTANCE}, then the interval will be in meters that the
         * {@link ParticleEmitter} has traveled.
         * <p>
         * Default is {@link Factor#TIME}.
         *
         * @param factor The {@link Factor} to use for this modifier.
         */
        public void setFactor(Factor factor) {
            mFactor = factor;
        }

        /**
         * Add an interval to this {@link ParticleModifier} that will move all particles to the
         * given <tt>endValue</tt> at the end of the interval.
         *
         * @param intervalLength The length of the interval. If factor is set to {@link
         *                       Factor#TIME}, then this parameter defines the length of the
         *                       interval in milliseconds; if factor is set to {@link
         *                       Factor#DISTANCE}, then this parameter defines the length of the
         *                       interval in meters that the {@link ParticleEmitter} has traveled.
         * @param endValue       The value all particles will interpolate toward across the
         *                       interval. This int should be derived from android.graphics.Color.
         *                       All particles will finish with this value.
         */
        public void addInterval(float intervalLength, int endValue) {
            float intervalStart = 0;
            if (mIntervals.size() > 0) {
                intervalStart = mIntervals.get(mIntervals.size() - 1).y;
            }
            mIntervals.add(new Vector(intervalStart, intervalStart + intervalLength, 0));
            mEndValues.add(endValue);
        }

        /**
         * @hide
         */
        @Override
        public Factor getFactor() {
            return mFactor;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInitialRange() {
            float[][] range = new float[2][3];
            range[0] = new float[] { Color.red(mInitialMin), Color.green(mInitialMin), Color.blue(mInitialMin) };
            range[1] = new float[] { Color.red(mInitialMax), Color.green(mInitialMax), Color.blue(mInitialMax) };
            return range;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInterpolatedIntervals() {
            if (mIntervals.size() == 0) {
                return null;
            }
            float[][] intervals = new float[mIntervals.size()][2];
            for (int i = 0; i < intervals.length; i++) {
                intervals[i][0] = mIntervals.get(i).x;
                intervals[i][1] = mIntervals.get(i).y;
            }
            return intervals;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInterpolatedPoints() {
            if (mEndValues.size() == 0) {
                return null;
            }
            float[][] points = new float[mEndValues.size()][3];
            for (int i = 0; i < points.length; i++) {
                int color = mEndValues.get(i);
                points[i][0] = Color.red(color);
                points[i][1] = Color.green(color);
                points[i][2] = Color.blue(color);
            }
            return points;
        }
    }

    /**
     * ParticleModifier for {@link Vector} properties.
     */
    public static class ParticleModifierVector implements ParticleModifier {
        private Factor mFactor = Factor.TIME;
        private Vector mInitialMin;
        private Vector mInitialMax;
        private List<Vector> mEndValues = new ArrayList<Vector>();
        private List<Vector> mIntervals = new ArrayList<Vector>();

        /**
         * Construct a new ParticleModifier that initializes the property for each particle to
         * a random value chosen from the given uniform distribution.
         *
         * @param initialMin The minimum value of the uniform distribution.
         * @param initialMax The maximum value of the uniform distribution.
         */
        public ParticleModifierVector(Vector initialMin, Vector initialMax) {
            mInitialMin = initialMin;
            mInitialMax = initialMax;
            mFactor = Factor.TIME;
        }

        /**
         * Set the {@link Factor} that defines the units used for the intervals. If set to
         * {@link Factor#TIME}, then each interval will be in milliseconds; if the factor
         * is set to {@link Factor#DISTANCE}, then the interval will be in meters that the
         * {@link ParticleEmitter} has traveled.
         * <p>
         * Default is {@link Factor#TIME}.
         *
         * @param factor The {@link Factor} to use for this modifier.
         */
        public void setFactor(Factor factor) {
            mFactor = factor;
        }

        /**
         * Add an interval to this {@link ParticleModifier} that will move all particles to the
         * given <tt>endValue</tt> at the end of the interval.
         *
         * @param intervalLength The length of the interval. If factor is set to {@link
         *                       Factor#TIME}, then this parameter defines the length of the
         *                       interval in milliseconds; if factor is set to {@link
         *                       Factor#DISTANCE}, then this parameter defines the length of the
         *                       interval in meters that the {@link ParticleEmitter} has traveled.
         * @param endValue       The value all particles will interpolate toward across the
         *                       interval. All particles will finish with this value.
         */
        public void addInterval(float intervalLength, Vector endValue) {
            float intervalStart = 0;
            if (mIntervals.size() > 0) {
                intervalStart = mIntervals.get(mIntervals.size() - 1).y;
            }
            mIntervals.add(new Vector(intervalStart, intervalStart + intervalLength, 0));
            mEndValues.add(endValue);
        }

        /**
         * @hide
         */
        @Override
        public Factor getFactor() {
            return mFactor;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInitialRange() {
            float[][] range = new float[2][3];
            range[0] = mInitialMin.toArray();
            range[1] = mInitialMax.toArray();
            return range;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInterpolatedIntervals() {
            if (mIntervals.size() == 0) {
                return null;
            }
            float[][] intervals = new float[mIntervals.size()][2];
            for (int i = 0; i < intervals.length; i++) {
                intervals[i][0] = mIntervals.get(i).x;
                intervals[i][1] = mIntervals.get(i).y;
            }
            return intervals;
        }
        /**
         * @hide
         */
        @Override
        public float[][] getInterpolatedPoints() {
            if (mEndValues.size() == 0) {
                return null;
            }
            float[][] points = new float[mEndValues.size()][3];
            for (int i = 0; i < points.length; i++) {
                points[i] = mEndValues.get(i).toArray();
            }
            return points;
        }
    }

    long mNativeRef;
    // TODO Move all these magic numbers to default member fields. Until then, if you change this
    // value, change in the builder class too
    private int mDelay = 0;
    private int mDuration = 2000;
    private boolean mLoop = true;
    private boolean mRunning = false;
    private boolean mFixedToEmitter = true;
    private int mEmissionRatePerSecondMin = 10;
    private int mEmissionRatePerSecondMax = 10;
    private int mEmissionRatePerMeterMin = 0;
    private int mEmissionRatePerMeterMax = 0;
    private int mParticleLifetimeMin = 2000;
    private int mParticleLifetimeMax = 2000;
    private int mMaxParticles = 500;
    private List<EmissionBurst> mEmissionBursts = new ArrayList<EmissionBurst>();
    private SpawnVolume mSpawnVolume = new SpawnVolumePoint(new Vector(0, 0, 0));
    private boolean mSpawnOnSurface = true;
    private Material.BlendMode mBlendMode = Material.BlendMode.ALPHA;
    private float mBloomThreshold = -1;

    /**
     * Create a new ParticleEmitter with individual particles that resemble the provided {@link
     * Surface}. To be used, the ParticleEmitter needs to be added to a {@link Node} through {@link
     * Node#setParticleEmitter(ParticleEmitter)}. The ParticleEmitter will conform to all the
     * transforms of its parent Node, meaning it can be moved and oriented with the scene graph.
     *
     * @param viroContext The ViroContext is required to generate particles.
     * @param surface     {@link Surface} representing how each individual particle should appear.
     *                    The Surface may be textured as well.
     * @deprecated use {@link ParticleEmitter#ParticleEmitter(ViroContext, Quad)} instead
     */
    public ParticleEmitter(ViroContext viroContext, Surface surface) {
        mNativeRef = nativeCreateEmitter(viroContext.mNativeRef, surface.mNativeRef);
    }

    /**
     * Create a new ParticleEmitter with individual particles that resemble the provided {@link
     * Quad}. To be used, the ParticleEmitter needs to be added to a {@link Node} through {@link
     * Node#setParticleEmitter(ParticleEmitter)}. The ParticleEmitter will conform to all the
     * transforms of its parent Node, meaning it can be moved and oriented with the scene graph.
     *
     * @param viroContext The ViroContext is required to generate particles.
     * @param quad     {@link Quad} representing how each individual particle should appear.
     *                    The Quad may be textured as well.
     */
    public ParticleEmitter(ViroContext viroContext, Quad quad) {
        mNativeRef = nativeCreateEmitter(viroContext.mNativeRef, quad.mNativeRef);
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
     * Set the delay in milliseconds to apply before this ParticleEmitter starts a new emission
     * cycle. Particles are produced during each emission cycle.
     * <p>
     * Default is 0 milliseconds.
     *
     * @param delayMillis The delay in milliseconds.
     */
    public void setDelay(int delayMillis) {
        mDelay = delayMillis;
        nativeSetDelay(mNativeRef, delayMillis);
    }

    /**
     * Get the delay in milliseconds to apply between emission cycles.
     *
     * @return The delay in milliseconds.
     */
    public int getDelay() {
        return mDelay;
    }

    /**
     * Set the duration of each emission cycle in milliseconds. Particles are produced continually
     * during each emission cycle.
     * <p>
     * The default is 2000 milliseconds.
     *
     * @param durationMillis The duration in milliseconds.
     */
    public void setDuration(int durationMillis) {
        mDuration = durationMillis;
        nativeSetDuration(mNativeRef, durationMillis);
    }

    /**
     * Get the duration of each emission cycle in milliseconds.
     *
     * @return The duration in milliseconds.
     */
    public int getDuration() {
        return mDuration;
    }

    /**
     * Set to true if the emitter should restart emitting particles at the end of each emission
     * cycle, after the delay.
     * <p>
     * Default is true.
     *
     * @param loop True to continually run emission cycles.
     */
    public void setLoop(boolean loop) {
        mLoop = loop;
        nativeSetLoop(mNativeRef, loop);
    }

    /**
     * Run the ParticleEmitter. This will wait for <tt>delay</tt> milliseconds then start the
     * first emission cycle. The cycle will continue for its set <tt>duration</tt>, and then
     * another cycle will start after the delay if <tt>loop</tt> is enabled. Emissions can be
     * stopped at any time by invoking {@link #pause()}.
     */
    public void run() {
        mRunning = true;
        nativeSetRun(mNativeRef, true);
    }

    /**
     * Pause the ParticleEmitter. This will immediately cease the generation of new particles,
     * but existing particles will live out their remaining lifetimes.
     */
    public void pause() {
        mRunning = false;
        nativeSetRun(mNativeRef, false);
    }

    /**
     * Returns true if the ParticleEmitter is not running.
     *
     * @return True if the ParticleEmitter is paused.
     */
    public boolean isPaused() {
        return !mRunning;
    }

    /**
     * Set whether the particles generated from this ParticleEmitter are fixed to the emitter's
     * location, or are free. When true, Viro uses the emitter's current position to determine each
     * particle's appearance and movement; when false, Viro uses each particle's individual spawn
     * location.
     * <p>
     * When false, particles are not "locked" to the emitter: they
     * can float away. For example, smoke particles from a moving train would continue floating
     * upward from the location they were spawned. When true, the smoke
     * particles would be "locked" to the train's emitter: they would always move with the train in
     * reference to the emitter's location.
     * <p>
     * Default is true.
     *
     * @param fixed True to fix particles to the emitter location.
     */
    public void setFixedToEmitter(boolean fixed) {
        mFixedToEmitter = fixed;
        nativeSetFixedToEmitter(mNativeRef, fixed);
    }

    /**
     * Returns true if the particles in this ParticleEmitter are fixed to the emitter's location.
     * See {@link #setFixedToEmitter(boolean)} for discussion.
     *
     * @return True if the particles are fixed to the emitter location.
     */
    public boolean isFixedToEmitter() {
        return mFixedToEmitter;
    }

    /**
     * Resets this ParticleEmitter to the beginning of its emission cycle. This will also clear
     * all existing particles.
     */
    public void resetEmissionCycle() {
        nativeResetEmissionCycle(mNativeRef);
    }

    /**
     * Set the number of particles this emitter spawns per second, during an emission cycle. The
     * value is chosen randomly from the uniform distribution defined by <tt>min</tt> and
     * <tt>max</tt>.
     * <p>
     * How fast an emitter produces particles is determined by either {@link
     * #setEmissionRatePerSecond(int, int)} or {@link #setEmissionRatePerMeter(int, int)} (in which the
     * particle output is proportional to how fast the emitter itself is moving). Depending on the
     * effect you wish to create, you may decide to one or both properties. For example, in a
     * situation where steam particles are emitted from a train upon movement, {@link
     * #setEmissionRatePerMeter(int, int)} is the sensible choice. This is also true if you, say, have
     * a spaceship that emits more smoke as it accelerates.
     * <p>
     * Default value is min 10, max 10.
     *
     * @param min The minimum number of particles to emit per second.
     * @param max The maximum number of particles to emit per second.
     */
    public void setEmissionRatePerSecond(int min, int max) {
        mEmissionRatePerSecondMin = min;
        mEmissionRatePerSecondMax = max;
        nativeSetEmissionRatePerSecond(mNativeRef, min, max);
    }

    /**
     * Get the number of particles emitted per second by this ParticleEmitter during an emission
     * cycle. This value is picked randomly from the returned uniform distribution. See {@link
     * #setEmissionRatePerSecond(int, int)} for discussion.
     *
     * @return The float[] with the minimum and maximum number of particles emitted per second.
     */
    public float[] getEmissionRatePerSecond() {
        return new float[] { mEmissionRatePerSecondMin, mEmissionRatePerSecondMax };
    }

    /**
     * Set the number of particles this ParticleEmitter spawns per meter traveled, during an
     * emission cycle. The value is chosen randomly from the uniform distribution defined by
     * <tt>min</tt> and <tt>max</tt>.
     * <p>
     * How fast an emitter produces particles is determined by either {@link
     * #setEmissionRatePerSecond(int, int)} or {@link #setEmissionRatePerMeter(int, int)} (in which
     * the particle output is proportional to how fast the emitter itself is moving). Depending on
     * the effect you wish to create, you may decide to one or both properties. For example, in a
     * situation where steam particles are emitted from a train upon movement, {@link
     * #setEmissionRatePerMeter(int, int)} is the sensible choice. This is also true if you, say,
     * have a spaceship that emits more smoke as it accelerates.
     * <p>
     * Default value is min 0, max 0, meaning zero particles produced per meter traveled.
     *
     * @param min The minimum number of particles to emit per meter.
     * @param max The maximum number of particles to emit per meter.
     */
    public void setEmissionRatePerMeter(int min, int max) {
        mEmissionRatePerMeterMin = min;
        mEmissionRatePerMeterMax = max;
        nativeSetEmissionRatePerMeter(mNativeRef, min, max);
    }

    /**
     * Get the number of particles emitted per meter traveled by this ParticleEmitter during an
     * emission cycle. This value is picked randomly from the returned uniform distribution. See
     * {@link #setEmissionRatePerMeter(int, int)} for discussion.
     *
     * @return The float[] with the minimum and maximum number of particles emitted per meter.
     */
    public float[] getEmissionRatePerMeter() {
        return new float[] { mEmissionRatePerSecondMin, mEmissionRatePerSecondMax };
    }

    /**
     * Set the lifetime of each particle, in milliseconds. The lifetime for each particle will be
     * chosen randomly from the uniform distribution defined by <tt>min</tt> and <tt>max</tt>.
     * <p>
     * How long a particle lives for (or how fast a particle dies) is defined by its lifetime.
     * Specifying the particle lifetime and the particle emisssion rate (via {@link
     * #setEmissionRatePerMeter(int, int)} or {@link #setEmissionRatePerSecond(int, int)})
     * determines the eventual steady state of the ParticleEmitter. In this steady state, there will
     * be a roughly constant number of existing, spawned particles for the emitter.
     * <p>
     * Default value is min 2000, max 2000.
     *
     * @param min The minimum lifetime of a particle, in milliseconds.
     * @param max The maximum lifetime of a particle, in milliseconds.
     */
    public void setParticleLifetime(int min, int max) {
        mParticleLifetimeMin = min;
        mParticleLifetimeMax = max;
        nativeSetParticleLifetime(mNativeRef, min, max);
    }

    /**
     * Get the lifetime of particles in milliseconds. This lifetime of each particle is picked
     * randomly from the returned uniform distribution. See {@link #setParticleLifetime(int, int)}
     * for discussion.
     *
     * @return The float[] with the minimum and maximum lifetime of each particle, in milliseconds.
     */
    public float[] getParticleLifetime() {
        return new float[] { mParticleLifetimeMin, mParticleLifetimeMax };
    }

    /**
     * Set the maximum number of <i>live</i> particles that can exist at any moment from this
     * ParticleEmitter. This includes particles created by {@link #setEmissionRatePerSecond(int,
     * int)}, {@link #setEmissionRatePerMeter(int, int)}, and {@link #setEmissionBursts(List)}. When
     * the cap is reached, new particles will only be spawned as existing particles die off.
     * <p>
     * Default value is 500.
     *
     * @param maxParticles The maximum number of life particles for this ParticleEmitter.
     */
    public void setMaxParticles(int maxParticles) {
        mMaxParticles = maxParticles;
        nativeSetMaxParticles(mNativeRef, maxParticles);
    }

    /**
     * Get the maximum number of live particles allowed for this ParticleEmitter. See {@link
     * #setMaxParticles(int)} for discussion.
     *
     * @return The maximum number of particles.
     */
    public int getMaxParticles() {
        return mMaxParticles;
    }

    /**
     * Set {@link EmissionBurst}s for this ParticleSystem for instantaneous particle creation. In
     * addition to emitting particles at a constant rate via {@link #setEmissionRatePerMeter(int,
     * int)} and {@link #setEmissionRatePerSecond(int, int)}, certain particle effects may require
     * the ability to instantaneously spawn <tt>N</tt> number of particles all at once in a single
     * burst. Fireworks, sparks, and explosions are some examples. To specify a burst, create {@link
     * EmissionBurst} objects. In each {@link EmissionBurst} you can specify a time (or distance
     * traveled) at which to burst a number of particles, in repetition if needed. These bursts are
     * done in conjunction with emission rates, and are also subjected to the same {@link
     * #setMaxParticles(int)} constraint.
     * <p>
     * There are no bursts assigned by default.
     *
     * @param bursts The {@link EmissionBurst} bursts to use in this ParticleSystem.
     */
    public void setEmissionBursts(List<EmissionBurst> bursts) {
        mEmissionBursts = bursts;
        if (bursts == null || bursts.size() == 0) {
            nativeSetParticleBursts(mNativeRef, null);
            return;
        }

        double burstsData[][] = new double[bursts.size()][6];
        for (int i = 0; i < bursts.size(); i ++) {
            EmissionBurst burst = bursts.get(i);
            burstsData[i][1] = burst.burstStart;
            burstsData[i][2] = burst.burstMin;
            burstsData[i][3] = burst.burstMax;
            burstsData[i][4] = burst.burstPeriod;
            burstsData[i][5] = burst.burstCycles;

            Factor burstFactor = burst.burstFactor;
            if (burstFactor == Factor.TIME) {
                burstsData[i][0] = 1;
            } else {
                burstsData[i][0] = 0;
            }
        }

        nativeSetParticleBursts(mNativeRef, burstsData);
    }

    /**
     * Get the {@link EmissionBurst}s for this ParticleSystem, which define moments of instantaneous
     * particle creation. See {@link #setEmissionBursts(List)} for discussion.
     *
     * @return List of each {@link EmissionBurst} used in this ParticleSystem.
     */
    public List<EmissionBurst> getEmissionBursts() {
        return mEmissionBursts;
    }

    /**
     * Set the {@link SpawnVolume}, which defines the volume over which particles will be created
     * for this ParticleEmitter.
     * <p>
     * By default, ParticleEmitter spawns particles at the location of the emitter. This is useful
     * for effects tied to a single source, like smoke rising from a chimney. However, other effects
     * may require more complex spawn volumes; for example, spawning snow or rain over an area of
     * land.
     * <p>
     * By setting a spawn volume, you can specify the shape within which to spawn particles.
     * Supported shapes are {@link SpawnVolumeBox} with width, height, and length, {@link
     * SpawnVolumeSphere} with radius, {@link SpawnVolumeEllipsoid} with x,y,z ellipsoid length, and
     * {@link SpawnVolumePoint}. All particles will spawn in a uniformly distributed pattern within
     * the shape.
     * <p>
     * Finally, there may be effects that require particles to spawn on the surface of a shape,
     * rather than within it. For example, fireworks require particles to be spawned on the surface
     * of a sphere. To achieve this effect, set <tt>spawnOnSurface</tt> to true. Particles will be
     * spawned in a uniformly distributed fashion on the surface of the specified shape.
     *
     * @param volume         The {@link SpawnVolume} on or within which to generate particles.
     * @param spawnOnSurface True to only spawn particles on the surface of the volume, false to
     *                       spawn particles within the volume as well.
     */
    public void setSpawnVolume(SpawnVolume volume, boolean spawnOnSurface) {
        nativeSetSpawnVolume(mNativeRef, volume.getName(), volume.getParams(), spawnOnSurface);
    }

    /**
     * Return the {@link SpawnVolume, which defines the volume over which particles will be created
     * for this ParticleEmitter. See {@link #setSpawnVolume(SpawnVolume, boolean)} for discussion.
     *
     * @return The {@link SpawnVolume} on or within which particles are generated.
     */
    public SpawnVolume getSpawnVolume() {
        return mSpawnVolume;
    }

    /**
     * Return true if particles are only generated on the surface of the ParticleEmitter's
     * {@link SpawnVolume}.
     *
     * @return True if particles only spawn on the surface, false if they spawn within the volume as
     * well.
     */
    public boolean getSpawnOnSurface() {
        return mSpawnOnSurface;
    }

    /**
     * Set the opacity of the particles generated by this ParticleEmitter. The {@link
     * ParticleModifierFloat} provided will define the initial opacity of the particles, and may
     * also indicate how the opacity will change over time or over distance.
     *
     * @param modifier The {@link ParticleModifierFloat} describing opacity behavior for the
     *                 particles.
     */
    public void setOpacityModifier(ParticleModifierFloat modifier) {
        setModifier("opacity", modifier);
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void setOpacityModifierLegacy(ParticleModifier modifier) {
        setModifier("opacity", modifier);
    }
    //#ENDIF

    /**
     * Set the scale of the particles generated by this ParticleEmitter. The {@link
     * ParticleModifierVector} provided will define the initial scale of the particles, and may
     * also indicate how scale will change over time or over distance. Each {@link Vector}
     * in the modifier defines the X, Y, and Z scale.
     *
     * @param modifier The {@link ParticleModifierVector} describing scale behavior for the
     *                 particles.
     */
    public void setScaleModifier(ParticleModifierVector modifier) {
        setModifier("scale", modifier);
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void setScaleModifierLegacy(ParticleModifier modifier) {
        setModifier("scale", modifier);
    }
    //#ENDIF

    /**
     * Set the rotation of the particles generated by this ParticleEmitter. The {@link
     * ParticleModifierVector} provided will define the initial rotation of the particles, and may
     * also indicate how rotation will change over time or over distance. Each {@link Vector}
     * in the modifier defines the X, Y, and Z Euler angle rotation in radians.
     *
     * @param modifier The {@link ParticleModifierVector} describing rotation behavior for the
     *                 particles.
     */
    public void setRotationModifier(ParticleModifierFloat modifier) {
        // Convert the float modifier into a vector modifier that the renderer expects.
        float[][] initalRange = modifier.getInitialRange();
        Factor factor = modifier.getFactor();

        Vector min = new Vector(0f, 0f, initalRange[0][0]);
        Vector max = new Vector(0f, 0f, initalRange[1][0]);
        ParticleModifierVector vecModifier = new ParticleModifierVector(min, max);

        float[][] intervals = modifier.getInterpolatedIntervals();
        float[][] points = modifier.getInterpolatedPoints();
        for (int i = 0; i < intervals.length; i ++){
            float intervalLength = intervals[i][1] - intervals[i][0];
            Vector point = new Vector(0,0, points[i][0]);
            vecModifier.addInterval(intervalLength, point);
        }

        vecModifier.setFactor(factor);
        setModifier("rotation", vecModifier);
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void setRotationModifierLegacy(ParticleModifier modifier) {
        setModifier("rotation", modifier);
    }
    //#ENDIF
    /**
     * Set the color of the particles generated by this ParticleEmitter. The {@link
     * ParticleModifierColor} provided will define the initial color of the particles, and may
     * also indicate how color will change over time or over distance.
     *
     * @param modifier The {@link ParticleModifierColor} describing color behavior for the
     *                 particles.
     */
    public void setColorModifier(ParticleModifierColor modifier) {
        setModifier("color", modifier);
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void setColorModifierLegacy(ParticleModifier modifier) {
        setModifier("color", modifier);
    }
    //#ENDIF

    /**
     * Set the velocity of the particles generated by this ParticleEmitter. The {@link
     * ParticleModifierVector} provided will define the initial velocity of the particles, and may
     * also indicate how velocity will change over time or over distance. Each {@link Vector} in the
     * modifier defines the X, Y, and Z velocity.
     * <p>
     * By default, ParticleEmitter radiates particles in a fountain-like fashion. By using {@link
     * #setVelocityModifier(ParticleModifierVector)} and {@link #setAccelerationModifier(ParticleModifierVector)}
     * a variety of more complex animations can be achieved. For example, falling, swaying snow can
     * be achieved with a fixed acceleration of -9.81 and a randomized initial horizontal velocity.
     * A similar configuration can be used to make steam particles emanate from a kettle. <p>>
     *
     * @param modifier The {@link ParticleModifierVector} describing velocity behavior for the
     *                 particles.
     */
    public void setVelocityModifier(ParticleModifierVector modifier) {
        setModifier("velocity", modifier);
    }
    //#ENDIF

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void setVelocityModifierLegacy(ParticleModifier modifier) {
        setModifier("velocity", modifier);
    }
    //#ENDIF

    /**
     * Set the acceleration of the particles generated by this ParticleEmitter. The {@link
     * ParticleModifierVector} provided will define the initial acceleration of the particles, and
     * may also indicate how acceleration will change over time or over distance. Each {@link
     * Vector} in the modifier defines the X, Y, and Z accelearation.
     * <p>
     * By default, ParticleEmitter radiates particles in a fountain-like fashion. By using {@link
     * #setVelocityModifier(ParticleModifierVector)} and {@link #setAccelerationModifier(ParticleModifierVector)}
     * a variety of more complex animations can be achieved. For example, falling, swaying snow can
     * be achieved with a fixed acceleration of -9.81 and a randomized initial horizontal velocity.
     * A similar configuration can be used to make steam particles emanate from a kettle. <p>>
     *
     * @param modifier The {@link ParticleModifierVector} describing acceleration behavior for the
     *                 particles.
     */
    public void setAccelerationModifier(ParticleModifierVector modifier) {
        setModifier("acceleration", modifier);
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void setAccelerationModifierLegacy(ParticleModifier modifier) {
        setModifier("acceleration", modifier);
    }
    //#ENDIF

    /**
     * Set an explosive impulse, which simulates the effect of a detonation or other impulsive
     * force.
     * <p>
     * Impulsive forces are defined by a detonation position and an impulse. The impulse is
     * specified in newton seconds and determines the <i>strength</i> of the force. The detonation
     * point is set in the coordinate system of the ParticleEmitter's parent {@link Node}. The
     * closer particles are to the detonation point, the larger their explosive force.
     * <p>
     * The behavior of an explosion may also be tuned with the <tt>decelerationPeriod</tt>
     * parameter. This enables a "dampening deceleration effect" against the explosive impulse
     * force, in order to slow down the explosion. The <tt>decelerationPeriod</tt> defines the
     * timeframe in milliseconds within which the particles will decelerate from their initial
     * velocity to 0.0 m/s. This is particularly useful for fireworks, which explode outward then
     * slow down / are dampened after a specific length of time.
     * <p>
     * Note that setting an explosive impulse automatically invalidates any velocity set via {@link
     * #setVelocityModifier(ParticleModifierVector)}, as an explosion directly manipulates an
     * object's velocity. Likewise, if a <tt>>decelerationPeriod</tt> is provided, it will
     * invalidate the acceleration set via {@link #setAccelerationModifier(ParticleModifierVector)}.
     *
     * @param impulse            The impulsive force, in newton seconds.
     * @param position           The detonation position.
     * @param decelerationPeriod The deceleration period in milliseconds. Set to less than zero to
     *                           not use a deceleration period.
     */
    public void setExplosiveImpulse(float impulse, Vector position, float decelerationPeriod) {
        nativeSetExplosiveImpulse(mNativeRef, impulse, position.toArray(), decelerationPeriod);
    }

    /**
     * Set the {@link Material.BlendMode} to use for particles from this ParticleEmitter. BlendMode
     * determines how a pixel's color, as it is being rendered, interacts with the color of the
     * pixel <i>already</i> in the framebuffer.
     * <p>
     * Default is {@link Material.BlendMode#ALPHA}.
     *
     * @param blendMode The BlendMode to use for this Material.
     */
    public void setBlendMode(Material.BlendMode blendMode) {
        mBlendMode = blendMode;
        nativeSetParticleBlendMode(mNativeRef, blendMode.getStringValue());
    }

    /**
     * Return the {@link Material.BlendMode} used by particles from this
     * ParticleEmitter.
     *
     * @return The {@link Material.BlendMode} used by this ParticleEmitter.
     */
    public Material.BlendMode getBlendMode() {
        return mBlendMode;
    }

    /**
     * Set the brightness value at which particles from this ParticleEmitter will begin to bloom.
     * <p>
     * Bloom is an effect that makes surfaces appear to glow by applying a Gaussian blur and
     * additive blend.
     * <p>
     * This value specifies at what 'brightness' the particles should start to bloom. Brightness is
     * effectively the magnitude of the final color of a particle (modified for the human eye:
     * specifically it is the dot product of the final color with (0.2126, 0.7152, 0.0722)).
     * <p>
     * For example, if this property is set to 0.0, then all particles will bloom. If this property
     * is set to 1.0, then only those pixels of the particle whose brightness exceeds 1.0 (after
     * lights are applied) will bloom.
     * <p>
     * Default is -1.0, which disables bloom.
     *
     * @param threshold The bloom threshold value to set. Set to less than zero to disable bloom
     *                  entirely.
     */
    public void setBloomThreshold(float threshold) {
        mBloomThreshold = threshold;
        nativeSetBloomThreshold(mNativeRef, threshold);
    }

    /**
     * Get the bloom threshold, the brightness at which particles from this ParticleEmitter will
     * begin to bloom. See {@link #setBloomThreshold(float)} for discussion.
     *
     * @return The bloom threshold.
     */
    public float getBloomThreshold() {
        return mBloomThreshold;
    }

    private void setModifier(String targetedModifier, ParticleModifier modifier){
        nativeSetParticleModifier(
                mNativeRef,
                targetedModifier,
                modifier.getFactor().getStringValue(),
                modifier.getInitialRange(),
                modifier.getInterpolatedIntervals(),
                modifier.getInterpolatedPoints());
    }

    private native long nativeCreateEmitter(long contextRef, long surfaceRef);
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

    /**
     * Builder for {@link ParticleEmitter} objects.
     * @deprecated use {@link ParticleEmitterBuilder#ParticleEmitterBuilder(ViroContext, Quad)} instead
     */
    public static ParticleEmitterBuilder builder(ViroContext viroContext,
                                                 Surface surface) {
        return new ParticleEmitterBuilder(viroContext, surface);
    }

    /**
     * Builder for {@link ParticleEmitter} objects.
     */
    public static ParticleEmitterBuilder builder(ViroContext viroContext,
                                                 Quad quad) {
        return new ParticleEmitterBuilder(viroContext, quad);
    }

    /**
     * Builder for creating {@link ParticleEmitter} objects.
     */
    public static class ParticleEmitterBuilder {
        private  ParticleEmitter particleEmitter;

        /**
         * Constructor for ParticleEmitterBuilder.
         * @deprecated use {@link ParticleEmitterBuilder#ParticleEmitter(ViroContext, Quad)} instead
         */
        public ParticleEmitterBuilder(ViroContext viroContext,Surface surface) {
            particleEmitter = new ParticleEmitter(viroContext, surface);
        }

        /**
         * Constructor for ParticleEmitterBuilder.
         */
        public ParticleEmitterBuilder(ViroContext viroContext,Quad quad) {
            particleEmitter = new ParticleEmitter(viroContext, quad);
        }

        /**
         * Refer to {@link ParticleEmitter#setDelay(int)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder delay(int delay) {
            particleEmitter.setDelay(delay);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setDuration(int)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder duration(int duration) {
            particleEmitter.setDuration(duration);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setLoop(boolean)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder loop(boolean loop) {
            particleEmitter.setLoop(loop);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setFixedToEmitter(boolean)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder fixedToEmitter(boolean fixedToEmitter) {
            particleEmitter.setFixedToEmitter(fixedToEmitter);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setEmissionRatePerSecond(int, int)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder emissionRatePerSecond(int emissionRatePerSecondMin, int emissionRatePerSecondMax) {
            particleEmitter.setEmissionRatePerSecond(emissionRatePerSecondMin, emissionRatePerSecondMax);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setEmissionRatePerMeter(int, int)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder emissionRatePerMeter(int emissionRatePerMeterMin, int emissionRatePerMeterMax) {
            particleEmitter.setEmissionRatePerMeter(emissionRatePerMeterMin, emissionRatePerMeterMax);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setParticleLifetime(int, int)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder particleLifetimeMin(int particleLifetimeMin, int particleLifetimeMax) {
            particleEmitter.setParticleLifetime(particleLifetimeMin, particleLifetimeMax);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setMaxParticles(int)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder maxParticles(int maxParticles) {
            particleEmitter.setMaxParticles(maxParticles);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setEmissionBursts(List)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder emissionBursts(List<EmissionBurst> emissionBursts) {
            particleEmitter.setEmissionBursts(emissionBursts);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setSpawnVolume(SpawnVolume, boolean)}.
         */
        public ParticleEmitterBuilder spawnVolume(SpawnVolume spawnVolume, boolean spawnOnSurface) {
            particleEmitter.setSpawnVolume(spawnVolume, spawnOnSurface);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setBlendMode(Material.BlendMode)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder blendMode(Material.BlendMode blendMode) {
            particleEmitter.setBlendMode(blendMode);
            return this;
        }

        /**
         * Refer to {@link ParticleEmitter#setBloomThreshold(float)}.
         *
         * @return This builder.
         */
        public ParticleEmitterBuilder bloomThreshold(float bloomThreshold) {
            particleEmitter.setBloomThreshold(bloomThreshold);
            return this;
        }

        /**
         * Return the built {@link ParticleEmitter} object.
         *
         * @return The built ParticleEmitter.
         */
        public ParticleEmitter build() {
            return particleEmitter;

        }
    }
}
