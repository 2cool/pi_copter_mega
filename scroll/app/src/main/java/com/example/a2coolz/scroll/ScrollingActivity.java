package com.example.a2coolz.scroll;

import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;


public class ScrollingActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener,SeekBar.OnSeekBarChangeListener {



    String _null="NULL";
    String a[][]={
            {"P_R_rateKP","P_R_rateKI","P_R_rateIMAX","P_R_stabKP","YAW_rate_KP","YAW_rateE_KI","YAW_rate_IMAX","YAW_stab_KP","MAX_ANGLE","powerK","balance"},
            {"STAB_KP","SPEED_KP","SPEED_I","SPEED_imax","MAX_SPEED_P","MAX_SPEED_M","CF_SPEED","CF_DIST","FILTR",_null,"Z stab"},
            {"STAB_KP","SPEED_KP","SPEED_I","SPEED_imax","max_speed","KF_SPEED","KF_DIST","FILTR",_null,_null,"XY stab"},
            {"high_to_lift_2_home","max_throttle","min_throttle","sens_xy","sens_z","min_hight","debug_n","camera_mod","gimbP_Z","gimbR_Z","secur"},
            {"DRAG_K","_0007","tiltPower_CF",_null,_null,_null,_null,_null,_null,_null,"mpu"},
            {"m power on",_null,_null,_null,_null,_null,_null,_null,_null,_null,"compas"},
            {"vedeoAdr","ppp_inet","telegram",_null,_null,_null,_null,_null,_null,_null,"rest"}

    };

    static int settings_i;
    static String s_title=new String("TITLE");
    SeekBar seekbar0;
    Toolbar toolbar;
    TextView[] textV=new TextView[20];
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_scrolling);
       // Toolbar toolbar =  findViewById(R.id.toolbar);
      //  toolbar.setTitle(s_title);
      //  setSupportActionBar(toolbar);



        Spinner spinner = findViewById(R.id.spinner1);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.planets_array, android.R.layout.simple_spinner_item);
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

        textV[0].setText("0.1");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_scrolling, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
int i=0;
    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

        TextView text=findViewById(R.id.ek0);
       text.setText(Integer.toString(i++));

    //    toolbar.setTitle("MPU1");
     //   setSupportActionBar(toolbar);
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {

    }

    double numb=0.1;
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        int i=seekBar.getId()-seekbar0.getId();

        double f=numb;//Float.parseFloat(textV[i].getText().toString());


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
