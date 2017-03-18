package com.viro.renderer.jni;

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

import com.viro.renderer.FrameListener;
import com.viro.renderer.RenderCommandQueue;
import com.viro.renderer.VideoSink;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Methods accessed by JNI to perform platform-dependent
 * functions. Primarily accessed via VROPlatformUtil.cpp.
 */
public class PlatformUtil {

    private static final String TAG = "Viro";

    private Context mContext;
    private RenderCommandQueue mRenderQueue;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners;
    private Map<Integer, VideoSink> mVideoSinks = new HashMap();
    private Handler mApplicationHandler;

    public PlatformUtil(RenderCommandQueue queue, List<FrameListener> frameListeners,
                        Context context, AssetManager assetManager) {
        mContext = context;
        mFrameListeners = frameListeners;
        mAssetManager = assetManager;
        mRenderQueue = queue;
        mApplicationHandler = new Handler(Looper.getMainLooper());
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
    public static String downloadURLToTempFile(String url) throws IOException {
        File file = File.createTempFile("Viro", "tmp");
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
        File file = new File(mContext.getCacheDir(), asset);

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

    private static void downloadURLSynchronous(String myurl, File file) throws IOException {
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
     * asynchronously. If backround is true, the task will be run in
     * a background thread. If background is false, it will be run on
     * the rendering thread.
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
     * asynchronously on an the application UI thread.
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
