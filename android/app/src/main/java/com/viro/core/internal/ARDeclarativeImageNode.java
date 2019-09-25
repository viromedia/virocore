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

import com.viro.core.ARImageTarget;

/**
 * @hide
 *
 * This class represents an ARDeclarativeNode that should be matched with
 * the given ARImageTarget
 */
//#IFDEF 'viro_react'
public class ARDeclarativeImageNode extends ARDeclarativeNode {

    private ARImageTarget mARImageTarget;

    public ARDeclarativeImageNode() {
        initWithNativeRef(nativeCreateARImageTargetNode());
    }

    public void setARImageTarget(ARImageTarget arImageTarget) {
        mARImageTarget = arImageTarget;
        nativeSetARImageTarget(mNativeRef, arImageTarget.getNativeRef());
    }

    public ARImageTarget getARImageTarget() {
        return mARImageTarget;
    }

    private native long nativeCreateARImageTargetNode();
    private native void nativeSetARImageTarget(long nativeRef, long arImageTargetRef);
}
//#ENDIF