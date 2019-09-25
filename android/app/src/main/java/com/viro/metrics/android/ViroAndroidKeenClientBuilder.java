//
//  Copyright (c) 2018-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.metrics.android;

import android.content.Context;

import com.viro.metrics.java.ViroFileEventStoreViro;
import com.viro.metrics.java.ViroKeenClient;
import com.viro.metrics.java.ViroKeenEventStore;
import com.viro.metrics.java.ViroKeenJsonHandler;
import com.viro.metrics.java.ViroKeenNetworkStatusHandler;

/**
 * {@link ViroKeenClient.Builder} with defaults suited for use on the Android
 * platform.
 * <p>
 * This client uses the built-in Android JSON libraries for reading/writing JSON in order to
 * minimize library size. For applications which already include a more robust JSON library such
 * as Jackson or GSON, configure the builder to use an appropriate {@link ViroKeenJsonHandler} via
 * the {@link #withJsonHandler(ViroKeenJsonHandler)} method.
 * </p>
 * <p>
 * To cache events in between batch uploads, this client uses a file-based event store with its
 * root in the application's cache directory. It is important to use a file-based (or
 * otherwise persistent, i.e. non-RAM) event store because the application process could be
 * destroyed without notice.
 * </p>
 * <p>
 * Other defaults are those provided by the parent {@link ViroKeenClient.Builder}
 * implementation.
 * </p>
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public class ViroAndroidKeenClientBuilder extends ViroKeenClient.Builder {

    private final Context context;

    public ViroAndroidKeenClientBuilder(Context context) {
        this.context = context;
    }

    @Override
    protected ViroKeenJsonHandler getDefaultJsonHandler() {
        return new ViroAndroidJsonHandlerViro();
    }

    @Override
    protected ViroKeenEventStore getDefaultEventStore() throws Exception {
        return new ViroFileEventStoreViro(context.getCacheDir());
    }

    @Override
    protected ViroKeenNetworkStatusHandler getDefaultNetworkStatusHandler() {
        return new ViroAndroidNetworkStatusHandlerViro(context);
    }

}
