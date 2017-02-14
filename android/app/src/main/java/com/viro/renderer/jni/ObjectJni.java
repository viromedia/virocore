/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.net.Uri;

/**
 * Java JNI wrapper for linking the following classes below across the bridge
 *
 * Android Java Object : com.viromedia.bridge.view.Object3d.java
 * Java JNI Wrapper : com.viro.renderer.ObjectJni.java
 * Cpp JNI Wrapper : Object_JNI.cpp
 * Cpp Object : VROOBJLoader.cpp
 */

public class ObjectJni extends BaseGeometry {
    private long mObjectNodeRef;
    private long mNodeRef;
    private AsyncObjListener mAsyncObjListener = null;
    private boolean mDestroyOnObjNodeCreation = false;
    private boolean mObjectLoadingFinished = false;
    private boolean mAttachOnObjLoadingFinish = false;

    public ObjectJni(String pathOrUrl, AsyncObjListener asyncObjListener) {
        Uri tempUri = Uri.parse(pathOrUrl);
        if (tempUri.getScheme() == null) {
            // if the scheme is null, then it's a local resource
            mObjectNodeRef = nativeLoadOBJFromFile(pathOrUrl);
        } else {
            mObjectNodeRef = nativeLoadOBJFromUrl(pathOrUrl);
        }
        mAsyncObjListener = asyncObjListener;
    }

    public void destroy() {
        if (mObjectLoadingFinished) {
            if (mObjectNodeRef != 0) {
                nativeDestroyNode(mObjectNodeRef);
                mObjectNodeRef = 0;
            }
            if (mNodeRef != 0) {
                nativeDestroyNode(mNodeRef);
                mNodeRef = 0;
            }
        } else {
            mDestroyOnObjNodeCreation = true;
        }
    }

    @Override
    public void attachToNode(NodeJni node) {
        // Just save the node ref for now. We'll attach geometry to this node when it's ready
        mNodeRef = node.mNativeRef;
        // If node with obj ready
        if (mObjectLoadingFinished) {
            nativeAttachToNode(mObjectNodeRef, mNodeRef);
            // above we copied geometry from mObjectNodeRef to mNodeRef, don't need mObjectNodeRef
            nativeDestroyNode(mObjectNodeRef);
            mObjectNodeRef = 0;
        } else {
            mAttachOnObjLoadingFinish = true;
        }
    }

    // Called from JNI upon successful loading of OBJ file
    public void nodeDidFinishCreation() {
        mObjectLoadingFinished = true;
        if (mAttachOnObjLoadingFinish) {
            nativeAttachToNode(mObjectNodeRef, mNodeRef);
            nativeDestroyNode(mObjectNodeRef);
            mObjectNodeRef = 0;
        }

        if (mAsyncObjListener != null) {
            // So that clients can override materials if they want to
            mAsyncObjListener.onObjLoaded();
        }

        // If destroy was called before obj was loaded
        if (mDestroyOnObjNodeCreation) {
            destroy();
        }
    }

    private native long nativeLoadOBJFromFile(String fileName);
    private native long nativeLoadOBJFromUrl(String url);
    private native void nativeDestroyNode(long nodeReference);
    private native void nativeAttachToNode(long boxReference, long nodeReference);
}