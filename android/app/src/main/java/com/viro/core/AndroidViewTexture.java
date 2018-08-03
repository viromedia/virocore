/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.view.ViewConfiguration;
import android.widget.FrameLayout;
import android.view.Surface;
import java.lang.ref.WeakReference;

/**
 * AndroidViewTexture renders an Android {@link View} onto a Viro {@link Texture}. This texture
 * can be applied onto any Viro {@link Geometry} to be rendered in the {@link Scene}. The width and
 * height of AndroidViewTexture define the bounds of the Android view: content outside these bounds
 * will be clipped.
 * <p>
 * By default, AndroidViewTexture will render Android Views without any hardware acceleration. This
 * ensures maximum compatibility with most Android OS versions while guaranteeing correct view
 * interaction behavior. However, there is a memory size limitation with this approach, resulting in
 * smaller viable width and height sizes for the texture. To check if your desired view size works
 * with software acceleration, use {@link AndroidViewTexture#supportsSoftwareSurfaceOfSize(Context,
 * int, int)}.
 * <p>
 * If specified, developers can also force the AndroidViewTexture to use hardware acceleration,
 * enabling larger texture sizes. Although most basic Android controls function properly with this
 * mode, it is important to note that some views may have unpredictable behavior, particularly when
 * handling user interaction.
 */
public class AndroidViewTexture extends Texture {
    private final static String TAG = AndroidViewTexture.class.getSimpleName();
    private Surface mViroRenderSurface;
    private AndroidViewSink mRenderableAndroidSink;
    private View mAttachedView;

    /**
     * Construct a new AndroidViewTexture that will render Android views into the
     * scene with the given pixel width and pixel height. By default, the
     * AndroidViewTexture will render Android Views onto its texture without
     * any hardware acceleration.
     * <p>
     * Once constructed, you can attach the Android View you wish to render by invoking
     * {@link #attachView(View)}.
     *
     * @param view     The {@link ViroView} containing your 3D scene.
     * @param pxWidth  The width of the texture in pixels.
     * @param pxHeight The height of the texture in pixels.
     */
    public AndroidViewTexture(ViroView view, int pxWidth, int pxHeight) {
        this(view, pxWidth, pxHeight, false);
    }

    /**
     * Construct a new AndroidViewTexture that will render Android views into the
     * scene with the given pixel width and height.
     * <p>
     * Once constructed, you can attach the Android View you wish to render by invoking
     * {@link #attachView(View)}.
     *
     * @param view                    The {@link ViroView} containing your 3D scene.
     * @param pxWidth                 The width of the texture in pixels.
     * @param pxHeight                The height of the texture in pixels.
     * @param useHardwareAcceleration True if this view should use hardware acceleration.
     */
    public AndroidViewTexture(ViroView view, int pxWidth, int pxHeight,
                              boolean useHardwareAcceleration) {
        ViroContext viroContext = view.getViroContext();
        mWidth = pxWidth;
        mHeight = pxHeight;
        mNativeRef = nativeCreateAndroidViewTexture(viroContext.mNativeRef, pxWidth, pxHeight);

        // Create an Android view sink with which to render on this texture.
        Context activityContext = view.getContext();
        mRenderableAndroidSink = new AndroidViewSink(activityContext, this, useHardwareAcceleration);

        // Configure the view sink as needed.
        FrameLayout.LayoutParams mRenderableParams = new FrameLayout.LayoutParams(pxWidth, pxHeight);
        mRenderableAndroidSink.setLayoutParams(mRenderableParams);
        mRenderableAndroidSink.setClipChildren(false);
        mRenderableAndroidSink.setClipToOutline(false);
        mRenderableAndroidSink.setClipToPadding(false);

        // Finally add our Android Sink to the render's layout.
        view.addView(mRenderableAndroidSink);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            detachView();
            // Release native resources associated with this VideoTexture.
            if (mNativeRef != 0) {
                nativeDeleteAndroidViewTexture(mNativeRef);
                mNativeRef = 0;
            }
        } finally {
            super.finalize();
        }
    }

    /**
     * Attach the Android {@link View} to be rendered with this AndroidViewTexture. This will detach
     * any currently attached View.
     *
     * @param view The Android View to be rendered with this AndroidViewTexture.
     */
    public void attachView(View view) {
        if (mAttachedView != null) {
            detachView();
        }

        mAttachedView = view;
        mRenderableAndroidSink.attachView(view);
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                mRenderableAndroidSink.requestLayout();
                mRenderableAndroidSink.invalidate();
            }
        });
    }

    /**
     * Detaches the Android {@link View} that was last attached via {@link
     * AndroidViewTexture#attachView(View)}.
     */
    public void detachView() {
        mRenderableAndroidSink.detachView();
        mRenderableAndroidSink.requestLayout();
        mRenderableAndroidSink.invalidate();
        mAttachedView = null;
    }

    /**
     * Triggers a Viro {@link ClickState} event at the given pixel coordinates in the currently
     * attached Android {@link View}. This should only be used for cases where you would like to map
     * ClickState interactivity for non-quad geometric 3D models with an applied AndroidViewTexture.
     * <p>
     * For quad objects, use {@link AndroidViewTexture#getClickListenerWithQuad(Quad)} instead.
     *
     * @param state  ClickState to be applied onto the view.
     * @param pixelX The X coordinate in pixels at which to apply the click event.
     * @param pixelY The Y coordinate in pixels at which to apply the click event.
     */
    public void dispatchClickStateEvent(ClickState state, float pixelX, float pixelY) {
        mRenderableAndroidSink.dispatchClickStateEvent(state, pixelX, pixelY);
    }

    /**
     * Constructs and returns a {@link ClickListener} that maps Viro clicks performed on the given
     * {@link Quad} to Android touch events, which will be automatically applied to the underlying
     * attached Android {@link View}. This takes into account the transformations that are applied
     * to the Quad, and as well as the size of the underlying texture.
     * <p>
     * To use the {@link ClickListener} returned by this method, you need to then attach it to the
     * Quad's parent Node, via {@link Node#setClickListener(ClickListener)}.
     *
     * @param quad The {@link Quad} from which click positions will originate.
     * @return The {@link ClickListener} connecting clicks between the underlying Android {@link
     * View} and the given Quad.
     */
    public ClickListener getClickListenerWithQuad(final Quad quad) {
        return new QuadClickListener(quad, this);
    }

    private static class QuadClickListener implements ClickListener {
        WeakReference<AndroidViewTexture> mTexture;
        WeakReference<Quad> mQuad;

        public QuadClickListener(Quad quad, AndroidViewTexture texture) {
            mQuad = new WeakReference<Quad>(quad);
            mTexture = new WeakReference<AndroidViewTexture>(texture);
        }

        @Override
        public void onClick(int source, Node node, Vector location) {
            // No-op
        }

        @Override
        public void onClickState(int source, Node node, ClickState clickState, Vector location) {
            Quad quad = mQuad.get();
            AndroidViewTexture texture = mTexture.get();
            if (quad == null || texture == null) {
                return;
            }

            // Localize the click location to be in reference with the given quad.
            Vector localLocation = node.convertWorldPositionToLocalSpace(location);

            // Convert from geometry space into texture space.
            float normalized_uv_y = 1.0f - ((localLocation.y / quad.getHeight()) + 0.5f);
            float normalized_uv_x = (localLocation.x / quad.getWidth()) + 0.5f;

            // Convert from texture space into Android View pixel space.
            float viewNorm_y = Math.abs(normalized_uv_y) * texture.getHeight();
            float viewNorm_x = Math.abs(normalized_uv_x) * texture.getWidth();

            // Finally dispatch the click event on the underlying android views.
            texture.dispatchClickStateEvent(clickState, viewNorm_x, viewNorm_y);
        }
    }

    /**
     * Determines if the current device supports rendering an Android layout in Software
     * Acceleration mode with the given width and height.
     *
     * @param context   The Android Context.
     * @param width     The desired width of your Android layout.
     * @param height    The desired height of your Android layout.
     * @return True if the provided dimensions are supported on the current device.
     */
    public static boolean supportsSoftwareSurfaceOfSize(Context context, int width, int height) {
        final long drawingCacheSize = ViewConfiguration.get(context).getScaledMaximumDrawingCacheSize();
        // Note: Android assumes a value of 4 bytes to represent a 32Bit drawing cache internally.
        final long projectedBitmapSize = width * height * 4;
        if (width > 0 && height > 0 && projectedBitmapSize > drawingCacheSize) {
            Log.w(TAG, "Viro: ViroAndroidTexture can not be displayed because it is"
                    + " too large to fit into a software layer (or drawing cache), needs "
                    + projectedBitmapSize + " bytes, only "
                    + drawingCacheSize + " available");
            return false;
        }
        return true;
    }

    // Called by the renderer
    void setVideoSink(android.view.Surface videoSink) {
        mViroRenderSurface = videoSink;
    }

    // Called by the renderer
    Surface getTextureRenderSurface() {
        return mViroRenderSurface;
    }

    /*
     Native Functions called into JNI
     */
    private native long nativeCreateAndroidViewTexture(long renderContextRef, int width, int height);
    private native void nativeDeleteAndroidViewTexture(long nativeTextureRef);
}