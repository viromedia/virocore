/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Surface;

import com.viro.core.internal.font.FontFamily;
import com.viro.core.internal.font.SystemFontLoader;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * Methods accessed by JNI to perform platform-dependent
 * functions. Primarily accessed via VROPlatformUtil.cpp.
 *
 * @hide
 */
public class PlatformUtil {

    private static final String TAG = "Viro";
    private static String ASSET_URL_PREFIX = "file:///android_asset";

    private Context mContext;
    private RenderCommandQueue mRenderQueue;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners;
    private Map<Integer, VideoSink> mVideoSinks = new HashMap();
    private Handler mApplicationHandler;
    private RandomString mRandomStringGenerator = new RandomString();

    public PlatformUtil(RenderCommandQueue queue, List<FrameListener> frameListeners,
                        Context context, AssetManager assetManager) {
        mContext = context;
        mFrameListeners = frameListeners;
        mAssetManager = assetManager;
        mRenderQueue = queue;
        mApplicationHandler = new Handler(Looper.getMainLooper());

        // Android devices store font configuration XML in /system/fonts/fonts.xml
        try {
            SystemFontLoader.init();
        } catch (Exception e) {
            Log.e(TAG, "Failed to load font configuration: all fonts will be system default", e);
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    private void dispose(){
        // Ensure that all referenced video sinks that were created are cleaned up.
        Iterator it = mVideoSinks.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry pair = (Map.Entry)it.next();
            VideoSink videoSink = (VideoSink) pair.getValue();
            mFrameListeners.remove(videoSink);
            videoSink.releaseSurface();
            it.remove();
        }

        mVideoSinks = null;
        mContext = null;
        mFrameListeners = null;
        mRenderQueue = null;
        mApplicationHandler = null;
    }

    /**
     * Set the {@link RenderCommandQueue} to a new value. All new
     * events will be processed on this queue, and its associated
     * thread.
     */
    public void setRenderCommandQueue(RenderCommandQueue queue) {
        mRenderQueue = queue;
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Bitmap loadBitmapFromAsset(String assetPath, boolean rgb565) throws IOException {
        InputStream in = null;
        Bitmap bitmap = null;
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = rgb565 ? Bitmap.Config.RGB_565 : Bitmap.Config.ARGB_8888;

            in = mAssetManager.open(assetPath);
            bitmap = BitmapFactory.decodeStream(in, null, options);
        } finally {
            if (in != null) {
                in.close();
            }
        }

        return bitmap;
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Bitmap loadBitmapFromFile(String path, boolean rgb565) throws IOException {
        InputStream in = null;
        Bitmap bitmap = null;
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = rgb565 ? Bitmap.Config.RGB_565 : Bitmap.Config.ARGB_8888;

            in = new FileInputStream(path);
            bitmap = BitmapFactory.decodeStream(in, null, options);
        } finally {
            if (in != null) {
                in.close();
            }
        }

        return bitmap;
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Surface createVideoSink(int textureId) {
        VideoSink videoSink = new VideoSink(textureId);
        mVideoSinks.put(textureId, videoSink);
        mFrameListeners.add(videoSink);

        return videoSink.getSurface();
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public void destroyVideoSink(int textureId) {
        VideoSink videoSink = mVideoSinks.remove(textureId);
        if (videoSink != null) {
            mFrameListeners.remove(videoSink);

            videoSink.releaseSurface();
        }
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

        // If the URL begins with file:///android_asset, then copy the asset
        // to file.
        if (url.startsWith(ASSET_URL_PREFIX)) {
            Log.i("Viro", "Copying asset at URL " + url);
            return copyAssetToFile(url.substring(ASSET_URL_PREFIX.length() + 1));
        }

        try {
            downloadURLSynchronous(url, file);
        }catch(UnknownHostException e) {
            Log.w(TAG, "Unknown host at URL [" + url + "], download failed");
            return null;
        }catch(FileNotFoundException e) {
            Log.w(TAG, "No file found at URL [" + url + "], not copying to temporary file");
            return null;
        }catch(IOException e) {
            Log.w(TAG, "IO error downloading file at URL [" + url + "], download failed");
            return null;
        }catch(Exception e) {
            Log.w(TAG, "Unknown error downloading file at URL [" + url + "]", e);
            return null;
        }

        return file.getAbsolutePath();
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    // Resource should be in the form res:/#######, where the id is in the path.
    public String copyResourceToFile(String resource) throws IOException {
        Uri uri = Uri.parse(resource);
        File file = new File(mContext.getCacheDir(),  uri.getPath());

        InputStream in = null;
        FileOutputStream out = null;

        // the path is /#######, we just want the #######
        writeResourceToFile(uri.getPath().substring(1), file);

        return file.getAbsolutePath();
    }

    public Map<String, String> copyResourceMap(Map<String, String> resourceMap) throws IOException {
        Map<String, String> toReturn = new HashMap<>();
        for (String key: resourceMap.keySet()) {
            File file = new File(mContext.getCacheDir(), key);
            toReturn.put(key, file.getAbsolutePath());

            Uri uri = Uri.parse(resourceMap.get(key));
            // the path is /#######, we just want the #######
            writeResourceToFile(uri.getPath().substring(1), file);
        }
        return toReturn;
    }

    public void writeResourceToFile(String resourceId, File outputFile) throws IOException {
        InputStream in = null;
        FileOutputStream out = null;

        try {
            int id = Integer.valueOf(resourceId);
            in = mContext.getResources().openRawResource(id);
            out = new FileOutputStream(outputFile);

            transfer(in, out);
        } finally {
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
        }
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public String copyAssetToFile(String asset) throws IOException {
        File file = new File(mContext.getCacheDir(), asset + "_" + mRandomStringGenerator.nextString());

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

    // Accessed by Native code
    public String findFontFile(String typeface, boolean isItalic, int weight) {
        FontFamily.Font font = SystemFontLoader.findFont(typeface, isItalic, weight);
        if (font == null) {
            return null;
        }
        return font.getPath();
    }

    // Accessed by Native code
    public int findFontIndex(String typeface, boolean isItalic, int weight) {
        FontFamily.Font font = SystemFontLoader.findFont(typeface, isItalic, weight);
        if (font == null) {
            return 0;
        }
        return font.getIndex();
    }

    private static void downloadURLSynchronous(String myurl, File file) throws IOException {
        InputStream in = null;
        FileOutputStream out = null;

        try {
            URL url = new URL(myurl);

            URLConnection conn = url.openConnection();
            conn.setReadTimeout(10000 /* milliseconds */);
            conn.setConnectTimeout(15000 /* milliseconds */);
            if (conn instanceof HttpURLConnection) {
                ((HttpURLConnection) conn).setRequestMethod("GET");

            }
            conn.setDoInput(true);

            // Starts the query
            conn.connect();
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
     * asynchronously on a background thread
     */
    public static void dispatchAsyncBackground(final int taskId) {
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                runTask(taskId);
            }
        });
    }

    /*
     * Run the the native function identified by the given task ID
     * asynchronously on the rendering thread.
     */
    public void dispatchRenderer(final int taskId) {
        mRenderQueue.queueEvent(new Runnable() {
                @Override
                public void run() {
                    runTask(taskId);
                }
        });
    }

    /*
     * Run the the native function identified by the given task ID
     * asynchronously on the application UI thread.
     */
    public void dispatchApplication(final int taskId) {
        Runnable myRunnable = new Runnable() {
            @Override
            public void run() {
                runTask(taskId);
            }
        };
        mApplicationHandler.post(myRunnable);
    }

    private static native void runTask(int taskId);

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
