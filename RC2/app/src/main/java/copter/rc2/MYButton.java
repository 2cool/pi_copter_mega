package copter.rc2;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;
import android.view.MotionEvent;

import java.util.concurrent.Callable;

/**
 * Created by igor on 8/20/2016.
 */
public class MYButton {
    private Paint red= new Paint(Paint.ANTI_ALIAS_FLAG);
    static private Paint black=new Paint(Paint.ANTI_ALIAS_FLAG);

    private int x, y,r;
    private String text;
    private boolean pressed=false;
    public boolean pressed(){
        boolean ret=pressed;
        pressed=false;
        return ret;
    }
    private boolean down=false;
    public MYButton(final int x, final int y, final int r,String t, int color){
        this.x=x;
        this.y=y;
        this.r=r;
        this.text=t;
        black.setColor(Color.BLACK);
        black.setTextSize(20);
        black.setAlpha(50);
        red.setColor(color);
        red.setAlpha(100);
       // red.setTextSize(20);




    }


    public boolean onTouchEvent(MotionEvent event) {
        int pIndex = event.getActionIndex();
        //p=new Point[event.getPointerCount()];
        // get pointer ID
        int pointerId = event.getPointerId(pIndex);
        // get masked (not specific to a pointer) action
        int maskedAction = event.getActionMasked();
        int dx = Math.abs(x - (int) event.getX(pIndex));
        int dy = Math.abs(y - (int) event.getY(pIndex));
        int r2 = dx * dx + dy * dy;
        boolean onButton= r2 <= (r+10) * (r+10);

        switch (maskedAction) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                if (onButton) {
                    down = onButton;
                    pressed = false;
                    return true;
                }
                break;
            case MotionEvent.ACTION_MOVE:
                if (down){
                    if (onButton==false)
                        down=false;
                    return true;
                }
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
            case MotionEvent.ACTION_CANCEL:

                 if (onButton && down){
                     pressed=true;
                     down=false;
                     return true;
                 }
                down=false;
                break;
        }
        return false;
    }


    public void draw(final Canvas c){

        c.drawCircle(x,y,r,red);
        if (down){
            c.drawCircle(x,y,r+10,red);

        }
        c.drawText(text,x-20,y+8,black);
    }
}
