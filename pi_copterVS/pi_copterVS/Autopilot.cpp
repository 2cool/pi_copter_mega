/*
message for LOG

MXT     max throttle
OFT		off_throttle
MD1		motors_do_on
MD0		motors do off
LWV		low voltage
GPE		gps error
CNF		controled fall
MXG		перегрузка
MODXXX  резим работі автопилота,XXX - число - control_bits

TFR	out of Perimetr gps
THG out of Perimetr high



*/

//при shift yaw на 180 грудусов летает в обратном направление при управлении наклоном телефона
//при старте его кидает в сторону. наверное проблема в необнулении стаб спид



#define WIND_X 5
#define WIND_Y 1
#define WIND_Z 0.5





#include "define.h"
#include "WProgram.h"

#include "mi2c.h"
#include "MS5611.h"
#include "Autopilot.h"
#include "Balance.h"

#include "commander.h"

#include "GPS.h"


#include "Telemetry.h"
#include "Stabilization.h"
#include "debug.h"
#include "Prog.h"

#include "mi2c.h"

#include <cstdio>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>




#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>

#include  "Log.h"


using namespace std;

int camera_mode;
enum { CAMMERA_OFF, CAMERA_RECORDING, CAMERA_TRANCLATE };



/*
chtoby zapustit' strim
python  Camera_video_stream.py
ffmpeg -rtsp_transport udp -i "rtsp://192.168.42.1:554/live" -c copy -f h264 udp://android_phone_address:5544

pn android run
ffplay udp://android_address:5544


*/


/*
void start_video() {
	if (camera_mode == CAMERA_RECORDING) {
		usleep(2000000);
		printf( "recording video START\n");
		string s = "/home/igor/ffmpeg_cedrus264_H3/ffmpeg -f v4l2 -channel 0 -video_size 640x480 -i /dev/video0 -pix_fmt nv12 -r 30 -b:v 64k -c:v cedrus264 /home/igor/logs/video";
		s += std::to_string(Log.counter_());
		s += "_";
		s += std::to_string(Log.run_counter);
		s += ".mp4 > /dev/null 2>&1";
		system(s.c_str());
		
	}
	if (camera_mode=CAMERA_TRANCLATE){
		printf( "transmiting video START\n");
		string adr = WiFi.get_client_addres();
		string s = "/home/igor/ffmpeg_cedrus264_H3/ffmpeg -f v4l2 -framerate 30 -video_size 640x480 -i /dev/video0 -f mpegts udp://"+adr+":1234 > /dev/null 2>&1";
		system(s.c_str());
		
		
	}
}

*/

//каждий новий режим работі добовляется в месадж





void AutopilotClass::init(){/////////////////////////////////////////////////////////////////////////////////////////////////

	if (init_shmPTR())
		return;


	shmPTR->sim800_reset = false;
	time_at_startd = old_time_at_startd = 0;
	camera_mode = CAMMERA_OFF;
	lowest_height = shmPTR->lowest_altitude_to_fly;
	last_time_data_recivedd = 0;
	
	Balance.init();
	MS5611.init();


	sens_z = 6;
	sens_xy = 0.2f;

	newData = false;

	//holdLocationStartStop(false);
	//holdAltitudeStartStop(false);

	height_to_lift_to_fly_to_home = HIGHT_TO_LIFT_ON_TO_FLY_TO_HOME;
	aPitch = aRoll = aYaw_=0;

	//was_connected_to_wifi = NO_WIFI_WATCH_DOG_IN_SECONS < 30;
	control_bits = DEFAULT_STATE;
	old_control_bits = 0;
	aPitch = aRoll = 0;

//	for (int i = 0; i < 10; i++)
//	while (MS5611.loop() != 0)
//		delay(10);

	controlDeltaTimed = 0;

	gimBalPitchZero = gimBalRollZero= gimbalPitch=0;

	mega_i2c.gimagl(gimBalPitchZero, gimBalRollZero);

}

float AutopilotClass::corectedAltitude4tel() {
	return MS5611.altitude();
	//return ((control_bits & Z_STAB) == 0) ? MS5611.altitude() : Stabilization.getAltitude();
}


void AutopilotClass::add_2_need_yaw(float speed, const float dt){ 
	aYaw_ += speed*dt;
	aYaw_ = wrap_180(aYaw_);
}



void AutopilotClass::add_2_need_altitude(float speed, const float dt){
	static bool set_alt = false;
	if (speed != 0) {
		set_alt = true;
		if (speed > 0) {
			if (speed > MAX_VER_SPEED_PLUS)
				speed = MAX_VER_SPEED_PLUS;
		}
		else
			if (speed < MAX_VER_SPEED_MINUS)
				speed = MAX_VER_SPEED_MINUS;


		if (speed<-0.2 && (tflyAtAltitude < lowest_height || flyAtAltitude < lowest_height))
			speed = max(-0.2, speed);

		tflyAtAltitude += speed * dt;

		//if (tflyAtAltitude < lowest_height)
		//	tflyAtAltitude = lowest_height;

		flyAtAltitude = tflyAtAltitude + Stabilization.getDist_Z(speed);
		//if (flyAtAltitude < lowest_height)
		//	flyAtAltitude = lowest_height;

		//printf("f@alt %f\n", flyAtAltitude);
	}
	else {
		if (set_alt) {
			set_alt = false;
			flyAtAltitude = tflyAtAltitude= MS5611.altitude();

		}
	}
}
//-------------------------------------------------------------------------
void AutopilotClass::smart_commander(const float dt){
	if (Commander.getPitch() != 0 || Commander.getRoll() != 0){
		const float addX = sens_xy*(Commander.getPitch());
		const float addY = -sens_xy*(Commander.getRoll());
		const float cyaw = Commander.getYaw()*GRAD2RAD;
		const float cosL = (float)cos(cyaw);
		const float sinL = (float)sin(cyaw);
		float speedX = addX * cosL + addY *sinL;
		float speedY = -(addX * sinL - addY *cosL);
		const float speed2 = (speedX*speedX + speedY*speedY);
		const float maxSpeed2 = Stabilization.max_speed_xy*Stabilization.max_speed_xy;
		if (speed2>maxSpeed2){
			float k = (float)sqrt(maxSpeed2 / speed2);
			speedY *= k;
			speedX *= k;

		}
		GPS.loc.add2NeedLoc(speedX, speedY, dt);
	}
	else{
		GPS.loc.setSpeedZero();
	}
}

static double last_beep_timed = 0;



void AutopilotClass::loop(){/////////////////////////////////////////////////////////////////////////////////////////////////

	
	
	
	const float dt = Mpu.timed - controlDeltaTimed; 
	if (dt < 0.05)
		return;

	controlDeltaTimed = Mpu.timed;

	if (shmPTR->control_bits_4_do)
		set_control_bits(shmPTR->control_bits_4_do);
	shmPTR->control_bits_4_do = 0;
	shmPTR->control_bits = control_bits;


	
	gimBalRollCorrection();

	


#ifdef LOST_BEEP
	
	if ( GPS.loc.lat_home!=0 && GPS.loc.lon_home!=0  && Mpu.timed - last_time_data_recivedd>3 && Mpu.timed - last_beep_timed > 3) {
		last_beep_timed = Mpu.timed;
		mega_i2c.beep_code(B_CONNECTION_LOST);
	}
#endif

	



	if (MS5611.fault() && motors_is_on() && go2homeState() == 0) {
		//sim.send_sos(e_BARROMETR_FAULT);
		going2HomeON(true);
	}



	if (control_bits&CONTROL_FALLING){
		aYaw_ = Mpu.get_yaw();
		off_throttle(false,"cntr_fall");
	}
	else{
		if (control_bits & PROGRAM){
			
			Prog.loop();
		}
		else{
			if (control_bits & GO2HOME){
				go2HomeProc(dt);
			}
			else{
				double timelag = Mpu.timed - last_time_data_recivedd;
				if (motors_is_on() && timelag > CONNECTION_LOST_TIMEOUT) {
					connectionLost_();
					return;
				}
#ifndef OFF_TIMELAG
				if ((timelag > TIMEOUT_LAG))
					Commander.data_reset();
#endif
				if (compass_onState())
					aYaw_ = Commander.get_yaw_minus_offset();
				if (control_bits & Z_STAB){
					const float thr = Commander.getThrottle();
					const float speed = (thr - MIDDLE_POSITION) * sens_z;
					add_2_need_altitude(speed, dt);
				}
				else{
					throttle = Commander.getThrottle();
				}
				if (control_bits & XY_STAB){
					smart_commander(dt);
				}
				else{
					aPitch = Commander.getPitch();
					aRoll = Commander.getRoll();
				}
			}
		}
			
	}

	if (Mpu.timed<30 || Mpu.acc_callibr_timed > Mpu.timed)
		mega_i2c.set_led_mode(2, 5, true);
	else {

		if (motors_is_on() == false) {
			mega_i2c.set_led_mode(0, 10, false);
		}
		else {
			if (control_bits&CONTROL_FALLING)
				mega_i2c.set_led_mode(2, 255, false);
			else {
				int n = ((control_bits&(XY_STAB | Z_STAB)) == (XY_STAB | Z_STAB)) ? 3 : 1;
				mega_i2c.set_led_mode(n, 255, (control_bits & GO2HOME) | (control_bits & PROGRAM));
			}

		}
	}



	if (shmPTR->sim800_reset) {
		shmPTR->sim800_reset = false;
		mega_i2c.sim800_reset();
	}



	log();

}

void AutopilotClass::log() {
	if (old_control_bits != control_bits && Log.writeTelemetry) {
		Log.block_start(LOG::AUTO);
		Log.loaduint32t(control_bits);
		old_control_bits = control_bits;
		Log.block_end();
	}
}
string AutopilotClass::get_set(){
	
	ostringstream convert;
	convert << \
		height_to_lift_to_fly_to_home << "," << \
		MAX_THROTTLE_ << "," << \
		MIN_THROTTLE_ << "," << \
		sens_xy << "," << \
		sens_z << "," << \
		lowest_height << "," << Debug.n_debug << "," << camera_mode << "," << -gimBalPitchZero << "," << -gimBalRollZero << ",";
	string ret = convert.str();
	return string(ret);
}

void AutopilotClass::set(const float ar[]){
	cout << "Autopilot set\n";
	int error = 1;

	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){
		int i = 0;
		error = 0;
		
		
		error += Commander._set(ar[i++], height_to_lift_to_fly_to_home);
		//Balance.set_min_max_throttle(ar[i++], ar[i++]);
		i += 2;
		error += Commander._set(ar[i++], sens_xy);
		error += Commander._set(ar[i++], sens_z);
		error += Commander._set(ar[i++], lowest_height,false);
		Debug.n_debug = (int)ar[i++];
		camera_mode = (int)ar[i++];
		gimBalPitchZero= -constrain(ar[i],-15,15);
		i++;
		gimBalRollZero = -constrain(ar[i],-15,15);
		i++;
		mega_i2c.gimagl(gimBalPitchZero, gimBalRollZero);
		if (error == 0){
			int ii = 0;
			cout << "Safe set:\n";

			for (ii = 0; ii < i; ii++){
				cout << ar[ii] << endl;
			}
			cout << ar[ii] << endl;
		}
	}
	if (error>0){
		cout << "ERROR\n";
	}
}







void AutopilotClass::set_new_altitude(float alt){
	tflyAtAltitude = flyAtAltitude = alt;
}

bool AutopilotClass::holdAltitude(float alt){

	tflyAtAltitude = flyAtAltitude = alt;
	control_bits |= Z_STAB;
	//setbuf(stdout, NULL);
	cout << "FlyAt: " << flyAtAltitude << "\t"<<Mpu.timed << endl;

	return true;
}

bool AutopilotClass::holdAltitudeStartStop(){

	if (!motors_onState() || go2homeState() || progState())
		return false;
	bool h = (control_bits & Z_STAB)==0;
	if (h){
		Stabilization.resset_z();
		return holdAltitude(MS5611.altitude());
	}
	else{
		control_bits ^= Z_STAB;
		throttle = HOVER_THROTHLE;
		Stabilization.resset_z();
		return true;
		
	}
	return false;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
enum{ JUMP = 0, HOWER = 1, GO_UP_OR_NOT = 2, TEST_ALT1 = 3, GO2HOME_LOC = 4, TEST4HOME_LOC = 5, START_FAST_DESENDING = 6, TEST_ALT2 = 7,SLOW_DESENDING=8 };
bool AutopilotClass::go2HomeProc(const float dt){
	
		
 switch (go2homeIndex){
	 

	case JUMP:{	
#ifdef FALL_IF_STRONG_WIND
		dist2home_at_begin2 = GPS.loc.dist2home_2;
#endif
		if (MS5611.altitude() < 3)
			holdAltitude(3);
		go2homeIndex=HOWER;
		break;
	}

	case HOWER:{	//висеть 20 секунд
		f_go2homeTimer += dt;
		if (f_go2homeTimer > ((howeAt2HOME)?HOWER_TIME:4)){ //for stabilization and connection
			go2homeIndex = GO_UP_OR_NOT;
		}
		break;

	}
	case GO_UP_OR_NOT:{
		const float accuracy = ACCURACY_XY + GPS.loc.accuracy_hor_pos_;
		if (fabs(GPS.loc.x2home) <= accuracy && fabs(GPS.loc.y2home) <= accuracy){
			f_go2homeTimer = 6; //min time for stab
			go2homeIndex = (MS5611.altitude() <= (FAST_DESENDING_TO_HIGH)) ? SLOW_DESENDING : START_FAST_DESENDING;
			GPS.loc.setNeedLoc2HomeLoc();
			break;
		}	
		//поднятся  на высоту  X м от стартовой высоты или опуститься
		tflyAtAltitude=flyAtAltitude = height_to_lift_to_fly_to_home;
		go2homeIndex = TEST_ALT1;
		break;
	}
	case TEST_ALT1:{
		if (fabs(MS5611.altitude() - flyAtAltitude) <= (ACCURACY_Z)){
			go2homeIndex = GO2HOME_LOC;
		}
		break;
	}
	case GO2HOME_LOC:{//перелететь на место старта
		// led_prog = 4;
		GPS.loc.setNeedLoc2HomeLoc();
		go2homeIndex = TEST4HOME_LOC;
		aYaw_ = -GPS.loc.dir_angle_GRAD;
		aYaw_ = wrap_180(aYaw_);
		break;
	}
	case TEST4HOME_LOC:{//прилет на место старта
			   
		const float accuracy = ACCURACY_XY + GPS.loc.accuracy_hor_pos_;
		if (fabs(GPS.loc.x2home) <= accuracy && fabs(GPS.loc.y2home) <= accuracy){
			go2homeIndex = START_FAST_DESENDING;
			f_go2homeTimer = 0;
		}
		break;
	}
	case START_FAST_DESENDING:
		f_go2homeTimer += dt;
		if (f_go2homeTimer > 5){
			go2homeIndex = TEST_ALT2;
			tflyAtAltitude = flyAtAltitude = (FAST_DESENDING_TO_HIGH);
		}
		break;


	case TEST_ALT2:{//спуск до FAST_DESENDING_TO_HIGH метров
		if (fabs(MS5611.altitude() - flyAtAltitude) < (ACCURACY_Z)){
			go2homeIndex = SLOW_DESENDING;
		}
			   
		break;
	}
	case SLOW_DESENDING:
	{ 
		if (MS5611.fault()) {
			control_falling(e_BARROMETR_FAULT);
			return false;
		}
		//плавній спуск
		if (MS5611.altitude()>lowest_height){
			float k = MS5611.altitude()*0.05f;
			if (k < 0.1f)
				k = 0.1f;
			flyAtAltitude -= (dt*k);
			tflyAtAltitude = flyAtAltitude;
		}
		else{
			tflyAtAltitude = flyAtAltitude = lowest_height;
		}
						   
		break;
	}
 }
#ifdef FALL_IF_STRONG_WIND
	if (GPS.loc.dist2home_2 - dist2home_at_begin2 > (MAX_DIST_ERROR_TO_FALL*MAX_DIST_ERROR_TO_FALL)){
		Autopilot.off_throttle(false, e_TOO_STRONG_WIND);
	}
#endif
	return true;
}
bool AutopilotClass::going2HomeON(const bool hower){
	
	Stabilization.setDefaultMaxSpeeds();

	howeAt2HOME = hower;//зависнуть на месте или нет

	bool res = holdAltitude(MS5611.altitude());
	res &= holdLocation(GPS.loc.lat_, GPS.loc.lon_);
	if (res){
		control_bits |= GO2HOME;
		f_go2homeTimer = 0;
		//Out.println("Hanging on the site!");
		cout << "go2home" << "\t"<<Mpu.timed << endl;
		go2homeIndex=JUMP;
	}
	return res;
}

bool AutopilotClass::going2HomeStartStop(const bool hower){
	
	if (!motors_onState())
		return false;
	bool f = (control_bits & GO2HOME);
	if (f){
		tflyAtAltitude = flyAtAltitude;
		control_bits ^=GO2HOME;
		holdLocation(GPS.loc.lat_, GPS.loc.lon_);
		return true;
	}
	else{
		if (progState())
			start_stop_program(true);
		return going2HomeON(hower);
	}
}


bool AutopilotClass::holdLocation(const long lat, const long lon){
	control_bits &= (0xffffffff ^ (COMPASS_ON | HORIZONT_ON));
	aPitch = aRoll = 0;
	//if (holdAltitude()){

		
		GPS.loc.setNeedLoc(lat,lon);
		cout << "Hower at: " << GPS.loc.lat_ << " " << GPS.loc.lon_ << "\t"<<Mpu.timed << endl;;

		Stabilization.init_XY(0, 0);


		control_bits |= XY_STAB;
		return true;
	//}
	//else
	//	return false;
}

bool AutopilotClass::holdLocationStartStop(){/////////////////////////////////////////////////////////////////////////////////////////////////
	if (!motors_onState() || go2homeState() || progState())
		return false;
	bool h = (control_bits &XY_STAB)==0;
	if (h){
		Stabilization.resset_xy_integrator();
		return holdLocation(GPS.loc.lat_, GPS.loc.lon_);
	}
	else{
		control_bits |= (COMPASS_ON | HORIZONT_ON);
			control_bits ^=  XY_STAB;
			Stabilization.resset_xy_integrator();
			//holdAltitude();
			return true;
	}
	return false;
}
/*
beep codes
{0, B00001000, B00001001, B00001010, B00001011, B00001100, B00001101, B00001110, B00001111, B00000001, B00000010, B00000011, B00000100, B00000101, B00000110, B00000111 };//4 beeps. 0 short 1 long beep
*/
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

bool AutopilotClass::motors_do_on(const bool start, const string msg){////////////////////////  M O T O R S  D O  ON  /////////////////////////////////////////////////////////////////////////

	cout << msg << "-";
	
	if (start){
		//printf( "\MS5611 err: %f\n",MS5611.getErrorsK());
#ifndef FALSE_WIRE
		cout << "on ";
		if (Mpu.timed < 25) {
			cout << "\n!!!calibrating!!! to end:"<< 25-millis()/1000 <<" sec." << "\t"<<Mpu.timed << endl;
			mega_i2c.beep_code(B_MS611_ERROR);
			return false;
		}
#else
		Emu.init(WIND_X, WIND_Y, WIND_Z);
#endif
		

#define MAX_MACC 0.1f

		if (Hmc.do_compass_motors_calibr || (Mpu.gyro_calibratioan && Hmc.calibrated)){

			if (Telemetry.low_voltage){
				Telemetry.addMessage(e_LOW_VOLTAGE);
				cout << " LOW VOLTAGE" << "\t"<<Mpu.timed << endl;
				mega_i2c.beep_code(B_LOW_VOLTAGE);
				return false;
			}

			if (Hmc.do_compass_motors_calibr==false && GPS.loc.accuracy_hor_pos_ > MIN_ACUR_HOR_POS_2_START ){
				cout << " GPS error" << "\t"<<Mpu.timed << endl;
				mega_i2c.beep_code(B_GPS_ACCURACY_E);
				Telemetry.addMessage(e_GPS_ERROR);

				//return false;
			}
			time_at_startd = Mpu.timed;
			Telemetry.update_voltage();
			
			control_bits |= MOTORS_ON;

			cout << "OK" << "\t"<<Mpu.timed<<endl;

			GPS.loc.setHomeLoc();

			MS5611.copterStarted();
			tflyAtAltitude = flyAtAltitude = MS5611.altitude();
			
			Mpu.max_g_cnt = 0;

			if (control_bits&Z_STAB) 
				holdAltitude(shmPTR->fly_at_start);
			
			if (control_bits&XYSTAB)
				holdLocation(GPS.loc.lat_, GPS.loc.lon_);

			//control_bits |= (HORIZONT_ON | COMPASS_ON);

			Stabilization.resset_z();
			Stabilization.resset_xy_integrator();
			aYaw_ = -Mpu.get_yaw();
			//fflush(Debug.out_stream);
			start_timed = Mpu.timed;

#ifdef DEBUG_MODE
			printf( "\nhome loc: %i %i \nhome alt set %i\n", GPS.loc.lat_, GPS.loc.lon_, (int)flyAtAltitude);
#endif

			Log.run_counter++;
			//if (camera_mode) {//---------------------------------------------------
			//	thread t(start_video);
			//	t.detach();

			//}


		}
		else{
			if (Hmc.calibrated == false){
				cout << "compas, ";
				mega_i2c.beep_code(4);

			}
			if (Mpu.gyro_calibratioan == false){
				cout << "gyro";
				mega_i2c.beep_code(5);

			}
			cout << " calibr FALSE" << "\t"<<Mpu.timed<<endl;
		}
	}//------------------------------OFF----------------
	else {
		old_time_at_startd = Mpu.timed;
		

#ifdef FALSE_WIRE
		Emu.init(WIND_X, WIND_Y, WIND_Z);
#endif
		cout << "off ";
		Telemetry.addMessage(i_OFF_MOTORS);
		off_throttle(true, msg);

		cout << "OK" << "\t"<<Mpu.timed<<endl;

		//if (camera_mode) {//----------------------------------
		//	thread t(stop_video);
		//	t.detach();

		//}
	}
	return true;
}

void AutopilotClass::control_falling(const string msg){
	if (motors_is_on() && (control_bits & CONTROL_FALLING) == 0){
		throttle = Mpu.fall_thr*Balance.powerK();
		aPitch = aRoll = 0;
#ifdef DEBUG_MODE
		printf( "CNTROLL FALLING\n");
#endif
		Telemetry.addMessage(msg);
		Telemetry.addMessage(i_CONTROL_FALL);
		control_bits = CONTROL_FALLING | MOTORS_ON;
	}
}

bool AutopilotClass::off_throttle(const bool force, const string msg){/////////////////////////////////////////////////////////////////////////////////////////////////
	
	if ( force)
	{


		cout << "force motors_off " << msg << ", alt: " << (int)MS5611.altitude() << ", time " << (int)Mpu.timed << endl;
		Balance.set_off_th_();
		Telemetry.addMessage(msg);
		control_bits = DEFAULT_STATE;
		return true;
	}
	else{
	//	if (control_bits_ & (255 ^ (COMPASS_ON | HORIZONT_ON)))
	//		return true;

		//if (MS5611.altitude()  < 2){
		//	motors_do_on(false,msg);
		//}
		//else{
			control_falling(msg);
			
			//Out.println(throttle);
		//}
	}
	return false;

}

void AutopilotClass::connectionLost_(){ ///////////////// LOST

	cout << "connection lost" << "\t"<<Mpu.timed<<endl;
	//Out.println("CONNECTION LOST");
	

	//Telemetry.addMessage(e_LOST_CONNECTION);
	//Out.println("con lost!");
	//Out.println(millis());

	Commander.init();

#ifdef OFF_MOTOR_IF_LOST_CONNECTION
if (motors_is_on())
off_throttle(true, "lost connection");
return;
#endif

	if (motors_is_on())
		if (go2homeState() == false && progState() == false) {
			aPitch = aRoll = 0;

			if (going2HomeON(true) == false && (Mpu.timed - last_time_data_recivedd) > (CONNECTION_LOST_TIMEOUT*3)) {
				off_throttle(false, e_NO_GPS_2_LONG);
			}
		}
	
}
void AutopilotClass::calibration() {/////////////////////////////////////////////////////////////////////////////////////////////////

	cout << "Set Calibr NOT USET.\n";
	/*
	if (abs(cPitch + Commander.pitch) > 0.1 || abs(cRoll + Commander.roll) > 0.1)
		return;

	cPitch += Commander.pitch;
	Commander.pitch = 0;
	cRoll += Commander.roll;
	Commander.roll = 0;
	*/
}





/*




if (ctrl_flag == MANUAL){//---------------manual
	if (flyAtPressure >= SET_HEIGHT_MANUAL){
		throttle = Commander.throttle;
		if (throttle > max_throtthle){
			throttle = max_throtthle;
		}
		startThrottle = throttle;
	}
	else{
		addZ = (startThrottle - Commander.throttle) * 12;
		holdHeight();
		if (throttle > max_throtthle)
			throttle = max_throtthle;
	}
	if (smart_ctrl == false){


		aPitch = Commander.pitch;
		aRoll = Commander.roll;
	}
	else{
		addX -= (Commander.pitch * 10 + addX)*0.1;
		addY += (Commander.roll * 10 - addY)*0.1;
		go2location();
	}
	aPitch = correctingAngle(aPitch);
	aRoll = correctingAngle(aRoll);
	wdt_mask |= CONTROL;

	*/

void AutopilotClass::compass_tr() {
	if (!progState() && !go2homeState())
		control_bits ^= COMPASS_ON;
}

void AutopilotClass::horizont_tr() {
	if ((control_bits & GO2HOME) == 0 && (control_bits & PROGRAM) == 0)
		control_bits ^= HORIZONT_ON;
}


void AutopilotClass::gimBalPitchADD(const float add) {
	if (!progState())
		if (mega_i2c.gimagl((gimBalPitchZero + gimbalPitch + add), gimBalRollZero))
			gimbalPitch += add;

}
void AutopilotClass::program_is_loaded(bool set) {
	if (set) 
		control_bits |= PROGRAM_LOADED;
	else
		control_bits &= -1 ^ PROGRAM_LOADED;
}
bool AutopilotClass::start_stop_program(const bool stopHere){
	if (motors_is_on()) {
		if (progState()) {
			control_bits ^= PROGRAM;
			
			Prog.clear();
			Stabilization.setDefaultMaxSpeeds();
			if (stopHere) {
				float alt = MS5611.altitude();
				if (alt < 10)
					alt = 10;
				holdAltitude(alt);
				holdLocation(GPS.loc.lat_, GPS.loc.lon_);
			}
			return true;
		}
		else {
			if (Prog.start()) {
				if (go2homeState())
					going2HomeStartStop(false);
				bool res = holdAltitude(MS5611.altitude());
				res &= holdLocation(GPS.loc.lat_, GPS.loc.lon_);
				if (res) {
					control_bits |= PROGRAM;
					cout << "prog started" << "\t"<<Mpu.timed << endl;;
					return true;
				}
			}
			return false;
		}
	}
	return false;
	
}

bool AutopilotClass::set_control_bits(uint32_t bits) {
	if (bits==0)
		return true;
	//	uint8_t mask = control_bits_^bits;
	//printf("comm=%i\n", bits);
	if (MOTORS_ON&bits)  {
		Hmc.do_compass_motors_calibr = false;
		bool on = motors_is_on() == false;
		on = motors_do_on(on, m_START_STOP);
		if (on == false) {
			cout << "motors on denied!"<< "\t"<<Mpu.timed<<endl;
		}
	}

	if (bits & GIMBAL_PLUS)
		gimBalPitchADD(1);
	if (bits & GIMBAL_MINUS)
		gimBalPitchADD(-1);
	if (control_bits&CONTROL_FALLING)
		return false;

	if (control_bits&MOTORS_ON) {
		if (bits & GO2HOME)
		going2HomeStartStop(false);

		if (bits & PROGRAM)
			start_stop_program(true);

		if (bits & Z_STAB)
			holdAltitudeStartStop();

		if (bits & XY_STAB)
			holdLocationStartStop();


		if (bits & COMPASS_ON)
			compass_tr();

		if (bits & HORIZONT_ON)
			horizont_tr();
	}
	else 
		control_bits ^= bits & (COMPASS_ON | HORIZONT_ON | XY_STAB | Z_STAB);
	
	//-----------------------------------------------
	if (bits & (MPU_ACC_CALIBR | MPU_GYRO_CALIBR)) {
		if (Mpu.timed > 25) {
			control_bits |= (MPU_ACC_CALIBR | MPU_GYRO_CALIBR);
			Mpu.new_calibration(!(bits&MPU_ACC_CALIBR));
			control_bits &= (0xffffffff ^ (MPU_ACC_CALIBR | MPU_GYRO_CALIBR));
		}
	}
	if (bits & COMPASS_MOTOR_CALIBR) {
		Hmc.start_motor_compas_calibr();
		if (Hmc.do_compass_motors_calibr)
			control_bits |= COMPASS_MOTOR_CALIBR;
	}
	if (bits & COMPASS_CALIBR) {
		control_bits |= COMPASS_CALIBR;
		Hmc.calibration(true);
		control_bits &= ~COMPASS_CALIBR;
	}
	if (bits & SHUTDOWN)
	{
		shutdown();
	}
	if (bits & REBOOT)
	{
		reboot();
	}

	return true;
}



int  AutopilotClass::reboot() {
	if (motors_is_on() == false) {
		cout << "REBOOT" << "\t"<<Mpu.timed << endl;
		shmPTR->reboot = 1;
		shmPTR->run_main = false;
		return 0;
	}else
		return -1;
}
int  AutopilotClass::shutdown() {
	if (motors_is_on() == false) {
		cout << "SHUTD" << "\t"<<Mpu.timed << endl;
		shmPTR->reboot = 2;
		shmPTR->run_main = false;
		return 0;
	}
	else
		return -1;
}
int  AutopilotClass::exit() {
	if (motors_is_on() == false) {
		cout << "EXIT" << "\t"<<Mpu.timed << endl;
		shmPTR->reboot = 3;
		shmPTR->run_main = false;
		return 0;
	}
	else
		return -1;
}







#define MAX_GIMBAL_ROLL 20
void AutopilotClass::gimBalRollCorrection() {
	static float old_g_roll = 1000;
	const float roll = Mpu.get_roll();
	if (abs(roll) > MAX_GIMBAL_ROLL)
		gimBalRollZero = -2 * (roll - ((roll > 0) ? MAX_GIMBAL_ROLL : -MAX_GIMBAL_ROLL));
	else
		gimBalRollZero = 0;


	if (old_g_roll != gimBalRollZero) {
		mega_i2c.gimagl(-(gimBalPitchZero + gimbalPitch), gimBalRollZero);
		old_g_roll = gimBalRollZero;
	}
}


AutopilotClass Autopilot;

