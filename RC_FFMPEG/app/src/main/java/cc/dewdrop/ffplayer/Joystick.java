package cc.dewdrop.ffplayer;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;
import android.view.MotionEvent;

public class Joystick {
    private Paint color;
    private boolean return_backX,return_backY;
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
                    Log.d("JOSTIC",Float.toString(jx)+" , "+Float.toString(jy));
                }else{
                    end();
                }
                break;
        }
        return true;

    }

    Paint cc;
    public Joystick(int _x, int _y, int _size, boolean return_back_X, boolean return_back_Y, Paint c) {
        x=_x;
        y=_y;
        size=_size;
        return_backX =return_back_X;
        return_backY=return_back_Y;
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
        c.drawCircle(old_posX-shiftX, old_posY-shiftY , size/10, color);

        // c.drawLine(x+size/2,y,x+size/2,y+size,cc);
        // c.drawLine(x,y+size/2,x+size,y+size/2,cc);


        c.drawCircle(x+size*0.5f, y+size*0.5f , size*0.5f, color);


    }

}
