package cc.dewdrop.ffplayer.myTools;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.Log;
import android.view.MotionEvent;

import cc.dewdrop.ffplayer.MainActivity;

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


   // Commander.fpv_zoom

    private int buf2send=0;
    public float gimbal_pitch_add(float dy,float fpv_zoom){
        final int pixel2angle=11;
        final double zoom=Math.max(1,Math.min(256,fpv_zoom))-1;
        final double _pixel2angle=pixel2angle*((zoom/28.3)+1);
      //  Log.d("ZOOMZ",Double.toString(_pixel2angle));
        final int d_ang=(int)(dy/_pixel2angle);
        buf2send+=d_ang;

        if (buf2send>0){
            MainActivity.camera_gimb_minus();
            buf2send--;
        }else if (buf2send<0){
            MainActivity.camera_gimb_plus();
            buf2send++;
        }
        return (float)_pixel2angle*d_ang;
    }


    public void paint(Canvas c){
        c.drawRect(camera_jesture_control,gray_opaq);
    }

    public boolean onTouchEvent(MotionEvent event, float fpv_zoom) {
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

                    old_y+=gimbal_pitch_add(gy-old_y, fpv_zoom);

                }
                break;
        }

        return true;
    }
}
