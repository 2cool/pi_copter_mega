package cc.dewdrop.ffplayer;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

public class Settings extends AppCompatActivity implements AdapterView.OnItemSelectedListener,SeekBar.OnSeekBarChangeListener{

    String _null="NULL";
    String a[][]={
            {"P_R_rateKP","P_R_rateKI","P_R_rateIMAX","P_R_stabKP","YAW_rate_KP","YAW_rateE_KI","YAW_rate_IMAX","YAW_stab_KP","MAX_ANGLE","powerK"},
            {"STAB_KP","SPEED_KP","SPEED_I","SPEED_imax","MAX_SPEED_P","MAX_SPEED_M","CF_SPEED","CF_DIST","FILTR",_null},
            {"STAB_KP","SPEED_KP","SPEED_I","SPEED_imax","max_speed","KF_SPEED","KF_DIST","FILTR",_null,_null},
            {"high_to_lift_2_home","max_throttle","min_throttle","sens_xy","sens_z","min_hight","debug_n","camera_mod","gimbP_Z","gimbR_Z"},
            {"DRAG_K","_0007","tiltPower_CF",_null,_null,_null,_null,_null,_null,_null},
            {"m power on",_null,_null,_null,_null,_null,_null,_null,_null,_null},
            {"vedeoAdr","ppp_inet","telegram",_null,_null,_null,_null,_null,_null,_null}

    };

    TextView[] textV=new TextView[20];
    SeekBar seekbar0;
    Button set;

    protected static int menu_n=0;
    float copter_set[][]=new float[6][9];

    protected void setTextView(final int num){
        if (a.length>num) {
            ((TextView) findViewById(R.id.textView1)).setText(a[num][0]);
            ((TextView) findViewById(R.id.textView2)).setText(a[num][1]);
            ((TextView) findViewById(R.id.textView3)).setText(a[num][2]);
            ((TextView) findViewById(R.id.textView4)).setText(a[num][3]);
            ((TextView) findViewById(R.id.textView5)).setText(a[num][4]);
            ((TextView) findViewById(R.id.textView6)).setText(a[num][5]);
            ((TextView) findViewById(R.id.textView7)).setText(a[num][6]);
            ((TextView) findViewById(R.id.textView8)).setText(a[num][7]);
            ((TextView) findViewById(R.id.textView9)).setText(a[num][8]);
            ((TextView) findViewById(R.id.textView10)).setText(a[num][9]);
            ((TextView) findViewById(R.id.textViewSet)).setText(a[num][10]);
            set.setEnabled(true);


        }else{
            ((TextView) findViewById(R.id.textView1)).setText(_null);
            ((TextView) findViewById(R.id.textView2)).setText(_null);
            ((TextView) findViewById(R.id.textView3)).setText(_null);
            ((TextView) findViewById(R.id.textView4)).setText(_null);
            ((TextView) findViewById(R.id.textView5)).setText(_null);
            ((TextView) findViewById(R.id.textView6)).setText(_null);
            ((TextView) findViewById(R.id.textView7)).setText(_null);
            ((TextView) findViewById(R.id.textView8)).setText(_null);
            ((TextView) findViewById(R.id.textView9)).setText(_null);
            ((TextView) findViewById(R.id.textView10)).setText(_null);
            ((TextView) findViewById(R.id.textViewSet)).setText(_null);
            set.setEnabled(false);

        }

    }


    protected void setEditText(int num,boolean flag){
        String sn=""+num;
        SharedPreferences settings = getPreferences(MODE_PRIVATE);
        Commander.upload_settings=Integer.parseInt(n.getText().toString());
        //   Commander.button="UP"+Integer.parseInt(n.getText().toString());
        //   Log.i("UPS", "button pressed "+Commander.button);
        Telemetry.n_settings=-2;
        for (int i=0; i<3; i++) {
            try {
                Thread.sleep(200);
            } catch (InterruptedException e) {
                Log.i("UPS", "sleep Error");
                e.printStackTrace();
            }
            if (Telemetry.n_settings!= -2){
                Log.i("UPS", "!=-2");
                if ( Telemetry.n_settings == Integer.parseInt(n.getText().toString()) ){
                    Log.i("UPS", "load set");
                    k0.setText(Float.toString(Telemetry.settings[0]));
                    k1.setText(Float.toString(Telemetry.settings[1]));
                    k3.setText(Float.toString(Telemetry.settings[2]));
                    k4.setText(Float.toString(Telemetry.settings[3]));
                    k5.setText(Float.toString(Telemetry.settings[4]));
                    k6.setText(Float.toString(Telemetry.settings[5]));
                    k7.setText(Float.toString(Telemetry.settings[6]));
                    k8.setText(Float.toString(Telemetry.settings[7]));
                    k9.setText(Float.toString(Telemetry.settings[8]));
                    k10.setText(Float.toString(Telemetry.settings[9]));

                }
                break;
            }
        }



        if (flag)
            n.setText("" + num);
    }

    @Override
    protected  void onStart(){
        super.onStart();
        setTextView(menu_n);
        setEditText(menu_n, true);

    }




    void update(){







        k0.setText(Float.toString(Telemetry.settings[0]));
        k1.setText(Float.toString(Telemetry.settings[1]));
        k3.setText(Float.toString(Telemetry.settings[2]));
        k4.setText(Float.toString(Telemetry.settings[3]));
        k5.setText(Float.toString(Telemetry.settings[4]));
        k6.setText(Float.toString(Telemetry.settings[5]));
        k7.setText(Float.toString(Telemetry.settings[6]));
        k8.setText(Float.toString(Telemetry.settings[7]));
        k9.setText(Float.toString(Telemetry.settings[8]));
        k10.setText(Float.toString(Telemetry.settings[9]));








        int i=0;

        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;
        textV[i].setText(Float.toString(copter_set[menu_n][i]=Telemetry.settings[i]));
        i++;

//------------------------------------------
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
        textV[i].setText(a[menu_n][i-10]);
        i++;
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        try {
            super.onCreate(savedInstanceState);
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
            setContentView(R.layout.settings_scrolling);


            set = findViewById(R.id.upload);

            Spinner spinner = findViewById(R.id.spinner1);
            ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,R.array.settings_list, android.R.layout.simple_spinner_item);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinner.setAdapter(adapter);
            spinner.setOnItemSelectedListener(this);


            (seekbar0 = findViewById(R.id.sb0)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb1)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb2)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb3)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb4)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb5)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb6)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb7)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb8)).setOnSeekBarChangeListener(this);
            ((SeekBar)findViewById(R.id.sb9)).setOnSeekBarChangeListener(this);

            int i=0;
            textV[i++]=findViewById(R.id.ek0);
            textV[i++]=findViewById(R.id.ek1);
            textV[i++]=findViewById(R.id.ek2);
            textV[i++]=findViewById(R.id.ek3);
            textV[i++]=findViewById(R.id.ek4);
            textV[i++]=findViewById(R.id.ek5);
            textV[i++]=findViewById(R.id.ek6);
            textV[i++]=findViewById(R.id.ek7);
            textV[i++]=findViewById(R.id.ek8);
            textV[i++]=findViewById(R.id.ek9);
//------------------------------------------
            textV[i++]=findViewById(R.id.tv0);
            textV[i++]=findViewById(R.id.tv1);
            textV[i++]=findViewById(R.id.tv2);
            textV[i++]=findViewById(R.id.tv3);
            textV[i++]=findViewById(R.id.tv4);
            textV[i++]=findViewById(R.id.tv5);
            textV[i++]=findViewById(R.id.tv6);
            textV[i++]=findViewById(R.id.tv7);
            textV[i++]=findViewById(R.id.tv8);
            textV[i++]=findViewById(R.id.tv9);
            update();



            k0 = (EditText) findViewById(R.id.e_k0);
            k1 = (EditText) findViewById(R.id.e_k1);
            k4 = (EditText) findViewById(R.id.e_k4);
            k3 = (EditText) findViewById(R.id.e_k3);
            k5 = (EditText) findViewById(R.id.e_k5);
            k6 = (EditText) findViewById(R.id.e_k6);
            k7 = (EditText) findViewById(R.id.e_k7);
            k8 = (EditText) findViewById(R.id.e_k8);
            k9 = (EditText) findViewById(R.id.e_k9);
            k10 = (EditText) findViewById(R.id.e_ka);

            n = (EditText) findViewById(R.id.e_n);
            n.addTextChangedListener(new TextWatcher() {
                public void afterTextChanged(Editable s) {
                    String sn = n.getText().toString();
                    if (sn.length() == 0) {
                        menu_n = 0;
                        n.setText("0");
                    } else
                        menu_n = Integer.parseInt(sn);

                    setTextView(menu_n);
                    setEditText(menu_n, false);
                }

                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }
            });
        }catch (Exception e){
            Log.i("ERROR","settings "+e.toString());
        }
    }


    public void exit_set(View view) {
        finish();
    }

    public void sendMessage(View view) {
        // Do something in response to button
        SharedPreferences settings = getPreferences(MODE_PRIVATE);
        SharedPreferences.Editor editor = settings.edit();

        Commander.n =Integer.parseInt(n.getText().toString());
        editor.putFloat(""+Commander.n,Commander.n);
        Commander.k0 =Float.parseFloat(k0.getText().toString());
        editor.putFloat("k0"+Commander.n, Commander.k0);

        Commander.k1 =Float.parseFloat(k1.getText().toString());
        editor.putFloat("k1"+Commander.n, Commander.k1);

        Commander.k3 =Float.parseFloat(k3.getText().toString());
        editor.putFloat("k3"+Commander.n, Commander.k3);

        Commander.k4 =Float.parseFloat(k4.getText().toString());
        editor.putFloat("k4"+Commander.n, Commander.k4);

        Commander.k5 =Float.parseFloat(k5.getText().toString());
        editor.putFloat("k5"+Commander.n, Commander.k5);

        Commander.k6 =Float.parseFloat(k6.getText().toString());
        editor.putFloat("k6" + Commander.n, Commander.k6);

        Commander.k7 =Float.parseFloat(k7.getText().toString());
        editor.putFloat("k7"+Commander.n, Commander.k7);

        Commander.k8 =Float.parseFloat(k8.getText().toString());
        editor.putFloat("k8"+Commander.n, Commander.k8);

        Commander.k9 =Float.parseFloat(k9.getText().toString());
        editor.putFloat("k9"+Commander.n, Commander.k9);

        Commander.k10 =Float.parseFloat(k10.getText().toString());
        editor.putFloat("k10" + Commander.n, Commander.k10);



        Commander.settings=true;
        Log.i("SETT","setting=true");

        editor.commit();

        finish();

    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

        menu_n=position;

    }


    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {

    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }
}
