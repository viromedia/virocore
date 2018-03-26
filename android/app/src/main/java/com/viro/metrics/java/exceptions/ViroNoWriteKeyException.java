package com.viro.metrics.java.exceptions;

/**
 * ViroNoWriteKeyException
 *
 * @author dkador
 * @since 1.0.1
 */
public class ViroNoWriteKeyException extends ViroKeenException {
    private static final long serialVersionUID = -8199471518510440670L;

    public ViroNoWriteKeyException() {
        super();
    }

    public ViroNoWriteKeyException(Throwable cause) {
        super(cause);
    }

    public ViroNoWriteKeyException(String message) {
        super(message);
    }

    public ViroNoWriteKeyException(String message, Throwable cause) {
        super(message, cause);
    }
}