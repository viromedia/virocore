//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
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
    private QueuedModel mQueuedModel;

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
     * For use when an Object load request is issued while another request is ongoing.
     */
    private static class QueuedModel {
        ViroContext mContext;
        Uri mUri;
        Type mType;
        AsyncObject3DListener mListener;

        public QueuedModel(ViroContext viroContext, Uri uri, Type type, AsyncObject3DListener asyncListener) {
            mContext = viroContext;
            mUri = uri;
            mType = type;
            mListener = asyncListener;
        }
    }

    /**
     * Construct a new Object3D. To load 3D model data into this Node, use
     * {@link #loadModel(ViroContext, Uri, Type, AsyncObject3DListener)}.
     */
    public Object3D() {
        mActiveRequestID = new AtomicLong(0);
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
     * If another model is already loaded into this Object3D, that model will be immediately
     * removed. If the Object3D is currently loading another model, that model will be disposed.
     * <p>
     *
     * @param viroContext   The {@link ViroContext} is required to load models.
     * @param uri           The URI of the model to load.
     * @param type          The model input format.
     * @param asyncListener Listener to respond to model loading status.
     */
    public void loadModel(ViroContext viroContext, Uri uri, Type type, AsyncObject3DListener asyncListener) {
        // If we're not currently loading any model
        if (mActiveRequestID.get() == 0) {
            removeAllChildNodes();
            long requestID = mActiveRequestID.incrementAndGet();
            nativeLoadModelFromURL(uri.toString(), mNativeRef, viroContext.mNativeRef, type.id, requestID);
            mAsyncListener = asyncListener;
        }
        else {
            // Otherwise we're currently loading a model; queue this for when the current load ends
            mQueuedModel = new QueuedModel(viroContext, uri, type, asyncListener);
        }
    }

    /**
     * Remove and dispose the model that was loaded in this Object3D. This will remove and delete
     * all the {@link Node}, {@link Geometry}, {@link Material}, and {@link Texture} objects created
     * during the last call to {@link #loadModel(ViroContext, Uri, Type, AsyncObject3DListener)}.
     * <p>
     * This is especially useful when removing models with a large memory footprint, to ensure
     * they're deleted quickly without having to wait for GC. Note this method does not dispose of
     * <i>this</i> Node; you can still load a new model into this Node via {@link
     * #loadModel(ViroContext, Uri, Type, AsyncObject3DListener)}.
     */
    public void disposeModel() {
        List<Node> children = new ArrayList<>(getChildNodes());
        for (Node child : children) {
            child.disposeAll(true);
        }

        if (mMaterialList != null) {
            for (Material material : mMaterialList) {
                material.dispose();
            }
            mMaterialList = null;
        }

        // Ensure that we also remove any native nodes that may not be tracked in java
        nativeRemoveAllChildNodes(mNativeRef);
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

    /**
     * MorphMode represents the method used to calculate and blend morph target data in this
     * Object3D.
     */
    public enum MorphMode {
        /**
         * Performs morph target blending calculations on the GPU. This is done by plumbing
         * each target's data through vertex array attributes and blending them in the vertex
         * shader.
         * <p>
         * Because the number of vertex array attributes are limited, Viro only supports at most
         * 7 morph targets on the GPU. If your model contains more than 7 morph targets, Viro
         * will automatically fall back into CPU mode.
         * <p>
         * This mode is ideal for models with a low number of morph targets.
         */
        GPU("gpu"),

        /**
         * Performs morph target blending calculations on the CPU.
         * <p>
         * Because calculations are CPU based, there is no pre-defined limit to the number
         * of morph targets your model can have. However, keep in mind that a large number
         * of morph targets can lead to a performance bottleneck on the CPU.
         * <p>
         * This mode is ideal for models with a large number of morph targets.
         */
        CPU("cpu"),

        /**
         * Morph target blending calculations are done on the CPU, but are interpolated back onto
         * the geometry of this model on the GPU.
         * <p>
         * This mode is ideal for animating large numbers of morph target properties
         * through animations with long durations.
         */
        HYBRID("hybrid");

        public final String mStringValue;
        private MorphMode(String value) {
            this.mStringValue = value;
        }
    }

    /**
     * Get the names of the available morph targets on this Node or any of its children.
     * If this Node represents a scene loaded from a 3D file format like FBX, then the returned
     * set will contain the names of all morph targets that were installed.
     *
     * @return The names of each morph target in this Node.
     */
    public Set<String> getMorphTargetKeys() {
        return new HashSet<String>(Arrays.asList(nativeGetMorphTargetKeys(mNativeRef)));
    }

    /**
     * Sets the weight of the morph target with the given name in this 3D model.
     *
     * @param name The name of the target to morph.
     * @param weight The amount by which to morph toward the target, between 0 and 1.
     */
    public void setMorphTargetWeight(String name, float weight) {
        nativeSetMorphTargetWithWeight(mNativeRef, name, weight);
    }

    /**
     * Sets the {@link MorphMode} this Object3D will use to render applied morph target weights.
     *
     * @param mode The MorphMode to apply.
     */
    public void setMorphMode(MorphMode mode) {
        nativeSetMorphMode(mNativeRef, mode.mStringValue);
    }

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
        inflateChildNodes(this);

        updateWorldTransforms();
        updateAllUmbrellaBounds();

        if (mAsyncListener != null) {
            mAsyncListener.onObject3DLoaded(this, type);
        }
        mActiveRequestID.set(0);
        if (mQueuedModel != null) {
            // Immediately dispose the just-loaded model since we're replacing it
            disposeModel();
            loadModel(mQueuedModel.mContext, mQueuedModel.mUri, mQueuedModel.mType, mQueuedModel.mListener);
            mQueuedModel = null;
        }
    }

    private void inflateChildNodes(Node currentNode) {
        long currentNativeRef = currentNode.getNativeRef();
        if (currentNativeRef == 0) {
            return;
        }

        Node childNodes[] = nativeCreateChildNodes(currentNativeRef);
        if (childNodes.length == 0) {
            return;
        }

        // Populate individual nodes and its properties one at a time, so as
        // to ensure we don't hit the jni local reference limit.
        for (Node childNode : childNodes) {
            currentNode.addNativelyAttachedChildNode(childNode);
            nativeIntializeNode(childNode, childNode.getNativeRef());
            inflateChildNodes(childNode);
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

        mActiveRequestID.set(0);
        if (mQueuedModel != null) {
            loadModel(mQueuedModel.mContext, mQueuedModel.mUri, mQueuedModel.mType, mQueuedModel.mListener);
            mQueuedModel = null;
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
    private native Node[] nativeCreateChildNodes(long nodeRef);
    private native void nativeIntializeNode(Node node, long nodeRef);
    private native void nativeSetMorphTargetWithWeight(long nodeReference, String target, float weight);
    private native void nativeSetMorphMode(long nodeRef, String mode);
    private native String[] nativeGetMorphTargetKeys(long nodeReference);
}
