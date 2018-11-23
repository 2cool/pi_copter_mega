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
    static public Img_button yaw_off, desc_off, pitch_off, roll_off, head_less,menu;
    static public Img_button pitch_roll_off,go_to_home;
    static public Img_button control_type_acc,showMap,showSettings,hold_alt,smart_ctrl,extra_buttons;
    static public Img_button[]motors_on=new Img_button[2];
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


    private static int old_commander_counter=-1;
    private static int old_tel_counter=-1;
    private  void updateControls(){
        if (old_tel_counter<Telemetry.get_counter() && old_commander_counter<Commander.get_coutner()) {
            old_tel_counter=Telemetry.get_counter();
            old_commander_counter=Commander.get_coutner();

            motors_on[0].enabled(Commander.link);
            motors_on[1].enabled(Commander.link);
            cam_p_c.gimbal_pitch_add(0,Commander.fpv_zoom);//update
            if (motors_on[0].is_pressed()==motors_on[1].is_pressed()) {
                motors_on[0].set(MainActivity.motorsOnF());
                motors_on[1].set(MainActivity.motorsOnF());
            }
            go_to_home.set(MainActivity.toHomeF());
            if (!MainActivity.horizontOnF()) //always is on it this prog
                MainActivity.horizonOn();
            if (!MainActivity.compassOnF())
                MainActivity.compassOn();

            smart_ctrl.set(MainActivity.smartCntrF());
            hold_alt.set(MainActivity.altHoldF());
            j_left.set_return_back_Y(MainActivity.altHoldF());

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

        pitch_off =new Img_button(sc.getRect(nX-2,jbuttonsY),
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        pitch_roll_off =new Img_button(sc.getRect(nX-1,jbuttonsY),
                context.getResources().getDrawable(R.drawable.rotate_on),
                context.getResources().getDrawable(R.drawable.rotate_off),true);


        roll_off =new Img_button(sc.getRect(nX-1,jbuttonsY),
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);



        yaw_off =new Img_button(sc.getRect(1,jbuttonsY),
                context.getResources().getDrawable(R.drawable.x_on),
                context.getResources().getDrawable(R.drawable.x_off),true);

        desc_off =new Img_button(sc.getRect(0,jbuttonsY),
                context.getResources().getDrawable(R.drawable.y_on),
                context.getResources().getDrawable(R.drawable.y_off),true);



        control_type_acc =new Img_button(sc.getRect(nX-2,0),
                context.getResources().getDrawable(R.drawable.touch),
                context.getResources().getDrawable(R.drawable.gyro),true);


        head_less =new Img_button(sc.getRect(1,0),
                context.getResources().getDrawable(R.drawable.touch),
                context.getResources().getDrawable(R.drawable.compass_on),true);
        

        fpv =new Img_button(sc.getRect(4,0),
                context.getResources().getDrawable(R.drawable.fpv_off),
                context.getResources().getDrawable(R.drawable.fpv),true);
        vrc =new Img_button(sc.getRect(5,0),
                context.getResources().getDrawable(R.drawable.vrc_off),
                context.getResources().getDrawable(R.drawable.vrc_on),true);
        photo =new Img_button(sc.getRect(6,0),
                context.getResources().getDrawable(R.drawable.photo),
                context.getResources().getDrawable(R.drawable.photo),false);

        menu=new Img_button(sc.getRect(7,0),
                context.getResources().getDrawable(R.drawable.menu),
                context.getResources().getDrawable(R.drawable.menu),false);



        motors_on[0]=new Img_button(sc.getRect(0,0),
                context.getResources().getDrawable(R.drawable.green),
                context.getResources().getDrawable(R.drawable.red),true);
        //on_off[0].enabled(false);

        motors_on[1]=new Img_button(sc.getRect(nX-1,0),
                context.getResources().getDrawable(R.drawable.green),
                context.getResources().getDrawable(R.drawable.red),true);
        //on_off[0].enabled(false);

        hold_alt=new Img_button(sc.getRect(2,0),
                context.getResources().getDrawable(R.drawable.alt_hold_offt),
                context.getResources().getDrawable(R.drawable.alt_hold_on),true);
        hold_alt.set(true);

        smart_ctrl=new Img_button(sc.getRect(nX-3,0),
                context.getResources().getDrawable(R.drawable.smart_off),
                context.getResources().getDrawable(R.drawable.smart_on),true);
        smart_ctrl.set(true);
        extra_buttons=new Img_button(sc.getRect(8,0),
                context.getResources().getDrawable(R.drawable.circle),
                context.getResources().getDrawable(R.drawable.extra_buttons),true);
       // extra_buttons.set(true);
        go_to_home =new Img_button(sc.getRect(9,0),
                context.getResources().getDrawable(R.drawable.go2home_off),
                context.getResources().getDrawable(R.drawable.go2home),false);



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


    void yaw_controls(final MotionEvent event){
        head_less.onTouchEvent(event);
        yaw_off.onTouchEvent(event);
        if (head_less.getStat()==3){
            yaw_off.set(head_less.is_pressed());
            j_left.set_block_X(yaw_off.is_pressed());
        }
        if (yaw_off.getStat()==3){
            j_left.set_block_X(yaw_off.is_pressed());
        }
    }
    void pitch_roll_controls(final MotionEvent event){
        control_type_acc.onTouchEvent(event);
        if (control_type_acc.getStat()==3){
            if (control_type_acc.is_pressed()){
                pitch_roll_off.set(true);
                j_right.set_block_X(true);
                j_right.set_block_Y(true);
            }else{
                pitch_off.set(false);
                roll_off.set(false);
                j_right.set_block_X(false);
                j_right.set_block_Y(false);
            }
        }
        if (control_type_acc.is_pressed()){
            pitch_roll_off.onTouchEvent(event);
            if (pitch_roll_off.getStat()==3){
                j_right.set_block_X(pitch_roll_off.is_pressed());
                j_right.set_block_Y(pitch_roll_off.is_pressed());
            }
        }else{
            pitch_off.onTouchEvent(event);
            if (pitch_off.getStat()==3)
                j_right.set_block_X(pitch_off.is_pressed());
            roll_off.onTouchEvent(event);
            if (roll_off.getStat()==3)
                j_right.set_block_Y(roll_off.is_pressed());
        }
    }
    void motors_control(final MotionEvent event){
        motors_on[0].onTouchEvent(event);
        motors_on[1].onTouchEvent(event);
        if ( motors_on[0].is_pressed()==motors_on[1].is_pressed() && MainActivity.motorsOnF()!=motors_on[0].is_pressed() &&
                ((motors_on[0].getStat())==3 || (motors_on[1].getStat())==3)) {
            MainActivity.start_stop();
            //Log.d("PWR","PWR+"+Integer.toString(stat1)+" "+Integer.toString(stat2));
        }
    }
    boolean main_onTouchEvent(final MotionEvent event){

        extra_buttons.onTouchEvent(event);
        menu.onTouchEvent(event);
        if (menu.getStat()==3)
            screen=viewMenu;

        fpv.onTouchEvent(event);
        if (fpv.getStat()==3)
            fpv_start_stop();
        if (fpv.is_pressed())
            fpv_photo_video(event);

        motors_control(event);

        mScaleGestureDetector.onTouchEvent(event);
        cam_p_c.onTouchEvent(event,Commander.fpv_zoom);

        yaw_controls(event);
        pitch_roll_controls(event);

        smart_ctrl.onTouchEvent(event);
        if (smart_ctrl.getStat()==3)
            MainActivity.smartCtrl();

        hold_alt.onTouchEvent(event);
        if (hold_alt.getStat()==3)
            MainActivity.altHold();

        desc_off.onTouchEvent(event);
        if (desc_off.getStat()==3)
            j_left.set_block_Y(desc_off.is_pressed());
        go_to_home.onTouchEvent(event);
        if (go_to_home.getStat()==3)
            MainActivity.toHome();

        j_left.onTouchEvent(event);
        j_right.onTouchEvent(event);

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
        monitor.setYaw(Telemetry.heading);
        monitor.setHeight(Telemetry._alt);
    }
    //-------------------------------------------------------------------------------------------








    double old_yaw=0;
    double ang_speed=0;
    double ma_pitch=0,ma_roll=0;
    void main_onDraw(final Canvas c){

        updateControls();

        extra_buttons.paint(c);
        final double max_speed=1/0.360;
        final double da=wrap_180(MainActivity.heading_t-old_yaw);
        ang_speed += (max_speed*da/MainActivity.updateTimeMsec - ang_speed)*0.1;
        old_yaw=MainActivity.heading_t;
        ma_pitch+=(MainActivity.pitch / maxAngle - ma_pitch)*0.1;
        ma_roll+=(MainActivity.roll  / maxAngle - ma_roll)*0.1;
        if (control_type_acc.is_pressed()) {
            j_right.setJosticY((float) (ma_pitch));
            j_right.setJosticX((float) (ma_roll));
        }
        Commander.roll=j_right.getX()*maxAngle;
        Commander.pitch=j_right.getY()*maxAngle;
        if (head_less.is_pressed()){
            if (!yaw_off.is_pressed()) {
                Commander.heading = (float) MainActivity.heading_t;
                j_left.setJosticX((float) ang_speed);
            }
        }else{
            Commander.heading=(float)wrap_180(Telemetry.heading);//*MainActivity.updateTimeMsec);
            Commander.headingOffset=+j_left.getX()*90;
        }




        batMon.setVoltage(0.25f*Telemetry.batVolt);

        Commander.throttle=0.5f+(j_left.getY())/2;

        double roll=j_right.getX() * maxAngle;
        double pitch=j_right.getY() * maxAngle;
        Commander.roll = (float)roll;
        Commander.pitch = (float)pitch;




        setMonitor();
        monitor.paint(c);

        yaw_off.paint(c);
        desc_off.paint(c);

        if (control_type_acc.is_pressed())
            pitch_roll_off.paint(c);
        else {
            pitch_off.paint(c);
            roll_off.paint(c);
        }

        head_less.paint(c);
        menu.paint(c);
        control_type_acc.paint(c);
        j_left.paint(c);
        j_right.paint(c);
        motors_on[0].paint(c);
        motors_on[1].paint(c);
        batMon.paint(c);
        fpv.paint(c);
        smart_ctrl.paint(c);
        hold_alt.paint(c);
        go_to_home.paint(c);
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




