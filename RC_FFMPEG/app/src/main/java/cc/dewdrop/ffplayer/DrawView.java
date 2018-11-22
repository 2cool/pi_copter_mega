package cc.dewdrop.ffplayer;
import android.content.Context;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;

import cc.dewdrop.ffplayer.myTools.BatteryMonitor;
import cc.dewdrop.ffplayer.myTools.Camera_pitch_cntr;
import cc.dewdrop.ffplayer.myTools.Img_button;
import cc.dewdrop.ffplayer.myTools.Joystick;
import cc.dewdrop.ffplayer.myTools.Monitor;
import cc.dewdrop.ffplayer.myTools.Square_Cells;

public class DrawView extends View {




    static public double wrap_180(double x) {return (x < -180 ? x+360 : (x > 180 ? x - 360: x));}
    public static ScaleGestureDetector mScaleGestureDetector;

    final static int  viewMain=0;
    final static int  viewMenu=1;

    static public Camera_pitch_cntr cam_p_c;
    static public Paint black;
    static public float sm[];
    BatteryMonitor batMon;
    static Joystick j_left,j_right;
    static public Img_button yaw_onoff,desc_onoff, pitch_onoff,roll_onoff,compass_onoff,menu;
    static public Img_button control_type,showMap,showSettings,hold_alt,smart_ctrl,extra_buttons;
    static public Img_button[]on_off=new Img_button[2];
    static public float maxAngle=35;
    Monitor monitor;
    static private int screen=viewMain;

    static public Img_button exitMenu,exitProg,reboot,shutdown,comp_calibr,comp_m_calibr,gps_on_off;
    static public Img_button  fpv,vrc,photo;

    static Paint green_c = new Paint();

    static public boolean is_on_screen_the_menu(){
        return screen==viewMenu;
    }
    static public void turn2MainScreen(){
        screen=viewMain;
    }

    Square_Cells sc;




    static long updateTime=0;
    //static private int old_control_bits=0;
    public static void setButtons(){
        if (System.currentTimeMillis()>updateTime) {
            updateTime=System.currentTimeMillis()+100;


            cam_p_c.gimbal_pitch_add(0,Commander.fpv_zoom);//update

            on_off[0].enabled(Commander.link);
            on_off[1].enabled(Commander.link);
            if (on_off[0].is_pressed()==on_off[1].is_pressed()) {
                on_off[0].set(MainActivity.motorsOnF());
                on_off[1].set(MainActivity.motorsOnF());
            }

            smart_ctrl.set(MainActivity.smartCntrF());
            hold_alt.set(MainActivity.altHoldF());
            j_left.set_return_back_Y(MainActivity.altHoldF());

            //  b_toHome.setChecked((control_bits & GO2HOME) != 0);


            //   cb_horizont.setChecked((control_bits & HORIZONT_ON)!=0);
            //  cb_compass.setChecked((control_bits & COMPASS_ON)!=0);
            //  b_prog.setChecked((control_bits & PROGRAM)!=0);


        }
    }





    void menu_DrawView(final Context context){


        sc=new Square_Cells(9,0,0.3f,sm);
        int nX=sc.getMaxX();
        int nY=sc.getMaxY();
        exitMenu =new Img_button(sc.getRect(0,0),
                context.getResources().getDrawable(R.drawable.left),
                context.getResources().getDrawable(R.drawable.left),false);

        exitProg=new Img_button(sc.getRect(0,1),
                context.getResources().getDrawable(R.drawable.exit),
                context.getResources().getDrawable(R.drawable.exit),false);

        reboot=new Img_button(sc.getRect(nX-1,1),
                context.getResources().getDrawable(R.drawable.reboot),
                context.getResources().getDrawable(R.drawable.reboot),false);

        shutdown=new Img_button(sc.getRect(nX-1,0),
                context.getResources().getDrawable(R.drawable.shutdown),
                context.getResources().getDrawable(R.drawable.shutdown),false);
        comp_calibr=new Img_button(sc.getRect(nX/2,0),
                context.getResources().getDrawable(R.drawable.compass_on),
                context.getResources().getDrawable(R.drawable.compass_on),true);
        comp_m_calibr=new Img_button(sc.getRect(nX/2,1),
                context.getResources().getDrawable(R.drawable.comp_m_c),
                context.getResources().getDrawable(R.drawable.comp_m_c),true);
        gps_on_off=new Img_button(sc.getRect(nX/2,2),
                context.getResources().getDrawable(R.drawable.gps_off),
                context.getResources().getDrawable(R.drawable.gps),true);



        showMap =new Img_button(sc.getRect(nX/2,3),
                context.getResources().getDrawable(R.drawable.route),
                context.getResources().getDrawable(R.drawable.route),false);
        showSettings=new Img_button(sc.getRect(1,0),
                context.getResources().getDrawable(R.drawable.settings),
                context.getResources().getDrawable(R.drawable.settings),false);

    }
    void main_DrawView(final Context context){
        float bR=sm[2]*1.2f;
        float thumb=sm[1]/sm[3];
        float k=0.1f;
        //  j_left=new Joystick((int)(sm[2]*k),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,false,white);
        //  j_right=new Joystick((int)(sm[0]-sm[2]*(1+k)),(int)(sm[1]-sm[2]*(1+k)),(int)sm[2],true,true,white);
        final float monSize=bR/2.5f;


        sc=new Square_Cells(13,0,0.3f,sm);
        int nX=sc.getMaxX();
        int nY=sc.getMaxY();
        cam_p_c=new Camera_pitch_cntr(new Rect((int)(sm[0]/2-sm[0]/7),(int)(sm[2]/2.4),(int)(sm[0]/2+sm[0]/7),(int)(sm[1])));




        monitor =new Monitor((int)(sm[0]/2),(int)(sm[1]-monSize/2),(int)(monSize),
                BitmapFactory.decodeResource(getResources(), R.drawable.angle),
                BitmapFactory.decodeResource(getResources(), R.drawable.cmps));

        batMon=new BatteryMonitor((float)((sm[0]-monSize)/2),(int)(sm[1]-monSize*1.7),monSize);

        final int  jbuttonsY=(int)(nY-bR/(sm[1]/nY))-1;
        j_left=new Joystick(bR*k,sm[1]-bR*(1+k),bR,true,false,false,false,green_c);
        j_right=new Joystick(sm[0]-bR*(1+k),sm[1]-bR*(1+k),bR,true,true,false,false,green_c);
        j_right.setLabel(Integer.toString((int)maxAngle));

        ///  Rect r=new Rect(100,100,100+(int)(sm[2]/3f),100+(int)(sm[2]/3f));


        final int bs=(int)(sm[2]/3);
        final int bs2= (int)(0.5*bs);

        pitch_onoff=new Img_button(sc.getRect(nX-2,jbuttonsY),
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        roll_onoff=new Img_button(sc.getRect(nX-1,jbuttonsY),
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);



        yaw_onoff=new Img_button(sc.getRect(1,jbuttonsY),
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        desc_onoff=new Img_button(sc.getRect(0,jbuttonsY),
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);



        control_type=new Img_button(sc.getRect(1,0),
                context.getResources().getDrawable(R.drawable.touch),
                context.getResources().getDrawable(R.drawable.gyro),true);


        compass_onoff=new Img_button(sc.getRect(2,0),
                context.getResources().getDrawable(R.drawable.compass_on),
                context.getResources().getDrawable(R.drawable.compass_off),true);

        fpv =new Img_button(sc.getRect(3,0),
                context.getResources().getDrawable(R.drawable.fpv_off),
                context.getResources().getDrawable(R.drawable.fpv),true);
        vrc =new Img_button(sc.getRect(4,0),
                context.getResources().getDrawable(R.drawable.vrc_off),
                context.getResources().getDrawable(R.drawable.vrc_on),true);
        photo =new Img_button(sc.getRect(5,0),
                context.getResources().getDrawable(R.drawable.photo),
                context.getResources().getDrawable(R.drawable.photo),false);

        menu=new Img_button(sc.getRect(nX-4,0),
                context.getResources().getDrawable(R.drawable.menu),
                context.getResources().getDrawable(R.drawable.menu),false);



        on_off[0]=new Img_button(sc.getRect(0,0),
                context.getResources().getDrawable(R.drawable.green),
                context.getResources().getDrawable(R.drawable.red),true);
        //on_off[0].enabled(false);

        on_off[1]=new Img_button(sc.getRect(nX-1,0),
                context.getResources().getDrawable(R.drawable.green),
                context.getResources().getDrawable(R.drawable.red),true);
        //on_off[0].enabled(false);

        hold_alt=new Img_button(sc.getRect(nX-3,0),
                context.getResources().getDrawable(R.drawable.alt_hold_offt),
                context.getResources().getDrawable(R.drawable.alt_hold_on),true);
        hold_alt.set(true);

        smart_ctrl=new Img_button(sc.getRect(nX-2,0),
                context.getResources().getDrawable(R.drawable.smart_off),
                context.getResources().getDrawable(R.drawable.smart_on),true);
        smart_ctrl.set(true);
        extra_buttons=new Img_button(sc.getRect(nX/2,0),
                context.getResources().getDrawable(R.drawable.circle),
                context.getResources().getDrawable(R.drawable.extra_buttons),true);
        extra_buttons.set(true);



    }
    //--------------------------------------------------------------------------------------------
    public DrawView(Context context) {
        super(context);

        black=new Paint();
        black.setColor(Color.BLACK);

        sm=MainActivity.screenMetrics;

        green_c.setColor(Color.GREEN);
        green_c.setStrokeWidth(sm[2]/100);
        green_c.setAlpha(255);

        main_DrawView(context);
        menu_DrawView(context);


    }




    private void fpv_start_stop(){
        if (fpv.is_pressed()) {
            MainActivity.startVideo();
            String myIP=Net.getIpAddress();
            Commander.fpv_addr=(byte)Integer.parseInt(myIP.substring(myIP.lastIndexOf('.')+1));
            Commander.fpv_port=5544;
            Commander.fpv_zoom=1;
            Commander.fpv=true;
        }else {
            MainActivity.stopVideo();
            Commander.fpv_zoom=0;
            Commander.fpv=true;
        }
    }
    private void fpv_photo_video(final MotionEvent event){
        vrc.onTouchEvent(event);
        if (vrcf != vrc.is_pressed()) {
            vrcf = vrc.is_pressed();
            MainActivity.stopVideo();
            Commander.fpv_code = (short) ((vrcf) ? 513 : 514);
            Commander.fpv = true;


            Thread thread = new Thread() {
                @Override
                public void run() {
                    try {
                        Thread.sleep(3000);
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                    MainActivity.startVideo();
                }
            };
            thread.start();
        }
        photo.onTouchEvent(event);
        if (photo.getStat() == 3) {
            Commander.fpv_code = 769;
            Commander.fpv = true;
        }
    }

    static boolean vrcf=false;
    static public boolean thumbed = false;
    boolean main_onTouchEvent(final MotionEvent event){



        extra_buttons.onTouchEvent(event);

        if (extra_buttons.is_pressed()){

        }
        menu.onTouchEvent(event);
        if (menu.getStat()==3)
            screen=viewMenu;



        control_type.onTouchEvent(event);
        smart_ctrl.onTouchEvent(event);
        if (smart_ctrl.getStat()==3 ){
            if (smart_ctrl.is_pressed()) {
                hold_alt.set(true);
                if (MainActivity.altHoldF()==false)
                    MainActivity.altHold();
            }
            MainActivity.smartCtrl();
        }

        hold_alt.onTouchEvent(event);
        if (hold_alt.getStat()==3)
            MainActivity.altHold();





        if (fpv.is_pressed())
            fpv_photo_video(event);




        desc_onoff.onTouchEvent(event);
        if (desc_onoff.getStat()==3)
            j_left.set_block_Y(desc_onoff.is_pressed());
        yaw_onoff.onTouchEvent(event);
        if (yaw_onoff.getStat()==3)
            j_left.set_block_X(yaw_onoff.is_pressed());
        pitch_onoff.onTouchEvent(event);
        if (pitch_onoff.getStat()==3)
            j_right.set_block_X(pitch_onoff.is_pressed());
        roll_onoff.onTouchEvent(event);
        if (roll_onoff.getStat()==3)
            j_right.set_block_Y(roll_onoff.is_pressed());


        on_off[0].onTouchEvent(event);
        on_off[1].onTouchEvent(event);


        if ( on_off[0].is_pressed()==on_off[1].is_pressed() && MainActivity.motorsOnF()!=on_off[0].is_pressed() &&
                ((on_off[0].getStat())==3 || (on_off[1].getStat())==3)) {
            MainActivity.start_stop();
            //Log.d("PWR","PWR+"+Integer.toString(stat1)+" "+Integer.toString(stat2));
        }

        j_left.onTouchEvent(event);
        j_right.onTouchEvent(event);

        compass_onoff.onTouchEvent(event);

        fpv.onTouchEvent(event);
        if (fpv.getStat()==3)
            fpv_start_stop();

        mScaleGestureDetector.onTouchEvent(event);
        cam_p_c.onTouchEvent(event,Commander.fpv_zoom);


        // invalidate();
        return true;

    }


    boolean menu_onTouchEvent(final MotionEvent event){
        exitMenu.onTouchEvent(event);
        if (exitMenu.getStat()==3) {
            screen = viewMain;
            return true;
        }
        showMap.onTouchEvent(event);
        exitProg.onTouchEvent(event);
        reboot.onTouchEvent(event);
        shutdown.onTouchEvent(event);
        comp_calibr.onTouchEvent(event);
        comp_calibr.onTouchEvent(event);
        comp_m_calibr.onTouchEvent(event);
        gps_on_off.onTouchEvent(event);
        showSettings.onTouchEvent(event);




        return true;
    }









    @Override
    public boolean onTouchEvent(MotionEvent event) {
        boolean ret = true;
        switch(screen) {
            case viewMain:
                ret = main_onTouchEvent(event);
                break;
            case viewMenu:
                ret = menu_onTouchEvent(event);
                break;
        }
        MainActivity.update=true;
        return ret;
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



    double old_yaw=0;
    double ang_speed=0;
    double ma_pitch=0,ma_roll=0;
    void main_onDraw(final Canvas c){


        j_left.set_return_back_Y(hold_alt.is_pressed());

        extra_buttons.paint(c);


        final double da=wrap_180(MainActivity.heading_t-old_yaw);
        ang_speed += (2.7777*da/MainActivity.updateTimeMsec - ang_speed)*0.1;
        old_yaw=MainActivity.heading_t;
        ma_pitch+=(MainActivity.pitch / maxAngle - ma_pitch)*0.1;
        ma_roll+=(MainActivity.roll  / maxAngle - ma_roll)*0.1;
        if (control_type.is_pressed()) {
            j_right.setJosticY((float) (ma_pitch));
            j_right.setJosticX((float) (ma_roll));
            //упроавление джостиком для YAW прсото для визуалицации процесса и не на что не влияет.
            j_left.setJosticX((float) ang_speed);
            yaw = (float) MainActivity.heading_t;//!!!
        }else{
            yaw+=j_left.getX()*0.360*MainActivity.updateTimeMsec; //1 оборот в сек.
            while (yaw>360) yaw-=360;
            while (yaw<-360)yaw+=360;
        }
        batMon.setVoltage(0.25f*Telemetry.batVolt);


        Commander.throttle=0.5f+(j_left.getY())/2;


        Commander.roll  =  j_right.getX()*maxAngle;
        Commander.pitch = -j_right.getY()*maxAngle;
        if (!yaw_onoff.is_pressed())
            Commander.heading=(float)wrap_180(yaw);

        setMonitor();
        monitor.paint(c);
        yaw_onoff.paint(c);
        desc_onoff.paint(c);
        pitch_onoff.paint(c);
        roll_onoff.paint(c);
        compass_onoff.paint(c);
        menu.paint(c);
        control_type.paint(c);
        j_left.paint(c);
        j_right.paint(c);
        on_off[0].paint(c);
        on_off[1].paint(c);
        batMon.paint(c);
        fpv.paint(c);
        smart_ctrl.paint(c);
        hold_alt.paint(c);

        if (fpv.is_pressed()) {
            vrc.paint(c);
            photo.paint(c);
        }


        cam_p_c.paint(c);

    }
    void menu_onDraw(final Canvas c){
        showMap.paint(c);
        exitMenu.paint(c);
        exitProg.paint(c);
        reboot.paint(c);
        shutdown.paint(c);
        comp_calibr.paint(c);
        comp_calibr.paint(c);
        comp_m_calibr.paint(c);
        gps_on_off.paint(c);
        showSettings.paint(c);
    }


    public void onDraw(Canvas c) {

        super.onDraw(c);
        //

        if (fpv.is_pressed()==false)
            c.drawRect(0,0,sm[0],sm[1],black);

        switch(screen) {
            case viewMain:
                main_onDraw(c);
                break;
            case viewMenu:
                menu_onDraw(c);
                break;
        }


    }
}




