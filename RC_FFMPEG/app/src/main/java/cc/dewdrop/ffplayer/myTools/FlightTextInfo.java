package cc.dewdrop.ffplayer.myTools;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;

public class FlightTextInfo {

    public static final int LOC=0,_2HM=1,THR=2,POW=3,VIBR=4,BAT=5,CAM_ANG=6,VSPEED=7;
    Rect r;
    final int FIELDS=8;
    public boolean f[]=new boolean[FIELDS];
    public String p[]={"","","","","","","",""};
    Paint txt;
    public FlightTextInfo(Rect r,
                          boolean loc,
                          boolean _2hom,
                          boolean thr,
                          boolean power,
                          boolean vibr,
                          boolean bat,
                          boolean cam_ang,
                          boolean vSpeed,
                          int txt_color){
        this.r=r;
        f[LOC]=loc;
        f[_2HM]=_2hom;
        f[THR]=thr;
        f[POW]=power;
        f[VIBR]=vibr;
        f[BAT]=bat;
        f[CAM_ANG]=cam_ang;
        f[VSPEED]=vSpeed;
        txt=new Paint();//txt_color);
        txt.setColor(txt_color);

        final int fsx=(int)(0.7*(r.right-r.left));
        final int fsy=(int)(0.7*(r.bottom-r.top));
        final String str="0000000  000000000";
        Rect b=new Rect();
        for (int s=15; s<300;s++) {
            txt.setTextSize(s);
            txt.getTextBounds(str,0,str.length(),b);
            if (fsx<=(b.right-b.left) || fsy<=(b.bottom-b.top)*FIELDS*1.5) {
                break;
            }
        }


    }
    public void paint(Canvas c){

        Rect b=new Rect();
        txt.getTextBounds("Text",0,4,b);
        int h=(int)(1.5*(b.bottom-b.top));
        int x=r.left;
        int y=r.top+h;
        for (int i=0; i<FIELDS;i++ ){
            if (f[i]){
                c.drawText(p[i],x,y,txt);
                y+=h;
            }
        }
    }
}
