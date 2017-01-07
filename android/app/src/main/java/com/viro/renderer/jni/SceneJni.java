/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Android Java Object  : com.viromedia.bridge.viewgroups.Scene.java
 * Java JNI Wrapper     : com.viro.renderer.jni.SceneJni.java
 * Cpp JNI wrapper      : Scene_JNI.cpp
 * Cpp Object           : VROScene.cpp
 */
public class SceneJni {
    public long mNativeRef;

    public SceneJni(NodeJni node) {
        // Set the root node of this scene
        mNativeRef = nativeCreateScene(node.mNativeRef);
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

    public void addDirectionalLight(NodeJni node, long color, float[] direction) {

        nativeAddDirectionalLight(mNativeRef, node.mNativeRef, color, direction[0], direction[1], direction[2]);
    }

    public void addAmbientLight(NodeJni node, long color) {

        nativeAddAmbientLight(mNativeRef, node.mNativeRef, color);
    }

    public void addOmniLight(NodeJni node, long color, float attenuationStartDistance,
                             float attenuationEndDistance, float[] position) {

        nativeAddOmniLight(mNativeRef, node.mNativeRef, color, attenuationStartDistance,
                attenuationEndDistance, position[0], position[1], position[2]);
    }

    public void addSpotLight(NodeJni node, long color, float attenuationStartDistance,
                             float attenuationEndDistance, float[] position, float[] direction,
                             float innerAngle, float outerAngle) {
        nativeAddSpotLight(mNativeRef, node.mNativeRef, color, attenuationStartDistance,
                attenuationEndDistance, position[0], position[1],position[2],
                                        direction[0], direction[1], direction[2],
                innerAngle, outerAngle);

    }
    public void destroy() {
        nativeDestroyScene(mNativeRef);
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateScene(long nodeRef);
    private native void nativeDestroyScene(long sceneReference);
    private native void nativeSetBackgroundVideoTexture(long sceneRef, long videoRef);
    private native void nativeSetBackgroundImageTexture(long sceneRef, long imageRef);
    private native void nativeSetBackgroundCubeImageTexture(long sceneRef, long textureRef);
    private native void nativeSetBackgroundRotation(long sceneRef, float degreeX, float degreeY,
                                                    float degreeZ);

    private native void nativeAddDirectionalLight(long sceneRef, long nodeRef, long color,
                                                  float directionX, float directionY,
                                                  float directionZ);
    private native void nativeAddAmbientLight(long sceneRef, long nodeRef, long color);
    private native void nativeAddOmniLight(long sceneRef, long nodeRef, long color,
                                           float attenuationStartDistance,
                                           float attenuationEndDistance,
                                           float positionX, float positionY, float positionZ);
    private native void nativeAddSpotLight(long sceneRef, long nodeRef, long color,
                                           float attenuationStartDistance,
                                           float attenuationEndDistance,
                                           float positionX, float positionY, float positionZ,
                                           float directionX, float directionY, float directionZ,
                                           float innerAngle, float outerAngle);
    private SceneDelegate mDelegate = null;
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
        mDelegate = delegate;
    }

    /* Called by Native */
    public void onSceneWillAppear() {
        if (mDelegate != null){
            mDelegate.onSceneWillAppear();
        }
    }

    /* Called by Native */
    public void onSceneDidAppear() {
        if (mDelegate != null) {
            mDelegate.onSceneDidAppear();
        }
    }

    /* Called by Native */
    public void onSceneWillDisappear(){
        if (mDelegate != null) {
            mDelegate.onSceneWillDisappear();
        }
    }

    /* Called by Native */
    public void onSceneDidDisappear() {
        if (mDelegate != null) {
            mDelegate.onSceneDidDisappear();
        }
    }
}