/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

/**
 * Helper to ask camera permission.
 * @hide
 */
public class CameraPermissionHelper {
  private static final String CAMERA_PERMISSION = Manifest.permission.CAMERA;
  private static final int CAMERA_PERMISSION_CODE = 0;

  /**
   * Check to see we have the necessary permissions for this app.
   */
  public static boolean hasCameraPermission(Context context) {
    return ContextCompat.checkSelfPermission(context, CAMERA_PERMISSION) ==
            PackageManager.PERMISSION_GRANTED;
  }

  /**
   * Check to see we have the necessary permissions for this app, and ask for them if we don't.
   */
  public static void requestCameraPermission(Activity activity) {
    ActivityCompat.requestPermissions(activity, new String[]{CAMERA_PERMISSION},
            CAMERA_PERMISSION_CODE);
  }

  /**
   * Given a set of permissions and results from {@link Activity#onRequestPermissionsResult(int, String[], int[])},
   * we check to see if the requested Camera permission had been granted or not.
   */
  public static boolean checkResult(int req, String[] perm, int[] result, Context context){
    if (req != CAMERA_PERMISSION_CODE) {
      return false;
    }

    for (int i = 0; i < perm.length; i ++){
      if (perm[i].equals(Manifest.permission.CAMERA) &&
              result[i] == PackageManager.PERMISSION_GRANTED) {
          return true;
        }
      }

    return false;
  }
}
