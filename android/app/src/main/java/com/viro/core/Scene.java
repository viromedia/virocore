/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Android Java Object  : com.viromedia.bridge.viewgroups.Scene.java
 * Java JNI Wrapper     : com.viro.core.Scene.java
 * Cpp JNI wrapper      : Scene_JNI.cpp
 * Cpp Object           : VROScene.cpp
 */
package com.viro.core;

import android.graphics.Bitmap;

import java.util.HashMap;
import java.util.Map;

/**
 * Scene represents a scene graph - a hierarchy of {@link Node}s that together represent the 3D
 * world. UI controls, 3D objects, lights, camera, and more are attached to the Nodes to build the
 * displayable 3D scene. To display a Scene in an Android View, use {@link ViroView}.
 * <p>
 * For an extended discussion of Viro's scene graph, refer to the <a
 * href="https://virocore.viromedia.com/docs/scenes">Scene Guide</a>.
 */
public class Scene {

    /**
     * AudioMaterial is used to define the Scene's reverb effects. These are used in conjunction
     * with {@link #setSoundRoom(ViroContext, Vector, AudioMaterial, AudioMaterial, AudioMaterial)}.
     * In general, harder surface materials are more reverberant than rooms made of soft, absorbent
     * materials.
     */
    public enum AudioMaterial {
        /**
         * Transparent, no reverb.
         */
        TRANSPARENT("transparent"),
        /**
         * Acoustic ceiling tiles.
         */
        ACOUSTIC_CEILING_TILES("acoustic_ceiling_tiles"),
        /**
         * Brick, bare.
         */
        BRICK_BARE("brick_bare"),
        /**
         * Painted brick.
         */
        BRICK_PAINTED("brick_painted"),
        /**
         * Coarse concrete.
         */
        CONCRETE_BLOCK_COARSE("concrete_block_coarse"),
        /**
         * Painted concrete.
         */
        CONCRETE_BLOCK_PAINTED("concrete_block_painted"),
        /**
         * Heavy curtain.
         */
        CURTAIN_HEAVY("curtain_heavy"),
        /**
         * Fiber glass.
         */
        FIBER_GLASS_INSULATION("fiber_glass_insulation"),
        /**
         * Thin glass.
         */
        GLASS_THIN("glass_thin"),
        /**
         * Thick glass.
         */
        GLASS_THICK("glass_thick"),
        /**
         * Grass.
         */
        GRASS("grass"),
        /**
         * Linoleum on concrete.
         */
        LINOLEUM_ON_CONCRETE("linoleum_on_concrete"),
        /**
         * Marble.
         */
        MARBLE("marble"),
        /**
         * Metal.
         */
        METAL("metal"),
        /**
         * Parquet on concrete.
         */
        PARQUET_ON_CONRETE("parquet_on_concrete"),
        /**
         * Rough plaster.
         */
        PLASTER_ROUGH("plaster_rough"),
        /**
         * Smooth plaster.
         */
        PLASTER_SMOOTH("plaster_smooth"),
        /**
         * Plywood panel.
         */
        PLYWOOD_PANEL("plywood_panel"),
        /**
         * Polished concrete or tile.
         */
        POLISHED_CONCRETE_OR_TILE("polished_concrete_or_tile"),
        /**
         * Sheet rock.
         */
        SHEET_ROCK("sheet_rock"),
        /**
         * Water or ice.
         */
        WATER_OR_ICE_SURFACE("water_or_ice_surface"),
        /**
         * Wood ceiling.
         */
        WOOD_CEILING("wood_ceiling"),
        /**
         * Wood panel.
         */
        WOOD_PANEL("wood_panel");

        private String mStringValue;
        private AudioMaterial(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, AudioMaterial> map = new HashMap<String, AudioMaterial>();
        static {
            for (AudioMaterial value : AudioMaterial.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static AudioMaterial valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    protected long mNativeRef;
    protected long mNativeDelegateRef;
    private Node mRootNode;
    private PhysicsWorld mPhysicsWorld;
    private VisibilityListener mListener = null;

    /**
     * Construct a new Scene.
     */
    public Scene() {
        setSceneRef(createNativeScene());
    }

    /**
     * Subclass constructor, does not set the scene-ref.
     *
     * @hide
     */
    Scene(boolean dummy) {

    }

    /**
     * @hide
     * @return
     */
    protected long createNativeScene() {
        return nativeCreateSceneController();
    }

    /**
     * After-construction method when the sceneRef is known. Initializes
     * the rest of the Scene.
     *
     * @hide
     */
    protected void setSceneRef(long sceneRef) {
        mNativeRef = sceneRef;
        mNativeDelegateRef = nativeCreateSceneControllerDelegate(mNativeRef);

        PortalScene root = new PortalScene(false);
        root.initWithNativeRef(nativeGetSceneNodeRef(mNativeRef));
        root.attachDelegate();
        mRootNode = root;
        mPhysicsWorld = new PhysicsWorld(this);
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
     * Release native resources associated with this Scene.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroySceneController(mNativeRef);
            mNativeRef = 0;
        }
        if (mNativeDelegateRef != 0) {
            nativeDestroySceneControllerDelegate(mNativeDelegateRef);
            mNativeDelegateRef = 0;
        }
    }

    /**
     * Retrieve the root {@link Node} of the scene graph.
     *
     * @return The root node.
     */
    public Node getRootNode() {
        return mRootNode;
    }

    /**
     * Get the physics simulation for the Scene.
     *
     * @return The {@link PhysicsWorld} for the Scene.
     */
    public PhysicsWorld getPhysicsWorld() {
        return mPhysicsWorld;
    }

    /**
     * Set the background of this Scene to display the texture. The provided {@link Texture} should
     * contain a spherical image or spherical video. The image or video will be rendered behind all other
     * content.
     *
     * @param texture The {@link Texture} containing the image, or {@link VideoTexture} containing
     *                     the video.
     */
    public void setBackgroundTexture(Texture texture) {
        nativeSetBackgroundTexture(mNativeRef, texture.mNativeRef);
    }

    /**
     * Rotate the background image or video by the given radians about the X, Y, and Z axes.
     *
     * @param rotation {@link Vector} containing rotation about the X, Y, and Z axes.
     */
    public void setBackgroundRotation(Vector rotation) {
        nativeSetBackgroundRotation(mNativeRef, rotation.x, rotation.y, rotation.z);
    }

    /**
     * Set the background of this Scene to display a cube-map. The provided {@link Texture} should
     * wrap a cube-map image. The cube-map will be rendered behind all other content.
     *
     * @param cubeTexture The {@link Texture} containing the cube-map.
     */
    public void setBackgroundCubeTexture(Texture cubeTexture) {
        nativeSetBackgroundCubeImageTexture(mNativeRef, cubeTexture.mNativeRef);
    }

    /**
     * Set the background of this Scene to display the given color. The color will be rendered
     * behind all other content.
     *
     * @param color The {@link android.graphics.Color}'s int value.
     */
    public void setBackgroundCubeWithColor(long color) {
        nativeSetBackgroundCubeWithColor(mNativeRef, color);
    }

    /**
     * Set the sound room, which enables reverb effects for all {@link SpatialSound} in the Scene.
     * The sound room is defined by three {@link AudioMaterial}s, each of which has unique
     * absorption properties which differ with frequency. The room is also give a size in meters,
     * which is centered around the user. Larger rooms are more reverberant than smaller rooms, and
     * harder surface materials are more reverberant than rooms made of soft, absorbent materials.
     *
     * @param viroContext {@link ViroContext} is required to set the sound room.
     * @param size        The size of the room's X, Y, and Z dimensions in meters, as a {@link
     *                    Vector}.
     * @param wall        The wall {@link AudioMaterial}.
     * @param ceiling     The ceiling {@link AudioMaterial}.
     * @param floor       The floor {@link AudioMaterial}.
     */
    public void setSoundRoom(ViroContext viroContext, Vector size, AudioMaterial wall,
                             AudioMaterial ceiling, AudioMaterial floor) {
        nativeSetSoundRoom(mNativeRef, viroContext.mNativeRef, size.x, size.y, size.z,
                wall.getStringValue(), ceiling.getStringValue(), floor.getStringValue());
    }

    /**
     * @hide
     * @param effects
     * @return
     */
    public boolean setEffects(String[] effects){
        return nativeSetEffects(mNativeRef, effects);
    }

    /*
     * Native Functions called into JNI
     */
    private native long nativeCreateSceneController();
    private native long nativeCreateSceneControllerDelegate(long sceneRef);
    private native void nativeDestroySceneController(long sceneReference);
    private native long nativeGetSceneNodeRef(long sceneRef);
    private native void nativeSetBackgroundTexture(long sceneRef, long imageRef);
    private native void nativeSetBackgroundCubeImageTexture(long sceneRef, long textureRef);
    private native void nativeSetBackgroundCubeWithColor(long sceneRef, long color);
    private native void nativeSetBackgroundRotation(long sceneRef, float radiansX, float radiansY,
                                                    float radiansZ);
    private native void nativeSetSoundRoom(long sceneRef, long renderContextRef, float sizeX,
                                           float sizeY, float sizeZ, String wallMaterial,
                                           String ceilingMaterial, String floorMaterial);
    private native boolean nativeSetEffects(long sceneRef, String[] effects);

    /**
     * @hide
     * @param sceneDelegateRef
     */
    protected native void nativeDestroySceneControllerDelegate(long sceneDelegateRef);

    /**
     * Receives callbacks in response to a {@link Scene} appearing and disappearing.
     */
    public interface VisibilityListener {

        /**
         * Callback invoked when a Scene is about to appear.
         */
        void onSceneWillAppear();

        /**
         * Callback invoked immediately after a Scene has appeared (after the transition).
         */
        void onSceneDidAppear();

        /**
         * Callback invoked when a Scene is about to disappear.
         */
        void onSceneWillDisappear();

        /**
         * Callback invoked immediately after a Scene has disappeared (after the transition).
         */
        void onSceneDidDisappear();
    }

    /**
     * Set the given {@link VisibilityListener} to receive callbacks when the Scene's visibility
     * status changes.
     */
    public void setVisibilityListener(VisibilityListener listener) {
        mListener = listener;
    }

    /*
     Called by Native functions.
     */

    /**
     * @hide
     */
    public void onSceneWillAppear() {
        if (mListener != null) {
            mListener.onSceneWillAppear();
        }
    }
    /**
     * @hide
     */
    public void onSceneDidAppear() {
        if (mListener != null) {
            mListener.onSceneDidAppear();
        }
    }

    /**
     * @hide
     */
    public void onSceneWillDisappear(){
        if (mListener != null) {
            mListener.onSceneWillDisappear();
        }
    }

    /**
     * @hide
     */
    public void onSceneDidDisappear() {
        if (mListener != null) {
            mListener.onSceneDidDisappear();
        }
    }

    /*
     Native Viro Physics JNI Functions
     */
    private native void nativeSetPhysicsWorldGravity(long sceneRef, float gravity[]);
    private native void findCollisionsWithRayAsync(long sceneRef, float[] from, float[] to,
                                           boolean closest, String tag,
                                           PhysicsWorld.HitTestListener callback);
    private native void findCollisionsWithShapeAsync(long sceneRef, float[] from, float[] to,
                                           String shapeType, float[] params, String tag,
                                           PhysicsWorld.HitTestListener callback);
    private native void nativeSetPhysicsWorldDebugDraw(long sceneRef, boolean debugDraw);

    /*
     Invoked by PhysicsWorld
     */
    /**
     * @hide
     * @param gravity
     */
    void setPhysicsWorldGravity(float gravity[]){
        nativeSetPhysicsWorldGravity(mNativeRef, gravity);
    }

    /**
     * @hide
     * @param debugDraw
     */
    void setPhysicsDebugDraw(boolean debugDraw){
        nativeSetPhysicsWorldDebugDraw(mNativeRef, debugDraw);
    }

    /**
     * @hide
     * @param fromPos
     * @param toPos
     * @param closest
     * @param tag
     * @param callback
     */
    void findCollisionsWithRayAsync(float[] fromPos, float toPos[], boolean closest,
                                    String tag, PhysicsWorld.HitTestListener callback){
        findCollisionsWithRayAsync(mNativeRef, fromPos, toPos, closest, tag, callback);
    }

    /**
     * @hide
     * @param from
     * @param to
     * @param shapeType
     * @param params
     * @param tag
     * @param callback
     */
    void findCollisionsWithShapeAsync(float[] from, float[] to, String shapeType, float[] params,
                                      String tag, PhysicsWorld.HitTestListener callback){
        findCollisionsWithShapeAsync(mNativeRef, from,to, shapeType, params, tag, callback);
    }

    /**
     * Creates a builder for building complex {@link Scene} objects
     * @return {@link SceneBuilder} object
     */
    public static SceneBuilder<? extends Scene, ? extends SceneBuilder> builder() {
        return new SceneBuilder<>();
    }

// +---------------------------------------------------------------------------+
// | SceneBuilder class for Scene
// +---------------------------------------------------------------------------+

    /**
     * SceneBuilder class for {@link Scene}
     */
    public static class SceneBuilder<R extends Scene, B extends SceneBuilder<R, B>> {
        private R scene;

        /**
         * Constructor for SceneBuilder class
         */
        public SceneBuilder() {
            this.scene = (R) new Scene();
        }

        /**
         * Refer to {@link Scene#setVisibilityListener(VisibilityListener)}
         */
        public SceneBuilder listener(VisibilityListener listener) {
            this.scene.setVisibilityListener(listener);
            return (B) this;
        }

        /**
         * Refer to {@link Scene#setBackgroundTexture(Texture)}
         */
        public SceneBuilder backgroundTexture(Texture backgroundTexture) {
            this.scene.setBackgroundTexture(backgroundTexture);
            return (B) this;
        }

        /**
         * Refer to {@link Scene#setBackgroundRotation(Vector)}
         */
        public SceneBuilder backgroundRotation(Vector backgroundRotation) {
            this.scene.setBackgroundRotation(backgroundRotation);
            return (B) this;
        }

        /**
         * Refer to {@link Scene#setBackgroundCubeWithColor(long)}}
         */
        public SceneBuilder backgroundCubeWithColor(long backgroundCubeWithColor) {
            this.scene.setBackgroundCubeWithColor(backgroundCubeWithColor);
            return (B) this;
        }

        /**
         * Refer to {@link Scene#setSoundRoom(ViroContext, Vector, AudioMaterial, AudioMaterial, AudioMaterial)}
         */
        public SceneBuilder soundRoom(ViroContext viroContext, Vector size, AudioMaterial wall,
                                      AudioMaterial ceiling, AudioMaterial floor) {
            this.scene.setSoundRoom(viroContext,size, wall, ceiling, floor);
            return (B) this;
        }

        /**
         * Returns the built {@link Scene} object
         */
        public R build() {
            return scene;
        }

    }
}