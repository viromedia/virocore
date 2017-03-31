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

public class ObjectJni extends BaseGeometry {
    private long mObjectNodeRef;
    private long mNodeRef;
    private AsyncObjListener mAsyncObjListener = null;
    private boolean mDestroyOnObjNodeCreation = false;

    public ObjectJni(Uri pathOrUrl, AsyncObjListener asyncObjListener) {
        nativeLoadOBJFromUrl(pathOrUrl.toString());
        mAsyncObjListener = asyncObjListener;
    }

    public ObjectJni(Uri pathOrUrl, AsyncObjListener asyncObjListener,
                     Map<String, String> resourceNamesToUris) {
        if (resourceNamesToUris == null) {
            nativeLoadOBJFromFile(pathOrUrl.toString());
        } else {
            nativeLoadOBJAndResourcesFromFile(pathOrUrl.toString(), resourceNamesToUris);
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
    public void attachToNode(NodeJni node) {
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

    private native void nativeLoadOBJFromFile(String fileName);
    private native void nativeLoadOBJAndResourcesFromFile(String fileName,
                                                          Map<String, String> resources);
    private native void nativeLoadOBJFromUrl(String url);
    private native void nativeDestroyNode(long nodeReference);
    private native void nativeAttachToNode(long boxReference, long nodeReference);
}