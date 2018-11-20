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
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Surface;

import com.viro.core.FrameListener;
import com.viro.core.ViroViewScene;
import com.viro.core.internal.font.FontFamily;
import com.viro.core.internal.font.SystemFontLoader;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Methods accessed by JNI to perform platform-dependent
 * functions. Primarily accessed via VROPlatformUtil.cpp.
 *
 * @hide
 */
public class PlatformUtil {

    // Avoids creating a new Runnable each time we dispatch a task to another thread.
    // After the task is run, this TaskRunnable is returned to the pool
    static class TaskRunnable implements Runnable {

        public int taskId;
        private WeakReference<List<TaskRunnable>> mPool;

        public TaskRunnable(List<TaskRunnable> pool) {
            mPool = new WeakReference<>(pool);
        }

        @Override
        public void run() {
            if (taskId == 0) {
                Log.w("Viro", "Invalid task ID [0] provided for asynchronous task!");
                return;
            }

            PlatformUtil.runTask(taskId);
            taskId = 0;

            List pool = mPool.get();
            if (pool != null) {
                synchronized (pool) {
                    pool.add(this);
                }
            }
        }
    };

    private static final String TAG = "Viro";
    private static String ASSET_URL_PREFIX = "file:///android_asset";

    private Context mContext;
    private RenderCommandQueue mRenderQueue;
    private AssetManager mAssetManager;
    private List<FrameListener> mFrameListeners;
    private Map<Integer, VideoSink> mVideoSinks = new HashMap();
    private Handler mApplicationHandler;
    private RandomString mRandomStringGenerator = new RandomString();
    private ExecutorService mExecutorService = Executors.newCachedThreadPool();
    private List<TaskRunnable> mTaskRunnablePool = new ArrayList<>();

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

    public void dispose() {

        if (mVideoSinks != null && mFrameListeners != null) {
            // Ensure that all referenced video sinks that were created are cleaned up.
            Iterator it = mVideoSinks.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry pair = (Map.Entry) it.next();
                VideoSink videoSink = (VideoSink) pair.getValue();
                mFrameListeners.remove(videoSink);
                videoSink.releaseSurface();
                it.remove();
            }
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
        } catch(FileNotFoundException e) {
            Log.w(TAG, "File not found for bitmap at asset path [" + assetPath + "]");
            return null;
        } catch(IOException e) {
            Log.w(TAG, "IO error loading bitmap at asset path [" + assetPath + "]");
            return null;
        } catch(Exception e) {
            Log.w(TAG, "Unknown error loading bitmap at asset path [" + assetPath + "]", e);
            return null;
        } finally {
            if (in != null) {
                in.close();
            }
        }

        return bitmap;
    }

    // Accessed by Native code
    public void getBitmapPixels(Bitmap bitmap, ByteBuffer buffer) {
        IntBuffer intBuffer = buffer.order(ByteOrder.LITTLE_ENDIAN).asIntBuffer();
        int[] pixels = new int[bitmap.getWidth() * bitmap.getHeight()];
        bitmap.getPixels(pixels, 0, bitmap.getWidth(), 0, 0, bitmap.getWidth(), bitmap.getHeight());
        intBuffer.put(pixels);
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
        } catch(FileNotFoundException e) {
            Log.w(TAG, "File not found for bitmap at path [" + path + "]");
            return null;
        } catch(Exception e) {
            Log.w(TAG, "Unknown error loading bitmap at path [" + path + "]", e);
            return null;
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

    public Surface createVideoSink(int textureId, int width, int height) {
        VideoSink videoSink = new VideoSink(textureId, width, height);
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
            // If the asset begins with `./`, remove that. Assets can be in subdirectories but
            // same directory syntax does not work. `../` will not work either, but we have nothing
            // to correct that to, so leave it as is.
            if (asset.startsWith("./")) {
                asset = asset.substring(2);
            }

            in = mAssetManager.open(asset);
            if (file.getParentFile() != null) {
                file.getParentFile().mkdirs();
            }
            out = new FileOutputStream(file);

            transfer(in, out);
        } catch (Exception e) {
            Log.e("Viro", "Failed to copy asset [" + asset + "] to local file", e);
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

    public String getCacheDirectory() {
        return mContext.getCacheDir().getAbsolutePath();
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
    public void dispatchAsyncBackground(final int taskId) {
        mExecutorService.execute(getTaskRunnable(taskId));
    }

    /*
     * Run the the native function identified by the given task ID
     * asynchronously on the rendering thread.
     */
    public void dispatchRenderer(final int taskId) {
        if( mRenderQueue != null) {
            mRenderQueue.queueEvent(getTaskRunnable(taskId));
        }
    }

    /*
     * Run the the native function identified by the given task ID
     * asynchronously on the application UI thread.
     */
    public void dispatchApplication(final int taskId) {
        mApplicationHandler.post(getTaskRunnable(taskId));
    }

    private static native void runTask(int taskId);

    /**
     * Get a new {@link TaskRunnable} for executing the given task ID. This grabs a
     * {@link TaskRunnable} from the queue, so that we can reduce object creation churn.
     *
     * @param taskId The ID of the native task ro run.
     * @return The {@link TaskRunnable}.
     */
    private TaskRunnable getTaskRunnable(int taskId) {
        TaskRunnable task = null;
        synchronized (mTaskRunnablePool) {
            if (!mTaskRunnablePool.isEmpty()) {
                task = mTaskRunnablePool.remove(mTaskRunnablePool.size() - 1);
            }
        }
        if (task == null) {
            task = new TaskRunnable(mTaskRunnablePool);
        }
        task.taskId = taskId;
        return task;
    }

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

    /**
     * Helper function to convert the given RGBA data to a PNG image and persist it to
     * disk. Used to debug image output algorithms.
     *
     * @param buffer The buffer containing the RGBA data to save.
     * @param file Persist the file as a PNG at this path.
     */
    public void saveRGBAImageToFile(ByteBuffer buffer, int width, int height, String file) {
        File storage = mContext.getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        String path = new File(storage, file + ".png").getAbsolutePath();
        Log.i(TAG, "Saving test image [width " + width + ", height: " + height + "] to path " + path);

        BufferedOutputStream bos = null;
        Bitmap bitmap;
        try {
            ByteBuffer pixelBuffer = ByteBuffer.allocate(buffer.capacity());
            pixelBuffer.put(buffer);
            pixelBuffer.rewind();

            bos = new BufferedOutputStream(new FileOutputStream(path));
            bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            flipVertical(pixelBuffer, width, height);
            bitmap.copyPixelsFromBuffer(pixelBuffer);
            bitmap.compress(Bitmap.CompressFormat.PNG, 90, bos);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        } finally {
            if (bos != null) try {
                bos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    // Accessed by Native code (VROPlatformUtil.cpp)
    public Bitmap loadBitmapFromByteBuffer(ByteBuffer data, boolean rgb565) throws IOException {
        Bitmap bitmap = null;
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = rgb565 ? Bitmap.Config.RGB_565 : Bitmap.Config.ARGB_8888;
            ByteBuffer dataBuffer = ByteBuffer.allocate(data.capacity());
            dataBuffer.put(data);
            dataBuffer.rewind();
            bitmap = BitmapFactory.decodeByteArray(dataBuffer.array(), 0 , dataBuffer.capacity());
        } catch(Exception e) {
            Log.w(TAG, "Unknown error loading bitmap", e);
            return null;
        }

        return bitmap;
    }

    private void flipVertical(ByteBuffer buf, int width, int height) {
        int i = 0;
        byte[] tmp = new byte[width * 4];
        while (i++ < height / 2) {
            buf.get(tmp);
            System.arraycopy(buf.array(), buf.limit() - buf.position(), buf.array(), buf.position() - width * 4, width * 4);
            System.arraycopy(tmp, 0, buf.array(), buf.limit() - buf.position(), width * 4);
        }
        buf.rewind();
    }

}
