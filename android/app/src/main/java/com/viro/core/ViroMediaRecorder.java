/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.core;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.media.MediaRecorder;
import android.net.Uri;
import android.opengl.GLES20;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.support.v4.content.ContextCompat;
import android.util.Log;

import com.viro.core.internal.MediaRecorderSurface;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * ViroMediaRecorder encapsulates the implementation and handling of screenshot capturing and
 * screen recording of a rendered scene associated with a given Renderer.
 */
public class ViroMediaRecorder {
    /*
     An Android Media Recorder that handles the starting, stopping, encoding and storing
     of recorded video and audio data.
     */
    private MediaRecorder mRecorder;

    /*
     An egl surface that is created from mRecorder's Android surface view, onto which
     we render video frames to record (through swapping eglsurfaces).
     */
    private MediaRecorderSurface mInputSurface = null;

    /*
     The native reference of MediaRecorderJNI in the renderer that is used to schedule screen
     capturing, and as well as to handle the native calls required for screen video recording.
     */
    private long mNativeRecorderRef;

    /*
     Contexts and UI handlers required to post recording tasks on to the ui thread.
     */
    private Context mAppContext;
    private Handler mUIHandler;

    /*
     Viewport dimensions for recording / getting a screenshot.
     */
    private int mViewportWidth;
    private int mViewportHeight;

    /*
     The file name under which to save the recoded video.
     */
    private String mVideoRecordingFilename = null;

    /*
     A video delegate upon which to notify errors that had occurred with current ongoing
     video recordings.
     */
    private ViroMediaRecorderDelegate mVideoRecordingErrorDelegate;

    /*
     True if stopRecordingAsync, or stopVideoRecordingAsyncWithError has been called on this recorder.
     Can be called either by the renderer or ui thread.
     */
    private AtomicBoolean mPendingStopRecording = new AtomicBoolean(false);

    /*
     Runnable to be executed on the UI thread to asynchronously stop videoRecordings
     and cleanup the recorder's resources as a part of the stopRecordingAsync, or
     stopVideoRecordingAsyncWithError call.
     */
    private Runnable mStopVideoRecordingRunnable = null;

    /*
     True if a video recording has been scheduled / is in progress.
     */
    private boolean mIsRecording = false;

    /*
     True if destroy() has been called on this ViroMediaRecorder, that then schedules the
     stopping of any ongoing recordings and the releasing of recording resources.
     */
    private boolean mIsPendingDestory = false;

    /*
     Guards the list of queued screen shots as they are both processed
     by the renderer thread and background ui thread.
     */
    private Object mQueuedScreenShotLock = new Object();

    /*
     A list of screenshot request, requested on this ViroMediaRecorder, that are scheduled
     to be taken on the next render frame.
     */
    private List<ScreenShotRunnable> mQueuedScreenShots;

    public ViroMediaRecorder(Context context, Renderer rendererJni, int width, int height) {
        mAppContext = context.getApplicationContext();
        mUIHandler = new Handler(Looper.getMainLooper());
        mViewportWidth = width;
        mViewportHeight = height;
        mQueuedScreenShots = new ArrayList<ScreenShotRunnable>();
        mVideoRecordingErrorDelegate = null;
        mNativeRecorderRef = nativeCreateNativeRecorder(rendererJni.mNativeRef);
    }

    public void destroy() {
        mIsPendingDestory = true;

        if (mIsRecording) {
            stopRecordingAsync(null);
        } else {
            nativeDeleteNativeRecorder(mNativeRecorderRef);
        }
    }

    public static boolean hasAudioAndRecordingPermissions(Context context) {
        boolean hasRecordPermissions = ContextCompat.checkSelfPermission(context,
                Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED;
        boolean hasExternalStoragePerm = ContextCompat.checkSelfPermission(context,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        return hasRecordPermissions && hasExternalStoragePerm;
    }

    public static boolean hasRecordingPermissions(Context context) {
        return ContextCompat.checkSelfPermission(context,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }

    /**
     * Schedules a screen recording to start on the next rendered frame and be saved with the
     * provided fileName. The screenshot is saved at a location determined by
     * ViroMediaRecorder.getMediaStorageDirectory(), and the success or failure of this request
     * are notified through the provided onErrorDelegate.
     */
    public void startRecordingAsync(String fileName, boolean saveToCameraRoll,
                                    ViroMediaRecorderDelegate onErrorDelegate) {
        // Create an empty callback so we don't have to nullcheck everywhere.
        if (onErrorDelegate == null) {
            onErrorDelegate = getEmptyCallback();
        }

        // If we are already recording, return with a ALREADY_RUNNING error.
        if (mIsRecording) {
            onErrorDelegate.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.ALREADY_RUNNING, null);
            return;
        }

        // Ensure the application has the right recording permissions
        if (!hasAudioAndRecordingPermissions(mAppContext)) {
            onErrorDelegate.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.NO_PERMISSIONS, null);
            return;
        }

        // Grab the absolute path at which to store our saved files.
        final String appDirPath = getMediaStorageDirectory(mAppContext, saveToCameraRoll);
        if (appDirPath == null) {
            onErrorDelegate.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
            return;
        }

        // Create destination file directories if needed.
        File appDir = new File(appDirPath);
        if (!appDir.exists() && !appDir.mkdir()) {
            onErrorDelegate.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
            return;
        }

        // Prepare the Android Media recorder.
        File outputFile = new File(appDir.getAbsolutePath() + "/" + fileName + ".mp4");
        if (!prepareAndroidMediaRecorder(mViewportWidth, mViewportHeight,
                outputFile.getAbsolutePath(), onErrorDelegate)) {
            return;
        }

        // Start recording
        mVideoRecordingFilename = outputFile.getAbsolutePath();
        mVideoRecordingErrorDelegate = onErrorDelegate;
        nativeEnableFrameRecording(mNativeRecorderRef, true);
    }

    private ViroMediaRecorderDelegate getEmptyCallback() {
        return new ViroMediaRecorderDelegate() {
            @Override
            public void onTaskCompleted(boolean success, ERROR errorCode, String fileName) {
                //No-op
            }
        };
    }

    /**
     * Grabs an absolute path to the directory within which to save all recorded /
     * snap-shotted files.
     *
     * @param context - Android application context needed to access the file system.
     * @param grabCameraRollDirectory - True if the saved image is to be persisted in the phone's Photo
     * gallery directory. Else the image is saved in the app's external directory.
     * @return The absolute path targeting the directory within which to record. Null is returned
     * if no such path was found.
     */
    protected static String getMediaStorageDirectory(Context context, boolean grabCameraRollDirectory) {
        ApplicationInfo appInfo = context.getApplicationInfo();
        CharSequence appLabel = context.getPackageManager().getApplicationLabel(appInfo);

        String appName;
        if (appLabel != null && appLabel.length() > 0) {
            appName = (String) appLabel;
        } else {
            appName = context.getPackageName();
        }

        String pathToPictures = null;
        if (grabCameraRollDirectory) {
            if (Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES) != null) {
                pathToPictures = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getAbsolutePath();
            } else if (Environment.getExternalStorageDirectory() != null) {
                // We were unable to get the external picture directory - fallback to the
                // external storage directory for saving large files.
                pathToPictures = Environment.getExternalStorageDirectory().getAbsolutePath();
            } else {
                Log.e("Viro","Unable to access the camera roll directory on this device!");
                return null;
            }
            pathToPictures = pathToPictures + "/" + appName;
        } else {
            // Use internal private storage.
            pathToPictures = context.getFilesDir().getAbsolutePath();
        }

        if (pathToPictures == null) {
            Log.e("Viro","Unable to access application's directory for storing media!");
            return null;
        }

        return pathToPictures;
    }

    private boolean prepareAndroidMediaRecorder(int width, int height,
                                                String fileName, ViroMediaRecorderDelegate completionCallback) {
        try {
            // Initialize the Android recorder settings
            initializeRecorder(width, height);

            // Set the directory for recording
            if (fileName == null || fileName.trim().isEmpty()) {
                completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
                return false;
            }
            mRecorder.setOutputFile(fileName);
            mRecorder.prepare();
        } catch (IOException e) {
            completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
            return false;
        } catch (Exception e) {
            completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.INITIALIZATION, null);
            return false;
        }

        mRecorder.setOnErrorListener(new MediaRecorder.OnErrorListener() {
            @Override
            public void onError(MediaRecorder mr, int what, int extra) {
                stopVideoRecordingAsyncWithError(ViroMediaRecorderDelegate.ERROR.UNKNOWN, mVideoRecordingErrorDelegate);
            }
        });

        // Finally Record
        mIsRecording = true;
        mRecorder.start();
        return true;
    }

    private void initializeRecorder(int width, int height) {
        // Note: Order of operations matter!
        mRecorder = new MediaRecorder();
        mRecorder.reset();
        mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
        mRecorder.setVideoSource(MediaRecorder.VideoSource.SURFACE);
        mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
        mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
        mRecorder.setVideoEncoder(MediaRecorder.VideoEncoder.H264);
        mRecorder.setAudioEncodingBitRate(16);
        mRecorder.setAudioSamplingRate(44100);
        mRecorder.setVideoEncodingBitRate(7000000);
        mRecorder.setVideoFrameRate(30);
        mRecorder.setVideoSize(width, height);
    }

    private synchronized void stopVideoRecordingAsyncWithError(final ViroMediaRecorderDelegate.ERROR error,
                                                               final ViroMediaRecorderDelegate completionCallback) {
        if (completionCallback == null) {
            return;
        }

        if (!mIsRecording || mPendingStopRecording.get()) {
            completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.ALREADY_STOPPED, null);
            return;
        }

        mPendingStopRecording.set(true);
        mStopVideoRecordingRunnable = new Runnable() {
            @Override
            public void run() {
                if (!mPendingStopRecording.get()) {
                    return;
                }

                boolean cleanupSucceded = true;
                try{
                    mRecorder.stop();
                    cleanupSucceded = mInputSurface.destroy();
                    mRecorder.release();
                } catch(RuntimeException ex) {
                    Log.e("Viro","Viro media recorder cleanup failed.");
                    cleanupSucceded = false;
                }

                if (cleanupSucceded) {
                    completionCallback.onTaskCompleted(true, error, mVideoRecordingFilename);
                } else {
                    completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.UNKNOWN, mVideoRecordingFilename);
                }

                mInputSurface = null;
                mRecorder = null;
                mVideoRecordingErrorDelegate = null;
                mVideoRecordingFilename = null;
                mIsRecording = false;
                mPendingStopRecording.set(false);

                if (mIsPendingDestory) {
                    nativeDeleteNativeRecorder(mNativeRecorderRef);
                }
            }
        };

        nativeEnableFrameRecording(mNativeRecorderRef, false);
    }

    /**
     * Schedules the conclusion of the current video recording, if any. The success or
     * failure of this request are notified through the provided completion callback.
     */
    public void stopRecordingAsync(ViroMediaRecorderDelegate completionCallback) {
        // Create an empty callback so we don't have to nullcheck everywhere.
        if (completionCallback == null) {
            completionCallback = getEmptyCallback();
        }

        if (!hasAudioAndRecordingPermissions(mAppContext)) {
            completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.NO_PERMISSIONS, null);
            return;
        }

        stopVideoRecordingAsyncWithError(ViroMediaRecorderDelegate.ERROR.NONE, completionCallback);
    }

    /**
     * Schedules a screenshot to be taken and saved with the provided fileName on the next
     * rendered frame. The screenshot is saved at a location determined by
     * ViroMediaRecorder.getMediaStorageDirectory(), and the success or failure
     * of this request are notified through the provided completion callback.
     */
    public void takeScreenShotAsync(String fileName, boolean saveToCameraRoll,
                                    ViroMediaRecorderDelegate completionCallback) {
        // Create an empty callback so we don't have to nullcheck everywhere.
        if (completionCallback == null) {
            completionCallback = getEmptyCallback();
        }

        // First, determine if we have the appropriate permissions and fail fast if not.
        if (!hasRecordingPermissions(mAppContext)) {
            completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.NO_PERMISSIONS, null);
            return;
        }

        // Setup the runnable to perform the screen capture
        try {
            synchronized (mQueuedScreenShotLock) {
                mQueuedScreenShots.add(new ScreenShotRunnable(mAppContext, mViewportWidth,
                        mViewportHeight, fileName, saveToCameraRoll, completionCallback));
            }
        } catch (Exception e) {
            completionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.INITIALIZATION, null);
            return;
        }

        // Finally schedule a screen capture on the next render pass.
        nativeScheduleScreenCapture(mNativeRecorderRef);
    }

    /*
     ScreenShotRunnable for persisting screenshot images on the device asynchronously.
     */
    private static class ScreenShotRunnable implements Runnable {
        final int mWidth;
        final int mHeight;
        final ByteBuffer mPixelBuf;
        final String mFileName;
        final Context mContext;
        final boolean mSaveToCameraRoll;
        boolean mRunnableSuccess;
        final ViroMediaRecorderDelegate mCompletionCallback;

        public ScreenShotRunnable(Context context,
                                  int width,
                                  int height,
                                  String name,
                                  boolean saveToCameraRoll,
                                  ViroMediaRecorderDelegate completionCallback) {
            mWidth = width;
            mHeight = height;
            mFileName = name;
            mPixelBuf = ByteBuffer.allocateDirect(mWidth * mHeight * 4);
            mPixelBuf.order(ByteOrder.LITTLE_ENDIAN);
            mContext = context;
            mCompletionCallback = completionCallback;
            mRunnableSuccess = false;
            mSaveToCameraRoll = saveToCameraRoll;
        }

        protected void grabScreenShot() {
            mPixelBuf.rewind();

            // Bind to the original egl display from which to read. For Android
            // this is the same as unbinding to let the egl surface bind to it.
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
            GLES20.glReadPixels(0, 0, mWidth, mHeight, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE,
                    mPixelBuf);
            int result = GLES20.glGetError();
            mRunnableSuccess = result == GLES20.GL_NO_ERROR;
            if (!mRunnableSuccess) {
                Log.e("Viro","GL Error when grabbing screen shot: " + result);
            }
        }

        protected void persistImageData() {
            // If we've failed to render the screen capture, return.
            if (!mRunnableSuccess) {
                mCompletionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.UNKNOWN, null);
                return;
            }

            final String appDirPath = getMediaStorageDirectory(mContext, mSaveToCameraRoll);
            if (appDirPath == null) {
                mCompletionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
                return;
            }

            File appDir = new File(appDirPath);
            if (!appDir.exists() && !appDir.mkdir()) {
                mCompletionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
                return;
            }

            BufferedOutputStream bos = null;
            File output = new File(appDir + "/" + mFileName+".png");
            try {
                bos = new BufferedOutputStream(new FileOutputStream(output.getAbsolutePath()));
                Bitmap bmp = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);
                mPixelBuf.rewind();
                reverseBuf(mPixelBuf, mWidth, mHeight);
                bmp.copyPixelsFromBuffer(mPixelBuf);
                bmp.compress(Bitmap.CompressFormat.PNG, 90, bos);
                bmp.recycle();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                mCompletionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
                return;
            } catch(OutOfMemoryError e) {
                e.printStackTrace();
                mCompletionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
                return;
            } catch (Exception e) {
                e.printStackTrace();
                mCompletionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.UNKNOWN, null);
                return;
            } finally {
                if (bos != null) try {
                    bos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    mCompletionCallback.onTaskCompleted(false, ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, null);
                    return;
                }
            }

            // Finally send intent to update the device's gallery
            if (mSaveToCameraRoll) {
                Intent mediaScanIntent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
                File f = new File(output.getAbsolutePath());
                Uri contentUri = Uri.fromFile(f);
                mediaScanIntent.setData(contentUri);
                mContext.sendBroadcast(mediaScanIntent);
            }

            mCompletionCallback.onTaskCompleted(true, ViroMediaRecorderDelegate.ERROR.NONE, output.getAbsolutePath());
        }

        private void reverseBuf(ByteBuffer buf, int width, int height) {
            int i = 0;
            byte[] tmp = new byte[width * 4];
            while (i++ < height / 2) {
                buf.get(tmp);
                System.arraycopy(buf.array(), buf.limit() - buf.position(), buf.array(), buf.position() - width * 4, width * 4);
                System.arraycopy(tmp, 0, buf.array(), buf.limit() - buf.position(), width * 4);
            }
            buf.rewind();
        }

        @Override
        public void run() {
            persistImageData();
        }
    }

    /*
     Called by the renderer to indicate that it has stopped swapping egl surfaces to the
     egl display as a part of the recording; it has stopped recording.
     */
    private void onNativeEnableFrameRecording(boolean isRecording) {
        if (mStopVideoRecordingRunnable != null && !isRecording) {
            mUIHandler.post(mStopVideoRecordingRunnable);
        }
    }

    /*
     Called by the renderer to bind to the eglsurface to push frames required for recording.
     */
    private void onNativeBindToEGLSurface() {
        if (mInputSurface == null) {
            mInputSurface = new MediaRecorderSurface(mRecorder.getSurface());

            if (!mInputSurface.eglSetup()) {
                stopVideoRecordingAsyncWithError(ViroMediaRecorderDelegate.ERROR.INITIALIZATION, mVideoRecordingErrorDelegate);
            }
        }

        if (!mInputSurface.makeCurrent()) {
            stopVideoRecordingAsyncWithError(ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }
    }

    /*
     Called by the renderer to unbind from the egl surface to restore the original rendering state,
     and is usually called after recording a frame.
     */
    private void onNativeUnBindEGLSurface() {
        if(!mInputSurface.restoreRenderState()) {
            stopVideoRecordingAsyncWithError(ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }
    }

    /*
     Called by the renderer to render a frame into the recorder.
     */
    private void onNativeSwapEGLSurface() {
        if (!mInputSurface.setPresentationTime(System.nanoTime())) {
            stopVideoRecordingAsyncWithError(ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }

        if (!mInputSurface.swapBuffers()) {
            stopVideoRecordingAsyncWithError(ViroMediaRecorderDelegate.ERROR.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }
    }

    /*
     Called by the renderer to inform ViroMediaRecorder to grab a screen shot from the latest
     rendered frame (thereby fuflling all pending queued screenshot pending request.
     */
    private void onNativeTakeScreenshot() {
        synchronized (mQueuedScreenShotLock) {
            if (mQueuedScreenShots.size() < 0) {
                return;
            }

            for (int i = 0; i < mQueuedScreenShots.size(); i++) {
                mQueuedScreenShots.get(i).grabScreenShot();
                AsyncTask.execute(mQueuedScreenShots.get(i));
            }

            mQueuedScreenShots.clear();
        }
    }

    /*
     Native calls to the renderer.
     */
    private native long nativeCreateNativeRecorder(long nativeRecorderRef);
    private native void nativeDeleteNativeRecorder(long nativeRecorderRef);
    private native void nativeEnableFrameRecording(long nativeRecorderRef, boolean enabled);
    private native void nativeScheduleScreenCapture(long nativeRecorderRef);
}

