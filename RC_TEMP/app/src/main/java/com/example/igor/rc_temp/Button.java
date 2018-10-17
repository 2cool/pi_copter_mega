package com.example.igor.rc_temp;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;
import android.view.MotionEvent;

public class Button {



    private boolean press,pressDown;
    RectF r;
    Paint p,gr;
    static Paint black;
    String txt;
    private int border;
    private boolean toggle;
    public boolean pressed(){
        boolean pressed=press;
        if (toggle==false)
            press=false;
        return pressed;
    }
    public Button(float x, float y, float size, String text, Paint pnt, boolean toggle_){
        toggle=toggle_;
        press=pressDown=false;
        r = new RectF();
        border=15;
        p=pnt;
        black=new Paint(Color.BLACK);
        black.setTextSize(size);

        txt=text;
        Rect tr=new Rect();
        black.getTextBounds(text,0,txt.length(),tr);
        r.top=y;
        r.left=x;
        r.bottom=y+(tr.bottom-tr.top)+border*2;
        r.right=x+(tr.right-tr.left)+border*2;
        gr=new Paint(black);
        gr.setColor(Color.GREEN);
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
                pressDown = (gx>=r.left && gx<=r.right && gy>=r.top && gy<=r.bottom);

                break;
            }
            case MotionEvent.ACTION_UP: // прерывание последнего касания
            case MotionEvent.ACTION_POINTER_UP: // прерывания касаний
                final float gx = event.getX(pointerIndex);
                final float gy = event.getY(pointerIndex);
                press^= pressDown&(gx>=r.left && gx<=r.right && gy>=r.top && gy<=r.bottom);
                pressDown=false;
                break;
            case MotionEvent.ACTION_MOVE: // движение
                break;
        }
        return true;

    }

    public void paint(Canvas c) {
        // c.drawRect(r, p);
        c.drawRoundRect(r,6,6,p);
        c.drawText(txt, r.left+border-2, r.bottom-border, pressDown?gr:(toggle?(press?gr:black):black));

    }

}
