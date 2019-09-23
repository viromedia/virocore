//
//  Copyright (c) 2017-present, ViroMedia, Inc.
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

package com.viro.core.internal;

import android.content.Context;

import com.viro.metrics.android.ViroAndroidKeenClientBuilder;
import com.viro.metrics.java.ViroKeenClient;
import com.viro.metrics.java.ViroKeenLogging;
import com.viro.metrics.java.ViroKeenProject;

public class MetricsClient {
    private static MetricsClient INSTANCE = null;

    private MetricsClient(Context context) {
        ViroKeenClient client = new ViroAndroidKeenClientBuilder(context).build();
        ViroKeenClient.initialize(client);
        ViroKeenProject project = new ViroKeenProject("5ab1966bc9e77c0001b45ba0", "715EDB702D9AD29A56E127F08864DBCED277F35D946AA4DD2D4BD712B2356CA9E2E17276E74A8B69E44A720D0F92AF3A0D81CAF61404ABA069EE794C3FBFC19F5D66CB32B192B0B43AEA6CA61CED029E564237DB7452DDDC6955103CFAC11320", null);
        client.setDefaultProject(project);
        // TODO remove before checking in
        ViroKeenLogging.enableLogging();
        client.setDebugMode(true);
    }
}
