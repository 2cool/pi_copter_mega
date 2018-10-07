package com.example.igor.rc_temp;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.MotionEvent;
import android.view.View;

public class DrawView extends View {




    Joystick j_left,j_right;

    static float sizeX = 0;
    static float sizeY = 0;


    Paint white = new Paint();
    Paint yellow = new Paint();

    public DrawView(Context context) {
        super(context);


        sizeX = MainActivity.screenMetrix[0] / MainActivity.screenMetrix[2];
        sizeY = MainActivity.screenMetrix[1] / MainActivity.screenMetrix[3];
        white.setColor(Color.WHITE);
        yellow.setColor(Color.YELLOW);
        j_left=new Joystick(100,200,400,false,white);
        j_right=new Joystick(800,200,400,true,yellow);
    }


    static public boolean thumbed = false;

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        j_left.onTouchEvent(event);
        j_right.onTouchEvent(event);
        // событие


        invalidate();
        return true;

    }


    static int i = 0;

    public void onDraw(Canvas c) {

        super.onDraw(c);

        j_left.paint(c);
        j_right.paint(c);

    }
}




