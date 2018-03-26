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
