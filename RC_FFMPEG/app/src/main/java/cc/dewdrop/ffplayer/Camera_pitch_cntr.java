package cc.dewdrop.ffplayer;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.MotionEvent;

public class Camera_pitch_cntr {
    private int camera_pitch_index=-1;

    private   Rect camera_jesture_control;
    private Paint gray_opaq;

    private float old_y;

    public  Camera_pitch_cntr(final Rect r){
        camera_jesture_control=r;
        gray_opaq =new Paint();
        gray_opaq.setColor(Color.GRAY);
        //gray_opaq.setAlpha(30);
        gray_opaq.setStyle(Paint.Style.STROKE);
        gray_opaq.setStrokeWidth(1);
    }
    final int pixel2angle=11;
    private int buf2send=0;
    public float gimbal_pitch_add(float dy){
        int d_ang=(int)(dy/pixel2angle);
        buf2send+=d_ang;

        if (buf2send>0){
            MainActivity.camera_gimb_minus();
            buf2send--;
        }else if (buf2send<0){
            MainActivity.camera_gimb_plus();
            buf2send++;
        }
        return pixel2angle*d_ang;
    }


    public void paint(Canvas c){
        c.drawRect(camera_jesture_control,gray_opaq);
    }

    public boolean onTouchEvent(MotionEvent event) {
        int actionMask = event.getActionMasked();
        int index = event.getActionIndex();
        final float gx = event.getX(index);
        final float gy = event.getY(index);
        switch (actionMask) {
            case MotionEvent.ACTION_DOWN: // первое касание
            case MotionEvent.ACTION_POINTER_DOWN: // последующие касания
            {
                if (camera_pitch_index==-1 &&
                        gx>=camera_jesture_control.left && gx<=camera_jesture_control.right &&
                        gy>=camera_jesture_control.top && gy<=camera_jesture_control.bottom){
                    camera_pitch_index=index;
                    old_y=gy;
                }
                break;
            }
            case MotionEvent.ACTION_UP: // прерывание последнего касания
            case MotionEvent.ACTION_POINTER_UP: // прерывания касаний
                if (camera_pitch_index>=0)
                    camera_pitch_index=-1;
                break;
            case MotionEvent.ACTION_MOVE: // движение
                if (camera_pitch_index>=0){

                    old_y+=gimbal_pitch_add(gy-old_y);

                }
                break;
        }

        return true;
    }
}
