/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Android Java Object  : com.viromedia.bridge.viewgroups.Scene.java
 * Java JNI Wrapper     : com.viro.renderer.SceneJni.java
 * Cpp JNI wrapper      : Scene_JNI.cpp
 * Cpp Object           : VROScene.cpp
 */
public class SceneJni {
    public long mNativeRef;

    public SceneJni(NodeJni node) {
        // Set the root node of this scene
        mNativeRef = nativeCreateScene(node.mNativeRef);
    }

    private native long nativeCreateScene(long nodeRef);
    private native void nativeDestroyScene(long sceneReference);
}
