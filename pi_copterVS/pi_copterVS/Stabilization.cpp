// 
// 
// 

#include "define.h"
#include "WProgram.h"
#include "Stabilization.h"
#include "mpu.h"
#include "AP_PID.h"
#include "mpu.h"
#include "MS5611.h"
#include "Autopilot.h"

#include "GPS.h"
#include "debug.h"
#include "Balance.h"
#include "Prog.h"
#include "Log.h"
#include "Settings.h"



void StabilizationClass::setMaxAng() {
	set_acc_xy_speed_imax(Balance.get_max_angle());
}
void StabilizationClass::setMinMaxI_Thr() {
	pids[ACCZ_PID].imax(Balance.get_min_throttle()-HOVER_THROTHLE, Balance.get_max_throttle() - HOVER_THROTHLE);
}
//"dist to speed","speed to acc","SPEED_KP","SPEED_I","SPEED_imax","max_speed","FILTR"
void StabilizationClass::init(){
	
	
	ACCXY_CF = 0.1;
	max_XY_ACC = 3;//6.8

	dist2speed_XY = 0.2f;//0.5

	speed2accXY = 2;

	//?????????????
	set_acc_xy_speed_kp(5.14);// 7);
	set_acc_xy_speed_kI(2);
	set_acc_xy_speed_imax(Balance.get_max_angle());
	max_speed_xy = MAX_HOR_SPEED;
	//--------------------------------------------------------------------------
	//����������� ������� ������ � ������� ������. ����������� ��� ������� ��������� � ���������
	ACCZ_CF = 0.1;

	max_Z_ACC = 2;
	alt2speedZ = 0.5;//1
	speed2accZ = 2;//1

	setMinMaxI_Thr();

	pids[ACCZ_PID].kP( 0.05 );
	pids[ACCZ_PID].kI(0.05);

	
	//pids[ACCZ_PID].imax(Balance.get_min_throttle() - HOVER_THROTHLE,  Balance.get_max_throttle() - HOVER_THROTHLE);
	max_speedZ_P =  MAX_VER_SPEED_PLUS;
	max_speedZ_M = MAX_VER_SPEED_MINUS;
	//----------------------------------------------------------------------------
	mc_pitch=mc_roll=mc_z=0;
	//cout << "stab init\n";

}
//bool flx = false, fly = false;

float StabilizationClass::accxy_stab(float dist, float maxA, float timeL) {
	
	return sqrt(2 * maxA*dist) - maxA*timeL;

}
float StabilizationClass::accxy_stab_rep(float speed, float maxA, float timeL) {
	float t = speed / maxA + timeL;
	return 0.5*maxA*t*t;
}

void StabilizationClass::setDefaultMaxSpeeds(){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	max_speed_xy = MAX_HOR_SPEED;
	max_speedZ_P = MAX_VER_SPEED_PLUS;
	max_speedZ_M = MAX_VER_SPEED_MINUS;
}


void StabilizationClass::max_speed_limiter(float &x, float &y) {
	const float speed2 = (x*x + y * y);
	const float maxSpeed2 = max_speed_xy * max_speed_xy;
	if (speed2 > maxSpeed2) {
		const float k = (float)sqrt(maxSpeed2 / speed2);
		x *= k;
		y *= k;
	}
}
void StabilizationClass::setNeedPos2Home() {
	needXR = needXV = needYR = needYV = 0;
}
void StabilizationClass::dist2speed(float &x, float &y) {
	x = dist2speed_XY * x;
	y = dist2speed_XY * y;
	max_speed_limiter(x, y);

}
void StabilizationClass::speed2dist(float &x, float &y) {
	max_speed_limiter(x, y);
	x /= dist2speed_XY;
	y /= dist2speed_XY;
}

void StabilizationClass::setNeedPos(float x, float y) {
	needXR=needXV = x;
	needYR=needYV = y;
}


void StabilizationClass::fromLoc2Pos(long lat, long lon, float &x, float &y) {
	GPS.loc.fromLoc2Pos(lat, lon, x, y);
	Mpu.getXYRelative2Zero(x, y);
}
void StabilizationClass::setNeedLoc(long lat, long lon, float &x, float &y) {
	fromLoc2Pos(lat, lon, x, y);
	setNeedPos(x, y);
	
}


void StabilizationClass::add2NeedPos(float speedX, float speedY, float dt) {
	static bool flagzx = false;
	static bool flagzy = false;
	if (speedX == 0) {
		if (flagzx == false) {
			flagzx = true;
			float x, y;
			fromLoc2Pos(GPS.loc.lat_, GPS.loc.lon_, x, y);
			needXR = needXV = x;
		}
	}
	else {
		flagzx = false;
		float distX = speedX, distY = speedY;
		speed2dist(distX, distY);
		needXR += speedX * dt;
		needXV = needXR + distX;
	}
	if (speedY == 0) {
		if (flagzy == false) {
			flagzy = true;
			float x, y; 
			fromLoc2Pos(GPS.loc.lat_, GPS.loc.lon_, x, y);
			needYR = needYV = y;
		}
	}
	else {
		flagzy = false;
		float distX = speedX, distY = speedY;
		speed2dist(distX, distY);
		needYR += speedY * dt;
		needYV = needYR + distY;
	}
}
float StabilizationClass::get_dist2goal(){
	float dx= Mpu.get_Est_X() - needXV;
	float dy = Mpu.get_Est_Y() - needYV;
	return sqrt(dx*dx + dy * dy);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//float old_gps_bearing = 0, cos_bear = 1,  sin_bear = 0;
void StabilizationClass::XY(float &pitch, float&roll){//dont work 
		float need_speedX, need_speedY;
		static float naccXF = 0, naccYF = 0;
		float tx, ty;
		if (Autopilot.progState() && Prog.intersactionFlag) {
			need_speedX = -Prog.need_speedX;
			need_speedY = -Prog.need_speedY;
		}
		else {
			tx = Mpu.get_Est_X();
			need_speedX = (tx-needXV);
			ty = Mpu.get_Est_Y();
			need_speedY = (ty-needYV);
			dist2speed(need_speedX, need_speedY);
		}//��������� ������ ��������� �� �������� a=v*v/(2s)
		
		tx = Mpu.get_Est_SpeedX();
		const float need_acx =  (need_speedX + tx)*speed2accXY;
		ty = Mpu.get_Est_SpeedY();
		const float need_acy =  (need_speedY + ty)*speed2accXY;
		float naccX = need_acx + Mpu.fw_accX;
		float naccY = need_acy + Mpu.fw_accY;

		const float k = max_XY_ACC/sqrt(naccX * naccX + naccY + naccY);
		if (k < 1) {
			naccX *= k;
			naccY *= k;
		}


		naccXF += (naccX - naccXF)*ACCXY_CF;
		naccYF += (naccY - naccYF)*ACCXY_CF;

		const float w_pitch = -pids[ACCX_SPEED].get_pid(naccXF, Mpu.dt);
		const float w_roll =   pids[ACCY_SPEED].get_pid(naccYF, Mpu.dt);

		//----------------------------------------------------------------������. � ������������� ������� ���������
		pitch = Mpu.cosYaw*w_pitch - Mpu.sinYaw*w_roll;
		roll = Mpu.cosYaw*w_roll + Mpu.sinYaw*w_pitch;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float StabilizationClass::Z(){

	static int hecnt = 0;
	if ((Autopilot.fly_at_altitude()+ MAX_HIGHT_ERROR_TO_FALL) < Mpu.get_Est_Alt() && hecnt++ > 5)
		Autopilot.off_throttle(false, e_TOO_HIGHT_FROM_NEED);
	//-------------stab
	static float naccZF=0;
		const float need_speedZ = getSpeed_Z(Autopilot.fly_at_altitude() - Mpu.get_Est_Alt());
		const float need_accZ =  speed2accZ*need_speedZ - Mpu.get_Est_SpeedZ();

		
		naccZF += (constrain((need_accZ - Mpu.faccZ), -max_Z_ACC, max_Z_ACC) - naccZF)*ACCZ_CF;


		const float fZ = HOVER_THROTHLE + pids[ACCZ_PID].get_pid(naccZF, Mpu.dt)*Balance.powerK();
		return fZ;
}

void StabilizationClass::resset_z(){
	pids[ACCZ_PID].reset_I();
	pids[ACCZ_PID].set_integrator(max(HOVER_THROTHLE,Autopilot.get_throttle()) - HOVER_THROTHLE);
	
}
void StabilizationClass::resset_xy_integrator(){
	pids[ACCX_SPEED].reset_I();
	pids[ACCY_SPEED].reset_I();
}

string StabilizationClass::get_z_set(){

	ostringstream convert;
	convert<<\
		alt2speedZ <<","<<\
		speed2accZ << "," << \
		max_Z_ACC<<","<<\
		pids[ACCZ_PID].kP() <<","<<\
		pids[ACCZ_PID].kI() <<","<<\


		max_speedZ_P <<"," <<\
		max_speedZ_M << "," << \
		ACCZ_CF;
	string ret = convert.str();
	return string(ret);

}



void StabilizationClass::setZ(const float  *ar){

	uint8_t i = 0;
	int error = 1;
	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){
		
		error = 0;
		float t;
		

		error += Settings._set(ar[i++], alt2speedZ);
		error += Settings._set(ar[i++], speed2accZ);
		error += Settings._set(ar[i++], max_Z_ACC);
		t = pids[ACCZ_PID].kP();
		if ((error += Settings._set(ar[i++],t))==0)
			pids[ACCZ_PID].kP(t);

		t = pids[ACCZ_PID].kI();
		if ((error += Settings._set(ar[i++], t))==0)
			pids[ACCZ_PID].kI(t);

	
		error += Settings._set(ar[i++], max_speedZ_P);
		error += Settings._set(ar[i++], max_speedZ_M);
		error += Settings._set(ar[i++], ACCZ_CF);
		

		//resset_z();
		
	}
	if (error == 0) {
		cout << "Stabilization Z set:\n";

		for (uint8_t ii = 0; ii < i; ii++) {
			cout << ar[ii] << ",";
		}
		cout << endl;
	}
	else
		cout << "ERROR to big or small. P=" << error;
}


string StabilizationClass::get_xy_set(){
	ostringstream convert;
	convert << \
		dist2speed_XY << "," << \
		speed2accXY << "," << \
		max_XY_ACC << "," << \
		pids[ACCX_SPEED].kP() << "," << \
		pids[ACCX_SPEED].kI() << "," << \
		pids[ACCX_SPEED].imax() << "," << \
		max_speed_xy << "," << \
		ACCXY_CF;
	string ret = convert.str();
	return string(ret);
}

void StabilizationClass::setXY(const float  *ar){

	int error = 1;
	uint8_t i = 0;
	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){

		error = 0;
		float t;
		

		error += Settings._set(ar[i++], dist2speed_XY);
		error+= Settings._set(ar[i++], speed2accXY);
		error += Settings._set(ar[i++], max_XY_ACC);
		

		t = pids[ACCX_SPEED].kP();
		if ((error += Settings._set(ar[i++], t))==0)
			set_acc_xy_speed_kp(t);

		t = pids[ACCX_SPEED].kI();
		if ((error += Settings._set(ar[i++], t))==0)
			set_acc_xy_speed_kI(t);

		t = pids[ACCX_SPEED].imax();
		if ((error += Settings._set(ar[i++], t))==0)
			set_acc_xy_speed_imax(t);
		
		error += Settings._set(ar[i++], max_speed_xy);
		error += Settings._set(ar[i++], ACCXY_CF);
	}
	if (error == 0) {
		cout << "Stabilization XY set : \n";

		for (uint8_t ii = 0; ii < i; ii++) {
			cout << ar[ii] << ",";
		}
		cout << endl;
	}
	else
		cout << "ERROR to big or small. P=" << error;

	
}












StabilizationClass Stabilization;
