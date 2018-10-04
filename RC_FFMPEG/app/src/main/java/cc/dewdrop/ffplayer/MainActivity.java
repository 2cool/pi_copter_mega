package cc.dewdrop.ffplayer;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;

import cc.dewdrop.ffplayer.utils.FFUtils;
import cc.dewdrop.ffplayer.widget.DrawView;
import cc.dewdrop.ffplayer.widget.FFVideoView;

public class MainActivity extends Activity  implements SensorEventListener {

    private FFVideoView mVideoView;

    RelativeLayout rl1;
    static DrawView drawView=null  ;

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







    private SensorManager mSensorManager;
    private Sensor accelerometer;
    static protected boolean sensorUpdateSpeedFastest=false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        mVideoView = findViewById(R.id.videoView);

        rl1 =findViewById(R.id.rl1);

        drawView = new DrawView(MainActivity.this);
        drawView.setBackgroundColor(Color.WHITE);
        rl1.addView(drawView);

        drawView.setBackgroundColor(00);

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        accelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);


        int speed=(sensorUpdateSpeedFastest)?SensorManager.SENSOR_DELAY_FASTEST:SensorManager.SENSOR_DELAY_NORMAL;
        mSensorManager.registerListener(this, accelerometer,speed );




    }





    public void onButtonClick(View view) {
        int id = view.getId();

        switch (id) {
            case R.id.button_protocol:

                break;
            case R.id.button_codec:

                break;
            case R.id.button_filter:

                break;
            case R.id.button_format:

                break;
            case R.id.button_play:
               // String videoPath = "udp://192.168.1.100:5544";//
                String videoPath =  Environment.getExternalStorageDirectory() + "/Movies/PERU.MP4";
                mVideoView.playVideo(videoPath);
                break;
        }
    }



    @Override
    public void onSensorChanged(SensorEvent event) {


        if (event.sensor.getType()==Sensor.TYPE_ACCELEROMETER) {
            if (MainActivity.drawView != null)
                MainActivity.drawView.postInvalidate();
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }
}
