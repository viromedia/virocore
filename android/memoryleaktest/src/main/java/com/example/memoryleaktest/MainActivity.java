package com.example.memoryleaktest;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import com.crashlytics.android.Crashlytics;
import io.fabric.sdk.android.Fabric;

/**
 * Created by vadvani on 11/10/17.
 */

public class MainActivity extends Activity {
    private static final String TAG = MainActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        Fabric.with(this, new Crashlytics());
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

        Button particleButton = (Button)findViewById(R.id.particle_button);
        particleButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "particleTest");
                Log.i(TAG, "Start Activity particle test.");
                startActivity(intent);
            }
        });

        Button stereoBgButton = (Button)findViewById(R.id.stereo_bg_button);
        stereoBgButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "bgStereoVideoTest");
                Log.i(TAG, "Start Activity Stereo Bg Test.");
                startActivity(intent);
            }
        });

        Button lightButton = (Button)findViewById(R.id.light_button);
        lightButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "lightTest");
                Log.i(TAG, "Start Activity light test.");
                startActivity(intent);
            }
        });

        Button eventButton = (Button)findViewById(R.id.event_button);
        eventButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "eventTest");
                Log.i(TAG, "Start Activity event test.");
                startActivity(intent);
            }
        });

        Button audioButton = (Button)findViewById(R.id.audio_button);
        audioButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "audioTest");
                Log.i(TAG, "Start Activity audio test.");
                startActivity(intent);
            }
        });

        Button imageButton = (Button)findViewById(R.id.arscene_button);
        imageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "arSceneTest");
                Log.i(TAG, "Start Activity AR Scene test.");
                startActivity(intent);
            }
        });

        Button animButton = (Button)findViewById(R.id.anim_button);
        animButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MemoryLeakTest.class);
                intent.putExtra("TestToRun", "animTest");
                Log.i(TAG, "Start Activity anim test.");
                startActivity(intent);
            }
        });

    }
}
