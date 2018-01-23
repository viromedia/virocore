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
package com.example.virosample.model;

import com.example.virosample.R;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;

/**
 * A "statically mocked database list" containing a map of randomized categories to corresponding
 * product lists. These lists of products are randomly generated from a defaultProductList for
 * the sake of this demo.
 */
public class ProductManagerDB {
    // A list of all known categories
    private ArrayList<Category> mCatList = new ArrayList<Category>();

    // A Map of Categories to product lists.
    private HashMap<String, ArrayList<Product>> mProducts = new HashMap<String, ArrayList<Product>>();

    private static final String LAMP_WEBPAGE = "https://www.amazon.com/Simple-Designs-LT2007-PRP-Chrome-Fabric/dp/B00CM5SVU2/ref=sr_1_9?s=hi&ie=UTF8&qid=1515010381&sr=1-9";

    // A static set of all known products to be displayed in the application.
    private Product[] defaultProductList = new Product[]{
            new Product("Furniture", R.drawable.furniture_0,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_1,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_2,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_3,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_4,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_5,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_6,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_7,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_8,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_9,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_10,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_11,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_12,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_13,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_14,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_15,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
            new Product("Furniture", R.drawable.furniture_16,"file:///android_asset/object_lamp.vrx", LAMP_WEBPAGE),
    };

    public ProductManagerDB(){
        // Create a static list of products to be accessed through this "DB".
        createCateogryForProduct("Top Picks", R.drawable.furniture_17, createRandomizedProductList());
        createCateogryForProduct("Chairs", R.drawable.furniture_18, createRandomizedProductList());
        createCateogryForProduct("Sofas", R.drawable.furniture_19, createRandomizedProductList());
        createCateogryForProduct("Lamps", R.drawable.furniture_20, createRandomizedProductList());
        createCateogryForProduct("Beds", R.drawable.furniture_21, createRandomizedProductList());
    }

    private void createCateogryForProduct(String catName, int icon, ArrayList<Product> list){
        mCatList.add(new Category(catName, icon));
        mProducts.put(catName, list);
    }

    private ArrayList<Product> createRandomizedProductList(){
        Random r = new Random();
        ArrayList<Integer> addedList = new ArrayList<Integer>();
        ArrayList<Product> productList = new ArrayList<Product>();
        for (int i = 0; i < defaultProductList.length; i ++){
            int randProduct = r.nextInt(defaultProductList.length);

            while(addedList.contains(randProduct)){
                randProduct = r.nextInt(defaultProductList.length);
            }

            addedList.add(randProduct);
            productList.add(defaultProductList[randProduct]);
        }
        return productList;
    }

    public ArrayList<Category> getCatlist(){
        return mCatList;
    }

    public ArrayList<Product> getProductList(Category cat){
        return mProducts.get(cat.mName);
    }

    public Product getProductByName(String name){
        for (ArrayList<Product> list : mProducts.values()){
            for (Product product : list){
                if (product.mName.equalsIgnoreCase(name)){
                    return product;
                }
            }
        }
        return null;
    }
}
