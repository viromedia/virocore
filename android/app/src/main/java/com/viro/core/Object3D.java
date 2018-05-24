/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
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
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Object3D defines a {@link Node} that can load FBX, OBJ, GLTF, or GLB models. These file formats
 * may contain not just single Geometry instances but entire subgraphs of Nodes. This Node acts
 * as the parent to the generated subgraph.
 * <p>
 * OBJ loading supports MTL files, for loading material properties and textures. For OBJ files,
 * Materials can also be manually injected by invoking {@link Geometry#setMaterials(List)}.
 * <p>
 * FBX loading supports Blinn/Phong material properties, diffuse, specular, and normal maps, skeletal and
 * keyframe animation, and PBR material properties (roughness, metalness, ambient occlusion) through
 * Stingray.
 * <p>
 * GLTF and GLB loading similarly support all material properties including PBR properties. Skeletal
 * and keyframe animation however is currently <i>not</i> supported by GLTF/GLB. This support is
 * forthcoming.
 * <p>
 *
 * For an extended discussion on 3D model loading, refer to the <a href="https://virocore.viromedia.com/docs/3d-objects">3D
 * Objects Guide</a>.
 */
public class Object3D extends Node {

    private AsyncObject3DListener mAsyncListener = null;
    private AtomicLong mActiveRequestID;
    private List<Material> mMaterialList;

    /**
     * Supported model formats for loading into an {@link Object3D}.
     */
    public enum Type {
        /**
         * OBJ model format. MTL files are also supported. MTL and textures should reside in the
         * same folder as the OBJ file. This format supports diffuse, specular, and normal maps.
         */
        OBJ(1),

        /**
         * FBX model format. FBX files need to be converted to VRX via the ViroFBX script. Textures
         * should reside in the same folder as the VRX file. This format supports skeletal and
         * keyframe animation, and diffuse, specular, and normal maps.
         */
        FBX(2),

        /**
         * GLTF model format. GLTF files do <i>>not</i> need ViroFBX script conversion: they can
         * be directly loaded into your application. Associated resources like textures should
         * reside in their relative paths as specified in the .gltf JSON file.
         * <p>
         * Viro currently only supports static GLTF models. Animated model (skeletal and keyframe)
         * support is forthcoming.
         * <p>
         */
        GLTF(3),

        /**
         * GLTF binary model format. Like GLTF, GLB files do not need ViroFBX script conversion.
         * Associated resources like textures should reside their relative paths as specified in
         * the JSON GLTF manifest (which should have been Base64 encoded into the GLB file).
         * <p>
         * Viro currently only supports static GLTF models. Animated model (skeletal and keyframe)
         * support is forthcoming.
         * <p>
         */
        GLB(4);

        Type(int assignedID) {
            id = assignedID;
        }

        public final int id;
        private static Map<Integer, Type> map = new HashMap<Integer, Type>();
        static {
            for (Type modelType : Type.values()) {
                map.put(modelType.id, modelType);
            }
        }

        public static Type valueOf(int id) {
            return map.get(id);
        }

        public static Type fromString(String sType) {
            if (sType.equalsIgnoreCase("OBJ")) {
                return Object3D.Type.OBJ;
            } else if (sType.equalsIgnoreCase("VRX")) {
                return Object3D.Type.FBX;
            } else if (sType.equalsIgnoreCase("GLTF")) {
                return Object3D.Type.GLTF;
            } else if (sType.equalsIgnoreCase("GLB")) {
                return Object3D.Type.GLB;
            } else {
                throw new IllegalArgumentException("String [" + sType + "] is not a valid object type.");
            }
        }
    }

    /**
     * Construct a new Object3D. To load 3D model data into this Node, use
     * {@link #loadModel(ViroContext, Uri, Type, AsyncObject3DListener)}.
     */
    public Object3D() {
        mActiveRequestID = new AtomicLong();
    }

    /**
     * Load the model at the given URI into this Node. The model will load asynchronously, and the
     * provided listener will receive a notification when model loading is completed. To load
     * Android assets, use URI's of the form <tt>file:///android_asset/[asset-name]</tt>.
     * <p>
     * If the model requires other resources (e.g. textures), then for FBX and OBJ models those
     * resources are expected to be found at the same base path as the model URI. For GLTB and GLB
     * models, resources are expected at the relative paths specified in the GLTF JSON.
     * <p>
     *
     * @param viroContext   The {@link ViroContext} is required to load models.
     * @param uri           The URI of the model to load.
     * @param type          The model input format.
     * @param asyncListener Listener to respond to model loading status.
     */
    public void loadModel(ViroContext viroContext, Uri uri, Type type, AsyncObject3DListener asyncListener) {
        removeAllChildNodes();
        long requestID = mActiveRequestID.incrementAndGet();
        nativeLoadModelFromURL(uri.toString(), mNativeRef, viroContext.mNativeRef, type.id, requestID);
        mAsyncListener = asyncListener;
    }

    /**
     * Load a model from bundled application resources.
     *
     * @param viroContext   The {@link ViroContext} is required to load models.
     * @param modelResource       The resource.
     * @param type                The model format.
     * @param asyncObjListener    Listener to respond to model loading status.
     * @param resourceNamesToUris Mapping of other resources required by the model, as defined in
     *                            the model file, to the URI's of those resources.
     * @hide
     */
    //#IFDEF 'viro_react'
    public void loadModel(ViroContext viroContext, String modelResource, Type type,
                          AsyncObject3DListener asyncObjListener,
                          Map<String, String> resourceNamesToUris) {
        removeAllChildNodes();

        long requestID = mActiveRequestID.incrementAndGet();
        nativeLoadModelFromResources(modelResource, resourceNamesToUris, mNativeRef, viroContext.mNativeRef,
                                                    type.id, requestID);
        mAsyncListener = asyncObjListener;
    }
    //#ENDIF

    @Override
    public void setLightReceivingBitMask(int bitMask) {
        mLightReceivingBitMask = bitMask;
        nativeSetLightReceivingBitMask(mNativeRef, bitMask, true);
    }

    @Override
    public void setShadowCastingBitMask(int bitMask) {
        mShadowCastingBitMask = bitMask;
        nativeSetShadowCastingBitMask(mNativeRef, bitMask, true);
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
     * @hide
     */
    void nodeDidFinishCreation(Material[] materials, int modelType, long geometryRef) {
        if (mDestroyed) {
            return;
        }

        Type type = Type.valueOf(modelType);
        /*
         If the model loaded is OBJ, create a Java Geometry to wrap the native Geometry.
         This enables developers to set/manipulate materials on the Geometry.
         */
        if (type == Type.OBJ && geometryRef != 0) {
            setGeometry(new Geometry(geometryRef));
        }

        mMaterialList = Arrays.asList(materials);

        if (mAsyncListener != null) {
            mAsyncListener.onObject3DLoaded(this, type);
        }
    }

    /**
     * Get a list of unique {@link Material} objects used by this 3D model. These may be modified
     * at runtime to change the textures and/or other material properties of the model. Materials
     * can often be identified by their name, which is set by the 3D model loader.<p>
     * <p>
     * Note that if a model file contains multiple geometries, this list will contain
     * <i>all</i> the materials across all geometries.
     *
     * @return A list containing each {@link Material} used by this 3D model.
     */
    public List<Material> getMaterials() {
        return mMaterialList;
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

    private native void nativeLoadModelFromURL(String url, long nodeRef, long contextRef,int modelType, long requestID);
    private native void nativeLoadModelFromResources(String modelResource, Map<String, String> assetResources, long nodeRef, long contextRef, int modelType, long requestID);
}
