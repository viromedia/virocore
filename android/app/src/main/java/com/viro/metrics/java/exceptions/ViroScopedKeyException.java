package com.viro.metrics.java.exceptions;

/**
 * ViroScopedKeyException
 *
 * @author dkador
 * @since 1.0.3
 */
public class ViroScopedKeyException extends ViroKeenException {
    private static final long serialVersionUID = -8250886829624436391L;

    public ViroScopedKeyException() {
        super();
    }

    public ViroScopedKeyException(Throwable cause) {
        super(cause);
    }

    public ViroScopedKeyException(String message) {
        super(message);
    }

    public ViroScopedKeyException(String message, Throwable cause) {
        super(message, cause);
    }
}
