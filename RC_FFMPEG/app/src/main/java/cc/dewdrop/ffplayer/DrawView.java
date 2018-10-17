package cc.dewdrop.ffplayer;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class DrawView extends View {




    cc.dewdrop.ffplayer.Joystick j_left,j_right;
  //  Button bt,bt2;

    static float sizeX = 0;
    static float sizeY = 0;


    static Paint white = new Paint();

    public DrawView(Context context) {
        super(context);

        float sm[]=MainActivity.screenMetrics;

        white.setColor(Color.WHITE);
        white.setStrokeWidth(sm[2]/100);
        white.setAlpha(100);

        float k=0.1f;
        j_left=new Joystick((int)(sm[2]*k),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,false,white);
        j_right=new Joystick((int)(sm[0]-sm[2]*(1+k)),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,true,white);



       // bt=new Button(500,100,(int)sm[3]/7,"Start",white,false);

      //  bt2=new Button(800,100,(int)sm[3]/7,"start",white,true);

    }


    static public boolean thumbed = false;

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        j_left.onTouchEvent(event);
        j_right.onTouchEvent(event);

       // bt.onTouchEvent(event);
       // bt2.onTouchEvent(event);



        //  if (bt.pressed())
        //  Log.d("BUTTON","YES");
        // событие


        invalidate();
        return true;

    }


    static int i = 0;

    public void onDraw(Canvas c) {

        super.onDraw(c);

        j_left.paint(c);
        j_right.paint(c);
       // bt.paint(c);
       // bt2.paint(c);

    }
}




