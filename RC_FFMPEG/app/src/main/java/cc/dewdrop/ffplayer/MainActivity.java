package cc.dewdrop.ffplayer;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.WindowManager;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import cc.dewdrop.ffplayer.utils.FFUtils;

import cc.dewdrop.ffplayer.widget.FFVideoView;

public class MainActivity extends Activity  implements SensorEventListener {
    Activity this_act;
   static private FFVideoView mVideoView;
   public static boolean update=true;
    public static int updateTimeMsec=50;
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





     void exit(){
        Net.net_runing = false;
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
        }
        Telemetry.logThread_f=false;
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
        }
        Disk.close_();

        super.finish();
        System.exit(0);

    }




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



    static int PERMISSION_ALL = 1;
    static String[] PERMISSIONS = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.ACCESS_FINE_LOCATION
    };

    public static boolean hasPermissions(Context context, String... permissions) {
        if (context != null && permissions != null) {
            for (String permission : permissions) {
                if (ActivityCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
                    return false;
                }
            }
        }
        return true;
    }


public static void verifyPermissions(Activity activity){

    while(!hasPermissions(activity, PERMISSIONS)){
        ActivityCompat.requestPermissions(activity, PERMISSIONS, PERMISSION_ALL);
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
}

    void openSettings(){
        Intent myIntent = new Intent(this, Settings.class);
        this.startActivity(myIntent);
    }

    //open map
    void openMap(){
        DrawView.fpv.set(false);
        mVideoView.stopVideo();
        Intent myIntent = new Intent(this_act, Map.class);
        this_act.startActivity(myIntent);
    }
    public static String getIpAddress() {
        try {
            for (Enumeration en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = (NetworkInterface) en.nextElement();
                for (Enumeration enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = (InetAddress) enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()&&inetAddress instanceof Inet4Address) {
                        String ipAddress=inetAddress.getHostAddress().toString();
                        Log.e("IP address",""+ipAddress);
                        return ipAddress;
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e("Socket exception", ex.toString());
        }
        return null;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            //Log.i("KEY","DOWN "+Integer.toString(keyCode));
            command_bits_|=GIMBAL_MINUS;
            return true;
        }else
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            //Log.i("KEY","DOWN "+Integer.toString(keyCode));
            command_bits_|=GIMBAL_PLUS;
            return true;
        }

        if (keyCode == KeyEvent.KEYCODE_BACK) {
            // your code
            if (DrawView.screen==DrawView.viewMenu) {
                DrawView.screen=DrawView.viewMain;
                return true;
            }
        }


        return super.onKeyDown(keyCode, event);
    }

    public  float mScaleFactor = 1.0f;
    private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener {
        @Override
        public boolean onScale(ScaleGestureDetector scaleGestureDetector){
            mScaleFactor *= scaleGestureDetector.getScaleFactor();
            mScaleFactor = Math.max(1f, Math.min(mScaleFactor, 4));
           // Log.d("SCALE",Float.toString((mScaleFactor-1)*33.667f+1));
            if (DrawView.fpv.pressed()){
                byte zoom=(byte)((mScaleFactor-1)*33.667f+1);
                if (zoom!=Commander.fpv_zoom){
                    Commander.fpv_zoom=zoom;
                    Commander.fpv=true;
                }
            }

            return true;
        }
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        verifyPermissions(this);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
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
        mVideoView = findViewById(R.id.videoView);
        screenMetrics=get_screen_size_in_pixels();
        drawView = new DrawView(MainActivity.this);
        rl1.addView(drawView);

        DrawView.mScaleGestureDetector = new ScaleGestureDetector(this, new ScaleListener());
        hideSystemUI();


     //   drawView.setBackgroundColor(Color.rgb(255,255,255));

        Telemetry.logThread_f=true;
        Telemetry.startlogThread();
        Net.net_runing=true;

        // setWifiTetheringEnabled(true);
        Net.context=this;
        net=new Net(9876,1000);
        net.start();

        this_act=this;

        new Thread() {
            @Override
            public void run() {

                while(runMainUpdate) {
                    try {
                        sleep(updateTimeMsec);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    if (MainActivity.drawView != null ){//&& update) {
                        update=false;
                        MainActivity.drawView.postInvalidate();
                        if (DrawView.showMap.getStat()==3)
                            openMap();
                        if (DrawView.showSettings.getStat()==3)
                            openSettings();
                        if (DrawView.exitProg.getStat()==3)
                            exit();
                        if (DrawView.reboot.getStat()==3)
                            command_bits_|=REBOOT;
                        if (DrawView.shutdown.getStat()==3)
                            command_bits_|=SHUTDOWN;
                        if (DrawView.gps_on_off.getStat()==3){
                            if (DrawView.gps_on_off.pressed())
                                startService(new Intent(MainActivity.this, GPSservice.class));
                            else
                                stopService(new Intent(MainActivity.this, GPSservice.class));
                        }
                    }
                }
            }
        }.start();

    }



    static public void startVideo(){
        String adr=getIpAddress();

        String videoPath = "udp://"+adr+":5544";//
    //   if (video_started==false) {
       // String videoPath = Environment.getExternalStorageDirectory() + "/Movies/PERU.MP4";
        mVideoView.playVideo(videoPath);
    }
    static public void stopVideo(){

        mVideoView.stopVideo();

        //   }
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

            update=DrawView.control_type.pressed();

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
