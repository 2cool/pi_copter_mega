package copter.rc2;

import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by igor on 8/21/2016.
 */
public class Programmer {
    static private int progSize=0;
    final static private GeoDot []dot=new GeoDot[100];

    static public boolean noData(){return progSize==0;}


    static private int progIndex=0;


    static public int size(){return progSize;}
    public static boolean isLast(final int index){return index==progSize-1;}

    static final double toRad=Math.PI/180.0;
    public static double bearing(double oldLat, double oldLon){


        double lat1=toRad*oldLat;
        double lon1=toRad*oldLon;

        double lat2=toRad*lat;
        double lon2=toRad*lon;

        double x, y;
        double rll = (lon2 - lon1);
        double rlat = (lat1);
        double rlat2 = (lat2);

        y = Math.sin(rll)*Math.cos(rlat2);
        x = Math.cos(rlat)*Math.sin(rlat2) - Math.sin(rlat)*Math.cos(rlat2)*Math.cos(rll);
        return Math.atan2(y, x);  //minus и как у гугла
    }
    public static double distance_(double oldLat, double oldLon){


        double lat1=toRad*oldLat;
        double lon1=toRad*oldLon;

        double lat2=toRad*lat;
        double lon2=toRad*lon;

        double R = 6371000;
        double f1 = (lat1);
        double f2 = (lat2);
        double df = (lat2 - lat1);
        double dq = (lon2 - lon1);

        double a = Math.sin(df / 2)*Math.sin(df / 2) + Math.cos(f1)*Math.cos(f2)*Math.sin(dq / 2)*Math.sin(dq / 2);
        return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));

    }

    private static double getLon(final double tx){
        return (tx - 67108864.0) / (134217728.0 / 360.0);
    }
    private static double getLat(final double ty){
        return (2 * Math.atan(Math.exp((ty - 67108864.0) / -(134217728.0 / (2 * Math.PI)))) -
                Math.PI / 2)/ (Math.PI / 180.0);
    }

    public static int getSize(){return progSize;}

    public static void deliteLast(){
        if (progSize>0)
            progSize--;
        if (progSize==0){
            altitude=50;
            speedProgress=1;
            vspeedProgress=1;
            cam_ang=35;
            led_prog=6;
        }
    }

    static public  void changeCameraAngle(){
        if (progSize>0) {
            dot[progSize-1].cam_ang=cam_ang;
        }
    }
    static public  void changeLedProg(){
        if (progSize>0) {
            dot[progSize-1].led_prog=led_prog;
        }
    }

    public  static boolean changeSpeedInLastDot(double speedH, double speedV){
        boolean err=false;
        if (progSize>0) {
            progSize--;
            double x = dot[progSize].tx;
            double y = dot[progSize].ty;
            if (err=addDot_(x,y,dot[progSize].alt,speedH,speedV,dot[progSize].direction,dot[progSize].timer)==-1)
                progSize++;
        }

        return err;
    }
    public  static boolean changeDirectionInLastDot(double dir,double speedH, double speedV){
        boolean err=false;
        if (progSize>0) {
            progSize--;
            double x = dot[progSize].tx;
            double y = dot[progSize].ty;
            if (err=addDot_(x,y,dot[progSize].alt,speedH,speedV,dir,dot[progSize].timer)==-1)
                progSize++;
        }
        return err;
    }

    public static boolean changeTimerInLastDot(double timer,double speedK, double speedVK){
        boolean err=false;
        if (progSize>0) {
            progSize--;
            double x = dot[progSize].tx;
            double y = dot[progSize].ty;
            if (err=addDot_(x,y,dot[progSize].alt,speedK,speedVK,dot[progSize].direction,timer)==-1)
                progSize++;
        }
        return err;
    }



 //  double addDot(double x, double y, double altitude, double speedH, double speedV, double direct){
    public static boolean changeAltitudeInLastDot(double alt,double speedK, double speedVK){
        boolean err=false;
        altitude=(int)alt;
        if (progSize>0) {
            progSize--;
            double x = dot[progSize].tx;
            double y = dot[progSize].ty;
            if (err=addDot_(x,y,alt,speedK,speedVK,dot[progSize].direction,dot[progSize].timer)==-1)
                progSize++;
        }
        return err;
    }


    double wrap_180(double x) {return x < -180 ? x+360 : (x > 180 ? x - 360: x);}

///////////////////////////////////////////////
    static final public double maxSpeed=10,maxUpSpeed=5,maxDownSpeed=-3;
    static public int cam_ang=35;
    static public int led_prog=6;


    static double fullDist;
    public static double direction = 0;
    static public double altitude=50,distance;
    static public double lat=0,lon=0;

    static public double speedProgress=1,vspeedProgress=1;



    static final double maxA=3,maxH=1;

    static public GeoDot get_current_dot(){
        if (progSize>0)
            return dot[progSize-1];
        else
            return null;


    }


    public static double speed,speedZ;

    static GeoDot add(double x, double y, double alt, double speedHK,double speedVK, double direct,double timer){
        if ( progSize>255) {
            Log.i("MAP","TIMER or SIZE ERROR");
            return null;
        }



        lat=getLat(y);
        lon=getLon(x);
        double oldLat,oldLon,oldAltitude;

        float camAng;
        if (progSize==0){
            distance=0;

            oldAltitude=0;
            if (direct<500)
                direction=direct;
            else
                direction=Telemetry.heading;
        }else{

            oldLat= 0.0000001*(double)dot[progSize-1].lat;
            oldLon=0.0000001*(double)dot[progSize-1].lon;
            oldAltitude=dot[progSize-1].alt;
            distance=distance_(oldLat,oldLon);
            if (direct<500)
                direction=direct;
            else
                direction=bearing(oldLat,oldLon)/toRad;
        }



        speed=maxSpeed*speedHK;
        speedZ=speedVK*((altitude-oldAltitude>0)?maxUpSpeed:maxDownSpeed);
        altitude=alt;
        GeoDot d=new GeoDot(progSize,x,y,timer,altitude,direction,cam_ang,distance,altitude-oldAltitude,speed, speedZ,led_prog);
        oldAltitude=altitude;


        return d;

    }

    static double addDot_(double x, double y, double altitude, double speedHK, double speedVK, double direct, double timer){


        GeoDot gd=add(x,y,altitude,speedHK,speedVK,direct,timer);

        if (gd==null)
            return -1;




        dot[progSize]=gd;


        progSize++;

        return gd.dDist;
    }
    static void clearLastDot(){
        if (progSize>0)
            progSize--;
    }
    // fromPointToLatLng()

    static GeoDot get(final int index){
        if (index>=0 && index<progSize)
            return dot[index];
        else
            return null;
    }
    static String kml="";
    static int cnt=0;

    static public boolean load(String s){
       // if (true)return true;
        progSize=0;
        String str[]=s.split("\n");
        kml="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
                "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n" +
                "<Document>\n" +
                "\t<name>log_track.kml</name>\n" +
                "\t\n" +
                "\t<Placemark>\n" +
                "\t\t<name>log_track</name>\n" +
                "\t\t<styleUrl>#inline1</styleUrl>\n" +
                "\t\t<LineString>\n" +
                "\t\t\t<tessellate>1</tessellate>\n" +
                "\t\t\t<coordinates>";
        for (progIndex=0; progIndex< str.length; progIndex++) {
            dot[progIndex] = new GeoDot(str[progIndex]);
            if (cnt>0)
                kml+=",";
            kml+=Double.toString(dot[progIndex].lon*0.0000001)+","+Double.toString(dot[progIndex].lat*0.0000001)+",0";
            cnt++;

        }

        kml+="</coordinates>\n" +
                "\t\t</LineString>\n" +
                "\t</Placemark>\n" +
                "</Document>\n" +
                "</kml>";
        File myFile = new File(Environment.getExternalStorageDirectory(), "RC/logProg.kml");
        if (myFile.exists()){
            myFile.delete();
        }
        if (!myFile.exists())
            try {
                myFile.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        try {
            FileOutputStream fos = new FileOutputStream(myFile , false);
            fos.write(kml.getBytes());
            fos.flush();
            fos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }





        final int i=progIndex-1;
        altitude=dot[i].alt;
        speedProgress=dot[i].speed/maxSpeed;
        vspeedProgress=dot[i].speedZ/((dot[i].speedZ>0)?maxUpSpeed:maxDownSpeed);
        cam_ang=(int)dot[i].cam_ang;
        led_prog=dot[i].led_prog;

        progSize=progIndex;
        return true;
    }

    static public String get(){
        String s="";
        for (int i=0; i<progSize; i++){
            s+=dot[i].toString()+"\n";
        }

        return s;
    }

    static int dataLen=0;

    static public int getFirst(byte buf[], int off){
        if (progSize==0)
            return 0;
        progIndex=1;
        off=dot[0].getFirst(buf,off);

        buf[off]=(byte)(0xff&progSize);
        buf[off+1]=(byte)(0xff&(progSize>>8));

        return off+2;


    }
    static public int getNext(byte buf[], int off){

        if (progIndex>=progSize)
            return 0;
       off=dot[progIndex].getNext(buf,off);
        progIndex++;
        return off;

    }

};