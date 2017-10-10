/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.net.Uri;

import java.util.Map;

/**
 * Java JNI wrapper for linking the following classes below across the bridge
 *
 * Android Java Object : com.viromedia.bridge.view.Object3d.java
 * Java JNI Wrapper : com.viro.renderer.ObjectJni.java
 * Cpp JNI Wrapper : Object_JNI.cpp
 * Cpp Object : VROOBJLoader.cpp
 */

public class Object3D extends BaseGeometry {
    private long mObjectNodeRef;
    private long mNodeRef;
    private AsyncObjListener mAsyncObjListener = null;
    private boolean mDestroyOnObjNodeCreation = false;

    public Object3D(Uri pathOrUrl, boolean isFBX, AsyncObjListener asyncObjListener) {
        nativeLoadModelFromUrl(pathOrUrl.toString(), isFBX);
        mAsyncObjListener = asyncObjListener;
    }

    public Object3D(Uri pathOrUrl, boolean isFBX, AsyncObjListener asyncObjListener,
                    Map<String, String> resourceNamesToUris) {
        if (resourceNamesToUris == null) {
            nativeLoadModelFromFile(pathOrUrl.toString(), isFBX);
        } else {
            nativeLoadModelAndResourcesFromFile(pathOrUrl.toString(), resourceNamesToUris, isFBX);
        }
        mAsyncObjListener = asyncObjListener;
    }

    public void destroy() {
        if (mObjectNodeRef != 0) {
            nativeDestroyNode(mObjectNodeRef);
            mObjectNodeRef = 0;
        }
        else {
            mDestroyOnObjNodeCreation = true;
        }

        // Do NOT destroy mNodeRef, as that belongs to the parent
        // node; it's only temporarily stored here while we wait for
        // the OBJ to load (if we destroy it, we'll get a crash if
        // we attempt to access the node after this Object was removed
        // from it)
        mNodeRef = 0;
    }

    @Override
    public void attachToNode(Node node) {
        // Just save the node ref for now. We'll attach geometry to this node when it's ready
        mNodeRef = node.mNativeRef;

        // If node with obj ready
        if (mObjectNodeRef != 0) {
            nativeAttachToNode(mObjectNodeRef, mNodeRef);
            // after copying geometry from mObjectNodeRef to mNodeRef, we're done here
            destroy();
        }
    }

    // Called from JNI upon successful loading of OBJ file
    public void nodeDidFinishCreation(long objectNodeRef) {
        mObjectNodeRef = objectNodeRef;
        // If destroy was called before obj was loaded
        if (mDestroyOnObjNodeCreation) {
            destroy();
        }
        else {
            if (mNodeRef != 0) {
                // after copying geometry to mNodeRef, we're done here
                nativeAttachToNode(mObjectNodeRef, mNodeRef);
                destroy();
            }

            if (mAsyncObjListener != null) {
                // So that clients can override materials if they want to
                mAsyncObjListener.onObjLoaded();
            }
        }
    }

    // Called from JNI upon OBJ failing to load
    public void nodeDidFailOBJLoad(String error) {
        if (mDestroyOnObjNodeCreation) {
            destroy();
        }
        else if (mAsyncObjListener != null) {
            mAsyncObjListener.onObjFailed(error);
        }
    }

    // Called from JNI after OBJ added to scene
    public void nodeDidAttachOBJ() {
        if (mAsyncObjListener != null) {
            mAsyncObjListener.onObjAttached();
        }
    }

    private native void nativeLoadModelFromFile(String fileName, boolean isFBX);
    private native void nativeLoadModelAndResourcesFromFile(String fileName,
                                                            Map<String, String> resources, boolean isFBX);
    private native void nativeLoadModelFromUrl(String url, boolean isFBX);
    private native void nativeDestroyNode(long nodeReference);
    private native void nativeAttachToNode(long boxReference, long nodeReference);
}