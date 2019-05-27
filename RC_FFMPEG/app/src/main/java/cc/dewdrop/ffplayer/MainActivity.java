package cc.dewdrop.ffplayer;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.support.v4.app.ActivityCompat;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.WindowManager;
import android.widget.RelativeLayout;

import java.lang.reflect.Method;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import cc.dewdrop.ffplayer.widget.FFVideoView;

public class MainActivity extends Activity  implements SensorEventListener {
    final static double RAD2GRAD = 57.29578;
    Activity this_act;
   static private FFVideoView mVideoView;
   public static boolean update=true;
    public static int updateTimeMsec=50;
    public static float pitch=0,roll=0,yaw=0;
    private static boolean runMainUpdate=true;
    public final static int MOTORS_ON=1, CONTROL_FALLING=2,Z_STAB=4,XY_STAB=8,GO2HOME=16,PROGRAM=32, COMPASS_ON=64,HORIZONT_ON=128;
    public final static int MPU_ACC_CALIBR=0x100, MPU_GYRO_CALIBR = 0x200, COMPASS_CALIBR=0x400,
            COMPASS_MOTOR_CALIBR=0x800, SHUTDOWN=0x1000, GIMBAL_PLUS=0x2000,GIMBAL_MINUS=0x4000,
            REBOOT=0x8000,PROGRAM_LOADED= 0x10000,SEC_MASK=0xFF000000;
    static public int control_bits=0;
    static public int command_bits_=0;
    private static boolean secure_flug=false;
    static boolean prog_is_loaded(){return (PROGRAM_LOADED&control_bits)!=0;}
    static boolean compassOnF(){return (COMPASS_ON&control_bits)!=0;}
    static boolean horizontOnF(){return (HORIZONT_ON&control_bits)!=0;}
    static boolean progF(){return (PROGRAM&control_bits)!=0;}
    static boolean toHomeF(){return (GO2HOME&control_bits)!=0;}
    static boolean motorsOnF(){return (MOTORS_ON&control_bits)!=0;}
    static boolean smartCntrF(){return (XY_STAB&control_bits)!=0;}
    static boolean altHoldF(){return (Z_STAB&control_bits)!=0;}
    static public double dt=1,old_time=0;
    //private static boolean game_speed=false;
    public static boolean gyroscopeWork=false, magnetometerWork =false;

    static protected boolean sensorUpdateSpeedFastest=true;
    Net net=null;

    public static float [] screenMetrics;
    RelativeLayout rl1;
    static DrawView drawView=null  ;




    public static void smartCtrl(){command_bits_|=XY_STAB; }
    public static void Prog(){command_bits_|=PROGRAM; }
    public static void toHome() {command_bits_|=GO2HOME;}
    public static void altHold(){command_bits_|=Z_STAB;}
    public static void horizonOn(){command_bits_|=HORIZONT_ON;}
    public static void compassOn(){command_bits_|=COMPASS_ON;}

    public static void compassOn(boolean f){
        if (f)
            command_bits_|=COMPASS_ON;
        else
            command_bits_&= (0xffffffff)^COMPASS_ON;
    }




    public static void camera_gimb_plus(){
        command_bits_|=GIMBAL_PLUS;
    }
    public static void camera_gimb_minus(){
        command_bits_|=GIMBAL_MINUS;
    }



    public static void start_stop(){
        command_bits_|=MOTORS_ON;
       // Commander.heading=(float)yaw;
       // Log.d("PWR","PWR");
    }


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
      //  DrawView.turn2MainScreen();
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
            openMap();
            return true;
        }
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            openSettings();
            return true;
        }
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            DrawView.turn2MainScreen();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    public  float mScaleFactor = 1.0f;
    private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener {
        @Override
        public boolean onScale(ScaleGestureDetector scaleGestureDetector){
            final double scd=(scaleGestureDetector.getScaleFactor()-1)/(1+((float)Commander.fpv_zoom/10));;
         //   Log.d("SCALE",Double.toString(m1)+" : "+Double.toString(scd));
            mScaleFactor *= (scd+1);
            mScaleFactor = Math.max(1f, Math.min(mScaleFactor, 4));
           // Log.d("SCALE",Float.toString((mScaleFactor-1)*33.667f+1));
            if (DrawView.fpv.is_pressed()){

                int zoom=(int)((mScaleFactor-1)*84.6f+1);
                if (zoom>101)
                    zoom=101;
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



      //  PackageManager packageManager = getPackageManager();
      //  gyroscopeWork = packageManager.hasSystemFeature(PackageManager.FEATURE_SENSOR_GYROSCOPE);
     //   magnetometerWork = packageManager.hasSystemFeature(PackageManager.FEATURE_SENSOR_COMPASS);;




        SensorManager mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        Sensor accelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        Sensor magnetic_field=mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
        Sensor magnetic_fi_=mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);


        Sensor gyroscope=mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);


        int speed=(sensorUpdateSpeedFastest)?SensorManager.SENSOR_DELAY_FASTEST:SensorManager.SENSOR_DELAY_NORMAL;
        mSensorManager.registerListener(this, accelerometer,speed );
        mSensorManager.registerListener(this, magnetic_field, speed);
        mSensorManager.registerListener(this, magnetic_fi_, speed);

        mSensorManager.registerListener(this, gyroscope, speed);
        rl1 =findViewById(R.id.rl1);
        mVideoView = findViewById(R.id.videoView);
        screenMetrics=get_screen_size_in_pixels();
        drawView = new DrawView(MainActivity.this);
        rl1.addView(drawView);

        DrawView.mScaleGestureDetector = new ScaleGestureDetector(this, new ScaleListener());
        hideSystemUI();


     //   drawView.setBackgroundColor(Color.rgb(255,255,255));
        Telemetry.init();
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

                        if (DrawView.showMap.getStat()==3)
                            openMap();
                        if (DrawView.showSettings.getStat()==3)
                            openSettings();
                        if (DrawView.exitProg.getStat()==3)
                            exit();
                        if (DrawView.comp_calibr.getStat()==3)
                            command_bits_|=COMPASS_CALIBR;
                        if (DrawView.comp_m_calibr.getStat()==3)
                            command_bits_|=COMPASS_MOTOR_CALIBR;
                        if (DrawView.reboot.getStat()==3)
                            command_bits_|=REBOOT;
                        if (DrawView.shutdown.getStat()==3)
                            command_bits_|=SHUTDOWN;
                        if (DrawView.gps_on_off.getStat()==3){
                            if (DrawView.gps_on_off.is_pressed())
                                startService(new Intent(MainActivity.this, GPSservice.class));
                            else
                                stopService(new Intent(MainActivity.this, GPSservice.class));
                        }


                        MainActivity.drawView.postInvalidate();

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
    float easing = 0.01F;

    float azimuth;
    float pitch_;
    float roll_;
    float[] gravity = new float[3];
    float[] geomagnetic = new float[3];
    float[] I = new float[16];
    float[] R_ = new float[16];
    float[] orientation = new float[3];
void arrayCopy(float s[],float d[]){
    d[0]=s[0];
    d[1]=s[1];
    d[2]=s[2];
}


double angK=0.3;
    @Override
    public void onSensorChanged(SensorEvent event) {
        final int type=event.sensor.getType();
        if (event.accuracy == SensorManager.SENSOR_STATUS_ACCURACY_LOW) return;

        if (type==Sensor.TYPE_GYROSCOPE){
            gyroscopeWork=true;
            long now=System.currentTimeMillis();
            if (old_time==0) {
                old_time = now;
                return;
            }
            dt=0.001*(now-old_time);
            old_time=now;
            pitch-=angK*0.5*RAD2GRAD*event.values[1]*dt;
            roll+=angK*0.5*RAD2GRAD*event.values[0]*dt;
            yaw+=0.5*RAD2GRAD*event.values[2]*dt;
            // Commander.yaw-=event.values[2]*dt;
             //Log.i("MATHr","roll="+(roll)+", pitch="+(pitch));
           // Log.i("GYRO","gyro_work "+Double.toString(f));
        }
        else
        if (type==Sensor.TYPE_ACCELEROMETER){

            double aRoll = angK*Math.atan2(event.values[1], event.values[2]) * RAD2GRAD ;
            double aPitch = angK*Math.atan2(event.values[0] , Math.sqrt(event.values[1] * event.values[1] + event.values[2] * event.values[2])) * RAD2GRAD ;
            double f=gyroscopeWork?Math.max(0.003,Math.min(0.03,dt*0.5)):0.03;
            pitch += (aPitch - pitch) * f;
            roll += (aRoll - roll) * f;
            //update=DrawView.control_type_acc.is_pressed();
            arrayCopy(event.values, gravity);

        }
        else
        if (type==Sensor.TYPE_ORIENTATION){
             magnetometerWork=true;
            double f=gyroscopeWork?1:0.1;
            double t_yaw=(DrawView.wrap_180((double)event.values[0]+90));
            if (t_yaw>90 && yaw<-90)
                yaw+=360;
            else if (t_yaw<-90 && yaw>90)
                yaw-=360;
            yaw += (t_yaw - yaw )*f;
            //  Commander.heading=(float)heading_t;
           // Log.d("SENhD", Double.toString(yaw));
        }else if (type==Sensor.TYPE_MAGNETIC_FIELD){
            arrayCopy(event.values, geomagnetic);
        }
        if (SensorManager.getRotationMatrix(R_, I, gravity, geomagnetic)) {
            SensorManager.getOrientation(R_, orientation);
            azimuth += easing * (orientation[0] - azimuth);
            pitch_ += easing * (orientation[1] - pitch_);
            roll_ += easing * (orientation[2] - roll_);
          //  Log.d("DKDKDKD"," "+(int)(pitch_*RAD2GRAD)+" "+(int)(roll_*RAD2GRAD)+" "+(int)((azimuth*RAD2GRAD)-yaw));
        }

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }
}
