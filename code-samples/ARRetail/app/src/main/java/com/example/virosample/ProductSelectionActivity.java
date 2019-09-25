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

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

import com.example.virosample.model.Category;
import com.example.virosample.model.Product;

import java.util.Collections;
import java.util.List;

import static com.example.virosample.ProductARActivity.INTENT_PRODUCT_KEY;

/**
 * ProductSelectionActivity displays a list of categories and products. Customers can tap on a
 * product to display it in AR. Upon selection of a product we launch into {@link ProductARActivity}.
 */
public class ProductSelectionActivity extends Activity {
    private ProductAdapter mProductAdapter;
    private CategoryAdapter mCategoryAdapter;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.product_activity_main_layout);

        // Init the scroll view, containing a list of categories, on the top of the layout
        setupCategoryScrollView();

        // Init the product grid view below the category scroll view. This will contain all
        // the products pertaining to the selected cateogry.
        setupProductGridView();
    }

    @Override
    protected void onResume(){
        super.onResume();

        // Refresh the list views
        mCategoryAdapter.invalidate();
        mProductAdapter.invalidate();
    }

    private void setupCategoryScrollView(){
        // Bind known categories to the horizontal recycler view
        mCategoryAdapter = new CategoryAdapter(getApplicationContext());
        RecyclerView categoryView = (RecyclerView) findViewById(R.id.categorial_list_view);
        categoryView.setAdapter(mCategoryAdapter);

        LinearLayoutManager manager = new LinearLayoutManager(getApplicationContext(), LinearLayoutManager.HORIZONTAL, false);
        categoryView.setLayoutManager(manager);

        // If a cateogry is clicked, refresh the product grid view with the latest products.
        mCategoryAdapter.setOnItemClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Category selectedCategory = (Category) v.getTag();
                mProductAdapter.setSelectedCategory(selectedCategory);
            }
        });
    }

    private void setupProductGridView(){
        // Bind the product data to the gridview.
        mProductAdapter = new ProductAdapter(this);
        GridView gridview = (GridView) findViewById(R.id.product_grid_view);
        gridview.setAdapter(mProductAdapter);

        // If a product is selected, enter AR mode with the selected product.
        gridview.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                Product selectedProduct = (Product) mProductAdapter.getItem(position);
                Intent intent = new Intent(ProductSelectionActivity.this, ProductARActivity.class);
                intent.putExtra(INTENT_PRODUCT_KEY, selectedProduct.mName);
                startActivity(intent);
            }
        });
    }

    public class CategoryAdapter extends RecyclerView.Adapter<CategoryAdapter.MyViewHolder>{
        private View.OnClickListener mClickListener;
        private List<Category> mCatList = Collections.emptyList();
        private ProductApplicationContext mContext;
        private int mSelectedCategoryIndex = -1;

        public CategoryAdapter(Context context) {
            mContext = (ProductApplicationContext) context.getApplicationContext();
            mSelectedCategoryIndex = -1;
        }

        public void setOnItemClickListener(View.OnClickListener clickListener){
            mClickListener = clickListener;
        }

        @Override
        public CategoryAdapter.MyViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View itemView = LayoutInflater.from(
                    parent.getContext()).inflate(
                            R.layout.product_activity_category_layout, parent, false);
            return new MyViewHolder(itemView);
        }

        @Override
        public void onBindViewHolder(CategoryAdapter.MyViewHolder holder, final int position) {
            Category cat = mCatList.get(position);
            holder.imageView.setImageDrawable(ContextCompat.getDrawable(mContext, cat.mImage));
            holder.txtview.setText(cat.mName);
            holder.imageView.setOnClickListener(new View.OnClickListener() {
                @Override

                public void onClick(View v) {
                    Category cat = mCatList.get(position);
                    v.setTag(cat);
                    mClickListener.onClick(v);
                    mSelectedCategoryIndex = position;
                    mCategoryAdapter.invalidate();
                }
            });

            // Note: If cateogry is selected, show highlighted area
            if (mSelectedCategoryIndex == position){
                holder.highlightView.setVisibility(View.VISIBLE);
            } else {
                holder.highlightView.setVisibility(View.GONE);
            }
        }

        @Override
        public int getItemCount() {
            return mCatList.size();
        }

        public void invalidate(){
            // Grab category data from the db.
            mCatList = mContext.getProductDB().getCatlist();

            // Notify data set has changed.
            notifyDataSetChanged();
        }

        protected class MyViewHolder extends RecyclerView.ViewHolder {
            public final ImageView imageView;
            public final TextView txtview;
            public final ImageView highlightView;

            public MyViewHolder(View view) {
                super(view);
                imageView = (ImageView) view.findViewById(R.id.category_image);
                txtview = (TextView) view.findViewById(R.id.category_name);
                highlightView = (ImageView) view.findViewById(R.id.category_highlight);
            }
        }
    }

    private class ProductAdapter extends BaseAdapter {
        private ProductApplicationContext mAppContext;
        private List<Product> mProductList = Collections.emptyList();
        private Category mCurrentCat;

        public ProductAdapter(Context context) {
            mAppContext = (ProductApplicationContext)context.getApplicationContext();
        }

        public void setSelectedCategory(Category category){
            mCurrentCat = category;
            invalidate();
        }

        @Override
        public int getCount() {
            return mProductList.size();
        }

        @Override
        public Object getItem(int position) {
            return mProductList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null){
                convertView = LayoutInflater.from(
                        parent.getContext()).inflate(
                        R.layout.product_activity_item_layout, parent, false);

            }

            Product product = mProductList.get(position);
            ImageView image = (ImageView) convertView.findViewById(R.id.product_image);
            image.setImageDrawable(ContextCompat.getDrawable(mAppContext, product.mImage));

            return convertView;
        }

        public void invalidate(){
            if (mCurrentCat == null){
                mProductList.clear();
            } else {
                mProductList = mAppContext.getProductDB().getProductList(mCurrentCat);
            }

            notifyDataSetChanged();
        }
    }
}
