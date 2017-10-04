/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;

/**
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Android Java Object  : com.viromedia.bridge.viewgroups.Scene.java
 * Java JNI Wrapper     : com.viro.renderer.jni.SceneControllerJni.java
 * Cpp JNI wrapper      : Scene_JNI.cpp
 * Cpp Object           : VROScene.cpp
 */
public class SceneControllerJni {
    public long mNativeRef;
    public long mNativeDelegateRef;

    public SceneControllerJni() {
        setSceneRef(nativeCreateSceneController());
        mNativeDelegateRef = nativeCreateSceneControllerDelegate(mNativeRef);
    }

    /*
     This method is used by child classes to replace the mNativeRef with
     a child's nativeRef.
     */
    protected void setSceneRef(long sceneRef) {
        mNativeRef = sceneRef;
        mNativeDelegateRef = nativeCreateSceneControllerDelegate(mNativeRef);
    }

    // Creates and returns a NodeJNI representing this Scene controller.
    // NOTE: The caller is responsible for releasing this NodeJni object.
    public NodeJni getSceneNode() {
        final NodeJni nodeJni = new NodeJni(false);
        long nodeRef = nativeGetSceneNodeRef(mNativeRef);
        nodeJni.setNativeRef(nodeRef);
        return nodeJni;
    }

    public void setBackgroundVideoTexture(VideoTextureJni videoTexture) {
        nativeSetBackgroundVideoTexture(mNativeRef, videoTexture.mNativeRef);
    }

    public void setBackgroundImageTexture(TextureJni imageTexture) {
        nativeSetBackgroundImageTexture(mNativeRef, imageTexture.mNativeRef);
    }

    public void setBackgroundRotation(float[] rotation) {
        nativeSetBackgroundRotation(mNativeRef, rotation[0], rotation[1], rotation[2]);
    }

    public void setBackgroundCubeImageTexture(TextureJni cubeTexture) {
        nativeSetBackgroundCubeImageTexture(mNativeRef, cubeTexture.mNativeRef);
    }

    public void setBackgroundCubeWithColor(long color) {
        nativeSetBackgroundCubeWithColor(mNativeRef, color);
    }

    public void setSoundRoom(RenderContextJni renderContext, float[] size, String wallMaterial,
                             String ceilingMaterial, String floorMaterial) {
        nativeSetSoundRoom(mNativeRef, renderContext.mNativeRef, size[0], size[1], size[2],
                wallMaterial, ceilingMaterial, floorMaterial);
    }

    public void setPhysicsWorldGravity(float gravity[]){
        nativeSetPhysicsWorldGravity(mNativeRef, gravity);
    }

    public void setPhysicsDebugDraw(boolean debugDraw){
        nativeSetPhysicsWorldDebugDraw(mNativeRef, debugDraw);
    }

    public void attachBodyToPhysicsWorld(NodeJni node){
        nativeAttachToPhysicsWorld(mNativeRef, node.mNativeRef);
    }

    public void detachBodyFromPhysicsWorld(NodeJni node){
        nativeDetachFromPhysicsWorld(mNativeRef, node.mNativeRef);
    }

    public void findCollisionsWithRayAsync(float[] fromPos, float toPos[], boolean closest,
                                           String tag, PhysicsWorldHitTestCallback callback){
        findCollisionsWithRayAsync(mNativeRef, fromPos, toPos, closest, tag, callback);
    }

    public void findCollisionsWithShapeAsync(float[] from, float[] to, String shapeType, float[] params,
                                             String tag, PhysicsWorldHitTestCallback callback){
        findCollisionsWithShapeAsync(mNativeRef, from,to, shapeType, params, tag, callback);
    }

    public void addParticleEmitter(ParticleEmitterJni emitter) {
        nativeAddParticleEmitter(mNativeRef, emitter.mNativeRef);
    }

    public void removeParticleEmitter(ParticleEmitterJni emitter) {
        nativeRemoveParticleEmitter(mNativeRef, emitter.mNativeRef);
    }

    public boolean setEffects(String[] effects){
        return nativeSetEffects(mNativeRef, effects);
    }

    public void destroy() {
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
    private native void nativeAttachToPhysicsWorld(long sceneRef, long nodeRef);
    private native void nativeDetachFromPhysicsWorld(long sceneRef, long nodeRef);
    private native void findCollisionsWithRayAsync(long sceneRef, float[] from, float[] to,
                                           boolean closest, String tag,
                                           PhysicsWorldHitTestCallback callback);
    private native void findCollisionsWithShapeAsync(long sceneRef, float[] from, float[] to,
                                           String shapeType, float[] params, String tag,
                                           PhysicsWorldHitTestCallback callback);
    private native void nativeSetPhysicsWorldDebugDraw(long sceneRef, boolean debugDraw);
    /**
     * Callback used to notify the bridge when the requested hit test
     * (with ray/shapes) with this given implemented callback have completed.
     */
    public interface PhysicsWorldHitTestCallback{
        void onComplete(boolean hasHit);
    }
}