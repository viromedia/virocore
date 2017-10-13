/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.net.Uri;
import android.util.Log;

import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Java JNI wrapper for linking the following classes below across the bridge
 *
 * Android Java Object : com.viromedia.bridge.view.Object3d.java
 * Java JNI Wrapper : com.viro.renderer.ObjectJni.java
 * Cpp JNI Wrapper : Object_JNI.cpp
 * Cpp Object : VROOBJLoader.cpp
 */

public class Object3D extends Node {

    private AsyncObject3DListener mAsyncListener = null;
    private AtomicLong mActiveRequestID;

    public enum Type {
        OBJ,
        FBX
    }

    /**
     * Construct a new Object3D. To load 3D model data into this Node, use
     * {@link #loadModel(Uri, Type, AsyncObject3DListener)}.
     */
    public Object3D() {
        mActiveRequestID = new AtomicLong();
    }

    public void loadModel(Uri url, Type type, AsyncObject3DListener asyncListener) {
        long requestID = mActiveRequestID.incrementAndGet();
        nativeLoadModelFromURL(url.toString(), mNativeRef, type == Type.FBX, requestID);

        mAsyncListener = asyncListener;
    }

    public void loadModel(String modelResource, Type type, AsyncObject3DListener asyncObjListener,
                          Map<String, String> resourceNamesToUris) {
        long requestID = mActiveRequestID.incrementAndGet();
        nativeLoadModelFromResources(modelResource, resourceNamesToUris, mNativeRef, type == Type.FBX, requestID);
        mAsyncListener = asyncObjListener;
    }

    /**
     * Release native resources associated with this Object3D.
     */
    public void dispose() {
        super.dispose();
    }

    /**
     * Called from JNI upon successful loading of an Object3D into this Node.
     *
     * @param isFBX True if the model loaded is FBX, false if OBJ.
     * @hide
     */
    public void nodeDidFinishCreation(boolean isFBX, long geometryRef) {
        if (mDestroyed) {
            return;
        }

        /*
         If the model loaded is OBJ, create a Java Geometry to wrap the native Geometry.
         This enables developers to set/manipulate materials on the Geometry.
         */
        if (!isFBX && geometryRef != 0) {
            setGeometry(new Geometry(geometryRef));
        }

        if (mAsyncListener != null) {
            mAsyncListener.onObject3DLoaded(this, isFBX ? Type.FBX : Type.OBJ);
        }
    }

    /**
     * Called from JNI upon loading failure.
     *
     * @param error The error message.
     * @hide
     */
    public void nodeDidFailOBJLoad(String error) {
        if (!mDestroyed && mAsyncListener != null) {
            mAsyncListener.onObject3DFailed(error);
        }
    }

    /**
     * Called from JNI to retrieve the active request ID.
     *
     * @return The active request ID.
     * @hide
     */
    public long getActiveRequestID() {
        return mActiveRequestID.get();
    }

    private native void nativeLoadModelFromURL(String url, long nodeRef, boolean isFBX, long requestID);
    private native void nativeLoadModelFromResources(String modelResource, Map<String, String> assetResources, long nodeRef, boolean isFBX, long requestID);
}