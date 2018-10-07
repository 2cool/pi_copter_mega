package cc.dewdrop.ffplayer.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;

import cc.dewdrop.ffplayer.MainActivity;


public class DrawView extends View {


    static float[] mouse_x, mouse_y;



    int upPI = 0;
    int downPI = 0;
    static boolean inTouch = false;





    static float sizeX=0;
    static float sizeY=0;


    Paint white=new Paint();
    Paint black=new Paint();
    public DrawView(Context context) {
        super(context);
        mouse_x = new float[10];
        mouse_y=new float[10];

        sizeX = MainActivity.screenMetrix[0] / MainActivity.screenMetrix[2];
        sizeY = MainActivity.screenMetrix[1] / MainActivity.screenMetrix[3];

    }


    static public boolean thumbed =false;
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        // событие
        int actionMask = event.getActionMasked();
        // индекс касания
        int pointerIndex = event.getActionIndex();
        // число касаний
        int pointerCount = event.getPointerCount();

        switch (actionMask) {
            case MotionEvent.ACTION_DOWN: // первое касание
                inTouch = true;
            case MotionEvent.ACTION_POINTER_DOWN: // последующие касания
                downPI = pointerIndex;
                break;

            case MotionEvent.ACTION_UP: // прерывание последнего касания
                inTouch = false;
               // sb.setLength(0);
            case MotionEvent.ACTION_POINTER_UP: // прерывания касаний
                upPI = pointerIndex;
                break;

            case MotionEvent.ACTION_MOVE: // движение
                //sb.setLength(0);

                for (int i = 0; i < 10; i++) {
                   // sb.append("Index = " + i);
                    if (i < pointerCount) {
                       // sb.append(", ID = " + event.getPointerId(i));
                        mouse_x[i]=event.getX(i);
                        mouse_y[i]=event.getY(i);

                    } else {

                    }

                }
                break;
        }


        if (inTouch) {

        }


        invalidate();
        return true;

    }















static int i=0;

    public void onDraw(Canvas c) {

        super.onDraw(c);



        white.setColor(Color.WHITE);
        black.setColor(Color.BLACK);
        c.drawCircle(mouse_x[0]    , mouse_y[0]-400, 10, white);
        c.drawCircle(mouse_x[1]    , mouse_y[1]-400, 10, white);


        i+=1;
    }






}
