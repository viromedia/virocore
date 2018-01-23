/*
 * Copyright (c) 2017-present, Viro, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.virosample;

import android.content.Intent;
import android.os.Bundle;
import com.example.virosample.model.Product;
import com.viro.core.RendererStartListener;

/**
 * A ViroCore ProductARActivity that coordinates the placing of a Product last selected in the
 * {@link ProductSelectionActivity} in AR.
 */
public class ProductARActivity extends ViroActivity implements RendererStartListener {
    final public static String INTENT_PRODUCT_KEY = "product_key";
    private Product mSelectedProduct = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String key = intent.getStringExtra(INTENT_PRODUCT_KEY);
        ProductApplicationContext context = (ProductApplicationContext)getApplicationContext();
        mSelectedProduct = context.getProductDB().getProductByName(key);
    }

    @Override
    public void onRendererStart() {
        // Do stuff here.
    }
}
