package com.example.igor.rc_temp;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;
import android.view.MotionEvent;

public class Joystick {
    private Paint color;
    private boolean return_backX,return_backY, block_X, block_Y;
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
        if (return_backX) {
            jx=0;
            old_posX=x+size*0.5f;
        }else
            old_posX-=shiftX;
        if (return_backY) {
            jy=0;
            old_posY=y+size*0.5f;
        }else
            old_posY-=shiftY;


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
        int pointerIndex = event.getActionIndex();
        // число касаний
        //  int pointerCount = event.getPointerCount();
        switch (actionMask) {
            case MotionEvent.ACTION_DOWN: // первое касание
            case MotionEvent.ACTION_POINTER_DOWN: // последующие касания
            {
                final float gx = event.getX(pointerIndex);
                final float gy = event.getY(pointerIndex);
                if (index<=0 && gx >= x && gx <= x + size && gy >= y && gy <= y + size) {
                    end();
                    index=pointerIndex;
                    shiftX = gx - old_posX;//??
                    shiftY = gy - old_posY;
                    trackX=old_posX=gx;
                    trackY=old_posY=gy;
                }
                break;
            }
            case MotionEvent.ACTION_UP: // прерывание последнего касания
                end();
                break;
            case MotionEvent.ACTION_POINTER_UP: // прерывания касаний
                if (index==event.getActionIndex()){
                    end();
                }
                break;
            case MotionEvent.ACTION_MOVE: // движение
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
                    end();
                }
                break;
        }
        if (block_X){
            jx=0;

        }
        if (block_Y){
            jy=0;

        }

      //  Log.d("JOSTIC",Float.toString(jx)+" , "+Float.toString(jy));
        return true;

    }

    Paint cc;

    public void set_return_back_X(boolean b){return_backX=b;}
    public void set_return_back_Y(boolean b){return_backY=b;}
    public void set_block_X(boolean b){block_X=b;}
    public void set_block_Y(boolean b){block_Y=b;}
    public Joystick(float _x, float _y, float _size, boolean return_back_X, boolean return_back_Y,boolean blockX, boolean blockY, Paint c) {
        x=_x;
        y=_y;
        size=_size;
        return_backX =return_back_X;
        return_backY=return_back_Y;
        block_X=blockX;
        block_Y=blockY;
        color=new Paint(c);
        color.setStyle(Paint.Style.STROKE);
        cc=new Paint(c);
        cc.setStrokeWidth(1);
        index=-1;
        end();
        jx = jy = 0;
        shiftY=shiftX=0;
        old_posX=x+size*0.5f;
        old_posY=y+size*0.5f;


    }

    public void paint(Canvas c) {



        c.drawCircle( block_X?x+size*0.5f:old_posX-shiftX, block_Y?y+size*0.5f:old_posY-shiftY , size/10, color);

        // c.drawLine(x+size/2,y,x+size/2,y+size,cc);
        // c.drawLine(x,y+size/2,x+size,y+size/2,cc);


        c.drawCircle(x+size*0.5f, y+size*0.5f , size*0.5f, color);


    }

}