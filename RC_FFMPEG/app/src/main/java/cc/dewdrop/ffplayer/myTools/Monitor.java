package cc.dewdrop.ffplayer.myTools;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;

import static java.lang.Math.cos;

public class Monitor {
    Paint white;
    private double hight,speed;
    private double pitch, roll, yaw;
    private float xpos,ypos;
    private Bitmap bm,cmps;
    public void setPitch(double a){
        pitch=-a;
    }
    public void setRoll(double a){
        roll=-a;
    }
    public void setHeight(double h){hight=h;}
    public void setSpeed(double s){speed=s;}
    public void setYaw(double a){
        while(a<0)
            a+=360;
        while (a>=360)
            a-=360;
        yaw=a;
    }
    private float scale;
    private float size;
    Paint gray;
    public Monitor(int x, int y, int size_, Bitmap bm_, Bitmap _cmps_){

        hight=0;
        yaw=0;
        speed=0;
        gray=new Paint();
        gray.setColor(Color.BLACK);
        gray.setAlpha(35);
        size=size_;
        // float sm[]=MainActivity.screenMetrics;
        scale=(float)size / bm_.getWidth();
        xpos=x;
        ypos=y;
        pitch=roll=0;
        bm=bm_;
        cmps=_cmps_;
        white = new Paint();
        white.setColor(Color.WHITE);
        white.setStrokeWidth(2);
        white.setTextSize(size_/6);
        //   white.setAlpha(255);
    }



    //  static float angle=0;//115;
    private Matrix matrix = new Matrix();
    public void paint(Canvas c) {

        //draw monitoring of pitch and yaw;
        c.drawRect(xpos-size*0.7f,ypos-size,xpos+size*0.7f,ypos+size,gray);
        matrix.reset();
        matrix.postRotate((float)roll);
        float k=(float)bm.getWidth()/185.0f;
        Bitmap cropped = Bitmap.createBitmap(
                bm,
                0,
                (int)(307*k+2.4*k*pitch),
                bm.getWidth(),
                bm.getWidth(),
                matrix,
                true);
        matrix.reset();
        matrix.postScale(scale,scale);
        Bitmap cropped2=Bitmap.createBitmap(
                cropped,
                (cropped.getHeight()-bm.getWidth())/2,
                (cropped.getWidth()-bm.getWidth())/2,bm.getWidth(),
                bm.getWidth(),
                matrix,
                true);
        c.drawBitmap(cropped2,xpos-cropped2.getWidth()/2,ypos-cropped2.getHeight()/2,white);


        //draw central horizontal line of pitch
        c.drawLine(xpos-50,ypos,xpos+50,ypos,white);
        c.drawLine(xpos,ypos-15,xpos,ypos+15,white);


        //draw division of roll
        for (float a= -60; a<=60; a+=20){
            float ang=(float)(a/180*Math.PI);
            float size1=size*0.65f;
            float x0=(float)Math.sin(ang)*size1;
            float y0=-(float)Math.cos(ang)*size1;
            c.drawLine(xpos+x0,ypos+y0,xpos+x0*1.1f,ypos+y0*1.1f,white);
        }
        //draw pointer of roll
        float size1 = size*0.6f;
        float x0=(float)Math.sin(roll/180*Math.PI)*size1;
        float y0=-(float)Math.cos(roll/180*Math.PI)*size1;
        c.drawLine(xpos+x0,ypos+y0,xpos+x0*1.2f,ypos+y0*1.2f,white);
        //draw height
        String text=Integer.toString((int)hight)+ " m";
        Rect r=new Rect();
        white.getTextBounds(text,0,text.length(),r);
        //draw speed
        c.drawText(text,xpos-bm.getWidth()/2.7f-(r.right-r.left),ypos+(r.bottom-r.top)/2,white);
        text=Integer.toString((int)speed)+" m/c";
        c.drawText(text,xpos+bm.getWidth()/3,ypos+(r.bottom-r.top)/2,white);
        //draw pointer of yaw
        c.drawLine(xpos, ypos-size1,xpos,ypos-size1*1.25f,white);


        //draw yaw
        matrix.reset();
        matrix.postScale(scale,scale);
        float compsL= (float)cmps.getWidth()*9.0f/25.0f;
        Bitmap compsN=Bitmap.createBitmap(
                cmps,
                (int)(16.0/360/25*yaw*cmps.getWidth()),
                0,
                (int)(compsL),
                cmps.getHeight(),
                matrix,
                true);
        c.drawBitmap(compsN,xpos-compsL*scale*0.5f,ypos-size1*1.5f,white);




    }


}
