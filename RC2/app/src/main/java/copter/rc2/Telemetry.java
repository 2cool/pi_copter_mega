package copter.rc2;

import android.content.Context;
import android.util.Log;


import java.util.Calendar;



public class Telemetry {

	static public boolean maxTelemetry=false;
	static private boolean connected=false;
	static private boolean motors_is_on=false;
	static private long oldMsgTimer;
	static private int motorsONtimer=0;
	static public String motors_on_timer="00:00";
	static private long oldTimeMS=0;
	static int messageCode;
	static public int satilites=0;
	static public double lat=47.9;
	static public double lon=33.2523;
	static public int r_accuracy_hor_pos=99,r_acuracy_ver_pos=99;//когда включаем смартконтрол мощность меняестя. (исправить)
	static public double roll=0,pitch=0;
	static public double autoLat=0,autoLon=0;
	static private double oldlat,oldlon;
	static public double dist=0,speed=0,v_speed=0,alt_time=0 ,speed_time=0,alt_speed;
	static public String messages=null;
	static public double heading=0;
//	static public String message="message";
	static float motorsTh[];

	static public String lat(){return Double.toString(lat);}

	static double ap_pitch,ap_roll,realThrottle;  //autopilot
	static private  double ap_throttle=0;
	static int gimbalPitch;
	static float _alt=0,old_alt1=-100000;
	static float  relAlt=0;
	static String batery="";
	static final int MIN_VOLT =1280;
	static final int VOLT_50  =1520;
	static boolean F_MIN_VOLT=false;
	static int min_volt=0;
	static boolean F_VOLT50 =false;
	static public double oldAlt=11110,oldLat=0,oldLon=0,oldHeading=110;
	//static public int no_gps_data_counter=100;
	static final long DELTA_TIME_4_MSG = 20;
	static final double GRAD2RAD = 0.01745329251994329576923690768489;


	static public boolean motorsWasOn=false;
	//counter,lat,lon,gpsHeight,bat1,bat2,bat3,pressure,temperature,heading*18000/PI,roll,pitch,messageCode,motorTh0,motorTh1,motorTh2,motorTh3,(bat0+200)/100
	


	static public float settings[]=new float[10];
	static public int n_settings;
	static private int old_control_bits=0;





	static public void init(){
		//Log.i("TTZZ","INIT TELEM");

		old_control_bits=satilites=0;
		r_accuracy_hor_pos=0;

		autoLat=autoLon=0;
		ap_throttle=0.5f;
		batery="";
		F_MIN_VOLT=false;
		F_VOLT50 =false;
		ap_roll=ap_pitch=roll=pitch=0;
		oldMsgTimer=System.currentTimeMillis()+DELTA_TIME_4_MSG;
	}




	static public float corectThrottle(){

		if (ap_throttle < 0.5)
			ap_throttle = 0.5;
		if (ap_throttle > 0.7)
			ap_throttle = 0.7;
		return (float)ap_throttle;//*z;


	}


	static public void readMessages(String msg){
		int i;
		if ((i=msg.indexOf("UPS",0))>=0){
			//Log.i("UPS","read mess "+msg);
			parse_settings(msg, i+3);
		//	i=msg.lastIndexOf(",",i);
		}else {
			messages = msg;
		}
		//



		if (true)return;
		/*

		//if (msgg.length()>2)
          //  Log.i("UPS", "message recived: "+msgg);

		if (msgg.length()>0) {
			//Log.i("OUT_M",msgg);
			Disk.write("RECIVED:" + msgg+"\n");
		}

		int i=0;
		if (Commander.newConnection_) {
			if (msgg.startsWith(Commander.BEGIN_CONVERSATION)){
			//if (msgg.indexOf(Commander.BEGIN_CONVERSATION) == 0) {
				i = 3;
				//Commander.newConnection_ = false;
			} else {
				Commander.new_connection();
				Log.i("OUT_M","new connection");
				return;
			}
			//return;
		}
		while (msgg.length()>=i+3){
			String msg=msgg.substring(i,i+3);
			i+=3;



			if (msg.indexOf("MOD",0)==0){
				if (beginConversationGET)
					Commander.newConnection_ = false;
				String smod=msgg.substring(i,i+3);
				i+=3;
				MainActivity.control_bits=Integer.parseInt(smod);


				if (old_control_bits!=MainActivity.control_bits){

					//if (MainActivity.control_bits==MainActivity.MOTORS_OFF){
					//	Disk.close();
					//}
					if ((MainActivity.control_bits&MainActivity.MOTORS_ON)==0){
						autoLat=0;
						autoLon=0;
					}else
						if ((MainActivity.control_bits&MainActivity.MOTORS_ON)==MainActivity.MOTORS_ON && autoLat==0 && autoLon==0){
							motorsWasOn=true;
							autoLat=lat;
							autoLon=lon;
						}
					if ((old_control_bits&MainActivity.GO2HOME)!=(MainActivity.control_bits&MainActivity.GO2HOME)) {
						if (MainActivity.toHomeF())
							Commander.throttle = 0.5f;
						//MainActivity.horizont_on=MainActivity.compass_on=(MainActivity.control_bits&MainActivity.XY_STAB)!=MainActivity.XY_STAB;

					}else if ((old_control_bits&MainActivity.PROGRAM)!=(MainActivity.control_bits&MainActivity.PROGRAM)) {
						if (MainActivity.progF())
							Commander.throttle = 0.5f;
					}else

						if ((old_control_bits&MainActivity.XY_STAB)!=(MainActivity.control_bits&MainActivity.XY_STAB)){
						//	if (MainActivity.smartCntrF())
						//		Commander.throttle = 0.5f;
							//MainActivity.horizont_on=MainActivity.compass_on=(MainActivity.control_bits&MainActivity.XY_STAB)!=MainActivity.XY_STAB;
						}else
							if ((old_control_bits&MainActivity.Z_STAB)!=(MainActivity.control_bits&MainActivity.Z_STAB)){
								if (MainActivity.altHoldF())
									Commander.throttle = 0.5f;
								else
									corectThrottle();
							}

					old_control_bits=MainActivity.control_bits;
				}
				//Log.i("CNTRB",msg+" "+MainActivity.control_bits);
			}else
			//добавить передачу жпс акураси через меседж
			if (msg.indexOf("HLC",0)==0){
				String locHome[]=msgg.substring(i).split(",",3);
				int n=locHome[0].length()+1+locHome[1].length();
				while (n%3!=0)
					n++;
				i+=n;
				autoLat=0.0000001*Integer.parseInt(locHome[0]);
				autoLon=0.0000001*Integer.parseInt(locHome[1]);
			}else
			if (msg.indexOf("UPS",0)==0){
                //Log.i("UPS","read mess "+msg);
                parse_settings(msgg, i);
				i=msgg.lastIndexOf(",",i);
			}



		}

		long timems = System.currentTimeMillis();
		if ((timems-oldTimeMS)>=1000) {
			if (oldTimeMS == 0) {
				oldTimeMS = System.currentTimeMillis();
			}else {
				if ((old_control_bits&31) >= MainActivity.MOTORS_ON) {
					motorsONtimer += (int) ((timems - oldTimeMS) / 1000);
					int minutes=(int)Math.ceil(motorsONtimer/60);
					int seconds=motorsONtimer-minutes*60;
					motors_on_timer=Integer.toString(minutes)+":"+((seconds<10)?"0":"")+Integer.toString(seconds);
				}
				oldTimeMS = timems;
			}
		}

*/

	}

	static private void parse_settings(String str, int b){
       // Log.i("UPS","parse: "+str.substring(b));
		String v[]=str.substring(b).split(",");
		int n=Integer.parseInt(v[0]);
		if (n<0){
			n_settings=-1;
			return;
		}
		int i=1;
		for (; i<v.length;i++){
			settings[i-1]=Float.parseFloat(v[i]);
		}
		for (; i<=10; i++){
			settings[i-1]=1;
		}
		n_settings=n;
	}













	static int load_int32(byte buf[],int i){
		int vall=       load_uint8(buf,i);
		vall+=    0x100*load_uint8(buf,i+1);
		vall+=  0x10000*load_uint8(buf,i+2);
		vall+=0x1000000*(int)buf[i+3];
		return vall;
	}
	static int load_int16(byte buf[],int i){
		int vall=load_uint8(buf,i);
		int valh=buf[i+1];
		return vall+valh*256;
	}
	static int load_uint8(byte buf[],int i){
		int vall=buf[i];
		vall&=255;
		//if (vall<0)
		//	vall=0-vall;
		return vall;
	}
	//////////////////////////////////////////////////////////////
	static int old_alt=0;








	static public double dist(double lat1,double lon1, double lat2, double lon2){
		final double R = 6371e3; // metres
		double φ1 = lat1*GRAD2RAD;
		double φ2 = lat2*GRAD2RAD;
		double Δφ = (lat2-lat1)*GRAD2RAD;
		double Δλ = (lon2-lon1)*GRAD2RAD;

		double a = Math.sin(Δφ/2) * Math.sin(Δφ/2) +
				Math.cos(φ1) * Math.cos(φ2) *
						Math.sin(Δλ/2) * Math.sin(Δλ/2);
		double c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));

		double d = R * c;
		return d;
	}
static int old_index=-1;
	static int log_s=0;
	static int block_cnt=0;










	static final int  mask=63;
	static byte [][]log_buffer=new byte[mask+1][1024];
	static int readedI=0;
	static int writedI=0;
	static boolean logThread_f=true;
	static boolean thread_started=false;
static void startlogThread(){
	if (thread_started==true)
		return;
	thread_started=true;
	Thread thread = new Thread() {
		@Override
		public void run() {
			while (logThread_f==true){
				if (writedI<readedI){
					int adr=writedI&mask;
					int mes_len=log_buffer[adr][0]&255;
					mes_len +=256*log_buffer[adr][1];
					Disk.write2Log(log_buffer[adr],0,mes_len);
					writedI++;
				}else {
					try {
						Thread.sleep(5);
					} catch (InterruptedException ex) {
						Thread.currentThread().interrupt();
					}
				}
			}

		}
	};
	thread.start();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
	static public void bufferReader_(byte buf[],int buf_len){

		int i=0;
		Commander.copter_is_busy=(buf_len<=4);
		if (Commander.copter_is_busy){
			return;
		}
		//if (buf_len<=4)
		//	return;
		MainActivity.control_bits=load_int32(buf,i);

		i+=4;


		realThrottle  = load_int16(buf,i);
		i+=2;
		realThrottle=(realThrottle-1000)*0.001f;


		int ilat=load_int32(buf,i);
		i+=4;
		lat=0.0000001*(double)ilat;
		int ilon=load_int32(buf,i);
		i+=4;
		lon=0.0000001*(double)ilon;
		//Log.i("DKDKD",Double.toString(lat)+ " "+Double.toHexString(lon));
		//---------------------------
		if (connected == false) {
			if (MainActivity.motorsOnF()){
				Disk.loadLatLonAlt("/sdcard/RC/start_location.save",false);
				oldlat=lat;
				oldlon=lon;
			}
			connected = true;
		}else
			if (MainActivity.motorsOnF() && motors_is_on != (MainActivity.motorsOnF()) ){
				autoLat=oldlat=lat;
				autoLon=oldlon=lon;
				Disk.saveLatLonAlt("/sdcard/RC/start_location.save",lat,lon,0);

			}
			if (motors_is_on!=MainActivity.motorsOnF())
				motors_is_on=MainActivity.motorsOnF();




		long timems = System.currentTimeMillis();
		if ((timems-oldTimeMS)>=1000) {
			if (oldTimeMS == 0) {
				oldTimeMS = System.currentTimeMillis();
			}else {
				if (MainActivity.motorsOnF()) {
					motorsONtimer += (int) ((timems - oldTimeMS) / 1000);
					int minutes=(int)Math.ceil(motorsONtimer/60);
					int seconds=motorsONtimer-minutes*60;
					motors_on_timer=Integer.toString(minutes)+":"+((seconds<10)?"0":"")+Integer.toString(seconds);
				}
				oldTimeMS = timems;
			}
		}



	    if (MainActivity.motorsOnF()){
				//вічисляем растояние до старта
			dist=(autoLat==0 || autoLon==0)?0:dist(autoLat,autoLon,lat,lon);
			double dDist=dist(oldlat,oldlon,lat,lon);
			oldlat=lat;
			oldlon=lon;
			long t=System.currentTimeMillis();
			double dt=0.001*(double)(t-speed_time);
			speed_time=t;
			speed+=(dDist/dt-speed)*dt;

		}
		r_accuracy_hor_pos = buf[i++];
		r_acuracy_ver_pos = buf[i++];
		_alt =load_int16(buf,i);
		i+=2;
		_alt*=0.1;

		if (old_alt1==-100000)
			old_alt1=_alt;
			double dalt=_alt-old_alt1;
			old_alt1=_alt;
			long t=System.currentTimeMillis();
			double dt=0.001*(double)(t-alt_time);
			alt_time=t;
			v_speed+=(dalt/dt-v_speed)*0.05;
			if ((MainActivity.control_bits&MainActivity.MOTORS_ON)==MainActivity.MOTORS_ON){
				relAlt=_alt;
			}
		pitch=buf[i++];
		roll=buf[i++];
		ap_pitch=buf[i++];
		ap_roll=buf[i++];
		int batVolt=load_int16(buf,i);
		i+=2;
		//int b0= 256+load_uint8(buf,i++);
		//int b1= 256+load_uint8(buf,i++);
		//int b2= 256+load_uint8(buf,i++);

		F_MIN_VOLT=batVolt<=MIN_VOLT;
		batery=Integer.toString(batVolt/4);
		gimbalPitch= buf[i++];
		heading=1.4173228346456692913385826771654*(float)buf[i++];

		//тут читаем сообщения если они кому нужни так как у нас все данніе риходят и муд
		int mes_len=load_int16(buf,i);
		i+=2;
		if ( mes_len >0 ) {
			readMessages(new String(buf, i, mes_len));
		}
		i+=mes_len;
		mes_len=load_int16(buf,i);
		i+=2;
		block_cnt=0;
		while (mes_len>0){
			System.arraycopy(buf, i-2,log_buffer[readedI&mask], 0,  mes_len+2);
			readedI++;

			block_cnt++;
			log_s+=mes_len;


			i+=mes_len;
			mes_len=load_int16(buf,i);
			i+=2;
		}
		//if (block_cnt>1)
		//Log.i("DISK","W... "+Integer.toString(log_s)+"  blocks  "+Integer.toString(block_cnt));

		float z = (float) Math.sqrt( (1 - pitch * pitch*GRAD2RAD*GRAD2RAD ) * (1 - roll * roll * GRAD2RAD*GRAD2RAD) );
		//ap_throttle=realThrottle*z;
		if ((MainActivity.control_bits&MainActivity.MOTORS_ON)==0){
			ap_throttle=realThrottle*z;
		}else {
			ap_throttle += (realThrottle * z - ap_throttle) * 0.03;
		}

/*
	//	msg+=",";
		final long cttt=System.currentTimeMillis();
		if (oldMsgTimer<=cttt) {
			oldMsgTimer+=DELTA_TIME_4_MSG;
			if (msg.matches("TELE,,,,,,,,,,,,,,"))
				Disk.write("n\n");
			else
				Disk.write(msg+"\n");
			while (oldMsgTimer<=cttt){
				oldMsgTimer+=DELTA_TIME_4_MSG;
				Disk.write("t\n");
			}

			oldMsgTimer=cttt;
		}
		readMessages(new String(buf,i,buf_len-i));

		float z = (float) Math.sqrt( (1 - pitch * pitch*GRAD2RAD*GRAD2RAD ) * (1 - roll * roll * GRAD2RAD*GRAD2RAD) );
		//ap_throttle=realThrottle*z;
		if ((MainActivity.control_bits&MainActivity.MOTORS_ON)==0){
			ap_throttle=realThrottle*z;
		}else {
			ap_throttle += (realThrottle * z - ap_throttle) * 0.03;
		}
		*/
	}




}
