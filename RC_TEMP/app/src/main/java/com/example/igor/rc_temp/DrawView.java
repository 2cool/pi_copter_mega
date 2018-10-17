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
    Img_button yaw_onoff;

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
                context.getResources().getDrawable(R.drawable.yaw),
                context.getResources().getDrawable(R.drawable.yaw_off),true);




    }


    static public boolean thumbed = false;

    @Override
    public boolean onTouchEvent(MotionEvent event) {


        yaw_onoff.onTouchEvent(event);


        j_left.onTouchEvent(event);
        j_right.onTouchEvent(event);


        //  if (bt.pressed())
        //  Log.d("BUTTON","YES");
        // событие

        j_left.set_block_X(yaw_onoff.pressed());
        invalidate();
        return true;

    }


    static int i = 0;

    public void onDraw(Canvas c) {

        super.onDraw(c);

        yaw_onoff.paint(c);

       // mCustomImage.draw(c);
        j_left.paint(c);
        j_right.paint(c);

    }
}




