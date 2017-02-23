/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public enum TextureFormat {
    RGBA8("RGBA8"),
    RGBA4("RGBA4"),
    RGB565("RGB565");

    private final String id;

    public static TextureFormat forString(String string) {
        for (TextureFormat format : TextureFormat.values()) {
            if (format.getID().equalsIgnoreCase(string)) {
                return format;
            }
        }
        throw new IllegalArgumentException("Invalid texture format [" + string + "]");
    }

    private TextureFormat(String id) {
        this.id = id;
    }

    public String getID() {
        return this.id;
    }

}
