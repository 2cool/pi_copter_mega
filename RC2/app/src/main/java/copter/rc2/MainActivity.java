package copter.rc2;



import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;

import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import android.widget.CheckBox;
import android.widget.RelativeLayout;
import android.widget.ToggleButton;

import java.lang.reflect.Method;
import java.util.Calendar;


//жпс координати збрасиваются при потере сигнала.
//и неотображается когда потеряна связь.

public class MainActivity extends Activity implements SensorEventListener {
    public final static int MOTORS_ON=1, CONTROL_FALLING=2,Z_STAB=4,XY_STAB=8,GO2HOME=16,PROGRAM=32, COMPASS_ON=64,HORIZONT_ON=128;
    public final static int MPU_ACC_CALIBR=0x100, MPU_GYRO_CALIBR = 0x200, COMPASS_CALIBR=0x400, COMPASS_MOTOR_CALIBR=0x800, SHUTDOWN=0x1000, GIMBAL_PLUS=0x2000,GIMBAL_MINUS=0x4000,REBOOT=0x8000,SEC_MASK=0xFF000000;



    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
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
                      //  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }






    static public Bitmap blank;

    static public int control_bits=0;
    static public int command_bits_=0;
    private static boolean secure_flug=false;

    public static double zoomN=0.17453292519943295769236907684886*2;//0.69813170079773183076947630739545;
    //private static boolean game_speed=false;
    private SensorManager mSensorManager;
    private Sensor accelerometer;
    private Sensor gyroscop;
    private Sensor magnetic_field;
    //float az;

    long acc_t=0,compas_t=0;

    double aa_i=0;
    long time_out=100;
    static boolean gpsON=false;
    static public DrawView drawView=null;
    Button b_start;
    static ToggleButton b_altHold,b_smartCTRL,b_toHome,b_prog;
    static Button b_menu;
    static public CheckBox ch_secure,cb_compass,cb_horizont;
    RelativeLayout rl1;

    boolean viewinit=false;
    Net net=null;

    static boolean progF(){return (PROGRAM&control_bits)!=0;}
    static boolean toHomeF(){return (GO2HOME&control_bits)!=0;}
    static boolean motorsOnF(){return (MOTORS_ON&control_bits)!=0;}
    static boolean smartCntrF(){return (XY_STAB&control_bits)!=0;}
    static boolean altHoldF(){return (Z_STAB&control_bits)!=0;}

    @Override
    protected void onDestroy() {
        super.onDestroy();




    }

/*


    private void setWifiTetheringEnabled(boolean enable) {
        WifiManager wifiManager = (WifiManager) getSystemService(WIFI_SERVICE);

        Method[] methods = wifiManager.getClass().getDeclaredMethods();
        for (Method method : methods) {
            if (method.getName().equals("setWifiApEnabled")) {
                try {
                    method.invoke(wifiManager, null, enable);
                } catch (Exception ex) {
                }
                break;
            }
        }
    }

*/









    static protected boolean sensorUpdateSpeedFastest=false;
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        blank= BitmapFactory.decodeStream(this.getResources().openRawResource(R.raw.blank));

        SharedPreferences settings = getPreferences(MODE_PRIVATE);
        sensorUpdateSpeedFastest=settings.getBoolean("sensorUpdateSpeedFastest", false);
        // static private Point screenP=new Point();
        // static public int zoom=1;
        // static public int type=3;

        DrawMap.zoom=settings.getInt("zoom",3);
        DrawMap.screenP.x=settings.getInt("screenPX",0);
        DrawMap.screenP.y=settings.getInt("screenPY",0);

        Telemetry.logThread_f=true;
        Telemetry.startlogThread();
        Net.net_runing=true;

        // setWifiTetheringEnabled(true);
        Net.context=this;
        net=new Net(9876,1000);

        //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        setContentView(R.layout.activity_main);

        b_altHold=(ToggleButton)findViewById(R.id.b_altHold);
        b_smartCTRL=(ToggleButton)findViewById(R.id.t_smartK);
        b_toHome=(ToggleButton)findViewById(R.id.b_toHome);
        b_menu= (Button)findViewById(R.id.b_menu);
        b_start=(Button)findViewById(R.id.b_start);
        b_prog= (ToggleButton)findViewById(R.id.b_Prog);

        rl1 =(RelativeLayout)findViewById(R.id.rl1);

        ch_secure=(CheckBox)findViewById(R.id.cb_secure);
        cb_compass=(CheckBox)findViewById(R.id.cb_compass);
        cb_horizont=(CheckBox)findViewById(R.id.cb_horizont);

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        accelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        magnetic_field=mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
        gyroscop=mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);


        int speed=(sensorUpdateSpeedFastest)?SensorManager.SENSOR_DELAY_FASTEST:SensorManager.SENSOR_DELAY_NORMAL;
        mSensorManager.registerListener(this, accelerometer,speed );
        mSensorManager.registerListener(this, magnetic_field, speed);
        mSensorManager.registerListener(this, gyroscop, speed);


        net.start();

        drawView = new DrawView(MainActivity.this);
        drawView.setBackgroundColor(Color.WHITE);
        rl1.addView(drawView);


        // startService(new Intent(MainActivity.this, GPSservice.class));

    }
    static boolean commands_off_full_th =false;


    //
    //settings
    static private MenuItem compass_cal=null;
    static private MenuItem compassMOT_cal=null;
    static private MenuItem gyroCalibr=null;
    //gps
    static private MenuItem exit=null;
    static private MenuItem shutdown=null;
    static private MenuItem reboot=null;
    static private MenuItem speed_r=null;
    static private MenuItem map=null;








    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);




        compass_cal         =menu.getItem(1);
        compassMOT_cal  =menu.getItem( 2);
        //gps
        gyroCalibr            =menu.getItem(4);
        exit                =menu.getItem(5);
        shutdown               =menu.getItem(6);
        reboot             =menu.getItem(7);
        speed_r         =menu.getItem(8);
        map             =menu.getItem(9);


        speed_r.setTitle((sensorUpdateSpeedFastest)?"FASTEST":"NORMAL");
        return true;
    }
    @Override
    public boolean onPrepareOptionsMenu (Menu menu){

        boolean link=Commander.link;
        boolean secure=Telemetry.realThrottle==0;

        compass_cal.setEnabled(secure&link);
        compassMOT_cal.setEnabled(secure&link);
        gyroCalibr.setEnabled(secure&link);
        exit.setEnabled(true);
        reboot.setEnabled(secure&link);
        shutdown.setEnabled(secure&link);

        DrawView.menu=true;
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public void onOptionsMenuClosed(Menu menu)
    {
        DrawView.menu=false;
        super.onOptionsMenuClosed(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        switch(id) {

            case R.id.SETTINGS: {

                Intent myIntent = new Intent(this, Settings.class);
                this.startActivity(myIntent);
                return true;

            }

          /*  case R.id.SET_HOR:
                command_bits_|=MPU_ACC_CALIBR;
               // Commander.button= "HOR";
                break;*/
            case R.id.EXIT: {
                Net.net_runing = false;
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException ex) {
                    Thread.currentThread().interrupt();
                }
                Telemetry.logThread_f=false;
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException ex) {
                    Thread.currentThread().interrupt();
                }
                Disk.close_();

                super.finish();
                System.exit(0);
                break;
            }
            case R.id.SHUTDOWN:{
                command_bits_|=SHUTDOWN;
                // Commander.button = "SHD";
                //  Commander.exit_main=true;

                break;
            }
            case R.id.REBOOT:{
                command_bits_|=REBOOT;
                // Commander.button = "SHD";
                // Commander.exit_main=true;

                break;
            }

            case R.id.COMP_M_CAL:
                command_bits_|=COMPASS_MOTOR_CALIBR;
                break;
            case R.id.COMP_CAL:
                command_bits_|=COMPASS_CALIBR;
                // Commander.button="CMC";
                break;
            case R.id.GPS:

                if (gpsON==false)
                    startService(new Intent(MainActivity.this, GPSservice.class));
                else
                    stopService(new Intent(MainActivity.this, GPSservice.class));
                gpsON^=true;
                // command_bits_|=COMPASS_MOTOR_CALIBR;
                // Commander.button="MCC";
                break;

            case R.id.GYRRO_CAL:
                command_bits_|=MPU_GYRO_CALIBR;
                //Commander.button="STS";
                break;
            case R.id.SPEED_R:

                SharedPreferences settings = getPreferences(MODE_PRIVATE);
                SharedPreferences.Editor editor = settings.edit();
                sensorUpdateSpeedFastest^=true;
                editor.putBoolean("sensorUpdateSpeedFastest", sensorUpdateSpeedFastest);
                editor.commit();

                finish();

                // speed_r.setTitle((game_speed)?"NORMAL":"FASTEST");
                break;
            case R.id.MAP:
                Intent myIntent = new Intent(this, Map.class);
                this.startActivity(myIntent);
                break;

        }

        return super.onOptionsItemSelected(item);
    }

    int i=0;
    static long old_time_acc=0;
    static long old_time=0;

    static double heading_t;
    static double pitch=0,roll=0;

    double dt_update=0;


    @Override
    public void onSensorChanged(SensorEvent event) {



        setButtons();


        long now=System.currentTimeMillis();
        if (old_time==0) {
            old_time = old_time_acc=now;
            return;
        }


/*
SensorEvent.values[0]	Rate of rotation around the x axis.	rad/s
SensorEvent.values[1]	Rate of rotation around the y axis.
SensorEvent.values[2]	Rate of rotation around the z axis.
 */

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

            double aRoll = Math.atan2(event.values[1], event.values[2]);
            double aPitch = Math.atan2(event.values[0] , Math.sqrt(event.values[1] * event.values[1] + event.values[2] * event.values[2]));

            double dt=0.001*(now-old_time_acc);
            old_time_acc=now;
            double F=Math.min(1,dt*0.3);
            pitch+=(aPitch-pitch)*F;
            roll+=(aRoll-roll)*F;

            double k=(float)(zoomN/0.69813170079773183076947630739545);
            Commander.pitch=(float)(pitch*k);
            Commander.roll=(float)(roll*k);


          //  Log.i("MATH","aroll="+(int)(aRoll*56.3)+", apitch="+(int)(aPitch*57.3));


            k=(float)(zoomN/0.69813170079773183076947630739545);
            Commander.ax+=((event.values[0]*k/9.8)-Commander.ax)*0.1;
            Commander.ay+=((event.values[1]*k/9.8)-Commander.ay)*0.1;

            //Log.d("SENhD", "Andr "+Double.toString(event.values[0]));

            //az=midZ.get(event.values[2]*k/10f);



        }

        if (event.sensor.getType()==Sensor.TYPE_ORIENTATION){
            heading_t = ((double)event.values[0]+90);

            if (heading_t>180)
                heading_t-=360;


            Commander.heading=(float)heading_t;

            //Log.d("SENhD", "Andr "+Double.toString(heading));

        }


    }
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            //Log.i("KEY","DOWN "+Integer.toString(keyCode));
            command_bits_|=GIMBAL_MINUS;
            //   Commander.button="CDN";
            return true;
        }else
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            //Log.i("KEY","DOWN "+Integer.toString(keyCode));
            // Commander.button="CUP";
            command_bits_|=GIMBAL_PLUS;
            return true;
        }


        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN || keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            //Log.i("KEY","UP "+Integer.toString(keyCode));

            return true;
        }

        return super.onKeyUp(keyCode, event);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    public void start_stop(View view) {
        if (secure_flug) {
            ch_secure.setChecked(false);
            secure_flug=false;
            b_start.setEnabled(false);
            commands_off_full_th=true;
            //   Commander.button = "S_S";
            command_bits_|=MOTORS_ON;
            Commander.heading=(float)heading_t;

        }
    }

    public static final double sin10g = 0.17453292519943295769236907684886;

    public void plus(View view){
        zoomN-=sin10g;
        if (zoomN<sin10g)
            zoomN=sin10g;
        Commander.zoomChanged();
    }

    public void minus(View view){
        zoomN+=sin10g;
        if (zoomN>sin10g*4)
            zoomN=sin10g*4;

    }


    static long updateTime=0;
    //static private int old_control_bits=0;
    private void setButtons(){
        if (System.currentTimeMillis()>updateTime) {
            updateTime=System.currentTimeMillis()+100;
            //  if (old_control_bits!=control_bits)
            //    old_control_bits=control_bits;
            // else
            //   return;
            b_menu.setEnabled(true);
            b_start.setText(((control_bits & MOTORS_ON)!=0) ? "Stop" : "Start");
            b_start.setEnabled(secure_flug && Commander.link);
            b_toHome.setChecked((control_bits & GO2HOME) != 0);
            b_altHold.setChecked((control_bits & Z_STAB) != 0);
            b_smartCTRL.setChecked((control_bits & XY_STAB) != 0);
            cb_horizont.setChecked((control_bits & HORIZONT_ON)!=0);
            cb_compass.setChecked((control_bits & COMPASS_ON)!=0);
            b_prog.setChecked((control_bits & PROGRAM)!=0);


        }
    }


    public void smartCtrl(View view){
        //  cb_horizont.setChecked(horizont_on=control_bits==XY_STAB);
        b_smartCTRL.setChecked(!b_smartCTRL.isChecked());
        command_bits_|=XY_STAB;
        //Commander.button="SCT";

    }
    public void openMenu(View view){
        openOptionsMenu();
    }

    public void Prog(View view){
        b_prog.setChecked(!b_prog.isChecked());
        if (b_prog.getTextSize()!=0)
            command_bits_|=PROGRAM;

    }

    public void toHome(View view) {
        b_toHome.setChecked(!b_toHome.isChecked());
        command_bits_|=GO2HOME;
    }

    public void altHold(View view){
        b_altHold.setChecked(!b_altHold.isChecked());
        command_bits_|=Z_STAB;
        // Commander.button="AHD";
    }


    // public void setCompCalibr(View view) {

    //    Commander.setCompassCalibration();
    //}

    public void secure(View view){
        secure_flug^=true;
        b_start.setEnabled(Commander.link && secure_flug);
    }
    public void power05(View view){
        Commander.throttle =0.5f;
    }


    public void horizont_on(View view){
        cb_horizont.setChecked(!cb_horizont.isChecked());
        command_bits_|=HORIZONT_ON;
        // Commander.button="HRT";
    }
    public void compas_on(View view){
        cb_compass.setChecked(!cb_compass.isChecked());
        command_bits_|=COMPASS_ON;
        // Commander.button="CMP";
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO Auto-generated method stub

    }




}
