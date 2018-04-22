/*
message for LOG

MXT     max throttle
OFT		off_throttle
MD1		motors_do_on
MD0		motors do off
LWV		low voltage
GPE		gps error
CNF		controled fall
MXG		����������
MODXXX  ����� ����� ����������,XXX - ����� - control_bits

TFR	out of Perimetr gps
THG out of Perimetr high



*/

//��� shift yaw �� 180 �������� ������ � �������� ����������� ��� ���������� �������� ��������
//��� ������ ��� ������ � �������. �������� �������� � ����������� ���� ����





enum { B_CONNECTION_LOST = 1, B_MS611_ERROR, B_ACC_ERROR, B_LOW_VOLTAGE, B_GPS_ACCURACY_E };


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
#include "Wi_Fi.h"
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
#include "SIM800.h"

using namespace std;

int camera_mode;
enum { CAMMERA_OFF, CAMERA_RECORDING, CAMERA_TRANCLATE };


int get_pid(const char* name) {
	FILE *in;
	char buff[512];

	if (!(in = popen("ps -e", "r"))) {
		return 1;
	}
	
	while (fgets(buff, sizeof(buff), in) != NULL) {
		//	cout << buff;
		string s = string(buff);
		if (s.find(name) != -1) {
			cout << s;
			int pid = stoi(s.substr(0, 5));
			fclose(in);
			return pid;
			
		}
	}
	pclose(in);
	return -1;
}

/*
chtoby zapustit' strim
python  Camera_video_stream.py
ffmpeg -rtsp_transport udp -i "rtsp://192.168.42.1:554/live" -c copy -f h264 udp://android_phone_address:5544

pn android run
ffplay udp://android_address:5544


*/

void stop_video() {

	int pid = get_pid("ffmpeg");
	if (pid != -1) {
		kill(pid, SIGQUIT);
		fprintf(Debug.out_stream, "video STOP\n");
	}

}

void start_video() {
	if (camera_mode == CAMERA_RECORDING) {
		usleep(2000000);
		fprintf(Debug.out_stream, "recording video START\n");
		string s = "/home/igor/ffmpeg_cedrus264_H3/ffmpeg -f v4l2 -channel 0 -video_size 640x480 -i /dev/video0 -pix_fmt nv12 -r 30 -b:v 64k -c:v cedrus264 /home/igor/logs/video";
		s += std::to_string(Log.counter_());
		s += "_";
		s += std::to_string(Log.run_counter);
		s += ".mp4 > /dev/null 2>&1";
		system(s.c_str());
		
	}
	if (camera_mode=CAMERA_TRANCLATE){
		fprintf(Debug.out_stream, "transmiting video START\n");
		string adr = WiFi.get_client_addres();
		string s = "/home/igor/ffmpeg_cedrus264_H3/ffmpeg -f v4l2 -framerate 30 -video_size 640x480 -i /dev/video0 -f mpegts udp://"+adr+":1234 > /dev/null 2>&1";
		system(s.c_str());
		
		
	}
}



//������ ����� ����� ����� ����������� � ������




void AutopilotClass::init(){/////////////////////////////////////////////////////////////////////////////////////////////////

	
	time_at_start = 0;
	camera_mode = CAMMERA_OFF;
	lowest_height = Debug.lowest_altitude_to_fly;
	last_time_data_recived = 0;
	
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
	control_bits = COMPASS_ON|HORIZONT_ON;
	old_control_bits = 0;
	aPitch = aRoll = 0;

//	for (int i = 0; i < 10; i++)
//	while (MS5611.loop() != 0)
//		delay(10);

	controlDeltaTime = millis();

	gimBalPitchZero = gimBalRollZero= gimbalPitch=0;

	mega_i2c.gimagl(gimBalPitchZero, gimBalRollZero);

}

float AutopilotClass::corectedAltitude4tel() {
	return ((control_bits & Z_STAB) == 0) ? MS5611.altitude() : Stabilization.getAltitude();
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

		tflyAtAltitude += speed * dt;
		if (tflyAtAltitude < lowest_height)
			tflyAtAltitude = lowest_height;

		flyAtAltitude = tflyAtAltitude + Stabilization.getDist_Z(speed);
		if (flyAtAltitude < lowest_height)
			flyAtAltitude = lowest_height;

		//fprintf(Debug.out_stream,"f@alt %f\n", flyAtAltitude);
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
uint32_t last_beep_time = 0;



void AutopilotClass::loop(){/////////////////////////////////////////////////////////////////////////////////////////////////

	
	
	const uint32_t t = millis();
	const float dt = 0.001f*(float)(t - controlDeltaTime); 
	if (dt < 0.05)
		return;

	uint sim_com = sim.get_commande();
	if (sim_com) 
		set_control_bits(sim_com);
	
	gimBalRollCorrection();

	


#ifdef LOST_BEEP
	
	if ( GPS.loc.lat_home!=0 && GPS.loc.lon_home!=0  && t - last_time_data_recived>3000 && t - last_beep_time > 3000) {
		last_beep_time = t;
		mega_i2c.beep_code(B_CONNECTION_LOST);
	}
#endif

	controlDeltaTime = t;



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
				uint32_t timelag = millis() - last_time_data_recived;
				if (motors_is_on() && timelag > CONNECTION_LOST_TIMEOUT) {
					connectionLost_();
					return;
				}

				if ((timelag > TIMEOUT_LAG))
					Commander.data_reset();

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

	if (Mpu.mputime<30000000 || Mpu.acc_callibr_time > Mpu.mputime)
		mega_i2c.set_led_mode(2, 5, true);
	else {

		if (motors_is_on() == false) {
			mega_i2c.set_led_mode(0, 1, false);
		}
		else {
			if (control_bits&CONTROL_FALLING)
				mega_i2c.set_led_mode(2, 2, false);
			else
				mega_i2c.set_led_mode(1, 2, (control_bits & GO2HOME) | (control_bits & PROGRAM));

		}
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
	fprintf(Debug.out_stream,"Autopilot set\n");
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
			fprintf(Debug.out_stream,"Safe set:\n");

			for (ii = 0; ii < i; ii++){
				fprintf(Debug.out_stream,"%f,",ar[ii]);
			}
			fprintf(Debug.out_stream,"%f\n",ar[ii]);
		}
	}
	if (error>0){
		fprintf(Debug.out_stream,"ERROR\n");
	}
}







void AutopilotClass::set_new_altitude(float alt){
	tflyAtAltitude = flyAtAltitude = alt;
}

bool AutopilotClass::holdAltitude(float alt){

	tflyAtAltitude = flyAtAltitude = alt;
	if ((control_bits & Z_STAB) == 0){
		control_bits |= Z_STAB;
		Stabilization.init_Z();
	}
	//setbuf(stdout, NULL);
	fprintf(Debug.out_stream,"FlyAt: %f \n",flyAtAltitude);

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

	case HOWER:{	//������ 20 ������
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
		//��������  �� ������  X � �� ��������� ������ ��� ����������
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
	case GO2HOME_LOC:{//���������� �� ����� ������
		// led_prog = 4;
		GPS.loc.setNeedLoc2HomeLoc();
		go2homeIndex = TEST4HOME_LOC;
		aYaw_ = -GPS.loc.dir_angle_GRAD;
		aYaw_ = wrap_180(aYaw_);
		break;
	}
	case TEST4HOME_LOC:{//������ �� ����� ������
			   
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


	case TEST_ALT2:{//����� �� FAST_DESENDING_TO_HIGH ������
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
		//������� �����
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

	howeAt2HOME = hower;//��������� �� ����� ��� ���

	bool res = holdAltitude(MS5611.altitude());
	res &= holdLocation(GPS.loc.lat_, GPS.loc.lon_);
	if (res){
		control_bits |= GO2HOME;
		f_go2homeTimer = 0;
		//Out.println("Hanging on the site!");
		fprintf(Debug.out_stream,"go2home\n");
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
		fprintf(Debug.out_stream,"Hower at: %i,%i\n",GPS.loc.lat_, GPS.loc.lon_);
		oldtime = millis();
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

	fprintf(Debug.out_stream,"%s - ",msg.c_str());
	
	if (start){
		//fprintf(Debug.out_stream, "\MS5611 err: %f\n",MS5611.getErrorsK());
#ifndef FALSE_WIRE
		fprintf(Debug.out_stream,"on ");
		if (millis() < 25000) {
			fprintf(Debug.out_stream,"\n!!!calibrating!!! to end:%i sec.\n", 25-millis()/1000);
			mega_i2c.beep_code(B_MS611_ERROR);
			return false;
		}
#else
		Emu.init(WIND_X, WIND_Y, WIND_Z);
#endif
		

#define MAX_MACC 0.1f
		if (Hmc.do_compass_motors_calibr == false && (abs(Mpu.maccX) > MAX_MACC || abs(Mpu.maccY) > MAX_MACC || abs(Mpu.maccZ) > MAX_MACC)) {
			fprintf(Debug.out_stream, "ACC ERROR!!! \n");
			mega_i2c.beep_code(B_ACC_ERROR);
			return false;
		}

		if (Hmc.do_compass_motors_calibr || (Mpu.gyro_calibratioan && Hmc.calibrated)){

			if (Telemetry.low_voltage){
				Telemetry.addMessage(e_LOW_VOLTAGE);
				fprintf(Debug.out_stream," LOW VOLTAGE\n");
				mega_i2c.beep_code(B_LOW_VOLTAGE);
				return false;
			}

			if (Hmc.do_compass_motors_calibr==false && GPS.loc.accuracy_hor_pos_ > MIN_ACUR_HOR_POS_2_START ){
				fprintf(Debug.out_stream," GPS error\n");
				mega_i2c.beep_code(B_GPS_ACCURACY_E);
				Telemetry.addMessage(e_GPS_ERROR);

				//return false;
			}
			time_at_start = millis();
			Telemetry.update_voltage();
			
			control_bits = MOTORS_ON;

			fprintf(Debug.out_stream,"OK\n");

			GPS.loc.setHomeLoc();

			MS5611.copterStarted();
			tflyAtAltitude = flyAtAltitude = MS5611.altitude();
			
			Mpu.max_g_cnt = 0;

			holdAltitude(Debug.fly_at_start);
			holdLocation(GPS.loc.lat_, GPS.loc.lon_);
			Stabilization.resset_z();
			Stabilization.resset_xy_integrator();
			aYaw_ = -Mpu.get_yaw();
			//fflush(Debug.out_stream);
			start_time = millis();

#ifdef DEBUG_MODE
			fprintf(Debug.out_stream, "\nhome loc: %i %i \nhome alt set %i\n", GPS.loc.lat_, GPS.loc.lon_, (int)flyAtAltitude);
#endif

			Log.run_counter++;
			if (camera_mode) {//---------------------------------------------------
				thread t(start_video);
				t.detach();

			}


		}
		else{
			if (Hmc.calibrated == false){
				fprintf(Debug.out_stream,"compas, ");
				mega_i2c.beep_code(4);

			}
			if (Mpu.gyro_calibratioan == false){
				fprintf(Debug.out_stream,"gyro");
				mega_i2c.beep_code(5);

			}
			fprintf(Debug.out_stream," calibr FALSE\n");
		}
	}//------------------------------OFF----------------
	else {
		time_at_start = 0;
#ifdef FALSE_WIRE
		Emu.init(WIND_X, WIND_Y, WIND_Z);
#endif
		fprintf(Debug.out_stream,"off ");
		Telemetry.addMessage(i_OFF_MOTORS);
		off_throttle(true, msg);

		fprintf(Debug.out_stream,"OK\n");

		//if (camera_mode) {//----------------------------------
			thread t(stop_video);
			t.detach();

		//}
	}
	return true;
}

void AutopilotClass::control_falling(const string msg){
	if (motors_is_on() && (control_bits & CONTROL_FALLING) == 0){
		throttle = Mpu.fall_thr*Balance.powerK();
		aPitch = aRoll = 0;
#ifdef DEBUG_MODE
		fprintf(Debug.out_stream, "CNTROLL FALLING\n");
#endif
		Telemetry.addMessage(msg);
		Telemetry.addMessage(i_CONTROL_FALL);
		control_bits = CONTROL_FALLING | MOTORS_ON;
	}
}

bool AutopilotClass::off_throttle(const bool force, const string msg){/////////////////////////////////////////////////////////////////////////////////////////////////
	
	if ( force)
	{


		fprintf(Debug.out_stream,"force motors_off %s, alt: %i, time %i\n", msg.c_str(), (int)MS5611.altitude(),millis()/1000);
		Balance.set_off_th_();
		Telemetry.addMessage(msg);
		control_bits = 0;
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

	fprintf(Debug.out_stream,"connection lost\n");
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

			if (going2HomeON(true) == false && (millis() - last_time_data_recived) > (CONNECTION_LOST_TIMEOUT*3)) {
				off_throttle(false, e_NO_GPS_2_LONG);
			}
		}
	
}
void AutopilotClass::calibration() {/////////////////////////////////////////////////////////////////////////////////////////////////

	fprintf(Debug.out_stream,"Set Calibr NOT USET.\n");
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
	if (!progState())
		control_bits ^= COMPASS_ON;
}

void AutopilotClass::horizont_tr() {
	if ((control_bits & GO2HOME) == 0 && (control_bits & PROGRAM) == 0)
		control_bits ^= HORIZONT_ON;
}





bool AutopilotClass::selfTest(){/////////////////////////////////////////////////////////////////////////////////////////////////
	//wdt_enable(WDTO_2S);
	fprintf(Debug.out_stream,"Self Test running\n");
	int ok = 0;
	if (Mpu.selfTest())
		ok += 1;
	if (Hmc.selfTest())
		ok += 2;
#ifdef BUZZER_R
	//OCR4B = 35000;
#endif
	if (ok == 3){
		delay(100);
#ifdef BUZZER_R
		//OCR4B = 0;
#endif
	}
	else{
		delay(10000);
	}
	//wdt_enable(WATCHDOG);
	return false;
}

void AutopilotClass::gimBalPitchADD(const float add) {
	if (!progState())
		if (mega_i2c.gimagl((gimBalPitchZero + gimbalPitch + add), gimBalRollZero))
			gimbalPitch += add;

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
					fprintf(Debug.out_stream, "prog started\n");
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
	//fprintf(Debug.out_stream,"comm=%i\n", bits);
	if (MOTORS_ON&bits)  {
		Hmc.do_compass_motors_calibr = false;
		bool on = motors_is_on() == false;
		on = motors_do_on(on, m_START_STOP);
		if (on == false) {
			fprintf(Debug.out_stream,"motors on denied!\n");
		}
	}

	if (bits & GIMBAL_PLUS)
		gimBalPitchADD(5);
	if (bits & GIMBAL_MINUS)
		gimBalPitchADD(-5);

	if (control_bits&CONTROL_FALLING)
		return false;

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
	//-----------------------------------------------
	if (bits & (MPU_ACC_CALIBR | MPU_GYRO_CALIBR)) {
		if (millis() > 25000) {
			control_bits |= (MPU_ACC_CALIBR | MPU_GYRO_CALIBR);
			Mpu.new_calibration(!(bits&MPU_ACC_CALIBR));
			control_bits &= (0xffffffff ^ (MPU_ACC_CALIBR | MPU_GYRO_CALIBR));
		}
	}
	if (bits & COMPASS_MOTOR_CALIBR) {
		//Hmc.start_motor_compas_calibr();
		//if (Hmc.do_compass_motors_calibr)
		//	control_bits |= COMPASS_MOTOR_CALIBR;
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
		fprintf(Debug.out_stream, "REBOOT \n");
		Debug.reboot = 1;
		Debug.run_main = false;
		return 0;
	}else
		return -1;
}
int  AutopilotClass::shutdown() {
	if (motors_is_on() == false) {
		fprintf(Debug.out_stream, "SHUTD \n");
		Debug.reboot = 2;
		Debug.run_main = false;
		return 0;
	}
	else
		return -1;
}
int  AutopilotClass::exit() {
	if (motors_is_on() == false) {
		fprintf(Debug.out_stream, "EXIT \n");
		Debug.reboot = 3;
		Debug.run_main = false;
		return 0;
	}
	else
		return -1;
}






float old_g_roll = 1000;
#define MAX_GIMBAL_ROLL 20
void AutopilotClass::gimBalRollCorrection() {
	const float roll = Mpu.get_roll();
	if (abs(roll) > MAX_GIMBAL_ROLL)
		gimBalRollZero = 2 * (roll - ((roll > 0) ? MAX_GIMBAL_ROLL : -MAX_GIMBAL_ROLL));
	else
		gimBalRollZero = 0;
	if (old_g_roll != gimBalRollZero) {
		mega_i2c.gimagl(-(gimBalPitchZero + gimbalPitch), gimBalRollZero);
		old_g_roll = gimBalRollZero;
	}
}


AutopilotClass Autopilot;

