package com.example.igor.rc_temp;

import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.widget.RelativeLayout;

import java.lang.reflect.Method;

public class MainActivity extends AppCompatActivity {

    public static float [] screenMetrix;
    RelativeLayout rl1;
    static DrawView drawView=null  ;
    float[] get_screen_size_in_pixels(){
        float [] screenXY=new float[4];
        final DisplayMetrics metrics = new DisplayMetrics();
        Display display = getWindowManager().getDefaultDisplay();
        Method mGetRawH = null, mGetRawW = null;
        try {


            display.getRealMetrics(metrics);
            screenXY[0] = metrics.widthPixels;
            screenXY[1] = metrics.heightPixels;
            screenXY[2] = metrics.xdpi;
            screenXY[3] = metrics.ydpi;


        } catch (Exception e3) {
            e3.printStackTrace();
        }
        return screenXY;
    }


    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    private void hideSystemUI() {
        // Enables regular immersive mode.
        // For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
        // Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE
                        // Set the content to appear under the system bars so that the
                        // content doesn't resize when the system bars hide and show.
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);


        rl1 =findViewById(R.id.rl1);


        screenMetrix=get_screen_size_in_pixels();

        drawView = new DrawView(MainActivity.this);

        rl1.addView(drawView);

        drawView.setBackgroundColor(Color.BLACK);


    }
}
