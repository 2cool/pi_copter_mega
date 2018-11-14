package com.example.igor.mycrosd_read_write_test;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashSet;

public class MainActivity extends AppCompatActivity {

    // Storage Permissions
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    /**
     * Checks if the app has permission to write to device storage
     *
     * If the app does not has permission then the user will be prompted to grant permissions
     *
     * @param activity
     */
    public static void verifyStoragePermissions(Activity activity) {
        // Check if we have write permission
        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE
            );
        }
    }

    public String getLOG_FNAME(){



        int cnt=999;
        try {
            InputStream is = new FileInputStream("/sdcard/RC/counter.txt");
            BufferedReader buf = new BufferedReader(new InputStreamReader(is));
            String s="";
            s = buf.readLine();
            cnt=Integer.parseInt(s);

            is.close();
        } catch (Exception e) {
            e.printStackTrace();
        }

        try{
            File file;

            file = new File(Environment.getExternalStorageDirectory(),"RC/counter.txt");
            file.createNewFile();
            //  boolean deleted = file.delete();

            OutputStream os = new FileOutputStream("/sdcard/RC/counter.txt");


            String t=Integer.toString(cnt+1);
            os.write(t.getBytes());
            os.close();

        } catch (Exception e) {
            e.printStackTrace();
        }

        return "RC/"+Integer.toString(cnt)+".log";
    }





    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        verifyStoragePermissions(this);
      //  verifyStoragePermissions(this);


        Log.d("LOGGG",getLOG_FNAME());


    }
}
