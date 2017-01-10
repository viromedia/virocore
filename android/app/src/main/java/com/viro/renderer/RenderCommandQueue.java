/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer;

public interface RenderCommandQueue {

    public void queueEvent(Runnable r);

}
