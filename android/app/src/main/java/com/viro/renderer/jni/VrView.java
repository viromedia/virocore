/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


import android.app.Application;
import android.view.View;

public interface VrView extends Application.ActivityLifecycleCallbacks {

    public RenderContextJni getRenderContextRef();

    public void setScene(SceneJni scene);

    public void setVrModeEnabled(boolean vrModeEnabled);

    public RendererJni getNativeRenderer();

    public View getContentView();

    public String getPlatform();

    public String getHeadset();

    public String getController();

    public void validateApiKey(String apiKey);

    public void destroy();

    public void setDebug(boolean debug);

    public void setDebugHUDEnabled(boolean enabled);

    public void recenterTracking();

}
