package copter.rc2;


import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;

/**
 * Created by igor on 8/26/2016.
 */
public class MapEdit extends Activity {


    static public boolean active=false;
    private EditText eLed_prog, eTimer,eSpeed,eVSpeed,eAltitude,eCamAng;
    static private SeekBar sTimer, sSpeed, sVSpeed,sAltitude,sCamAng, sLed_prog;


   static public  String getNum(double num, int n){
        n++;
       if (Math.abs(num)<0.1)
           num=0;
        String s=Double.toString(num);
        int dotI=s.indexOf('.');
        if (s.length()-dotI > n){
            return s.substring(0,dotI+n);
        }else
            return s;
    }

    protected void update(){

        GeoDot d=Programmer.get_current_dot();
        if (d!=null){
          //  eCamAng.setText(Integer.toString((int)d.cam_ang));
            //90 -  -10
            sCamAng.setProgress((int)d.cam_ang+10);
            eCamAng.setText(getNum(d.cam_ang,2));
            eLed_prog.setText(getNum(d.led_prog,1));
            sLed_prog.setProgress(d.led_prog);
            eTimer.setText(getNum(d.timer,2));
            sTimer.setProgress((int)d.timer);
            eSpeed.setText(getNum(d.speed,2));
            sSpeed.setProgress((int)(100*(d.speed/Programmer.maxSpeed)));
            double zsk=((d.speedZ > 0)?(d.speedZ/Programmer.maxUpSpeed):(d.speedZ/Programmer.maxDownSpeed));
            eVSpeed.setText(getNum(d.speedZ,2));
            sVSpeed.setProgress((int)(100*zsk));
            eAltitude.setText(getNum(d.alt,2));
            sAltitude.setProgress((int)(d.alt));

        }else {


            eCamAng.setText(Integer.toString(Programmer.cam_ang));
            sCamAng.setProgress(Programmer.cam_ang+10);
            eLed_prog.setText(getNum(Programmer.led_prog,1));
            sLed_prog.setProgress(Programmer.led_prog);
            eTimer.setText(Integer.toString(0));
            sTimer.setProgress(0);
            eSpeed.setText(getNum(Programmer.speed, 2));
            sSpeed.setProgress((int) (100 * Programmer.speedProgress));
            eVSpeed.setText(getNum(Programmer.speedZ, 2));
            sVSpeed.setProgress((int) (100 * Programmer.vspeedProgress));
            eAltitude.setText(getNum(Programmer.altitude, 2));
            sAltitude.setProgress((int) (Programmer.altitude));


        }

    }
    static int inListener=0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        active=true;
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.dot_edit);


        ((TextView)findViewById(R.id.textViewDistance)).setText(Integer.toString((int)Programmer.distance)+" m.");
        eTimer=(EditText)findViewById(R.id.editTextTimer);
        eTimer.setEnabled(false);
        sTimer=(SeekBar)findViewById(R.id.seekBarTimer);
       // sTimer.setEnabled(false);

        eCamAng=(EditText)findViewById(R.id.editTextCamAng);
        eCamAng.setEnabled(false);
        sCamAng=(SeekBar)findViewById(R.id.seekBarCamAng);
        sLed_prog=(SeekBar)findViewById(R.id.sled_prog);
        eLed_prog=(EditText)findViewById(R.id.eled_prog);
        eLed_prog.setEnabled(false);
        eSpeed=(EditText)findViewById(R.id.editTextSpeed);

        eSpeed.setEnabled(false);
        sSpeed=(SeekBar)findViewById(R.id.seekBarSpeed);
        sSpeed.setProgress(100);
        eVSpeed=(EditText)findViewById(R.id.editTextVerSpeed);
        eVSpeed.setEnabled(false);
        sVSpeed=(SeekBar)findViewById(R.id.seekBarVerSpeed);
        sVSpeed.setProgress(100);
        eAltitude=(EditText)findViewById(R.id.editTextAltitude);
        eAltitude.setEnabled(false);
        sAltitude=(SeekBar)findViewById(R.id.seekBarAltitude);

        update();



        sTimer.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                // TODO Auto-generated method stub
                if (++inListener==1) {

                    double timer=progress;
                    if (!DrawMap.prog.changeTimerInLastDot(timer,Programmer.speedProgress,Programmer.vspeedProgress))
                    update();
                }
                inListener--;


            }
        });


        sAltitude.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                // TODO Auto-generated method stub
                if (++inListener==1) {

                    double altitude=progress;
                    if (!DrawMap.prog.changeAltitudeInLastDot(altitude,Programmer.speedProgress,Programmer.vspeedProgress))
                        Programmer.altitude=altitude;
                    update();
                }
                inListener--;


            }
        });


        sLed_prog.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                // TODO Auto-generated method stub
                if (++inListener==1) {

                    double led_prog=progress;
                    Programmer.led_prog=(int)(led_prog);
                    Programmer.changeLedProg();

                    update();
                }
                inListener--;


            }
        });




        sCamAng.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
                // TODO Auto-generated method stub
                if (++inListener==1) {

                    double camAng=progress;
                    Programmer.cam_ang=(int)(camAng-10);
                    Programmer.changeCameraAngle();

                    update();
                }
                inListener--;


            }
        });

        sSpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {

                // TODO Auto-generated method stub
                if (++inListener==1) {
                    double speedProgress=0.01*(double)sSpeed.getProgress();
                    if (speedProgress<0.1)
                        speedProgress=0.1;
                    if (!DrawMap.prog.changeSpeedInLastDot(speedProgress,Programmer.vspeedProgress))
                        Programmer.speedProgress=speedProgress;
                    update();
                }
                inListener--;

            }
        });

        sVSpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {

                // TODO Auto-generated method stub
                if (++inListener==1) {
                    double vspeedProgress=0.01*(double)sVSpeed.getProgress();
                    if (vspeedProgress<0.03)
                        vspeedProgress=0.037;
                    if (!DrawMap.prog.changeSpeedInLastDot(Programmer.speedProgress,vspeedProgress))
                        Programmer.vspeedProgress=vspeedProgress;
                    update();
                }
                inListener--;

            }
        });









        eCamAng.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {
                if (++inListener==1) {
                    String st=s.toString();
                    if (st.length()>0 && !st.equals("-")) {
                        Programmer.cam_ang = Integer.parseInt(st);
                        if (Programmer.cam_ang<-90)
                            Programmer.cam_ang=-90;
                        else
                        if (Programmer.cam_ang>10)
                            Programmer.cam_ang=10;
                        Programmer.changeCameraAngle();

                    }
                }
                inListener--;

            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });




        eAltitude.addTextChangedListener(new TextWatcher() {
            public void afterTextChanged(Editable s) {



            }
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });







    }
    @Override
    protected void onPause() {
        super.onPause();
        active=false;
    }














}
