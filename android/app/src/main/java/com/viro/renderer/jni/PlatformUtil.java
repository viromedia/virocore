package com.viro.renderer.jni;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.opengl.GLSurfaceView;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Surface;

import com.viro.renderer.FrameListener;
import com.viro.renderer.VideoSink;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Methods accessed by JNI to perform platform-dependent
 * functions. Primarily accessed via VROPlatformUtil.cpp.
 */
public class PlatformUtil {

    private Context mContext;
    private GLSurfaceView mSurfaceView;
    private AssetManager mAssetManager;

    public PlatformUtil(GLSurfaceView view, Context context, AssetManager assetManager) {
        mContext = context;
        mAssetManager = assetManager;
        mSurfaceView = view;
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Bitmap loadBitmapFromAsset(String assetPath) throws IOException {
        InputStream in = null;
        Bitmap bitmap = null;
        try {
            in = mAssetManager.open(assetPath);
            bitmap = BitmapFactory.decodeStream(in);
        } finally {
            if (in != null) {
                in.close();
            }
        }

        return bitmap;
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Bitmap loadBitmapFromFile(String path) throws IOException {
        InputStream in = null;
        Bitmap bitmap = null;
        try {
            in = new FileInputStream(path);
            bitmap = BitmapFactory.decodeStream(in);
        } finally {
            if (in != null) {
                in.close();
            }
        }

        return bitmap;
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public int getAudioSampleRate() {
        AudioManager audioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        String nativeParam = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        return Integer.parseInt(nativeParam);
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public int getAudioBufferSize() {
        AudioManager audioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        String nativeParam = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        return Integer.parseInt(nativeParam);
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public String downloadURLToTempFile(String url) throws IOException {
        File file = File.createTempFile("Viro", "tmp");
        downloadURLSynchronous(url, file);

        return file.getAbsolutePath();
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public String copyAssetToFile(String asset) throws IOException {
        File file = File.createTempFile("Viro", ".tmp");

        InputStream in = null;
        FileOutputStream out = null;
        try {
            in = mAssetManager.open(asset);
            out = new FileOutputStream(file);

            transfer(in, out);
        } finally {
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
        }

        return file.getAbsolutePath();
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public void deleteFile(String path) throws IOException {
        File file = new File(path);
        file.delete();
    }

    private void downloadURLSynchronous(String myurl, File file) throws IOException {
        InputStream in = null;
        FileOutputStream out = null;

        try {
            URL url = new URL(myurl);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setReadTimeout(10000 /* milliseconds */);
            conn.setConnectTimeout(15000 /* milliseconds */);
            conn.setRequestMethod("GET");
            conn.setDoInput(true);
            // Starts the query
            conn.connect();
            int response = conn.getResponseCode();

            in = conn.getInputStream();
            out = new FileOutputStream(file);
            transfer(conn.getInputStream(), out);
        } finally {
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
        }
    }

    /*
     * Run the the native function identified by the given task ID
     * asynchronously. If backround is true, the task will be run in
     * a background thread. If background is false, it will be run on
     * the rendering thread.
     */
    public void dispatchAsync(final int taskId, boolean background) {
        if (background) {
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    runTask(taskId);
                }
            });
        }
        else {
            mSurfaceView.queueEvent(new Runnable() {
                @Override
                public void run() {
                    runTask(taskId);
                }
            });
        }
    }

    private native void runTask(int taskId);

    /**
     * Copies an {@link InputStream} to an {@link OutputStream}.
     *
     * @param is InputStream to read from. Will be left open after the transfer.
     * @param os OutputStream to write to. Left open after the transfer.
     * @return The number of bytes transferred.
     * @throws IOException If the copy process fails in any way.
     */
    public static long transfer(InputStream is, OutputStream os) throws IOException {
        byte[] copy_buffer = new byte[4096];
        long total = 0;

        int len;
        while ((len = is.read(copy_buffer)) > 0) {
            os.write(copy_buffer, 0, len);
            total += len;
        }

        return total;
    }

}
