package copter.rc2;

/**
 * Created by igor on 8/19/2016.
 */




import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.SubMenu;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;


public class Map extends Activity {
    static public DrawMap drawmap=null;

    static public Programmer prog=new Programmer();

    @Override
    protected  void onStart() {
        super.onStart();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        //setContentView(drawmap);


        SharedPreferences settings = getPreferences(MODE_PRIVATE);


        DrawMap.zoom=settings.getInt("zoom",3);
        DrawMap.screenP.x=settings.getInt("screenPX",0);
        DrawMap.screenP.y=settings.getInt("screenPY",0);
        DrawMap.type=settings.getInt("mapName",7);
        String s=settings.getString("program","");
        if (s.length()>10)
            Programmer.load(s);

        drawmap = new DrawMap(Map.this);
        drawmap.setBackgroundColor(Color.WHITE);
        setContentView(drawmap);
        cont=this;

    }


    void save(){
        DrawMap.threadRun=false;
        SharedPreferences settings = getPreferences(MODE_PRIVATE);
        SharedPreferences.Editor editor = settings.edit();
        editor.putInt("zoom",DrawMap.zoom);
        editor.putInt("screenPX",DrawMap.screenP.x);
        editor.putInt("screenPY",DrawMap.screenP.y);
        editor.putInt("mapName",DrawMap.type);
        editor.putString("program",Programmer.get());
        editor.commit();

    }

    static public Context cont;
    @Override
    protected void onPause(){
        super.onPause();
        save();
    }
    @Override
    protected void onDestroy() {
        super.onDestroy();
        save();

    }



    static private MenuItem load_prog=null;
    static private MenuItem save_prog=null;


    public static boolean menuON=false;


    static String[] progName=new String[100];
    static int progs=0;


    static String[] logName=new String[1000];
    static int logs=0;






    void add2menu_prog_loading(Menu menu){
        SubMenu fileMenu = menu.addSubMenu("Load prog");

        File f = new File("/sdcard/RC/PROGS");
        if (f.exists()==false) {
            File folder = new File("/sdcard/RC/PROGS");
            folder.mkdirs();
        }
        File file[] = f.listFiles();
        progs=0;
        for (int i=0; i<file.length; i++) {
            String t[]=file[i].toString().split("/");
            if (t[t.length-1].endsWith(".prog")) {
                String ts=t[t.length - 1];
                ts=ts.substring(0,ts.length()-5);
                fileMenu.add(0, progs+20, 0, ts);
                progName[progs]=t[t.length - 1];
                progs++;
                if (progs>=100)
                    break;
            }

        }
    }



    void add2menu_log_loading(Menu menu){
        SubMenu fileMenu = menu.addSubMenu("Load log");

        File f = new File("/sdcard/RC");
        if (f.exists()==false) {
            File folder = new File("/sdcard/RC");
            folder.mkdirs();
        }
        File file[] = f.listFiles();
        logs=0;
        for (int i=0; i<file.length; i++) {
            String t[]=file[i].toString().split("/");
            if (t[t.length-1].endsWith(".log")) {
                String ts=t[t.length - 1];
                ts=ts.substring(0,ts.length()-5);
                fileMenu.add(0, logs+1000, 0, ts);
                logName[logs]=t[t.length - 1];
                logs++;
                if (logs>=100)
                    break;
            }

        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {

        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.map_menu, menu);


        load_prog   = menu.getItem(0);
        save_prog   = menu.getItem(1);

        add2menu_prog_loading(menu);
        add2menu_log_loading(menu);


      //  fileMenu.add(0, 415, 0, "Back");




        return true;
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id>=20 && id <120){


                    try {
                        FileInputStream stream = new FileInputStream("/sdcard/RC/PROGS/"+progName[id-20]);
                        int len=stream.available();
                        byte buf[]=new byte[len];
                        try {
                            stream.read(buf);
                            Programmer.load(new String(buf));

                        } finally {
                            stream.close();
                        }
                    }
                    catch (Exception e) {

                    }


                }

        if (id>=1000 ){


                    try {
                        FileInputStream stream = new FileInputStream("/sdcard/RC/"+logName[id-1000]);
                        int len=stream.available();
                        byte buf[]=new byte[len];
                        try {
                            int rlen=stream.read(buf);
                            LogReader.load(new String(buf));
                        } finally {
                            stream.close();
                        }
                    }
                    catch (Exception e) {

                    }


                }















        switch(id) {

            case R.id.GOOGLE_MAP:{
                DrawMap.type=0;
                break;
            }
            case R.id.OPEN_CYCLE_MAP:{
                DrawMap.type=1;
                break;
            }
            case R.id.MICROSOFT_EARTH:{
                DrawMap.type=7;
                break;
            }

            case R.id.SAVE_PROG:{



                AlertDialog.Builder alertDialog = new AlertDialog.Builder(Map.this);
                alertDialog.setTitle("File name");

                final EditText input = new EditText(Map.this);
                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                        LinearLayout.LayoutParams.MATCH_PARENT,
                        LinearLayout.LayoutParams.MATCH_PARENT);
                input.setLayoutParams(lp);
                alertDialog.setView(input);

                alertDialog.setPositiveButton("YES",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                String fn = input.getText().toString();
                                try {
                                    FileOutputStream stream = new FileOutputStream("/sdcard/RC/PROGS/"+fn+".prog");
                                    try {
                                        stream.write(Programmer.get().getBytes());
                                    } finally {
                                        stream.close();
                                        invalidateOptionsMenu();
                                    }
                                }
                                catch (Exception e) {

                                }
                            }
                        });


                alertDialog.setNegativeButton("NO",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.cancel();
                            }
                        });
                alertDialog.show();














                break;
            }
            case R.id.EXIT:{
                super.finish();
                break;
            }


        }
        menuON=false;
        return super.onOptionsItemSelected(item);
    }


    @Override
    public boolean onPrepareOptionsMenu (Menu menu){
        menuON=true;
        return super.onPrepareOptionsMenu(menu);
    }


    @Override
    public void onOptionsMenuClosed(Menu menu)
    {
        menuON=false;
        super.onOptionsMenuClosed(menu);
    }
}
