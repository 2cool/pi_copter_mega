package copter.rc2;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Toast;


@SuppressLint({ "DrawAllocation", "WrongCall" }) public class DrawView extends View {




 static public UPD_MON monitor=new UPD_MON();

 static public InetMaps im=new InetMaps(monitor);
	public MyTile myile;
	Canvas canvas=null;	
	float starty;
	
	int BORDER=10;
	Rect touch=null;

    static float zoom=1.56f;

    Paint black = new Paint();
    Paint red=new Paint();
    Paint yellow=new Paint();
    Paint grey=new Paint();
	Paint power=new Paint();
	Paint gpsColor=new Paint();
   static public boolean thumbed =false;
	static final double GRAD2RAD = 0.01745329251994329576923690768489;
    int thumbWidth=0;

    
  //  public static float throttle=0.06f;
  //  static public float heading=0;

    
    public DrawView(Context context) {
        super(context);
       
        
       thumbWidth=150;//button.getWidth();
       BORDER=thumbWidth/10;
       power.setColor(Color.RED);
        black.setColor(Color.BLACK);
        black.setAlpha(127);
        black.setTextSize(20);
        red.setColor(Color.RED);
		red.setTextSize(20);
        yellow.setColor(Color.YELLOW);
        grey.setColor(Color.GRAY);
       // grey.
        red.setStrokeWidth(10);
        yellow.setStrokeWidth(5);
       // black.setStrokeWidth(5);
		gpsColor.setColor(Color.BLACK);
		gpsColor.setTextSize(50);
    }
    
    int x=0;

	static float _R,cx,cy;
	static int touchHigh;
    static boolean menu=false;
	static long lastTime=0;
    @SuppressLint("NewApi")



	public double  wrap_PI(final double x){
		return	(x < -Math.PI ? x+Math.PI*2 : (x > Math.PI ? x - Math.PI*2: x));
	}

	public double  wrap_PI_GRAD( double x){
		x*=(Math.PI/180.0);
		return	(x < -Math.PI ? x+Math.PI*2 : (x > Math.PI ? x - Math.PI*2: x));
	}


static boolean onDrawAtwork=false;
    public void onDraw(Canvas c) {



		if (onDrawAtwork)
			return;
		else
			onDrawAtwork=true;
    	try{



            zoom=(float)(1.0/(Math.sin(MainActivity.zoomN)));

			if (Commander.link && menu==false){
				canvas=c;
				Rect bounds=new Rect();
				black.getTextBounds("11", 0, 1, bounds);

				int textHeight=bounds.height()*4/3;
				int textI=textHeight+10;

				 touchHigh=this.getHeight()-BORDER*2;
				touch=new Rect(BORDER,BORDER,BORDER+thumbWidth,touchHigh+BORDER);

				//Telemetry.min_volt=(int)(422*Commander.throttle);


				{
					long time=System.currentTimeMillis();
					if (time - lastTime > 2000) {
						lastTime=time;
						int t_green = 255, t_red = 255;
						if (Telemetry.min_volt < Telemetry.VOLT_50) {
							int t = Telemetry.min_volt - Telemetry.MIN_VOLT;
							if (t < 0)
								t = 0;
							t_green = (int) ((double) (255 * t) / (Telemetry.VOLT_50 - Telemetry.MIN_VOLT));
						} else {
							int t = Telemetry.min_volt - Telemetry.VOLT_50;
							if (t < 0)
								t = 0;
							t_red = 255 - (int) ((double) (255 * t) / (422 - Telemetry.VOLT_50));
						}

						power.setARGB(255, t_red, t_green, 0);
					}
				}
    	 c.drawRect(touch, (MainActivity.motorsOnF())?power:black);

		if (MainActivity.motorsOnF() && (MainActivity.altHoldF()==false || MainActivity.smartCntrF()==false)) {
			c.drawRect(new Rect(670, 0, 670 + thumbWidth, 450), red);
		}


    	 //need power


    	 if (MainActivity.toHomeF()==false && MainActivity.progF()==false){
				{
					int y = touchHigh - (int) (Commander.throttle * touchHigh);
					Rect r2 = new Rect(BORDER, y + BORDER, BORDER + thumbWidth, y + BORDER * 2);
					c.drawRect(r2, (MainActivity.motorsOnF()) ? black : power);
					c.drawLine(BORDER, BORDER + touchHigh / 2, BORDER + thumbWidth, BORDER + touchHigh / 2, black);
				}
    	 }
		//real power
		{

			int y=touchHigh-(int)(Telemetry.realThrottle*touchHigh);
			Rect r2=new Rect(BORDER,y+BORDER,BORDER+thumbWidth/3,y+BORDER * 2);
			c.drawRect(r2, black);

			r2=new Rect(BORDER+thumbWidth/3*2,y+BORDER,BORDER+thumbWidth,y+BORDER * 2);
			c.drawRect(r2, black);
			//c.drawLine(BORDER, BORDER+touchHigh/2, BORDER + thumbWidth, BORDER+touchHigh/2, black);

		}





    	 int wY=(int)(1.5*thumbWidth);
    	 
			Paint gps_paint=black;
			black.setTextSize(28);
				red.setTextSize(28);
			//Telemetry.maxTelemetry) {
			/*	if (MainActivity.motorsOnF()) {
					String sx=(Double.toString(( (Telemetry.lat*(1.0/25600)))))+"00";
					int i=sx.indexOf("00");
					if (i>2)
						sx=sx.substring(0,i);
					String sy=(Double.toString(( (Telemetry.lon*(1.0/25600)))))+"00";
					i=sy.indexOf("00");
					if (i>2)
						sy=sy.substring(0,i);
					c.drawText(sx, wY, textI += textHeight * 2, gps_paint);
					c.drawText(sy, wY, textI += textHeight, gps_paint);
				}else {*/
					c.drawText((Double.toString(Telemetry.lat) + "0000000").substring(0, 10), wY, textI += textHeight * 2, black);
					c.drawText((Double.toString(Telemetry.lon) + "0000000").substring(0, 10), wY, textI += textHeight, black);
					c.drawText("2h:"+Integer.toString((int)Telemetry.dist)+" H:"+Integer.toString(Telemetry.r_accuracy_hor_pos)+"  V:"+Integer.toString(Telemetry.r_acuracy_ver_pos), wY, textI += textHeight, gps_paint);
				//}
				c.drawText((Double.toString(Telemetry.realThrottle)+"00").substring(0,4) + " Throt", wY, textI += textHeight, (Telemetry.realThrottle == 0) ? black : red);

                c.drawText((Double.toString(Math.abs((double)Telemetry.power/1000))+"00").substring(0,4) + " Power", wY, textI += textHeight, (Telemetry.realThrottle == 0) ? black : red);
				textI += 4;
				c.drawText((Double.toString((double)Telemetry.vibration/1000)+"00").substring(0,4) + " Vibr", wY, textI += textHeight, (Telemetry.realThrottle == 0) ? black : red);
				textI += 4;
				c.drawText("bat " + Telemetry.batery + "v", wY, textI += textHeight, (Telemetry.F_MIN_VOLT)?red:black);

				//c.drawText(Integer.toString(Telemetry.satilites) + " sat " + Telemetry.r_accuracy_hor_pos + " acu", wY, textI += textHeight, gps_paint);
				//c.drawText(Double.toString(Telemetry.ap_pitch) + " pitch", wY, textI += textHeight, black);
				//c.drawText(Double.toString(Telemetry.ap_roll) + " roll", wY, textI += textHeight, black);
				c.drawText((Float.toString(Telemetry._alt)) + " M", wY, textI += textHeight, black);
				c.drawText((Integer.toString(Telemetry.gimbalPitch)) + " cam a.", wY, textI += textHeight, black);
				c.drawText(MapEdit.getNum(Telemetry.speed,2) + "hor. m/s", wY, textI += textHeight, black);
				c.drawText(MapEdit.getNum(Telemetry.v_speed,2) + "ver. m/s", wY, textI += textHeight, black);


				c.drawText(Telemetry.motors_on_timer,300,430,black);


	    	 cx = c.getWidth()/2;
			 cy = getHeight()/2;
			 cx *= 1.2;
			 cy*=1.1;

	    	// c.drawCircle(cx, cy, Telemetry.m0>>2, black);
	    	// c.drawCircle(cx+100, cy, Telemetry.m1>>2, black);
	    	// c.drawCircle(cx, cy+100, Telemetry.m2>>2, black);
	    	// c.drawCircle(cx+100, cy+100, Telemetry.m3>>2, black);



	    	 _R = cy * 0.7f;


	    	 c.drawCircle(cx, cy, _R, black);


	    	  float r;






	    	 r=zoom*_R*0.5f;
             if (r<_R) {
                 c.drawCircle(cx, cy, r, grey);
                 c.drawCircle(cx, cy, r - 2, black);
             }
	    	  r=zoom*_R*0.342f;
             if (r<_R) {
                 c.drawCircle(cx, cy, r, grey);
                 c.drawCircle(cx, cy, r - 2, black);
             }
	    	 r=zoom*_R*0.173f;
             if (r<_R) {
                 c.drawCircle(cx, cy, r, grey);
                 c.drawCircle(cx, cy, r - 2, black);
             }



	    	 float x,y;
			if (MainActivity.toHomeF()==false && MainActivity.progF()==false ) {



			//	x = cx - zoom * (float) Commander.ay * _R;
			//	y = cy - zoom * (float) Commander.ax * _R;

				x = cx - zoom * (float) Commander.roll * _R;
				y = cy - zoom * (float) Commander.pitch * _R;

			//	x = cx - zoom * (float) Commander.sended_ay * _R;
			//	y = cy - zoom * (float) Commander.sended_ax * _R;
				c.drawCircle(x, y, 20, ((MainActivity.control_bits&MainActivity.HORIZONT_ON)!=0) ? red : black);
				c.drawCircle(x, y, 16, black);
			}


			x = cx - zoom * (float) Telemetry.ap_roll*0.01745f * _R;
			y = cy - zoom * (float) Telemetry.ap_pitch*0.01745f * _R;
			c.drawCircle(x, y, 16, yellow);
			c.drawCircle(x, y, 10, black);



	    	 double dd= wrap_PI_GRAD(Telemetry.heading-(Commander.heading - Commander.c_heading));//+Commander.c_heading;
	    	 c.drawLine(cx, cy, cx + (float) Math.sin(dd) * _R * 0.8f, cy - (float) Math.cos(dd) * _R * 0.8f, red);//compas

             dd=wrap_PI_GRAD(dd*(180/Math.PI) - Commander.headingOffset);//+Commander.c_heading;
				c.drawLine(cx, cy, cx + (float) Math.sin(dd) * _R * 0.8f, cy - (float) Math.cos(dd)*_R*0.8f, yellow);//compas

		//		Log.i("DKDKD", "head:" + Double.toString(Telemetry.heading - Commander.heading - Commander.c_heading));

   	    	 y=cy-zoom*((float)Math.sin(-Telemetry.pitch*GRAD2RAD))*_R;
	    	 x=cx+zoom*((float)Math.sin(-Telemetry.roll*GRAD2RAD))*_R;
	    	 c.drawCircle(x, y, 15, red);
	    	 c.drawCircle(x, y, 3, black);
	    	 
	    	 
	    	
	    	
	    	 
	    
	    	 
	    	  c.drawLine(cx-_R, cy, cx+_R, cy, grey);
	    	 c.drawLine(cx, cy-_R, cx, cy+_R, grey);
	    	 
	    	 
    	 }else{


				//Bitmap bitmap;


				//Matrix matrix = new Matrix();

				//matrix.postScale(10, 15);
				//matrix.postRotate(45);

				//bitmap = Bitmap.createBitmap(MainActivity.blank, 0, 0, MainActivity.blank.getWidth(), MainActivity.blank.getHeight(), matrix, true);

			//	Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);

			//	c.drawBitmap(MainActivity.blank, 0, 0, paint);

				c.drawText(Double.toString(Telemetry.lat), 20, 100, gpsColor);
				c.drawText(Double.toString(Telemetry.lon), 20, 140, gpsColor);





			}
    //	
    	
            //c.drawLine(0, 0, this.getWidth(), this.getHeight()-x, black);  
           // x++;
    	}catch (Exception e){}
		onDrawAtwork=false;
    }
    
    @Override
   	public boolean onTouchEvent(MotionEvent event) {

   	    // get pointer index from the event object
   	    int pIndex = event.getActionIndex();
   	    
   	    //p=new Point[event.getPointerCount()];
    

   	    // get pointer ID
   	    int pointerId = event.getPointerId(pIndex);

   	    // get masked (not specific to a pointer) action
   	    int maskedAction = event.getActionMasked();

   		    switch (maskedAction) {
   	
	   		    case MotionEvent.ACTION_DOWN:
	   		    case MotionEvent.ACTION_POINTER_DOWN: {
	   		    	
	   		      // We have a new pointer. Lets add it to the list of pointers
	   		  // Log.e("DOWN","DOWN "+Integer.toString(pIndex)+" "+Float.toString(event.getX(pIndex))+" "+Float.toString(event.getY(pIndex)));
	   		    	float x=event.getX(pIndex);
	   		    	float y=event.getY(pIndex);
	   		    //	if (y<touch.bottom && y>touch.top && x>touch.left && x<touch.right){

	   		    		//starty=y;
	   		    	//}
                    thumbed =true;
	   		     break;
	   		    }
	   		 case MotionEvent.ACTION_MOVE:  // a pointer was moved
	   			// Log.e("DOWN","MOVE "+Integer.toString(pIndex)+" "+Float.toString(event.getX(pIndex))+" "+Float.toString(event.getY(pIndex)));
				 starty=BORDER+touchHigh/2;
	   			float x=event.getX(pIndex);
   		    	float y=event.getY(pIndex);
   		    	if (y<touch.bottom && y>touch.top && x>touch.left && x<touch.right){

					if (MainActivity.toHomeF()==false && MainActivity.progF()==false) {
						float dy = starty - y;
						Commander.throttle += dy * 0.00002;
						if (Commander.throttle > 0.99)
							Commander.throttle = 0.99f;
						if (Commander.throttle < 0.06)
							Commander.throttle = 0.06f;
					}
   		    		//Commander.throttle=throttle;
   		    	    //Log.e("DOWN",Float.toString(throttle));
   		    	}else {
					double dx=x-cx;
					double dy=y-cy;
					double distFromCentr=Math.sqrt(dx*dx+dy*dy);
					if (distFromCentr<_R){
						dy=starty-y;
						if (dx>0)
							dy=-dy;
						Commander.headingOffset-=dy*0.011;
						  if (Commander.headingOffset>180)
						      Commander.headingOffset-=(float)(360);
						  if (Commander.headingOffset<=-180)
						      Commander.headingOffset+=(float)(360);

					}


                }
	   			 
	   			 
	   			 
	   			 break;

	   		    case MotionEvent.ACTION_UP:
	   		    case MotionEvent.ACTION_POINTER_UP:
	   		    case MotionEvent.ACTION_CANCEL:
                    thumbed =false;

   	    }
   	    invalidate();
   	    
   		
   	    return true;
   	  }



}