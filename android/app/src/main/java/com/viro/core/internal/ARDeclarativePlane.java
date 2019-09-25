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

import com.viro.core.ARPlaneAnchor;

/**
 * @hide
 */
//#IFDEF 'viro_react'
public class ARDeclarativePlane extends ARDeclarativeNode {

    public ARDeclarativePlane(float minWidth, float minHeight, ARPlaneAnchor.Alignment alignment) {
        super();
        initWithNativeRef(nativeCreateARPlane(minWidth, minHeight, alignment.getStringValue()));
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        }
        finally {
            super.finalize();
        }
    }

    @Override
    public void dispose() {
        super.dispose();
    }

    public void setMinWidth(float minWidth) {
        nativeSetMinWidth(mNativeRef, minWidth);
    }
    public void setMinHeight(float minHeight) {
        nativeSetMinHeight(mNativeRef, minHeight);
    }
    public void setAlignment(ARPlaneAnchor.Alignment alignment) {
        nativeSetAlignment(mNativeRef, alignment.getStringValue());
    }

    private native long nativeCreateARPlane(float minWidth, float minHeight, String alignment);
    private native void nativeSetMinWidth(long nativeRef, float minWidth);
    private native void nativeSetMinHeight(long nativeRef, float minHeight);
    private native void nativeSetAlignment(long nativeRef, String alignment);

}
//#ENDIF