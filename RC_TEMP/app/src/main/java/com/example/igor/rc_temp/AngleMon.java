package com.example.igor.rc_temp;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;

public class AngleMon {
    Paint white;
    private float pitch, roll;
    private float xpos,ypos;
    private Bitmap bm;
    public void setPitch(float a){
        pitch=a;
    }
    public void setRoll(float a){
        roll=a;
    }
    private float scale;
    public AngleMon(int x, int y,int size, Bitmap bm_){
       // float sm[]=MainActivity.screenMetrics;
        scale=(float)size / bm_.getWidth();
        xpos=x;
        ypos=y;
        pitch=roll=0;
        bm=bm_;
        white = new Paint();
        white.setColor(Color.WHITE);
      //  white.setStrokeWidth(sm[2]/100);
     //   white.setAlpha(255);
    }



  //  static float angle=0;//115;
    private Matrix matrix = new Matrix();
    public void paint(Canvas c) {


        matrix.reset();
       // angle+=1;
       // pitch=angle;
       // roll=angle;
       // pitch=0;

       // if (roll>270)
       //     roll=360-roll;
        if (roll>90) {
            roll = 180 - roll;
            pitch+=180;
        }
        else if (roll<-90) {
            roll = 180 + roll;
            pitch+=180;
        }
        matrix.postRotate(pitch);



        Bitmap cropped = Bitmap.createBitmap(bm, 0, 614+(int)(4.8*(roll)), bm.getWidth(), bm.getWidth(), matrix, false);
        matrix.reset();
        matrix.postScale(scale,scale);
        Bitmap cropped2=Bitmap.createBitmap(cropped,
                (cropped.getHeight()-bm.getWidth())/2,
                (cropped.getWidth()-bm.getWidth())/2,bm.getWidth(),
                bm.getWidth(),matrix,false);
        c.drawBitmap(cropped2,xpos-cropped2.getWidth()/2,ypos-cropped2.getHeight()/2,white);



        c.drawLine(xpos-50,ypos,xpos+50,ypos,white);
        c.drawLine(xpos,ypos-15,xpos,ypos+15,white);
    }


}
