/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

/**
 * @hide
 */
//#IFDEF 'viro_react'
public class OpenCV {

    private static final String OUTPUT_PREFIX = "edge_";

    private Context mContext;

    public OpenCV(Context context) {
        mContext = context;
    }

    /** Runs Canny edge detection on a given asset */
    public Bitmap edgeDetectImage(String assetName) {
        /*
         first write asset to cache
          */
        String inputFilePath = writeAssetImageToCache(assetName);
        String outputFilePath = mContext.getCacheDir().getAbsolutePath() + File.separator + OUTPUT_PREFIX + assetName;
        nativeRunEdgeDetection(inputFilePath, outputFilePath);

        File imgFile = new File(outputFilePath);
        return BitmapFactory.decodeFile(imgFile.getAbsolutePath());
    }

    public Bitmap readAndWriteBitmap() {
        String inputFilePath = writeAssetImageToCache("ben.jpg");
        String outputFilePath = mContext.getCacheDir().getAbsolutePath() + File.separator + OUTPUT_PREFIX + "foobario.png";

        nativeReadWriteBitmap(inputFilePath, outputFilePath);

        return BitmapFactory.decodeFile(new File(outputFilePath).getAbsolutePath());
    }

    private String writeAssetImageToCache(String inputName) {
        return writeAssetImageToCache(inputName, inputName);
    }

    private String writeAssetImageToCache(String inputName, String outputName) {
        String outputFilePath = mContext.getCacheDir().getAbsolutePath() + File.separator + outputName;

        try {
            /*
             write from assets to image cache
              */
            InputStream is = mContext.getAssets().open(inputName);
            int size = is.available();
            byte[] buffer = new byte[size];
            is.read(buffer);
            is.close();


            FileOutputStream fos = new FileOutputStream(new File(outputFilePath));
            fos.write(buffer);
            fos.close();
        }
        catch (Exception e) {
            Log.e("OpenCVJni", "oops! failed to write image [" + inputName + "] to output[" + outputName + "]", e);
            return null; // return null if we errored writing it out!
        }
        return outputFilePath;
    }

    private native void nativeRunEdgeDetection(String input, String output);

    private native void nativeReadWriteBitmap(String in, String out);
}
//#ENDIF
