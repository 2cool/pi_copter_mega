package copter.rc2;

/**
 * Created by igor on 8/21/2016.
 */
public class GeoDot {
    public int index;
    public double tx,ty;
    public double alt,direction,timer,cam_ang,dDist,dAlt;
    public int led_prog;
    double speed=0,speedZ=0;
    public int lat,lon;



    private static double getLon(final double tx){
        return (tx - 67108864.0) / (134217728.0 / 360.0);
    }
    private static double getLat(final double ty){
        return (2 * Math.atan(Math.exp((ty - 67108864.0) / -(134217728.0 / (2 * Math.PI)))) -
                Math.PI / 2)/ (Math.PI / 180.0);
    }

    GeoDot(int index, double tx, double ty,double _timer,double _alt, double _dir, double _cam_ang,double dDist,double dAlt, double speed, double speedZ,int led_prog){
        this.index=index;
        this.tx=tx;
        this.ty=ty;
        this.lat=(int)(getLat(ty)*10000000.0);
        this.lon=(int)(getLon(tx)*10000000.0);

        alt=_alt;
        direction=_dir;
        timer=_timer;
        cam_ang=_cam_ang;
        this.dDist=dDist;
        this.dAlt=dAlt;
        this.speed=speed;
        this.speedZ=speedZ;
        this.led_prog=led_prog;
    }

    GeoDot(String str){
        String s[]=str.split(",");
        int i=0;
        index=Integer.parseInt(s[i++]);
        tx=Integer.parseInt(s[i++]);
        ty=Integer.parseInt(s[i++]);
        lat=(int)(getLat(ty)*10000000.0);
        lon=(int)(getLon(tx)*10000000.0);

        alt=Integer.parseInt(s[i++]);
        direction=Integer.parseInt(s[i++]);
        timer=Integer.parseInt(s[i++]);
        cam_ang=Integer.parseInt(s[i++]);
        dDist=Integer.parseInt(s[i++]);
        dAlt=Integer.parseInt(s[i++]);
        speed=Integer.parseInt(s[i++]);
        speed*=0.1;
        speedZ=Integer.parseInt(s[i++]);
        speedZ*=0.1;
        led_prog=(s.length>i)?led_prog=Integer.parseInt(s[i++]):6;
    }
    public String toString(){
        String s="";
        s+=Integer.toString(index)+",";
        s+=Integer.toString((int)tx)+",";
        s+=Integer.toString((int)ty)+",";
        s+=Integer.toString((int)alt)+",";
        s+=Integer.toString((int)direction)+",";
        s+=Integer.toString((int)timer)+",";
        s+=Integer.toString((int)cam_ang)+",";
        s+=Integer.toString((int)dDist)+",";
        s+=Integer.toString((int)dAlt)+",";
        s+=Integer.toString((int)(speed*10))+",";
        s+=Integer.toString((int)(speedZ*10))+",";
        s+=Integer.toString((int)led_prog);
        return s;
    }
    static public final int LAT_LON = 1, DIRECTION = 2, ALTITUDE = 4,  CAMERA_ANGLE = 8, TIMER = 16,SPEED_XY=32,SPEED_Z=64,LED_CONTROL=128;
    static void load_int4buf(byte buf[], int i, int val){
        buf[i+0]=(byte)(val&255);
        buf[i+1]=(byte)((val>>8)&255);
        buf[i+2]=(byte)((val>>16)&255);
        buf[i+3]=(byte)((val>>24)&255);

    }
    static byte load_int2buf(byte buf[], int i, int val){
        buf[i+0]=(byte)(val&255);
        buf[i+1]=(byte)(val>>8);
        return (byte)(buf[i]^buf[i+1]);
    }


    private static double oldLat,oldLon,oldAlt,oldDir,oldCamAng,oldTimer;
    private static int old_led_prog;
    private static int oldSXY=0,oldSZ=0;
    int getFirst(byte buf[], int off){

        oldLat=lat;
        oldLon=lon;
        oldAlt=alt;
        oldDir=direction;
        oldCamAng=cam_ang;
        oldTimer=timer;
        old_led_prog=led_prog;

        int i=off;
        int mask=(LAT_LON+DIRECTION+ALTITUDE+CAMERA_ANGLE+TIMER+SPEED_XY+SPEED_Z+LED_CONTROL);
        buf[i++]=(byte)(mask&255);

        buf[i++]=(byte)index;
        buf[i++]=(byte)timer;

        final double k=10;
        int sxy=(int)Math.abs(speed*k);
        oldSXY=sxy;
        buf[i++]=(byte)(sxy&255);

        int sz=(int)(speedZ*k);
        buf[i++]=(byte)sz;
        oldSZ=sz;

        load_int4buf(buf,i,lat);
        i+=4;
        load_int4buf(buf,i,lon);
        i+=4;

        double d=direction;
        d*= 0.70555555555555555555555555555556;
        buf[i++]=(byte)d;

        load_int2buf(buf,i,(int)alt);
        i+=2;

        d=cam_ang;
        d*= 0.70555555555555555555555555555556;
        buf[i++]=(byte)d;
        buf[i++]=(byte)led_prog;


        return i;

    }

    int getNext(byte buf[],int i) {

        int masAdr=i;
        i++;
        int prog_mask=0;

        buf[i++]=(byte)index;
        if (oldTimer!=timer){
            oldTimer=timer;
            prog_mask|=TIMER;
            buf[i++]=(byte)timer;
        }

        final double k=10;
        int sxy=(int)Math.abs(speed*k);

        if (oldSXY!=sxy) {
            oldSXY = sxy;
            buf[i++] = (byte) (sxy & 255);
            prog_mask|=SPEED_XY;
        }
        int sz=(int)(speedZ*k);

        if (oldSZ!=sz) {
            prog_mask|=SPEED_Z;
            buf[i++] = (byte)sz;
            oldSZ = sz;
        }

        if (oldLat!=lat || oldLon!=lon){
            prog_mask|=LAT_LON;
            oldLat=lat;
            oldLon=lon;
            load_int4buf(buf,i,lat);
            i+=4;
            load_int4buf(buf,i,lon);
            i+=4;
        }
        if (oldDir!=direction){
            oldDir=direction;
            prog_mask|=DIRECTION;
            double d=direction;
            d*= 0.70555555555555555555555555555556;
            buf[i++]=(byte)d;
        }
        if (oldAlt!=alt){
            oldAlt=alt;
            prog_mask|=ALTITUDE;
            load_int2buf(buf,i,(int)alt);
            i+=2;
        }
        if (oldCamAng!=cam_ang){
            oldCamAng=cam_ang;
            prog_mask|=CAMERA_ANGLE;
            double d=cam_ang;
            d*= 0.70555555555555555555555555555556;
            buf[i++]=(byte)d;
        }

        if (old_led_prog!=led_prog){
            old_led_prog=led_prog;
            prog_mask|=LED_CONTROL;
            buf[i++]=(byte)led_prog;
        }




        buf[masAdr]=(byte)(prog_mask&255);
        return i;
    }


};
