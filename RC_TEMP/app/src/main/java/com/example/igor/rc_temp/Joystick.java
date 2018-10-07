package com.example.igor.rc_temp;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;
import android.view.MotionEvent;

public class Joystick {
    private Paint color;
    private boolean return_back;
    private float x,y,size;

    private float trackX,trackY,old_posX,old_posY,shiftX,shiftY;
    private float jx,jy;
    private int index;

    public float getX(){return jx;}
    public float getY(){return jy;}
    private boolean setX(float xp){
        float t_jx=(xp-x - size*0.5f)/(size*0.5f);
        if (t_jx<=1 && t_jx>=-1){
            jx=t_jx;
            return true;
        }else
            return false;
    }
    private boolean setY(float yp){
        float t_jy=(yp-y-size*0.5f)/(size*0.5f);
        if (t_jy<=1 && t_jy>=-1){
            jy=t_jy;
            return true;
        }else
            return false;
    }
    private void end(){
        index=-1;
        if (return_back) {

            jx = jy = 0;
            old_posX=x+size*0.5f;
            old_posY=y+size*0.5f;
        }else{
            old_posX-=shiftX;
            old_posY-=shiftY;

        }
        shiftX=shiftY=0;

    }
    private int findPointerIndex(MotionEvent event){
        float min=10e24f;
        int index=-1;
        for (int i=0; i< event.getPointerCount(); i++){
            final float px = event.getX(i);
            final float py = event.getY(i);
            final float dx=px-trackX;
            final float dy=py-trackY;
            final float dist2=dx*dx+dy*dy;
            if (dist2<min){
                index=i;
                min=dist2;
            }
        }
        if (min>10000)
            index=-1;
        return index;

    }


    public boolean onTouchEvent(MotionEvent event) {

        // событие
        int actionMask = event.getActionMasked();
        // индекс касания
     //   int pointerIndex = event.getActionIndex();
        // число касаний
        int pointerCount = event.getPointerCount();


        switch (actionMask) {
            case MotionEvent.ACTION_DOWN: // первое касание
                Log.d("TOUCH","ACTION_DOWN");
            case MotionEvent.ACTION_POINTER_DOWN: // последующие касания
            {

                Log.d("TOUCH","ACTION_POINTER_DOWN");



                final float gx = event.getX(pointerCount - 1);
                final float gy = event.getY(pointerCount - 1);
                index=findPointerIndex(event);

                if (index==-1 && gx >= x && gx <= x + size && gy >= y && gy <= y + size) {
                    end();
                    index=pointerCount - 1;
                    shiftX = gx - old_posX;//??
                    shiftY = gy - old_posY;
                    trackX=old_posX=gx;
                    trackY=old_posY=gy;
                }
                break;
            }
            case MotionEvent.ACTION_UP: // прерывание последнего касания
                Log.d("TOUCH","ACTION_UP");
            case MotionEvent.ACTION_POINTER_UP: // прерывания касаний
                Log.d("TOUCH", "ACTION_POINTER_UP");

               // index=findPointerIndex(event);

                break;
            case MotionEvent.ACTION_MOVE: // движение

                //sb.setLength(0);

                if (index>=0)
                    index=findPointerIndex(event);
                if (index >= 0) {
                    trackX = event.getX(index);
                    trackY = event.getY(index);

                    if (setX(trackX - shiftX)) {
                        old_posX = trackX;
                    }
                    if (setY(trackY - shiftY))
                        old_posY = trackY;
                }else{
                    Log.d("TOUCH", "-1");
                    end();
                }

                break;
        }




        return true;

    }


    public Joystick(int _x, int _y, int _size, boolean return_back_, Paint c) {
        x=_x;
        y=_y;
        size=_size;
        return_back =return_back_;
        color=c;
        index=-1;
       end();
        jx = jy = 0;
        shiftY=shiftX=0;
        old_posX=x+size*0.5f;
        old_posY=y+size*0.5f;


    }

    public void paint(Canvas c) {
        c.drawCircle(old_posX-shiftX, old_posY-shiftY , 50, color);



        c.drawLine(x, y,x+size, y,color);
        c.drawLine(x,y,x,y+size,color);
        c.drawLine(x, y+size,x+size, y+size,color);
        c.drawLine(x+size,y,x+size,y+size,color);
    }

}
