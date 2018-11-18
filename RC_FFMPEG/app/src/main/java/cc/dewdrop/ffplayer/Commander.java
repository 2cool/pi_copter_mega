package cc.dewdrop.ffplayer;

import android.util.Log;

public class Commander {


    static final int MPU6050_DLPF_BW_256     =    0x00;
    static final int MPU6050_DLPF_BW_188   =      0x01;
    static final int MPU6050_DLPF_BW_98    =      0x02;
    static final int MPU6050_DLPF_BW_42    =      0x03;
    static  final int MPU6050_DLPF_BW_20    =      0x04;
    static  final int MPU6050_DLPF_BW_10     =     0x05;
    static final int MPU6050_DLPF_BW_5      =     0x06;


    static public boolean settings=false;
    static public float heading=0,yaw=0,roll=0,pitch=0,ax=0,ay=0, throttle =0.5f,c_heading=0,headingOffset=0;
    static public float gx=0,gy=0,gz=0;
    //static public float k0 =1, k1 =1, k4 =1, k3 =1, k5 =1,k6=1,k7=1,k8=1,k9=1,k10=1;
    static public float []sets=new float[10];
    static public int n=0;
    static public boolean link=false;

    static final public float NO_DATA=Float.POSITIVE_INFINITY;
    static public float sended_ax=0,sended_ay=0;
    static public byte buf[];
    static public boolean copter_is_busy=true;
    static final double GRAD2RAD = 0.01745329251994329576923690768489;
    static final double RAD2GRAD = 57.295779513082320876798154814105;
    static private void init(){
        copter_is_busy=true;
        settings=false;
        heading=ax=ay=roll=pitch=yaw=0;
        throttle =0.5f;
        c_heading=headingOffset=0;
        for (int i=0; i<10;i++)
            sets[i]=NO_DATA;
        n=0;
        link=false;


    }

    // static public void setCompassCalibration(){
    //     c_heading=heading-(float)Telemetry.heading;//-heading;
    // }







    static public void new_connection(){
        copter_is_busy=true;
        init();
        Telemetry.init();
    }
    static float shrink(float f){
        double t=Math.floor(f * 1000);
        return (float)(t*0.001);
    }

    static byte load_32int2buf(byte buf[], int i, int val){
        buf[i+0]=(byte)(val&255);
        buf[i+1]=(byte)(val>>8);
        buf[i+2]=(byte)(val>>16);
        buf[i+3]=(byte)(val>>24);
        return (byte)(buf[i]^buf[i+1]);
    }

    static byte load_16int2buf(byte buf[], int i, int val){
        buf[i+0]=(byte)(val&255);
        buf[i+1]=(byte)(val>>8);
        return (byte)(buf[i]^buf[i+1]);
    }
    static byte load_double2buf(byte buf[], int i, float dval){
        int val=(int)(dval*32765);
        return load_16int2buf(buf,i,val);
    }

    static public int upload_settings=-1;
    static public boolean program=false;
    static boolean firstProgStep=false;
    static boolean fpv=false;
    static byte fpv_addr;
    static short fpv_port;
    static short fpv_code=0;
    static byte fpv_zoom=0;

    //static boolean exit_main=false;

    static public void startUPLoadingProgram(){
        if (program==false){
            firstProgStep=true;
            program=true;
        }
    }

    static private int getProgram(byte buf[], int off){
        int i=0;
        if (firstProgStep){
            i=Programmer.getFirst(buf,off);
            firstProgStep=false;
        }else{
            i=Programmer.getNext(buf,off);
            //if (i==0)
            //    firstProgStep=true;
        }
        return i;
    }
//----------------------------------------------------


    static private int getSettings(byte buf[],int offset){
        int i=0;
        String msg = ""  +
                Integer.toString(n) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++]) + "," +
                Float.toString(sets[i++])+ "," ;
        // Disk.write("SETTINGS:"+msg+"\n");
        // Log.i("OUT_M", msg);
        //  System.arraycopy(msg,0,buf,0,msg.length());
        for (i=0; i<msg.length(); i++){
            buf[offset+i]=(byte)msg.charAt(i);
        }
        return msg.length();
    }


    static int old_commande=0;
    static public int get32to8bMask(int v){
        int mask=v&255;
        mask^=((v>>8)&255);
        mask^=((v>>16)&255);
        mask^=((v>>24)&255);
        return mask;
    }

    static public int get16to8bMask(int v){
        int mask=v&255;
        mask^=((v>>8)&255);

        return mask;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static public int get(byte buf[]){

        if (copter_is_busy){

            return 4;
        }

        int i=0;
        //   buf[0]=(byte)MainActivity.command_bits_;
        //  buf[1]=buf[2]=buf[3]=0;


        int mask=get32to8bMask(MainActivity.command_bits_);

        load_32int2buf(buf,i,MainActivity.command_bits_);
        if (MainActivity.command_bits_!=old_commande) {
            // Log.i("COMMANDER", " " + MainActivity.command_bits_);
            Log.i("COMMANDER", " " + MainActivity.command_bits_);
            old_commande=MainActivity.command_bits_;
        }
        if ((MainActivity.command_bits_ & MainActivity.Z_STAB) !=0) {
            if ((MainActivity.control_bits & MainActivity.Z_STAB) != 0)
                throttle=Telemetry.corectThrottle();
            else
                throttle = 0.5f;
        }
        Log.d("COMM",Float.toString(throttle));
        MainActivity.command_bits_=0;

        i+=4;
        int t=(int)(throttle *32000.0);
        load_16int2buf(buf, i, t);
        mask^=get16to8bMask(t);
        i+=2;

        final double RANGK=10;

        t=(int)((heading-c_heading)*RANGK);
        load_16int2buf(buf, i, t);
        mask^=get16to8bMask(t);
        i+=2;
        t=(int)(headingOffset*RANGK);
        load_16int2buf(buf,i,t);
        mask^=get16to8bMask(t);
        i+=2;

        float tax,tay;
        if ((MainActivity.smartCntrF())) {
            tax = ax;
            tay = ay;
        }else{
            double da=GRAD2RAD*(Telemetry.heading-(heading-c_heading));
            tax=(float)(ax*Math.cos(da)-ay*Math.sin(da));
            tay=(float)(ax*Math.sin(da)+ay*Math.cos(da));
        }


        tay=(float)(roll*GRAD2RAD);
        tax=(float)(pitch*GRAD2RAD);


        t = (int) (tax*RAD2GRAD * RANGK);
        load_16int2buf(buf, i, t);
        mask^=get16to8bMask(t);
        i+=2;
        t = (int) (tay*RAD2GRAD * RANGK);
        load_16int2buf(buf, i, t);
        mask^=get16to8bMask(t);
        i+=2;

        buf[3]=(byte)mask;
        if (fpv){
            fpv=false;
            buf[i++]='F';
            buf[i++]='P';
            buf[i++]='V';
            buf[i++]=fpv_addr;
            buf[i++]=(byte)(0xff&fpv_port);
            buf[i++]=(byte)(0xff&(fpv_port>>8));
            buf[i++]=fpv_zoom;
            buf[i++]=(byte)(0xff&fpv_code);
            buf[i++]=(byte)(0xff&(fpv_code>>8));
            fpv_code=0;

        }else

        if (program){
            buf[i++]='P';
            buf[i++]='R';
            buf[i++]='G';
            int prLen=getProgram(buf,i);
            i+=prLen;
            if(prLen==0) {
                program = false;
                i-=3;
            }
        }
        else
        if (settings){
            settings = false;
            buf[i++]='S';
            buf[i++]='E';
            buf[i++]='T';
            i+=getSettings(buf,i);
        }
        else
        if (upload_settings>=0){
            buf[i++]='U';
            buf[i++]='P';
            buf[i++]='S';
            buf[i++]=(byte)upload_settings;
            upload_settings=-1;
        }

        sended_ay=ay;
        sended_ax=ax;


        return i;

    }

}

