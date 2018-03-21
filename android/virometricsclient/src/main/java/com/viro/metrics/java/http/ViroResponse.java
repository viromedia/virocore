package com.viro.metrics.java.http;

/**
 * Encapsulates an HTTP response.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public final class ViroResponse {

    ///// PROPERTIES /////

    public final int statusCode;
    public final String body;

    ///// PUBLIC CONSTRUCTORS /////

    public ViroResponse(int statusCode, String body) {
        this.statusCode = statusCode;
        this.body = body;
    }

    ///// PUBLIC METHODS /////

    public boolean isSuccess() {
        return isSuccessCode(statusCode);
    }

    ///// PRIVATE STATIC METHODS /////

    /**
     * Checks whether an HTTP status code indicates success.
     *
     * @param statusCode The HTTP status code.
     * @return {@code true} if the status code indicates success (2xx), otherwise {@code false}.
     */
    private static boolean isSuccessCode(int statusCode) {
        return (statusCode / 100 == 2);
    }

}