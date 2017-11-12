package com.example.memoryleaktest;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

/**
 * Created by vadvani on 11/10/17.
 */

public class MainActivity extends Activity {
    private static final String TAG = MainActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_memory_leak_test);
        Button boxButton = (Button)findViewById(R.id.box_button);
        boxButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "boxTest");
                Log.i(TAG, "Start Activity box button.");
                startActivity(intent);
            }
        });

        Button sphereButton = (Button)findViewById(R.id.sphere_button);
        sphereButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "sphereTest");
                Log.i(TAG, "Start Activity sphere Test.");
                startActivity(intent);
            }
        });

        Button videoButton = (Button)findViewById(R.id.video_button);
        videoButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "videoTest");
                Log.i(TAG, "Start Activity video test.");
                startActivity(intent);
            }
        });

        Button objectButton = (Button)findViewById(R.id.object_button);
        objectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "objectTest");
                Log.i(TAG, "Start Activity object test.");
                startActivity(intent);
            }
        });
    }
}
