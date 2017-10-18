/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;

/**
 * The Scene object sits at the top of the scene graph, a hierarchy of {@link Node} objects
 * representing the 3D world. Scenes are the 3D equivalent of the Views found in most
 * 2D application frameworks. They contain all the content that Viro renders in AR/VR: UI controls,
 * 3D objects, lights, and more.
 */
public class SceneController {

    /*
     * Java JNI wrapper for linking the following classes across the bridge:
     *
     * Android Java Object  : com.viromedia.bridge.viewgroups.Scene.java
     * Java JNI Wrapper     : com.viro.renderer.jni.SceneControllerJni.java
     * Cpp JNI wrapper      : Scene_JNI.cpp
     * Cpp Object           : VROScene.cpp
     */
    protected long mNativeRef;
    protected long mNativeDelegateRef;
    private Node mRootNode;
    private PhysicsWorld mPhysicsWorld;

    /**
     * Construct a new Scene.
     */
    public SceneController() {
        setSceneRef(createNativeScene());
        mNativeDelegateRef = nativeCreateSceneControllerDelegate(mNativeRef);

        mRootNode = new Node(false);
        mRootNode.setNativeRef(nativeGetSceneNodeRef(mNativeRef));
        mPhysicsWorld = new PhysicsWorld(this);
    }

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

    /**
     * Retrieve the root {@link Node} of the scene graph.
     *
     * @return The root node.
     */
    public Node getRootNode() {
        return mRootNode;
    }

    /**
     * Get the physics simulation for the scene.
     *
     * @return The {@link PhysicsWorld} for the scene.
     */
    public PhysicsWorld getPhysicsWorld() {
        return mPhysicsWorld;
    }

    public void setBackgroundVideoTexture(VideoTexture videoTexture) {
        nativeSetBackgroundVideoTexture(mNativeRef, videoTexture.mNativeRef);
    }

    public void setBackgroundImageTexture(Texture imageTexture) {
        nativeSetBackgroundImageTexture(mNativeRef, imageTexture.mNativeRef);
    }

    public void setBackgroundRotation(float[] rotation) {
        nativeSetBackgroundRotation(mNativeRef, rotation[0], rotation[1], rotation[2]);
    }

    public void setBackgroundCubeImageTexture(Texture cubeTexture) {
        nativeSetBackgroundCubeImageTexture(mNativeRef, cubeTexture.mNativeRef);
    }

    public void setBackgroundCubeWithColor(long color) {
        nativeSetBackgroundCubeWithColor(mNativeRef, color);
    }

    public void setSoundRoom(ViroContext viroContext, float[] size, String wallMaterial,
                             String ceilingMaterial, String floorMaterial) {
        nativeSetSoundRoom(mNativeRef, viroContext.mNativeRef, size[0], size[1], size[2],
                wallMaterial, ceilingMaterial, floorMaterial);
    }

    void setPhysicsWorldGravity(float gravity[]){
        nativeSetPhysicsWorldGravity(mNativeRef, gravity);
    }

    void setPhysicsDebugDraw(boolean debugDraw){
        nativeSetPhysicsWorldDebugDraw(mNativeRef, debugDraw);
    }

    void findCollisionsWithRayAsync(float[] fromPos, float toPos[], boolean closest,
                                           String tag, PhysicsWorld.HitTestCallback callback){
        findCollisionsWithRayAsync(mNativeRef, fromPos, toPos, closest, tag, callback);
    }

    void findCollisionsWithShapeAsync(float[] from, float[] to, String shapeType, float[] params,
                                             String tag, PhysicsWorld.HitTestCallback callback){
        findCollisionsWithShapeAsync(mNativeRef, from,to, shapeType, params, tag, callback);
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

    public void dispose() {
        nativeDestroySceneControllerDelegate(mNativeDelegateRef);
        nativeDestroySceneController(mNativeRef);
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateSceneController();
    private native long nativeCreateSceneControllerDelegate(long sceneRef);
    private native void nativeDestroySceneController(long sceneReference);
    private native long nativeGetSceneNodeRef(long sceneRef);
    protected native void nativeDestroySceneControllerDelegate(long sceneDelegateRef);
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
    private WeakReference<SceneDelegate> mDelegate = null;

    public interface SceneDelegate {
        void onSceneWillAppear();
        void onSceneDidAppear();
        void onSceneWillDisappear();
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

    /* Called by Native */
    public void onSceneWillAppear() {
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneWillAppear();
        }
    }

    /* Called by Native */
    public void onSceneDidAppear() {
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneDidAppear();
        }
    }

    /* Called by Native */
    public void onSceneWillDisappear(){
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneWillDisappear();
        }
    }

    /* Called by Native */
    public void onSceneDidDisappear() {
        SceneDelegate delegate;
        if (mDelegate != null && (delegate = mDelegate.get()) != null) {
            delegate.onSceneDidDisappear();
        }
    }

    /**
     * Native Viro Phsyics JNI Functions
     */
    private native void nativeSetPhysicsWorldGravity(long sceneRef, float gravity[]);
    private native void findCollisionsWithRayAsync(long sceneRef, float[] from, float[] to,
                                           boolean closest, String tag,
                                           PhysicsWorld.HitTestCallback callback);
    private native void findCollisionsWithShapeAsync(long sceneRef, float[] from, float[] to,
                                           String shapeType, float[] params, String tag,
                                           PhysicsWorld.HitTestCallback callback);
    private native void nativeSetPhysicsWorldDebugDraw(long sceneRef, boolean debugDraw);

}