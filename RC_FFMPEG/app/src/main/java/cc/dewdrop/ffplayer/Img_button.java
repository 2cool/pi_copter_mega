package cc.dewdrop.ffplayer;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.MotionEvent;

public class Img_button {
    private Drawable imageOn,imageOff;

    private boolean pressed;
    private boolean thumb_on;
    private boolean toggle;
    private int status;
    private boolean enabled=true;
    private long last_time=0;

    public boolean is_pressed(){

        return pressed;
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
        status=0;
        thumb_on=false;
        pressed=b;
    }
    Rect r;
    private int id;
    public void enabled(boolean b){
        enabled=b;
        if (b==false) {
            status=0;
            pressed=false;
            thumb_on=false;
            imageOn.setAlpha(60);
            imageOff.setAlpha(60);
        }else{
            imageOn.setAlpha( 255);
            imageOff.setAlpha(255);
        }
    }
    public Img_button(Rect r_, Drawable imgOff_, Drawable imgOn_, boolean toggle_){

        imageOn= imgOn_.mutate().getConstantState().newDrawable();
        imageOff= imgOff_.mutate().getConstantState().newDrawable();

        r=r_;
        imageOn.setBounds(r);
        imageOff.setBounds(r);
        if (toggle_==false){
            imageOn.getBounds().top+=2;
            imageOn.getBounds().left+=2;
            imageOn.getBounds().bottom-=2;
            imageOn.getBounds().right-=2;


        }
        toggle=toggle_;
        pressed=thumb_on=false;
        id=-1;

    }

    public boolean onTouchEvent(MotionEvent event) {
        if (enabled==false || System.currentTimeMillis()-last_time<300)
            return true;
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
                        thumb_on=true;
                        status=1;
                        id= event.getPointerId(pointerIndex);
                        ret=true;
                        Log.d("IMBNT","down");
                    }


                }
                break;
            }
            case MotionEvent.ACTION_UP: // прерывание последнего касания
            case MotionEvent.ACTION_POINTER_UP: // прерывания касаний


                if (id==event.getPointerId(pointerIndex)) {
                    if (gx >= r.left && gx <= r.right && gy >= r.top && gy <= r.bottom) {
                        last_time=System.currentTimeMillis();
                        if (toggle)
                            pressed^=true;
                        thumb_on=false;
                        id=-1;
                        status=3;
                        ret=true;
                        Log.d("BUTTON","up");
                    }else
                        status=0;

                }
                break;
            case MotionEvent.ACTION_MOVE: // движение

                if (id==event.getPointerId(pointerIndex)) {
                    if (!(gx >= r.left && gx <= r.right && gy >= r.top && gy <= r.bottom)) {
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


        (pressed?imageOn:imageOff).draw(c);


    }
}
