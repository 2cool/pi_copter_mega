package cc.dewdrop.ffplayer;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

public class Settings extends AppCompatActivity implements AdapterView.OnItemSelectedListener,SeekBar.OnSeekBarChangeListener{

    String _null="";
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
    SeekBar []seek_bar=new SeekBar[10];
    Button set;

    protected static int menu_n=0;





    protected void download_settings(int num, boolean upload_anyway){
      //  String sn=""+num;
       // SharedPreferences settings = getPreferences(MODE_PRIVATE);
        Log.i("UPS", "SET_EDIT_TEXT");
        if (upload_anyway || Telemetry.n_settings!=num ||  Telemetry.settings[0]==Commander.NO_DATA) {
            Commander.upload_settings = num;//Integer.parseInt(n.getText().toString());
            //   Commander.button="UP"+Integer.parseInt(n.getText().toString());
            Log.i("UPS", "DOWNLOADING SETINGS");
            Telemetry.n_settings=-2;
            boolean uploaded=false;
            for (int i=0; i<3; i++) {
                try {
                    Thread.sleep(200);
                } catch (InterruptedException e) {
                    Log.i("UPS", "sleep Error");
                    e.printStackTrace();
                }
                if (Telemetry.n_settings!= -2){
                    Log.i("UPS", "DOWNLOADING DONE");
                    if ( Telemetry.n_settings != num){//Integer.parseInt(n.getText().toString()) ){
                        Log.i("UPS", "DOWNLOADING ERROR");
                        uploaded=false;
                    }else {
                        uploaded=true;
                        break;
                    }
                }
            }
            if (uploaded==false)
                for (int i=0; i<10; i++)
                    Telemetry.settings[i] = Commander.NO_DATA;
            update();
        }


      //  if (flag)
        //    n.setText("" + num);
    }

    @Override
    protected  void onStart(){
        super.onStart();
        download_settings(menu_n,false);

    }


    void reset(View view){
        update();
    }

    void update(){
        Log.i("UPS", "UPDATE");
        int i=0;
        for (; i<10; i++) {
            seek_bar[i].setProgress(50);
            textV[i+10].setText(a[menu_n][i]);
            if (Telemetry.settings[i] != Commander.NO_DATA) {
                textV[i].setText(Float.toString(Telemetry.settings[i]));
                textV[i+10].setEnabled(true);
                textV[i].setEnabled(true);
                seek_bar[i].setEnabled(true);
            }
            else {
                textV[i].setText("");
                textV[i+10].setEnabled(false);
                textV[i].setEnabled(false);
                seek_bar[i].setEnabled(false);
            }
        }

//------------------------------------------





        set.setEnabled(true);
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        try {
            super.onCreate(savedInstanceState);

            setContentView(R.layout.settings_scrolling);


            set = findViewById(R.id.upload);

            Spinner spinner = findViewById(R.id.spinner1);
            ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,R.array.settings_list, android.R.layout.simple_spinner_item);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinner.setAdapter(adapter);
            spinner.setOnItemSelectedListener(this);
            spinner.setSelection(menu_n);

            int i=0;
            (seek_bar[i++] = findViewById(R.id.sb0)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb1)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb2)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb3)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb4)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb5)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb6)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb7)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb8)).setOnSeekBarChangeListener(this);
            (seek_bar[i++] = findViewById(R.id.sb9)).setOnSeekBarChangeListener(this);

            i=0;
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

            download_settings(menu_n,true);

        }catch (Exception e){
            Log.i("ERROR","settings "+e.toString());
        }
    }



    public void upload_settings(View view) {
        Log.i("UPS", "UPLOADING SETINGS");
        Commander.n =menu_n;
        for (int i=0;i<10;i++) {
            if (textV[i].getText().toString().length()>0)
                Commander.sets[i] = Float.parseFloat(textV[i].getText().toString());
            else
                Commander.sets[i]=1;
        }
        Commander.settings=true;
        Log.i("SETT","setting=true");
        download_settings(menu_n,true);
        //finish();
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        menu_n=position;
        download_settings(menu_n,false);
    }


    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {

    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        int i=seekBar.getId()-seek_bar[0].getId();

        double f=Telemetry.settings[i];
        double k=0.2*(50-progress)/50.0;
        f-=f*k;
        textV[i].setText(Float.toString((float)(f)));
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }
}
