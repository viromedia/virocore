//
//  Copyright (c) 2018-present, ViroMedia, Inc.
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

import java.nio.ByteBuffer;

/**
 * CameraImageListener receives a callback each time the AR camera image is updated. This can be
 * used to process camera imagery, for your own purposes, as it is rendered to the device. This
 * listener is installed via {@link ViroViewARCore#setCameraImageListener(ViroContext, CameraImageListener)}.
 */
public interface CameraImageListener {

    /**
     * Callback invoked when the AR camera image has been updated. The updated image is stored in
     * the provided {@link ByteBuffer} as RGBA8888 data. The image is of the given width and
     * height. The following code converts the image data to a {@link android.graphics.Bitmap}, for
     * example:
     * <p>
     * <pre>
     * Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
     * bitmap.copyPixelsFromBuffer(buffer);
     * </pre>
     * <p>
     *
     * It is important to not store the given buffer, as it will be overwritten with new image
     * data when this callback is invoked for the next frame. If you need to preserve the image data,
     * copy it to a separate buffer.
     *
     * @param buffer The buffer containing the RGBA8888 data.
     * @param width The width of the image.
     * @param height The height of the image.
     * @param intrinsics The {@link CameraIntrinsics} of the device's camera, which describe the camera's
     *                   physical characteristics.
     */
    public void onCameraImageUpdated(ByteBuffer buffer, int width, int height, CameraIntrinsics intrinsics);

}
