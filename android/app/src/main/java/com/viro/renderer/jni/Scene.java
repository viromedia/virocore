/*
 * Copyright © 2016 Viro Media. All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Android Java Object  : com.viromedia.bridge.viewgroups.Scene.java
 * Java JNI Wrapper     : com.viro.renderer.jni.Scene.java
 * Cpp JNI wrapper      : Scene_JNI.cpp
 * Cpp Object           : VROScene.cpp
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;

/**
 * The Scene object sits at the top of the scene graph, a hierarchy of {@link Node} objects
 * representing the 3D world. Scenes are the 3D equivalent of the Views found in most
 * 2D application frameworks. They contain all the content that Viro renders in AR/VR: UI controls,
 * 3D objects, lights, and more.
 */
public class Scene {

    protected long mNativeRef;
    protected long mNativeDelegateRef;
    private Node mRootNode;
    private PhysicsWorld mPhysicsWorld;
    private WeakReference<SceneDelegate> mDelegate = null;

    /**
     * Construct a new Scene.
     */
    public Scene() {
        setSceneRef(createNativeScene());
        mNativeDelegateRef = nativeCreateSceneControllerDelegate(mNativeRef);

        mRootNode = new Node(false);
        mRootNode.setNativeRef(nativeGetSceneNodeRef(mNativeRef));
        mPhysicsWorld = new PhysicsWorld(this);
    }

    /**
     * @hide
     * @return
     */
    protected long createNativeScene() {
        return nativeCreateSceneController();
    }

    /**
     * @hide
     *
     * This method is used by child classes to replace the mNativeRef with
     * a child's nativeRef.
     */
    protected void setSceneRef(long sceneRef) {
        mNativeRef = sceneRef;
        mNativeDelegateRef = nativeCreateSceneControllerDelegate(mNativeRef);
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
     * Set the background of this Scene to display the given video. The provided {@link
     * VideoTexture} should be a spherical video to display properly. The video will be rendered
     * behind all other content.
     *
     * @param videoTexture The {@link VideoTexture} containing the video.
     */
    public void setBackgroundVideoTexture(VideoTexture videoTexture) {
        nativeSetBackgroundVideoTexture(mNativeRef, videoTexture.mNativeRef);
    }

    /**
     * Set the background of this Scene to display an image. The provided {@link Texture} should
     * contain a spherical image to display properly. The image will be rendered behind all other
     * content.
     *
     * @param imageTexture The {@link Texture} containing the image.
     */
    public void setBackgroundImageTexture(Texture imageTexture) {
        nativeSetBackgroundImageTexture(mNativeRef, imageTexture.mNativeRef);
    }

    /**
     * Rotate the background image or video by the given degrees about the X, Y, and Z axes.
     *
     * @param rotation {@link Vector} containing rotation about the X, Y, and Z axes.
     */
    public void setBackgroundRotation(Vector rotation) {
        nativeSetBackgroundRotation(mNativeRef, rotation.x, rotation.y, rotation.z);
    }

    /**
     * Set the background of this Scene to display a cube-map. The provided {@link Texture} should
     * wrap a cube-map image, constructed via {@link Texture#Texture(Image, Image, Image, Image,
     * Image, Image, TextureFormat)}. The cube-map will be rendered behind all other content.
     *
     * @param cubeTexture The {@link Texture} containing the cube-map.
     */
    public void setBackgroundCubeImageTexture(Texture cubeTexture) {
        nativeSetBackgroundCubeImageTexture(mNativeRef, cubeTexture.mNativeRef);
    }

    /**
     * Set teh background of this Scene to display the given color. The color will be rendered
     * behind all other content.
     *
     * @param color The {@link android.graphics.Color}'s int value.
     */
    public void setBackgroundCubeWithColor(long color) {
        nativeSetBackgroundCubeWithColor(mNativeRef, color);
    }

    /**
     * TODO VIRO-1981
     *
     * @param viroContext
     * @param size
     * @param wallMaterial
     * @param ceilingMaterial
     * @param floorMaterial
     */
    public void setSoundRoom(ViroContext viroContext, float[] size, String wallMaterial,
                             String ceilingMaterial, String floorMaterial) {
        nativeSetSoundRoom(mNativeRef, viroContext.mNativeRef, size[0], size[1], size[2],
                wallMaterial, ceilingMaterial, floorMaterial);
    }

    /**
     * Adds a {@link ParticleEmitter} to the scene.
     *
     * @param emitter The emitter to add to the scene.
     */
    public void addParticleEmitter(ParticleEmitter emitter) {
        nativeAddParticleEmitter(mNativeRef, emitter.mNativeRef);
    }

    /**
     * Removes a {@link ParticleEmitter} from the scene.
     *
     * @param emitter The emitter to remove from the scene.
     */
    public void removeParticleEmitter(ParticleEmitter emitter) {
        nativeRemoveParticleEmitter(mNativeRef, emitter.mNativeRef);
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
    private native void nativeSetBackgroundVideoTexture(long sceneRef, long videoRef);
    private native void nativeSetBackgroundImageTexture(long sceneRef, long imageRef);
    private native void nativeSetBackgroundCubeImageTexture(long sceneRef, long textureRef);
    private native void nativeSetBackgroundCubeWithColor(long sceneRef, long color);
    private native void nativeSetBackgroundRotation(long sceneRef, float degreeX, float degreeY,
                                                    float degreeZ);
    private native void nativeSetSoundRoom(long sceneRef, long renderContextRef, float sizeX,
                                           float sizeY, float sizeZ, String wallMaterial,
                                           String ceilingMaterial, String floorMaterial);
    private native void nativeAddParticleEmitter(long sceneRef, long particleRef);
    private native void nativeRemoveParticleEmitter(long sceneRef, long particleRef);
    private native boolean nativeSetEffects(long sceneRef, String[] effects);

    /**
     * @hide
     * @param sceneDelegateRef
     */
    protected native void nativeDestroySceneControllerDelegate(long sceneDelegateRef);

    /**
     * The SceneDelegate receives callbacks in response to a {@link Scene} appearing and disappearing.
     */
    public interface SceneDelegate {

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
     * Registers the given delegate for callbacks. Registering the same
     * delegate twice will still only result in that delegate being
     * called once.
     */
    public void registerDelegate(SceneDelegate delegate) {
        mDelegate = new WeakReference<SceneDelegate>(delegate);
    }

    /*
     Called by Native functions.
     */

    /**
     * @hide
     */
    public void onSceneWillAppear() {
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneWillAppear();
        }
    }
    /**
     * @hide
     */
    public void onSceneDidAppear() {
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneDidAppear();
        }
    }

    /**
     * @hide
     */
    public void onSceneWillDisappear(){
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneWillDisappear();
        }
    }

    /**
     * @hide
     */
    public void onSceneDidDisappear() {
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneDidDisappear();
        }
    }

    /*
     Native Viro Physics JNI Functions
     */
    private native void nativeSetPhysicsWorldGravity(long sceneRef, float gravity[]);
    private native void findCollisionsWithRayAsync(long sceneRef, float[] from, float[] to,
                                           boolean closest, String tag,
                                           PhysicsWorld.HitTestCallback callback);
    private native void findCollisionsWithShapeAsync(long sceneRef, float[] from, float[] to,
                                           String shapeType, float[] params, String tag,
                                           PhysicsWorld.HitTestCallback callback);
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
                                    String tag, PhysicsWorld.HitTestCallback callback){
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
                                      String tag, PhysicsWorld.HitTestCallback callback){
        findCollisionsWithShapeAsync(mNativeRef, from,to, shapeType, params, tag, callback);
    }

}