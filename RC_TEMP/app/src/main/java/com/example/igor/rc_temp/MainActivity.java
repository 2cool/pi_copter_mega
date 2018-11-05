package com.example.igor.rc_temp;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.widget.RelativeLayout;

import java.lang.reflect.Method;

public class MainActivity extends AppCompatActivity implements SensorEventListener {



    public static int updateTimeMsec=20;
    public static double pitch,roll,heading_t;
    private static boolean runMainUpdate=true;
    public final static int MOTORS_ON=1, CONTROL_FALLING=2,Z_STAB=4,XY_STAB=8,GO2HOME=16,PROGRAM=32, COMPASS_ON=64,HORIZONT_ON=128;
    public final static int MPU_ACC_CALIBR=0x100, MPU_GYRO_CALIBR = 0x200, COMPASS_CALIBR=0x400, COMPASS_MOTOR_CALIBR=0x800, SHUTDOWN=0x1000, GIMBAL_PLUS=0x2000,GIMBAL_MINUS=0x4000,REBOOT=0x8000,SEC_MASK=0xFF000000;
    static public int control_bits=0;
    static public int command_bits_=0;
    private static boolean secure_flug=false;
    static boolean progF(){return (PROGRAM&control_bits)!=0;}
    static boolean toHomeF(){return (GO2HOME&control_bits)!=0;}
    static boolean motorsOnF(){return (MOTORS_ON&control_bits)!=0;}
    static boolean smartCntrF(){return (XY_STAB&control_bits)!=0;}
    static boolean altHoldF(){return (Z_STAB&control_bits)!=0;}

    //private static boolean game_speed=false;
    private SensorManager mSensorManager;
    private Sensor accelerometer;
    private Sensor gyroscop;
    private Sensor magnetic_field;
    static protected boolean sensorUpdateSpeedFastest=false;
    Net net=null;

    public static float [] screenMetrics;
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

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        accelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        magnetic_field=mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
        gyroscop=mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);

        int speed=(sensorUpdateSpeedFastest)?SensorManager.SENSOR_DELAY_FASTEST:SensorManager.SENSOR_DELAY_NORMAL;
        mSensorManager.registerListener(this, accelerometer,speed );
        mSensorManager.registerListener(this, magnetic_field, speed);
        mSensorManager.registerListener(this, gyroscop, speed);




        rl1 =findViewById(R.id.rl1);
        screenMetrics=get_screen_size_in_pixels();
        drawView = new DrawView(MainActivity.this);
        rl1.addView(drawView);
        drawView.setBackgroundColor(Color.rgb(60,0,0));

        Telemetry.logThread_f=true;
        Telemetry.startlogThread();
        Net.net_runing=true;

        // setWifiTetheringEnabled(true);
        Net.context=this;
        net=new Net(9876,1000);
        net.start();



            new Thread() {
                @Override
                public void run() {

                    while(runMainUpdate) {
                        try {
                            sleep(updateTimeMsec);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        if (MainActivity.drawView != null)
                            MainActivity.drawView.postInvalidate();
                    }
                }
            }.start();

    }


    private double now=0,old_time=0,dt_update=0,old_time_acc=0;

    @Override
    public void onSensorChanged(SensorEvent event) {
        double k=Math.min(1,DrawView.maxAngle/35);
        if (event.sensor.getType()==Sensor.TYPE_GYROSCOPE){

            double dt=0.001*(now-old_time);
            old_time=now;
            pitch-=event.values[1]*dt;
            roll+=event.values[0]*dt;
            // Commander.yaw-=event.values[2]*dt;

            // Log.i("MATHr","roll="+(int)(Commander.roll*56.3)+", pitch="+(int)(Commander.pitch*57.3)+", yaw="+(int)(Commander.yaw*57.3));

            dt_update+=dt;
            if (dt_update>0.05) {
                dt_update=0;
                new Thread() {
                    @Override
                    public void run() {
                        if (MainActivity.drawView != null)
                            MainActivity.drawView.postInvalidate();
                    }
                }.start();
            }
        }

        if (event.sensor.getType()==Sensor.TYPE_ACCELEROMETER){


            double aRoll = k*Math.atan2(event.values[1], event.values[2])/ Math.PI * 180 ;
            double aPitch = k*Math.atan2(event.values[0] , Math.sqrt(event.values[1] * event.values[1] + event.values[2] * event.values[2]))/ Math.PI * 180 ;

         //   double dt=0.001*(now-old_time_acc);
            old_time_acc=now;
            double F=1;//Math.min(1,dt*0.3);
            pitch+=(aPitch-pitch)*F;
            roll+=(aRoll-roll)*F;





            //  Log.i("MATH","aroll="+(int)(aRoll*56.3)+", apitch="+(int)(aPitch*57.3));


          //  k=(float)(zoomN/0.69813170079773183076947630739545);
          //  Commander.ax+=((event.values[0]*k/9.8)-Commander.ax)*0.1;
         //   Commander.ay+=((event.values[1]*k/9.8)-Commander.ay)*0.1;

            //Log.d("SENhD", "Andr "+Double.toString(event.values[0]));

            //az=midZ.get(event.values[2]*k/10f);



        }

        if (event.sensor.getType()==Sensor.TYPE_ORIENTATION){
            heading_t = ((double)event.values[0]+90);

            if (heading_t>180)
                heading_t-=360;
          //  Commander.heading=(float)heading_t;
            //Log.d("SENhD", "Andr "+Double.toString(heading));
        }

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }
}
