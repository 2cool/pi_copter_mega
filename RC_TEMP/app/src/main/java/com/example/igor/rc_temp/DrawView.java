package com.example.igor.rc_temp;

import android.content.Context;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class DrawView extends View {
    static public float sm[];
    BatteryMonitor batMon;
    Joystick j_left,j_right;
    static public Img_button yaw_onoff,desc_onoff, pitch_onoff,roll_onoff,compass_onoff,settings;
    static public Img_button control_type;
    static public Img_button[]on_off=new Img_button[2];
    static public float maxAngle=35;
    Monitor monitor;

    static float sizeX = 0;
    static float sizeY = 0;

    Drawable connectedImg;
    static Paint white = new Paint();

    /*
    |1 2 3 4 ....... -4 -3 -2 -1|
     if need middle pos then 0


     */
    Rect getRect(double x, double y){
        int addX=0;
        int addY=0;
        double size=sm[2]/3.8;//1sm
        double border=size*0.3;
        double maxX=Math.floor(sm[0]/(border+size));
        double maxY=Math.floor(sm[1]/(border+size));
        double borderX=(sm[0]-maxX*size)/(maxX+1);
        double borderY=(sm[1]-maxY*size)/(maxY+1);
        if (x<0)
            x=maxX+x;
        else
            if (x>0)
                x--;
        else {
                x = Math.floor(maxX / 2 - 1);
                if (maxX/2==Math.ceil(maxX/2)) {
                    addX=(int)(size/2+borderX);
                }else{
                    x++;
                }
            }
        if (y<0)
            y=maxY+y;
        else
            if (y>0)
                y--;
        else
            {
                y = Math.floor(maxY / 2 - 1);
                if (maxY/2==Math.ceil(maxY/2)) {
                    addY=(int)(size/2+borderY);
                }else{
                    y++;
                }
            }

        int x0=addX+(int)(x*(size+borderX)+borderX);
        int y0=addY+(int)(y*(size+borderY)+borderY);
        return new Rect(x0,y0,(int)(x0+size),(int)(y0+size));
    }

//--------------------------------------------------------------------------------------------
    public DrawView(Context context) {
        super(context);

        sm=MainActivity.screenMetrics;

        white.setColor(Color.WHITE);
        white.setStrokeWidth(sm[2]/100);
        white.setAlpha(255);
        float bR=sm[2]*1.2f;
        float thumb=sm[1]/sm[3];
        float k=0.1f;
      //  j_left=new Joystick((int)(sm[2]*k),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,false,white);
      //  j_right=new Joystick((int)(sm[0]-sm[2]*(1+k)),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,true,white);
        final float monSize=bR/2.5f;
        monitor =new Monitor((int)(sm[0]/2),(int)(sm[1]-monSize/2),(int)(monSize),
                BitmapFactory.decodeResource(getResources(), R.drawable.angle),
                BitmapFactory.decodeResource(getResources(), R.drawable.cmps));

        batMon=new BatteryMonitor((float)((sm[0]-monSize)/2),4,monSize);

        j_left=new Joystick(bR*k,sm[1]-bR*(1+k),bR,true,false,false,false,white);
        j_right=new Joystick(sm[0]-bR*(1+k),sm[1]-bR*(1+k),bR,true,true,false,false,white);
        j_right.setLabel(Integer.toString((int)maxAngle));

      ///  Rect r=new Rect(100,100,100+(int)(sm[2]/3f),100+(int)(sm[2]/3f));


        final int bs=(int)(sm[2]/3);
        final int bs2= (int)(0.5*bs);



        connectedImg=context.getResources().getDrawable(R.drawable.wifi_on);
        connectedImg.setBounds(getRect(0,1));




        pitch_onoff=new Img_button(getRect(-1,-5),
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        roll_onoff=new Img_button(getRect(-2,-5),
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);



        yaw_onoff=new Img_button(getRect(1,-5),
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        desc_onoff=new Img_button(getRect(2,-5),
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);



        control_type=new Img_button(getRect(4,1),
                context.getResources().getDrawable(R.drawable.touch),
                context.getResources().getDrawable(R.drawable.gyro),true);


        compass_onoff=new Img_button(getRect(5,1),
                context.getResources().getDrawable(R.drawable.compass_on),
                context.getResources().getDrawable(R.drawable.compass_off),true);

        settings=new Img_button(getRect(6,1),
                context.getResources().getDrawable(R.drawable.settings),
                context.getResources().getDrawable(R.drawable.settings),false);



        on_off[0]=new Img_button(getRect(1,1),
                context.getResources().getDrawable(R.drawable.green),
                context.getResources().getDrawable(R.drawable.red),true);

        on_off[1]=new Img_button(getRect(-1,1),
                context.getResources().getDrawable(R.drawable.green),
                context.getResources().getDrawable(R.drawable.red),true);






    }

    boolean power_on[]={false,false,false};

    long power_time[]={0,0};
   int old_telemetry_counter=0;
    void testPowerButtons(){
        final int timeout = 100;
        for (int i=0; i<=1; i++)
            if (power_time[i]==0 && on_off[i].thumbON() && on_off[i].pressed()!=power_on[i]){
                power_time[i]=System.currentTimeMillis();
                power_on[i]^=true;
            }
       if (power_on[0]==power_on[1] && power_on[0]!=power_on[2] && power_time[0]>0 && power_time[1]>0 && Math.abs(power_time[0]-power_time[1])<timeout){
            power_on[2]^=true;
           MainActivity.command_bits_^=MainActivity.MOTORS_ON;
            power_time[0]=power_time[1]=0;
           old_telemetry_counter=Telemetry.telemetry_couter;


       }else{
           for (int i=0; i<=1; i++)
                if (power_time[i]>0 && System.currentTimeMillis()-power_time[i]>timeout){
                    power_time[i]=0;
                    power_on[i]=power_on[2];
                    on_off[i].set(power_on[i]);
                }
       }
       if (power_time[0]==0 && power_time[1]==0 && Telemetry.telemetry_couter-old_telemetry_counter>1){
            boolean f=((MainActivity.control_bits&MainActivity.MOTORS_ON)>0);
           // power_time[2]=0;
           on_off[0].set(f);
           on_off[1].set(f);
       }
    }

    static public boolean thumbed = false;

    @Override
    public boolean onTouchEvent(MotionEvent event) {


        yaw_onoff.onTouchEvent(event);
        desc_onoff.onTouchEvent(event);
        pitch_onoff.onTouchEvent(event);
        roll_onoff.onTouchEvent(event);
        control_type.onTouchEvent(event);
        on_off[0].onTouchEvent(event);
        on_off[1].onTouchEvent(event);
        j_left.onTouchEvent(event);
        j_right.onTouchEvent(event);
        compass_onoff.onTouchEvent(event);

        //  if (bt.pressed())
        //  Log.d("BUTTON","YES");
        // событие

        j_left.set_block_X(yaw_onoff.pressed());
        j_left.set_block_Y(desc_onoff.pressed());
        //
        if (control_type.pressed()==false) {
            j_right.set_block_X(pitch_onoff.pressed());
            j_right.set_block_Y(roll_onoff.pressed());
        }


       // invalidate();
        return true;

    }


    static int i = 0;
float angle;

    //-------------------------------------------------------------------------------------------
    void setMonitor(){
        monitor.setSpeed(Telemetry.speed);
        monitor.setRoll(-Telemetry.roll);
        monitor.setPitch(Telemetry.pitch);
        monitor.setYaw(yaw);//Telemetry.heading);
        monitor.setHeight(Telemetry._alt);
    }
//-------------------------------------------------------------------------------------------
    float pitch,roll,yaw,speed,hight;


    public void onDraw(Canvas c) {

        super.onDraw(c);
        //



        testPowerButtons();



        if (Commander.link ){
            connectedImg.draw(c);
        }
        if (control_type.pressed()) {
            j_right.setJosticY((float) (MainActivity.pitch / maxAngle));
            j_right.setJosticX((float) (MainActivity.roll  / maxAngle));
        }
        batMon.setVoltage(0.25f*Telemetry.batVolt);


        Commander.throttle=0.5f+(j_left.getY())/2;
        yaw+=j_left.getX()*0.360*MainActivity.updateTimeMsec;
        while (yaw>360) yaw-=360;
        while (yaw<-360)yaw+=360;
        Commander.roll=j_right.getX()*maxAngle;
        Commander.pitch=-j_right.getY()*maxAngle;
        Commander.heading=yaw;

        Log.d("COMM","Yaw="+Float.toString(yaw)+". Roll="+Float.toString(Commander.roll));
       // pitch+=(j_right.getY()*maxAngle-pitch)*0.03f;
       // speed=-pitch;

       // roll+=(j_right.getX()*maxAngle-roll)*0.03f;
      //  yaw+=j_left.getX()*3;
      //  hight-=j_left.getY()*0.1;

        setMonitor();




        monitor.paint(c);
        yaw_onoff.paint(c);
        desc_onoff.paint(c);
        pitch_onoff.paint(c);
        roll_onoff.paint(c);
        compass_onoff.paint(c);
        settings.paint(c);
        control_type.paint(c);
        j_left.paint(c);
        j_right.paint(c);
        on_off[0].paint(c);
        on_off[1].paint(c);
        batMon.paint(c);

    }






}




