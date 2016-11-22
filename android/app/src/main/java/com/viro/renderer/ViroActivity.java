/**
 * Copyright (c) 2015-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
package com.viro.renderer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class ViroActivity extends AppCompatActivity {
    private ViroGvrLayout mViroGvrLayout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mViroGvrLayout = new ViroGvrLayout(this, true);
        setContentView(mViroGvrLayout);
    }
}
