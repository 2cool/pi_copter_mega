package cc.dewdrop.ffplayer;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.view.MotionEvent;

public class Img_button {
    private Drawable imageOn,imageOff;

    private boolean press;
    private boolean thumb_on;
    private boolean toggle;
    private int status;


    public boolean pressed(){

        return press;
    }
    public int getStat(){
        int stat=status;
        if (status==3)
            status=0;
        return stat;
    }
    public boolean thumbON(){
        return thumb_on;
    }
    public void set(boolean b){
        press=b;
    }
    Rect r;
    private int id;
    public Img_button(Rect r_, Drawable imgOff_, Drawable imgOn_, boolean toggle_){
        imageOn=imgOn_;
        imageOff=imgOff_;

        r=r_;
        imageOn.setBounds(r);
        imageOff.setBounds(r);

        toggle=toggle_;
        press=thumb_on=false;
        id=-1;

    }

    public boolean onTouchEvent(MotionEvent event) {
        boolean ret=false;
        // событие
        int actionMask = event.getActionMasked();
        // индекс касания
        int pointerIndex = event.getActionIndex();
        // число касаний
        //  int pointerCount = event.getPointerCount();

        //  index=event.findPointerIndex(pointerIndex);
        final float gx = event.getX(pointerIndex);
        final float gy = event.getY(pointerIndex);
        switch (actionMask) {
            case MotionEvent.ACTION_DOWN: // первое касание
            case MotionEvent.ACTION_POINTER_DOWN: // последующие касания
            {

                if  (id<0){
                    if ( gx>=r.left && gx<=r.right && gy>=r.top && gy<=r.bottom) {
                        press^=true;
                        thumb_on=true;
                        status=1;
                        id= event.getPointerId(pointerIndex);
                        ret=true;
                    }


                }
                break;
            }
            case MotionEvent.ACTION_UP: // прерывание последнего касания
            case MotionEvent.ACTION_POINTER_UP: // прерывания касаний


                if (id==event.getPointerId(pointerIndex)) {
                    if (gx >= r.left && gx <= r.right && gy >= r.top && gy <= r.bottom) {
                        press ^= (toggle)?false:true;//pressDown & true;
                        thumb_on=false;
                        id=-1;
                        status=3;
                        ret=true;
                    }

                }
                break;
            case MotionEvent.ACTION_MOVE: // движение

                if (id==event.getPointerId(pointerIndex)) {
                    if (!(gx >= r.left && gx <= r.right && gy >= r.top && gy <= r.bottom)) {
                        press ^= (toggle)?false:true;//pressDown & true;
                        thumb_on=false;
                        id=-1;
                        status=2;
                        ret=true;
                    }

                }
                break;
        }
        return ret;

    }

    public void paint(Canvas c) {


        (press?imageOn:imageOff).draw(c);


    }
}