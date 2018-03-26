package com.viro.metrics.java.http;

import java.net.Proxy;
import java.net.URL;

/**
 * Encapsulates an HTTP request.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public final class ViroRequest {

    ///// PROPERTIES /////

    public final URL url;
    public final String method;
    public final String authorization;
    public final ViroOutputSource body;
    public final Proxy proxy;

    ///// PUBLIC CONSTRUCTORS /////

    public ViroRequest(URL url, String method, String authorization, ViroOutputSource body) {
        this(url, method, authorization, body, null);
    }

    public ViroRequest(URL url, String method, String authorization, ViroOutputSource body, Proxy proxy) {
        this.url = url;
        this.method = method;
        this.authorization = authorization;
        this.body = body;
        this.proxy = proxy;
    }

}
