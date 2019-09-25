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

import android.app.Application;

import com.example.virosample.model.ProductManagerDB;

/**
 * Create a reference cache of ProductManagerDB in the application layer so as to be able to
 * access product data across our activities.
 */
public class ProductApplicationContext extends Application {
    private ProductManagerDB mProductDB;

    public ProductManagerDB getProductDB(){
        if (mProductDB == null){
            mProductDB = new ProductManagerDB();
        }

        return mProductDB;
    }
}
