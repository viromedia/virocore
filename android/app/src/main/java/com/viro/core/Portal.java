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

package com.viro.core;

/**
 * Portal is a special {@link Node} that serves as an entry or window into an associated
 * {@link PortalScene}.
 * <p>
 * The Portal's contents (its subnodes and geometry) should be a 3D representation of a
 * window or door, aligned with the X axis in its local coordinate system. The transparent
 * sections of the Portal will display the {@link PortalScene} for which the Portal serves
 * as an entrance.
 *
 * @see PortalScene
 */
public class Portal extends Node {

    /**
     * Construct a new Portal.
     */
    public Portal() {
        super(false);
        initWithNativeRef(nativeCreatePortal());
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
     * Release native resources associated with this Portal.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyPortal(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreatePortal();
    private native void nativeDestroyPortal(long portalRef);
}
