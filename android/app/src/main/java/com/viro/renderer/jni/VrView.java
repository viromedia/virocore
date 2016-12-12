/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public interface VrView {

    public RenderContextJni getRenderContextRef();

    public void setScene(SceneJni scene);

    public void setVrModeEnabled(boolean vrModeEnabled);

    public RendererJni getNativeRenderer();
}
