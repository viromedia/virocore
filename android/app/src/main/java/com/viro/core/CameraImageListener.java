/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import java.nio.ByteBuffer;

/**
 * CameraImageListener receives a callback each time the AR camera image is updated. This can be
 * used to process camera imagery for your own purposes as it is rendered to the device. This
 * listener is installed via {@link ARScene#setCameraImageListener(CameraImageListener)}.
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
     * @param intrinsics Camera Intrinsics of the device camera. See {@link CameraIntrinsics}
     */
    public void onCameraImageUpdated(ByteBuffer buffer, int width, int height, CameraIntrinsics intrinsics);

}
