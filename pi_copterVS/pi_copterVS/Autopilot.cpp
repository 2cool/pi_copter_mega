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



#define WIND_X 10
#define WIND_Y 10
#define WIND_Z 0.5





#include "define.h"
#include "WProgram.h"

#include "mi2c.h"
#include "MS5611.h"
#include "Autopilot.h"
#include "Balance.h"

#include "commander.h"

#include "GPS.h"

#include "Settings.h"
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



void AutopilotClass::init(){/////////////////////////////////////////////////////////////////////////////////////////////////

	if (init_shmPTR())
		return;
#ifdef FLY_EMULATOR
	Emu.init(WIND_X, WIND_Y, WIND_Z);
#endif
	shmPTR->sim800_reset = false;
	time_at_startd = old_time_at_startd = 0;
	lowest_height = shmPTR->lowest_altitude_to_fly;
	last_time_data_recived = 0;
	
	Balance.init();
	MS5611.init();

	fall_thr = FALLING_THROTTLE;
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
	return Mpu.get_Est_Alt();
	//return ((control_bits & Z_STAB) == 0) ? Mpu.get_Est_Alt() : Stabilization.getAltitude();
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
			speed = fmax(-0.2, speed);

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
			flyAtAltitude = tflyAtAltitude= Mpu.get_Est_Alt();

		}
	}
}
//-------------------------------------------------------------------------
void AutopilotClass::smart_commander(const float dt){
	////if (Commander.getPitch() != 0 || Commander.getRoll() != 0){
		const float addX = sens_xy*(Commander.getPitch());
		const float addY = -sens_xy*(Commander.getRoll());
		const float cyaw = Commander.getYaw()*GRAD2RAD;
		const float cosL = (float)cos(cyaw);
		const float sinL = (float)sin(cyaw);
		float speedX = addX * cosL + addY *sinL;
		float speedY = -(addX * sinL - addY *cosL);

		Stabilization.add2NeedPos(speedX, speedY, dt);
	//}
	//else{
	//	GPS.loc.setSpeedZero();
	//}
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
	
	if (last_time_data_recived &&  GPS.loc.lat_zero!=0 && GPS.loc.lon_zero!=0  && Mpu.timed - last_time_data_recived>3 && Mpu.timed - last_beep_timed > 3) {
		last_beep_timed = Mpu.timed;
		Telemetry.addMessage(e_LOST_CONNECTION);
		mega_i2c.beep_code(B_CONNECTION_LOST);
	}
#endif

	//-----------------------------------------
	static int uspd_cnt = 0;
	if (motors_is_on() && fabs(Mpu.get_roll()) > 110 && Mpu.get_Est_Alt() < 5) {
		if (uspd_cnt++ > 10)
			off_throttle(true, "USD");
	}else
		uspd_cnt = 0;
	//-----------------------------------------

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
				double timelag = Mpu.timed - last_time_data_recived;
				if (last_time_data_recived && motors_is_on() && timelag > CONNECTION_LOST_TIMEOUT) {
					connectionLost_();
					return;
				}
#ifndef OFF_TIMELAG
				if (timelag > TIMEOUT_LAG) {
					float _timeout_LAG = TIMEOUT_LAG;
					if (Mpu.get_Est_Alt() > 15 || Mpu.get_Est_Alt() / Mpu.get_Est_SpeedZ() > 5)
						_timeout_LAG = 2;
					if (timelag > _timeout_LAG)
						Commander.data_reset();
				}
#endif
				
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
		Balance.get_max_throttle() << "," << \
		Balance.get_min_throttle() << "," << \
		sens_xy << "," << \
		sens_z << "," << \
		lowest_height << "," << \
		shmPTR->fly_at_start << "," << \
		Debug.n_debug;// << "," << \
		-gimBalPitchZero << "," << \
		-gimBalRollZero;
	string ret = convert.str();
	return string(ret);
}

void AutopilotClass::set(const float ar[]){
	cout << "Autopilot set\n";
	int error = 1;

	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){
		int i = 0;
		error = 0;
		
		
		error += Settings._set(ar[i++], height_to_lift_to_fly_to_home);
		error += Balance.set_min_max_throttle(ar[i++], ar[i++]);
		//i += 2;
		error += Settings._set(ar[i++], sens_xy);
		error += Settings._set(ar[i++], sens_z);
		error += Settings._set(ar[i++], lowest_height);
		error += Settings._set(ar[i++], shmPTR->fly_at_start);
		if (shmPTR->fly_at_start + 1 < lowest_height)
			shmPTR->fly_at_start = lowest_height + 1;
		Debug.n_debug = (int)ar[i++];

		//gimBalPitchZero= -constrain(ar[i],-15,15);i++;
		//gimBalRollZero = -constrain(ar[i],-15,15);i++;
		mega_i2c.gimagl(gimBalPitchZero, gimBalRollZero);
		if (error == 0){
			int ii = 0;
			cout << "Save set:\n";

			for (ii = 0; ii < i; ii++){
				cout << ar[ii] << ",";
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
		return holdAltitude(Mpu.get_Est_Alt());
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
			dist2home_at_begin2 = Mpu.dist2home_2();
			if (Mpu.get_Est_Alt() < 3)
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
			if (fabs(Mpu.get_Est_X()) <= accuracy && fabs(Mpu.get_Est_Y()) <= accuracy){
				f_go2homeTimer = 6; //min time for stab
				go2homeIndex = (Mpu.get_Est_Alt() <= (FAST_DESENDING_TO_HIGH)) ? SLOW_DESENDING : START_FAST_DESENDING;
				Stabilization.setNeedPos2Home();
				break;
			}	
			//��������  �� ������  X � �� ��������� ������ ��� ����������
			tflyAtAltitude=flyAtAltitude = height_to_lift_to_fly_to_home;
			go2homeIndex = TEST_ALT1;
			break;
		}
		case TEST_ALT1:{
			if (fabs(Mpu.get_Est_Alt() - flyAtAltitude) <= (ACCURACY_Z)){
				go2homeIndex = GO2HOME_LOC;
			}
			break;
		}
		case GO2HOME_LOC:{//���������� �� ����� ������
			// led_prog = 4;
			Stabilization.setNeedPos2Home();
			go2homeIndex = TEST4HOME_LOC;
			aYaw_ = -RAD2GRAD * atan2(Mpu.get_Est_Y(),Mpu.get_Est_X());// GPS.loc.dir_angle_GRAD;

			aYaw_ = wrap_180(aYaw_);
			break;
		}
		case TEST4HOME_LOC:{//������ �� ����� ������
			   
			const float accuracy = ACCURACY_XY + GPS.loc.accuracy_hor_pos_;
			if (fabs(Mpu.get_Est_X()) <= accuracy && fabs(Mpu.get_Est_Y()) <= accuracy){
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
			if (fabs(Mpu.get_Est_Alt() - flyAtAltitude) < (ACCURACY_Z)){
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
			//������ �����
			if (Mpu.get_Est_Alt() >lowest_height){
				float k = Mpu.get_Est_Alt()*0.05f;
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

	if (Mpu.dist2home_2() - dist2home_at_begin2 > (MAX_DIST_ERROR_TO_FALL*MAX_DIST_ERROR_TO_FALL)){
		Autopilot.off_throttle(false, e_TOO_STRONG_WIND);
	}

	return true;
}
bool AutopilotClass::going2HomeON(const bool hower){
	
	Stabilization.setDefaultMaxSpeeds();

	howeAt2HOME = hower;//��������� �� ����� ��� ���

	bool res = holdAltitude(Mpu.get_Est_Alt());
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
	
	aPitch = aRoll = 0;
	//if (holdAltitude()){

	
		//GPS.loc.setNeedLoc(lat,lon);
		cout << "Hower at: " << GPS.loc.lat_ << " " << GPS.loc.lon_ << "\t"<<Mpu.timed << endl;;

		//Stabilization.init_XY(0, 0);
		Stabilization.setNeedPos(Mpu.get_Est_X(), Mpu.get_Est_Y());

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
		//Stabilization.resset_xy_integrator();
		return holdLocation(GPS.loc.lat_, GPS.loc.lon_);
	}
	else{
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
#ifndef FLY_EMULATOR
		cout << "on ";
		if (Mpu.timed < 25) {
			cout << "\n!!!calibrating!!! to end:"<< 25-millis()/1000 <<" sec." << "\t"<<Mpu.timed << endl;
			mega_i2c.beep_code(B_MS611_ERROR);
			return false;
		}
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

				if (not_start_motors_if_gps_error)
					return false;
			}
			time_at_startd = Mpu.timed;
			Telemetry.update_voltage();
			
			control_bits |= MOTORS_ON;

			cout << "OK" << "\t"<<Mpu.timed<<endl;

			GPS.loc.setHomeLoc();
			Mpu.set_XYZ_to_Zero();  // ��� ����� �� ���. ���  ������ x y z = 0;
			//tflyAtAltitude = flyAtAltitude = 0;// Mpu.get_Est_Alt();
			
			Mpu.max_g_cnt = 0;

			if (control_bits&Z_STAB) 
				holdAltitude(shmPTR->fly_at_start);
			
			if (control_bits&XYSTAB)
				holdLocation(GPS.loc.lat_, GPS.loc.lon_);

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
		throttle = fall_thr*Balance.powerK();
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


		cout << "force motors_off " << msg << ", alt: " << (int)Mpu.get_Est_Alt() << ", time " << (int)Mpu.timed << endl;
		Balance.set_off_th_();
		Telemetry.addMessage(msg);
		control_bits = DEFAULT_STATE;
		return true;
	}
	else{
	//	if (control_bits_ & (255 ^ (COMPASS_ON | HORIZONT_ON)))
	//		return true;

		//if (Mpu.get_Est_Alt()  < 2){
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
#ifdef FLY_EMULATOR
	//if (true)
	//	return;
#endif

	shmPTR->connected = 0;
	shmPTR->commander_buf_len = 0;
	shmPTR->telemetry_buf_len = 0;

	cout << "connection lost" << "\t"<<Mpu.timed<<endl;
	Telemetry.addMessage(e_LOST_CONNECTION);
	Commander.controls2zero();

#ifdef OFF_MOTOR_IF_LOST_CONNECTION
if (motors_is_on())
off_throttle(true, "lost connection");
return;
#endif

	if (motors_is_on())
		if (go2homeState() == false && progState() == false) {
			aPitch = aRoll = 0;

			if (going2HomeON(true) == false && (Mpu.timed - last_time_data_recived) > (CONNECTION_LOST_TIMEOUT*3)) {
				off_throttle(false, e_NO_GPS_2_LONG);
			}
		}
	
}
void AutopilotClass::calibration() {/////////////////////////////////////////////////////////////////////////////////////////////////
	
	cout << "Set Calibr NOT USET.\n";
	/*
	if (fabs(cPitch + Commander.pitch) > 0.1 || fabs(cRoll + Commander.roll) > 0.1)
		return;

	cPitch += Commander.pitch;
	Commander.pitch = 0;
	cRoll += Commander.roll;
	Commander.roll = 0;
	*/
}

void AutopilotClass::set_gimBalPitch(const float angle) {
	if (mega_i2c.gimagl(gimBalPitchZero + angle, gimBalRollZero))
		gimbalPitch = angle;
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
				float alt = Mpu.get_Est_Alt();
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
				bool res = holdAltitude(Mpu.get_Est_Alt());
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
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
bool AutopilotClass::set_control_bits(uint32_t bits) {
	if (bits==0)
		return true;
	//	uint8_t mask = control_bits_^bits;
	//printf("comm=%i\n", bits);

	//cout << bits << endl;
	mega_i2c.beep_code(B_COMMAND_RECEIVED);

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
	}
	else 
		control_bits ^= bits & ( XY_STAB | Z_STAB);
	
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






#define MAX_GIMAGL_PITCH 20
#define MAX_GIMBAL_ROLL 20
void AutopilotClass::gimBalRollCorrection() {
	static float old_g_roll = 1000;
	const float roll = Mpu.get_roll();
	if (fabs(roll) > MAX_GIMBAL_ROLL)
		gimBalRollZero = -2 * (roll - ((roll > 0) ? MAX_GIMBAL_ROLL : -MAX_GIMBAL_ROLL));
	else
		gimBalRollZero = 0;


	if (old_g_roll != gimBalRollZero) {
		mega_i2c.gimagl((gimBalPitchZero + gimbalPitch), gimBalRollZero);
		old_g_roll = gimBalRollZero;
	}
}


AutopilotClass Autopilot;

