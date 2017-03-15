/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.keys;

/**
 * Listener with callback for response from Dyanmo
 */
public interface KeyValidationListener {

    void onResponse(boolean success);
}
