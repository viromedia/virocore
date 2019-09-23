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

package com.viro.core.internal;

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
//#IFDEF 'viro_react'
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
//#ENDIF