package com.viro.metrics.java.http;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Interface to allow writing to an {@link OutputStream} - in particular the output stream
 * of an HTTP request - directly from a source. This avoids having to write into a buffer, pass the
 * buffer to the request handler, and then have the handler copy the buffer to the connection's
 * output stream.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public interface ViroOutputSource {

    /**
     * Writes data to the given {@link OutputStream}. This method is used to send the body
     * of an HTTP request.
     * <p>
     * Implementers of this method should NOT close {@code out}; the calling code will ensure
     * that it is closed.
     *
     * @param out The {@link OutputStream} to which the request body should be written.
     * @throws IOException Implementers may throw an IOException if an error is encountered
     *                     while writing.
     */
    void writeTo(OutputStream out) throws IOException;

}
