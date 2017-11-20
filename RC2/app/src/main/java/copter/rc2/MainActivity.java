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





    protected boolean shouldAskPermissions() {
        return (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP_MR1);
    }

    @TargetApi(23)
    protected void askPermissions() {
        String[] permissions = {
                "android.permission.READ_EXTERNAL_STORAGE",
                "android.permission.WRITE_EXTERNAL_STORAGE"
        };
        int requestCode = 200;
        requestPermissions(permissions, requestCode);
    }




    static public Bitmap blank;

    static public int control_bits=0;
    static public int command_bits_=0;
    private static boolean secure_flug=false;

    public static double zoomN=0.17453292519943295769236907684886*2;//0.69813170079773183076947630739545;
//private static boolean game_speed=false;
	private SensorManager mSensorManager;
	private Sensor accelerometer;
	private Sensor magnetic_field;
	//float az;

	long acc_t=0,compas_t=0;

	double aa_i=0;
	long time_out=100;
	
	static public DrawView drawView=null;
    Button b_start;
    static ToggleButton b_altHold,b_smartCTRL,b_toHome,b_prog;
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


       if (shouldAskPermissions()) {
            askPermissions();
        }

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
        b_smartCTRL=(ToggleButton)findViewById(R.id.t_smartK);;
        b_toHome=(ToggleButton)findViewById(R.id.b_toHome);
        b_start=(Button)findViewById(R.id.b_start);
        b_prog= (ToggleButton)findViewById(R.id.b_Prog);

        rl1 =(RelativeLayout)findViewById(R.id.rl1);

        ch_secure=(CheckBox)findViewById(R.id.cb_secure);
        cb_compass=(CheckBox)findViewById(R.id.cb_compass);
        cb_horizont=(CheckBox)findViewById(R.id.cb_horizont);
        
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
	    accelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
	    magnetic_field=mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
       
        
        int speed=(sensorUpdateSpeedFastest)?SensorManager.SENSOR_DELAY_FASTEST:SensorManager.SENSOR_DELAY_NORMAL;
		mSensorManager.registerListener(this, accelerometer,speed );
	    mSensorManager.registerListener(this, magnetic_field, speed);

        net.start();

        drawView = new DrawView(MainActivity.this);
        drawView.setBackgroundColor(Color.WHITE);
        rl1.addView(drawView);

    }
    static boolean commands_off_full_th =false;



    static private MenuItem reset=null;
    static private MenuItem exit=null;
    static private MenuItem gyroCalibr=null;
    static private MenuItem setHor=null;
    static private MenuItem compass_cal=null;
    static private MenuItem speed_r=null;
    static private MenuItem map=null;



    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);




        compass_cal         =menu.getItem(1);

        setHor              =menu.getItem(3);
        gyroCalibr            =menu.getItem(4);
        exit                =menu.getItem(5);
        reset               =menu.getItem(6);
        speed_r             =menu.getItem(7);
        map             =menu.getItem(8);


      //  max_thhrotle_menu.setEnabled(commands_off_full_th == 0);
        boolean secure=Commander.link;

        compass_cal.setEnabled(secure);
        setHor.setEnabled(secure);
        gyroCalibr.setEnabled(secure);
        exit.setEnabled(true);
        reset.setEnabled(secure);
        DrawView.menu=true;

        speed_r.setTitle((sensorUpdateSpeedFastest)?"FASTEST":"NORMAL");





        return true;
    }
    @Override
    public boolean onPrepareOptionsMenu (Menu menu){
        boolean secure=true;//Telemetry.realThrottle==0;

        compass_cal.setEnabled(secure);
        setHor.setEnabled(secure);
        gyroCalibr.setEnabled(secure);
        exit.setEnabled(true);
        reset.setEnabled(secure);
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


            case R.id.COMP_CAL:
                command_bits_|=COMPASS_CALIBR;
               // Commander.button="CMC";
                break;
            case R.id.COMP_M_CAL:
                command_bits_|=COMPASS_MOTOR_CALIBR;
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
	
    static double heading_t;
	@Override
	public void onSensorChanged(SensorEvent event) {



        setButtons();
		

		
		
		long now=System.currentTimeMillis();
    	if (event.sensor.getType()==Sensor.TYPE_ACCELEROMETER){

            double k=(float)(zoomN/0.69813170079773183076947630739545);
    		Commander.ax+=((event.values[0]*k/9.8)-Commander.ax)*0.1;
    		Commander.ay+=((event.values[1]*k/9.8)-Commander.ay)*0.1;

  //Log.d("SENhD", "Andr "+Double.toString(event.values[0]));

    		//az=midZ.get(event.values[2]*k/10f);

            new Thread() {
                @Override
                public void run() {
                    if (MainActivity.drawView!=null)
                        MainActivity.drawView.postInvalidate();
                }
            }.start();
    	}

        if (event.sensor.getType()==Sensor.TYPE_ORIENTATION){
            heading_t = ((double)event.values[0]+90);

            if (heading_t>180)
                heading_t-=360;

          //  Log.d("SENhD", "Andr "+Double.toString(heading_t));
           // double heading=heading_t/180*3.1415926;

           // Log.i("COMP", "@ " + heading_t);
		//	if (heading<0)
			//	heading+=2*Math.PI;

          //  if (heading>Math.PI)
          //      heading-=2*Math.PI;

         //   while (Commander.heading-heading  > Math.PI)
        //        heading += 2*Math.PI;
         //   while (Commander.heading-heading < -Math.PI)
         //       heading -= 2*Math.PI;
			
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
