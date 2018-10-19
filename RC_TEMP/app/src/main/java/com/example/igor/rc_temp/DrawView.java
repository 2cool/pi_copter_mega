package com.example.igor.rc_temp;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class DrawView extends View {


    Joystick j_left,j_right;
    Img_button yaw_onoff,desc_onoff, pitch_onoff,roll_onoff,compass_onoff,settings;

    static float sizeX = 0;
    static float sizeY = 0;


    static Paint white = new Paint();

    public DrawView(Context context) {
        super(context);

        float sm[]=MainActivity.screenMetrics;

        white.setColor(Color.WHITE);
        white.setStrokeWidth(sm[2]/100);
        white.setAlpha(255);
        float bR=sm[1]*0.5f;
        float thumb=sm[1]/sm[3];
        float k=0.1f;
      //  j_left=new Joystick((int)(sm[2]*k),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,false,white);
      //  j_right=new Joystick((int)(sm[0]-sm[2]*(1+k)),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,true,white);

        j_left=new Joystick(bR*k,sm[1]-bR*(1+k),bR,true,true,false,false,white);
        j_right=new Joystick(sm[0]-bR*(1+k),sm[1]-bR*(1+k),bR,true,true,false,false,white);

        Rect r=new Rect(100,100,100+(int)(sm[2]/3f),100+(int)(sm[2]/3f));



        int bs=(int)(sm[2]/2.5);
        yaw_onoff=new Img_button(10,10,bs,
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        desc_onoff=new Img_button(50+bs,10,bs,
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);

        pitch_onoff=new Img_button(sm[0]-bs-10,10,bs,
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        roll_onoff=new Img_button(sm[0]-bs -bs-50,10,bs,
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);


        compass_onoff=new Img_button((int)((sm[0]-bs)*0.5),10,bs,
                context.getResources().getDrawable(R.drawable.compass_on),
                context.getResources().getDrawable(R.drawable.compass_off),true);
        int bs2= (int)(0.5*bs);
        settings=new Img_button((int)((sm[0]-bs2)*0.5),sm[1]-bs2-10,bs2,
                context.getResources().getDrawable(R.drawable.settings),
                context.getResources().getDrawable(R.drawable.settings),false);
    }


    static public boolean thumbed = false;

    @Override
    public boolean onTouchEvent(MotionEvent event) {


        yaw_onoff.onTouchEvent(event);
        desc_onoff.onTouchEvent(event);
        pitch_onoff.onTouchEvent(event);
        roll_onoff.onTouchEvent(event);

        j_left.onTouchEvent(event);
        j_right.onTouchEvent(event);
        compass_onoff.onTouchEvent(event);

        //  if (bt.pressed())
        //  Log.d("BUTTON","YES");
        // событие

        j_left.set_block_X(yaw_onoff.pressed());
        j_left.set_block_Y(desc_onoff.pressed());
        j_right.set_block_X(pitch_onoff.pressed());
        j_right.set_block_Y(roll_onoff.pressed());

        invalidate();
        return true;

    }


    static int i = 0;

    public void onDraw(Canvas c) {

        super.onDraw(c);

        yaw_onoff.paint(c);
        desc_onoff.paint(c);
        pitch_onoff.paint(c);
        roll_onoff.paint(c);
        compass_onoff.paint(c);
        settings.paint(c);

       // mCustomImage.draw(c);
        j_left.paint(c);
        j_right.paint(c);

    }
}




