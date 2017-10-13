package com.viro.renderer.jni;
/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

/**
 The animation system requires lazy loading of materials from
 the bridge's material manager. To faciliate this we have this
 class, LazyMaterialJni, which corresponds to the native class
 VROLazyMaterialJNI. VROLazyMaterialJNI implements the renderer's
 lazy loading material interface, VROLazyMaterial. Its implementation
 invokes *this* abstract class, LazyMaterialJni, which is implemented
 by the React bridge to grab the associated material.

 More clearly:

 Renderer invokes get() on VROLazyMaterial when it needs a material
 VROLazyMaterialJNI implements VROLazyMaterial
 VROLazyMaterialJNI get() invokes abstract java class LazyMaterialJni
 LazyMaterialReact (in MaterialManager in the bridge) extends LazyMaterialJni,
 returning the desired material.

 @hide
 */
public abstract class LazyMaterial {

    protected long mNativeRef;

    public LazyMaterial() {
        mNativeRef = nativeCreateLazyMaterial();
    }
    public abstract LazyMaterial copy();

    public void destroy() {
        nativeDestroyLazyMaterial(mNativeRef);
    }

    /*
     Return the pointer to the native VROMaterial that we're lazily
     loading. This is invoked natively.
     */
    public abstract long get();

    /*
     Get the native reference to this lazy material.
     */
    public long getNativeRef() {
        return mNativeRef;
    }

    private native long nativeCreateLazyMaterial();
    private native long nativeDestroyLazyMaterial(long nativeRef);

}
