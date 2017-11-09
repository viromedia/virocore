/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge
 *
 * Android Java Object : com.viromedia.bridge.view.Object3d.java
 * Java JNI Wrapper : com.viro.renderer.ObjectJni.java
 * Cpp JNI Wrapper : Object_JNI.cpp
 * Cpp Object : VROOBJLoader.cpp
 */
package com.viro.core;

import android.net.Uri;

import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Object3D defines a {@link Node} that can load FBX or OBJ models. These file formats
 * may contain not just single Geometry instances but entire subgraphs of Nodes. This Node
 * acts as the parent to the generated subgraph.
 * <p>
 * OBJ loading supports MTL files, for loading material properties and textures. For OBJ
 * files, Materials can also be manually injected by invoking {@link Geometry#setMaterials(List)}.
 * <p>
 * FBX loading fully supports material properties, diffuse, specular, and normal maps, and
 * skeletal and keyframe animation.
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

    /**
     * Load an FBX or OBJ model at the given URI into this Node. The model will load asynchronously,
     * and the provided listener will receive a notification when model loading is completed. To
     * load Android assets, use URI's of the form <tt>file:///android_asset/[asset-name]</tt>. If
     * the model requires other resources (e.g. textures), those are expected to be found at the
     * same base path as the model URI.
     *
     * @param uri           The URI of the model to load.
     * @param type          The type of model (FBX or OBJ).
     * @param asyncListener Listener to respond to model loading status.
     */
    public void loadModel(Uri uri, Type type, AsyncObject3DListener asyncListener) {
        removeAllChildNodes();
        long requestID = mActiveRequestID.incrementAndGet();
        nativeLoadModelFromURL(uri.toString(), mNativeRef, type == Type.FBX, requestID);
        mAsyncListener = asyncListener;
    }

    /**
     * Load an FBX or OBJ model from bundled application resources.
     *
     * @param modelResource       The resource of the FBX or OBJ.
     * @param type                The type of model (FBX or OBJ).
     * @param asyncObjListener    Listener to respond to model loading status.
     * @param resourceNamesToUris Mapping of other resources required by the model, as defined in
     *                            the model's FBX or OBJ file, to the URI's of those resources.
     * @hide
     */
    public void loadModel(String modelResource, Type type, AsyncObject3DListener asyncObjListener,
                          Map<String, String> resourceNamesToUris) {
        removeAllChildNodes();

        long requestID = mActiveRequestID.incrementAndGet();
        nativeLoadModelFromResources(modelResource, resourceNamesToUris, mNativeRef, type == Type.FBX, requestID);
        mAsyncListener = asyncObjListener;
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
    void nodeDidFinishCreation(boolean isFBX, long geometryRef) {
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
    void nodeDidFailOBJLoad(String error) {
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
    long getActiveRequestID() {
        return mActiveRequestID.get();
    }

    private native void nativeLoadModelFromURL(String url, long nodeRef, boolean isFBX, long requestID);
    private native void nativeLoadModelFromResources(String modelResource, Map<String, String> assetResources, long nodeRef, boolean isFBX, long requestID);
}