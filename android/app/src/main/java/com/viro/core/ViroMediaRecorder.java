/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
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
import android.net.rtp.AudioCodec;
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
 * ViroMediaRecorder enables you to take a screenshot or record a video of the scene rendered in
 * your {@link ViroView}.
 */
public class ViroMediaRecorder {

    /**
     * Enum indicating what error (if any) was encountered during a recording operation.
     */
    public enum Error {
        /**
         * The recording was a success.
         */
        NONE(-1),
        /**
         * Recording failed for an unknown reason.
         */
        UNKNOWN(0),
        /**
         * Recording failed because we did not have {@link Manifest.permission#RECORD_AUDIO} or
         * {@link Manifest.permission#WRITE_EXTERNAL_STORAGE} permissions.
         */
        NO_PERMISSIONS(1),
        /**
         * The recording failed because of an unknown error initializing.
         */
        INITIALIZATION(2),
        /**
         * The recording failed because it could not write to the given file. This could mean
         * the directory to store the file didn't exist or a similar IO error.
         */
        WRITE_TO_FILE(3),
        /**
         * The recording failed to start because a previous recording was still running.
         */
        ALREADY_RUNNING(4),
        /**
         * The recording failed to stop because no recording was active.
         */
        ALREADY_STOPPED(5);

        private final int val;
        Error(int numVal) {
            val = numVal;
        }

        /**
         * @hide
         */
        public int toInt() {
            return val;
        }
    }

    /**
     * Listener for responding to errors when starting a recording, and for responding to errors
     * during a recording.
     */
    public interface RecordingErrorListener {

        /**
         * Callback that is triggered as a result of an error during recording.
         *
         * @param errorCode Error code.
         */
        void onRecordingFailed(Error errorCode);
    }

    /**
     * Listener for responding to screenshot capture success and errors.
     */
    public interface ScreenshotFinishListener {
        /**
         * Callback that is triggered when screenshot capture is successful. The
         * developer is responsible for managing the {@link Bitmap} (ie. calling
         * {@link Bitmap#recycle()} when necessary)
         *
         * @param bitmap The bitmap containing the captured screenshot.
         * @param filePath The absolute file path of the recorded / saved file.
         */
        void onSuccess(Bitmap bitmap, String filePath);

        /**
         * Callback that is triggered when screenshot capture encounters an error.
         *
         * @param errorCode Error code.
         */
        void onError(Error errorCode);
    }

    /**
     * Listener for responding to video recording success and errors.
     */
    public interface VideoRecordingFinishListener {
        /**
         * Callback that is triggered when video recording is successful.
         *
         * @param filePath The absolute file path of the recorded / saved file.
         */
        void onSuccess(String filePath);

        /**
         * Callback that is triggered when video recording encounters an error.
         *
         * @param errorCode Error code.
         */
        void onError(Error errorCode);
    }

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
    private RecordingErrorListener mVideoRecordingErrorDelegate;

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

    private boolean mIsDestroyed = false;

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

    /*
     Default Audio recording settings.
     See https://developer.android.com/guide/topics/media/media-formats.html#audio-formats.
     */
    private int mAudioSamplingBitRate = 44100;
    private int mAudioEncodingBitRate = mAudioSamplingBitRate * 16;
    private int mAudioEncoder = MediaRecorder.AudioEncoder.AAC;

    ViroMediaRecorder(Context context, Renderer rendererJni, int width, int height) {
        mAppContext = context.getApplicationContext();
        mUIHandler = new Handler(Looper.getMainLooper());

        setWidth(width);
        setHeight(height);

        mQueuedScreenShots = new ArrayList<ScreenShotRunnable>();
        mVideoRecordingErrorDelegate = null;
        mNativeRecorderRef = nativeCreateNativeRecorder(rendererJni.mNativeRef);
    }

    void setWidth(int width) {
        // Ensure width is even, otherwise we can get a crash in stagefright when
        // it does conversions (SoftVideoEncoderOMXComponent::ConvertRGB32ToPlanar)
        mViewportWidth = (width % 2 == 0) ? width : width - 1;
    }

    void setHeight(int height) {
        // Ensure the height is even, otherwise we can get a crash in stagefright when
        // it does conversions (SoftVideoEncoderOMXComponent::ConvertRGB32ToPlanar)
        mViewportHeight = (height % 2 == 0) ? height : height - 1;
    }

    void dispose() {
        mIsPendingDestory = true;
        if (mIsRecording) {
            stopRecordingAsync(null);
        } else {
            deleteNativeRecorder();
        }
    }

    private void deleteNativeRecorder(){
        if (mIsDestroyed){
            return;
        }

        mIsDestroyed = true;
        nativeDeleteNativeRecorder(mNativeRecorderRef);
    }

    private static boolean hasAudioAndRecordingPermissions(Context context) {
        boolean hasRecordPermissions = ContextCompat.checkSelfPermission(context,
                Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED;
        boolean hasExternalStoragePerm = ContextCompat.checkSelfPermission(context,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        return hasRecordPermissions && hasExternalStoragePerm;
    }

    private static boolean hasRecordingPermissions(Context context) {
        return ContextCompat.checkSelfPermission(context,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
    }

    /**
     * Schedules a screen recording to start on the next rendered frame. The recording will continue
     * until {@link #stopRecordingAsync(VideoRecordingFinishListener)} is invoked. When finished,
     * the recording will be saved to the given file. The file will be stored in the device's photos
     * if <tt>saveToCameraRoll</tt> is true; otherwise it is stored in {@link Context#getFilesDir()}.
     *
     * @param fileName         The name of the file that will store the finished recording.
     * @param saveToCameraRoll True to save the recording to the camera roll. False to save the file
     *                         in {@link Context#getFilesDir()}.
     * @param errorListener   Listener that is invoked if the recording operation encounters an
     *                         error.
     */
    public void startRecordingAsync(String fileName, boolean saveToCameraRoll,
                                    RecordingErrorListener errorListener) {
        // Create an empty callback so we don't have to nullcheck everywhere.
        if (errorListener == null) {
            errorListener = getEmptyStartListener();
        }

        // If we are already recording, return with a ALREADY_RUNNING error.
        if (mIsRecording) {
            errorListener.onRecordingFailed(Error.ALREADY_RUNNING);
            return;
        }

        // Ensure the application has the right recording permissions
        if (!hasAudioAndRecordingPermissions(mAppContext)) {
            errorListener.onRecordingFailed(Error.NO_PERMISSIONS);
            return;
        }

        // Grab the absolute path at which to store our saved files.
        final String appDirPath = getMediaStorageDirectory(mAppContext, saveToCameraRoll);
        if (appDirPath == null) {
            errorListener.onRecordingFailed(Error.WRITE_TO_FILE);
            return;
        }

        // Create destination file directories if needed.
        File appDir = new File(appDirPath);
        if (!appDir.exists() && !appDir.mkdir()) {
            errorListener.onRecordingFailed(Error.WRITE_TO_FILE);
            return;
        }

        // Prepare the Android Media recorder.
        File outputFile = new File(appDir.getAbsolutePath() + "/" + fileName + ".mp4");
        if (!prepareAndroidMediaRecorder(mViewportWidth, mViewportHeight,
                outputFile.getAbsolutePath(), errorListener)) {
            return;
        }

        // Start recording
        mVideoRecordingFilename = outputFile.getAbsolutePath();
        mVideoRecordingErrorDelegate = errorListener;
        nativeEnableFrameRecording(mNativeRecorderRef, true);
    }

    private RecordingErrorListener getEmptyStartListener() {
        return new RecordingErrorListener() {
            @Override
            public void onRecordingFailed(Error errorCode) {

            }
        };
    }

    private VideoRecordingFinishListener getEmptyVideoRecordingFinishListener() {
        return new VideoRecordingFinishListener() {
            @Override
            public void onSuccess(String filePath) { /* no-op */ }
            @Override
            public void onError(Error errorCode) { /* no-op */ }
        };
    }

    private ScreenshotFinishListener getEmptyScreenshotFinishListener() {
        return new ScreenshotFinishListener() {
            @Override
            public void onSuccess(Bitmap bitmap, String filePath) { /* no-op */ }
            @Override
            public void onError(Error errorCode) { /* no-op */ }
        };
    }

    /**
     * Grabs an absolute path to the directory within which to save all recorded /
     * snap-shotted files.
     *
     * @param context Android application context needed to access the file system.
     * @param grabCameraRollDirectory True if the saved image is to be persisted in the phone's Photo
     * gallery directory. Else the image is saved in the app's external directory.
     * @return The absolute path targeting the directory within which to record. Null is returned
     * if no such path was found.
     */
    private static String getMediaStorageDirectory(Context context, boolean grabCameraRollDirectory) {
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
                                                String fileName, RecordingErrorListener completionCallback) {
        try {
            // Initialize the Android recorder settings
            initializeRecorder(width, height);

            // Set the directory for recording
            if (fileName == null || fileName.trim().isEmpty()) {
                completionCallback.onRecordingFailed(Error.WRITE_TO_FILE);
                return false;
            }
            mRecorder.setOutputFile(fileName);
            mRecorder.prepare();
        } catch (IOException e) {
            completionCallback.onRecordingFailed(Error.WRITE_TO_FILE);
            return false;
        } catch (Exception e) {
            completionCallback.onRecordingFailed(Error.INITIALIZATION);
            return false;
        }

        mRecorder.setOnErrorListener(new MediaRecorder.OnErrorListener() {
            @Override
            public void onError(MediaRecorder mr, int what, int extra) {
                stopVideoRecordingAsyncWithError(Error.UNKNOWN, mVideoRecordingErrorDelegate);
            }
        });

        // Finally Record
        mIsRecording = true;
        mRecorder.start();
        return true;
    }

    /**
     * Reconfigures the audio settings of the underlying Android MediaRecorder used by the
     * ViroMediaRecorder. This must be called before any recording for it to take effect.
     *
     * @param encodingBitRate Sets the audio encoding bit rate for recording.
     * @param samplingBitRate Sets the audio sampling rate for recording. The sampling rate is
     *                        ultimately determined by the format of the audio recording and
     *                        capabilities of the platform.
     * @param encoder         Sets the audio encoder to be used for recording.
     * @see android.media.MediaRecorder.AudioEncoder
     */
    public void configureAudioInput(int encodingBitRate, int samplingBitRate, int encoder) {
        mAudioEncodingBitRate = encodingBitRate;
        mAudioSamplingBitRate = samplingBitRate;
        mAudioEncoder = encoder;
    }

    private void initializeRecorder(int width, int height) {
        // Note: Order of operations matter!
        mRecorder = new MediaRecorder();
        mRecorder.reset();
        mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
        mRecorder.setVideoSource(MediaRecorder.VideoSource.SURFACE);
        mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
        mRecorder.setVideoEncoder(MediaRecorder.VideoEncoder.H264);
        mRecorder.setAudioEncoder(mAudioEncoder);
        mRecorder.setAudioEncodingBitRate(mAudioEncodingBitRate);
        mRecorder.setAudioSamplingRate(mAudioSamplingBitRate);
        mRecorder.setVideoEncodingBitRate(7000000);
        mRecorder.setVideoFrameRate(30);
        mRecorder.setVideoSize(width, height);
    }

    private boolean cleanup() {
        boolean cleanupSucceded = true;
        try{
            // Check if recording was already stopped before
            if (mIsRecording && mRecorder != null) {
                mRecorder.stop();
                cleanupSucceded = mInputSurface.destroy();
                mRecorder.release();
            }
        } catch(RuntimeException ex) {
            Log.e("Viro","Viro media recorder cleanup failed", ex);
            cleanupSucceded = false;
        }

        mInputSurface = null;
        mRecorder = null;
        mIsRecording = false;
        mPendingStopRecording.set(false);

        if (mIsPendingDestory) {
            deleteNativeRecorder();
        }

        return cleanupSucceded;
    }

    private synchronized void stopVideoRecordingAsyncWithError(final Error error,
                                                               final RecordingErrorListener completionCallback) {
        if (completionCallback == null) {
            return;
        }
        if (!mIsRecording || mPendingStopRecording.get()) {
            completionCallback.onRecordingFailed(Error.ALREADY_STOPPED);
            return;
        }

        mPendingStopRecording.set(true);
        mStopVideoRecordingRunnable = new Runnable() {
            @Override
            public void run() {
                if (!mPendingStopRecording.get()) {
                    return;
                }

                cleanup();
                completionCallback.onRecordingFailed(error);

                mVideoRecordingErrorDelegate = null;
                mVideoRecordingFilename = null;
            }
        };
        nativeEnableFrameRecording(mNativeRecorderRef, false);
    }

    private synchronized void stopVideoRecordingAsync(final VideoRecordingFinishListener completionCallback) {
        if (completionCallback == null) {
            return;
        }
        if (!mIsRecording || mPendingStopRecording.get()) {
            completionCallback.onError(Error.ALREADY_STOPPED);
            return;
        }

        mPendingStopRecording.set(true);
        mStopVideoRecordingRunnable = new Runnable() {
            @Override
            public void run() {
                if (!mPendingStopRecording.get()) {
                    return;
                }

                boolean cleanupSucceded = cleanup();
                if (cleanupSucceded) {
                    completionCallback.onSuccess(mVideoRecordingFilename);
                } else {
                    completionCallback.onError(Error.UNKNOWN);
                }

                mVideoRecordingErrorDelegate = null;
                mVideoRecordingFilename = null;
            }
        };
        nativeEnableFrameRecording(mNativeRecorderRef, false);
    }

    /**
     * Schedules the conclusion of the current video recording, if any. The success or
     * failure of this request are notified through the provided
     * {@link VideoRecordingFinishListener}.
     *
     * @param finishListener Callback interface that is invoked on success or failure.
     */
    public void stopRecordingAsync(VideoRecordingFinishListener finishListener) {
        // Create an empty callback so we don't have to nullcheck everywhere.
        if (finishListener == null) {
            finishListener = getEmptyVideoRecordingFinishListener();
        }

        if (!hasAudioAndRecordingPermissions(mAppContext)) {
            finishListener.onError(Error.NO_PERMISSIONS);
            return;
        }
        stopVideoRecordingAsync(finishListener);
    }

    /**
     * Schedules a screenshot to be taken and saved on the next rendered frame. When finished, the
     * screenshot will be saved to the given file. The file will be stored in the device's photos if
     * <tt>saveToCameraRoll</tt> is true; otherwise it is stored in {@link Context#getFilesDir()}.
     *
     * @param fileName         The name of the file that will store the screenshot.
     * @param saveToCameraRoll True to save the screenshot to the camera roll. False to save the
     *                         file in {@link Context#getFilesDir()}.
     * @param finishListener   Callback interface that is invoked on success or failure.
     */
    public void takeScreenShotAsync(String fileName, boolean saveToCameraRoll,
                                    ScreenshotFinishListener finishListener) {
        // Create an empty callback so we don't have to nullcheck everywhere.
        if (finishListener == null) {
            finishListener = getEmptyScreenshotFinishListener();
        }

        // First, determine if we have the appropriate permissions and fail fast if not.
        if (!hasRecordingPermissions(mAppContext)) {
            finishListener.onError(Error.NO_PERMISSIONS);
            return;
        }

        // Setup the runnable to perform the screen capture
        try {
            synchronized (mQueuedScreenShotLock) {
                mQueuedScreenShots.add(new ScreenShotRunnable(mAppContext, mViewportWidth,
                        mViewportHeight, fileName, saveToCameraRoll, finishListener));
            }
        } catch (Exception e) {
            finishListener.onError(Error.INITIALIZATION);
            return;
        }

        // Finally schedule a screen capture on the next render pass.
        nativeScheduleScreenCapture(mNativeRecorderRef);
    }

    /**
     * Schedules a screenshot to be taken and saved on the next rendered frame. When finished, a
     * bitmap representing the taken screenshot will be provided in the ScreenshotFinishListener
     * callback.
     *
     * @param finishListener Callback interface that is invoked on success or failure.
     */
    public void takeScreenShotAsync(ScreenshotFinishListener finishListener) {
        takeScreenShotAsync(null, false, finishListener);
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
        final ScreenshotFinishListener mCompletionCallback;

        public ScreenShotRunnable(Context context,
                                  int width,
                                  int height,
                                  String name,
                                  boolean saveToCameraRoll,
                                  ScreenshotFinishListener completionCallback) {
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

            GLES20.glGetError(); // Clear any existing error
            GLES20.glReadPixels(0, 0, mWidth, mHeight, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE,
                    mPixelBuf);
            int result = GLES20.glGetError();
            mRunnableSuccess = (result == GLES20.GL_NO_ERROR);
            if (!mRunnableSuccess) {
                Log.e("Viro","GL error when grabbing screen shot: " + result);
            }
        }

        private void persistImageData() {
            if (mFileName == null){
                persistImageBitmap();
            } else {
                persistImageDataFile();
            }
        }

        private void persistImageBitmap(){
            Bitmap bitmap = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);
            mPixelBuf.rewind();
            reverseBuf(mPixelBuf, mWidth, mHeight);
            bitmap.copyPixelsFromBuffer(mPixelBuf);
            mCompletionCallback.onSuccess(bitmap, null);
        }

        private void persistImageDataFile(){
            // If we've failed to render the screen capture, return.
            if (!mRunnableSuccess) {
                mCompletionCallback.onError(Error.UNKNOWN);
                return;
            }

            final String appDirPath = getMediaStorageDirectory(mContext, mSaveToCameraRoll);
            if (appDirPath == null) {
                mCompletionCallback.onError(Error.WRITE_TO_FILE);
                return;
            }

            File appDir = new File(appDirPath);
            if (!appDir.exists() && !appDir.mkdir()) {
                mCompletionCallback.onError(Error.WRITE_TO_FILE);
                return;
            }

            BufferedOutputStream bos = null;
            File output = new File(appDir + "/" + mFileName+".jpg");
            Bitmap bitmap;
            try {
                bos = new BufferedOutputStream(new FileOutputStream(output.getAbsolutePath()));
                bitmap = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);
                mPixelBuf.rewind();
                reverseBuf(mPixelBuf, mWidth, mHeight);
                bitmap.copyPixelsFromBuffer(mPixelBuf);
                bitmap.compress(Bitmap.CompressFormat.JPEG, 100, bos);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                mCompletionCallback.onError(Error.WRITE_TO_FILE);
                return;
            } catch(OutOfMemoryError e) {
                e.printStackTrace();
                mCompletionCallback.onError(Error.WRITE_TO_FILE);
                return;
            } catch (Exception e) {
                e.printStackTrace();
                mCompletionCallback.onError(Error.UNKNOWN);
                return;
            } finally {
                if (bos != null) try {
                    bos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    mCompletionCallback.onError(Error.WRITE_TO_FILE);
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

            mCompletionCallback.onSuccess(bitmap, output.getAbsolutePath());
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

    /**
     * Called by the renderer to indicate that it has stopped swapping egl surfaces to the
     * egl display as a part of the recording; it has stopped recording.
     * @hide
     */
    public void onNativeEnableFrameRecording(boolean isRecording) {
        if (mStopVideoRecordingRunnable != null && !isRecording) {
            mUIHandler.post(mStopVideoRecordingRunnable);
        }
    }

    /**
     * Called by the renderer to bind to the eglsurface to push frames required for recording.
     * @hide
     */
    public void onNativeBindToEGLSurface() {
        if (mInputSurface == null) {
            mInputSurface = new MediaRecorderSurface(mRecorder.getSurface());

            if (!mInputSurface.eglSetup()) {
                stopVideoRecordingAsyncWithError(Error.INITIALIZATION, mVideoRecordingErrorDelegate);
            }
        }

        if (!mInputSurface.makeCurrent()) {
            stopVideoRecordingAsyncWithError(Error.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }
    }

    /**
     * Called by the renderer to unbind from the egl surface to restore the original rendering state,
     * and is usually called after recording a frame.
     * @hide
     */
    public void onNativeUnbindEGLSurface() {
        if(!mInputSurface.restoreRenderState()) {
            stopVideoRecordingAsyncWithError(Error.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }
    }

    /**
     * Called by the renderer to render a frame into the recorder.
     * @hide
     */
    public void onNativeSwapEGLSurface() {
        if (!mInputSurface.setPresentationTime(System.nanoTime())) {
            stopVideoRecordingAsyncWithError(Error.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }

        if (!mInputSurface.swapBuffers()) {
            stopVideoRecordingAsyncWithError(Error.WRITE_TO_FILE, mVideoRecordingErrorDelegate);
        }
    }

    /**
     * Called by the renderer to inform ViroMediaRecorder to grab a screen shot from the latest
     * rendered frame (thereby fulfilling all pending queued screenshot requests).
     * @hide
     */
    public void onNativeTakeScreenshot() {
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
    private native long nativeCreateNativeRecorder(long sceneRendererRef);
    private native void nativeDeleteNativeRecorder(long nativeRecorderRef);
    private native void nativeEnableFrameRecording(long nativeRecorderRef, boolean enabled);
    private native void nativeScheduleScreenCapture(long nativeRecorderRef);
}

