// 
// 
// 

#include "define.h"
#include "WProgram.h"
#include "Stabilization.h"
#include "AP_PID.h"
#include "mpu.h"
#include "MS5611.h"
#include "Autopilot.h"

#include "GPS.h"
#include "debug.h"
#include "Balance.h"
#include "Prog.h"
#include "Log.h"
void StabilizationClass::init(){

	{

	


		accXY_stabKP = 0.2f;//0.5
		accXY_stabKP_Rep = 1.0f / accXY_stabKP;
		set_acc_xy_speed_kp(6.1);
		set_acc_xy_speed_kI(3);
		set_acc_xy_speed_imax(Balance.get_max_angle());
		max_speed_xy = MAX_HOR_SPEED;


		XY_KF_DIST = 0.1f;
		XY_KF_SPEED = 0.1f;
		XY_FILTER = 0.06;
	}
	//--------------------------------------------------------------------------
	last_accZ = 1;

	
	//повистовл€ь фильтри низких и високих частот. подобранние дл€ каждого источника и обединить



	dalt2speedZ = 0.3;
	dalt2speedZ_Rep = 1.0f / dalt2speedZ;
	dspeedZ2accZ = 1;

	pids[ACCZ_PID].kP(0.05);
	pids[ACCZ_PID].kI(0.0025);
	pids[ACCZ_PID].imax( MAX_THROTTLE_ - HOVER_THROTHLE);
	max_stab_z_P =  MAX_VER_SPEED_PLUS;
	max_stab_z_M = MAX_VER_SPEED_MINUS;


	Z_CF_DIST = 0.03;//CF filter
	Z_CF_SPEED = 0.01;//CF filter //при 0.005 Ќа ошибку в ACC на 0.1 ошибка в исоте на метр.
	Z_FILTER = 0.2;//filter

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



//33 max speed on pressureat 110000
const float air_drag_k = 9.8f / (float)(PRESSURE_AT_0 * 33 * 33);
float air_drag(const float speed){
	return abs(speed)*speed*MS5611.pressure*air_drag_k;
}
float air_drag_wind(const float a){
	float w=(float)sqrt(abs(a / (MS5611.pressure*air_drag_k)));
	return (a < 0) ? -w : w;
}
void StabilizationClass::setDefaultMaxSpeeds(){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	max_speed_xy = MAX_HOR_SPEED;
	max_stab_z_P = MAX_VER_SPEED_PLUS;
	max_stab_z_M = MAX_VER_SPEED_MINUS;
}

/*
void StabilizationClass::init_XY(const float sx,  const float sy){
	sX = sx;
	sY = sy;
	//resset_xy_integrator();
//	gps_sec = GPS.loc.mseconds;

}
*/
int cnnnnn = 0;

#define MAX_A 1

/*
void StabilizationClass::set_XY_2_GPS_XY() {
	sX = GPS.loc.dX;
	sY = GPS.loc.dY;
}
*/




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
	x = accXY_stabKP * x;
	y = accXY_stabKP * y;
	max_speed_limiter(x, y);

}
void StabilizationClass::speed2dist(float &x, float &y) {
	max_speed_limiter(x, y);
	x *= accXY_stabKP_Rep;
	y *= accXY_stabKP_Rep;
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


	float distX = speedX, distY = speedY;
	speed2dist(distX, distY);

	needXR += speedX * dt;
	needXV = needXR + distX;

	needYR += speedY * dt;
	needYV = needYR + distY;

}
float StabilizationClass::get_dist2goal(){
	float dx= Mpu.get_Est_X() - needXV;
	float dy = Mpu.get_Est_Y() - needYV;
	return sqrt(dx*dx + dy * dy);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//float old_gps_bearing = 0, cos_bear = 1,  sin_bear = 0;
void StabilizationClass::XY(float &pitch, float&roll){

		
		float need_speedX, need_speedY;


		if (Autopilot.progState() && Prog.intersactionFlag) {
			need_speedX = -Prog.need_speedX;
			need_speedY = -Prog.need_speedY;
		}
		else {
			need_speedX = (Mpu.get_Est_X()-needXV);
			need_speedY = (Mpu.get_Est_Y()-needYV);
			
		}
		dist2speed(need_speedX, need_speedY);
		float w_pitch, w_roll;

		float need_acx = constrain((need_speedX + Mpu.get_Est_SpeedX()), -5, 5);
		float need_acy = constrain((need_speedY + Mpu.get_Est_SpeedY()), -5, 5);
#ifdef FOR_TESTS
		need_acx = need_acy = 0;///////////////////////////////////
#endif
		w_pitch = -pids[ACCX_SPEED].get_pid(need_acx + Mpu.get_w_faccX(), Mpu.dt);
		w_roll = pids[ACCY_SPEED].get_pid(need_acy + Mpu.get_w_faccY(), Mpu.dt);


		//----------------------------------------------------------------преобр. в относительную систему координат
		pitch = Mpu.cosYaw*w_pitch - Mpu.sinYaw*w_roll;
		roll = Mpu.cosYaw*w_roll + Mpu.sinYaw*w_pitch;






		if (Log.writeTelemetry) {
			Log.block_start(LOG::XYSTAB);

			Log.loadFloat(need_speedX);
			Log.loadFloat(need_speedY);
			Log.loadFloat(pitch);
			Log.loadFloat(roll);

			Log.block_end();
		}



	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float old_altitude = 0;

//int tttcnt = 0;


float StabilizationClass::Z(){////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	

	

		float need_speedZ = getSpeed_Z(Autopilot.fly_at_altitude() - Mpu.get_Est_Alt());
		float need_accZ = dspeedZ2accZ*constrain(need_speedZ-Mpu.get_Est_SpeedZ(), max_stab_z_M, max_stab_z_P);
#ifdef FOR_TESTS
		need_accZ = 0;////////////////////
#endif

		float fZ = HOVER_THROTHLE + pids[ACCZ_PID].get_pid(constrain(need_accZ - Mpu.faccZ,-2,2), Mpu.dt)*Balance.powerK();

		if (Log.writeTelemetry) {
			Log.block_start(LOG::ZSTAB);

			Log.loadFloat(need_speedZ);
			Log.loadFloat(fZ);
			Log.block_end();
		}
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
	dalt2speedZ<<","<<pids[ACCZ_PID].kP()<<","<<\
	pids[ACCZ_PID].kI()<<","<<pids[ACCZ_PID].imax()<<","<<\
	max_stab_z_P<<","<<max_stab_z_M<<","<<\
	Z_CF_SPEED<<","<<Z_CF_DIST<<","<< Z_FILTER;
	string ret = convert.str();
	return string(ret);

}



void StabilizationClass::setZ(const float  *ar){


	int error = 1;
	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){
		
		error = 0;
		float t;
		uint8_t i = 0;

		error += Commander._set(ar[i++], dalt2speedZ);
		dalt2speedZ_Rep = 1.0f / dalt2speedZ;

		t = pids[ACCZ_PID].kP();
		if ((error += Commander._set(ar[i++],t))==0)
			pids[ACCZ_PID].kP(t);

		t = pids[ACCZ_PID].kI();
		if ((error += Commander._set(ar[i++], t))==0)
			pids[ACCZ_PID].kI(t);

		t = pids[ACCZ_PID].imax();
		if ((error += Commander._set(ar[i++], t))==0)
			pids[ACCZ_PID].imax(t);

		error += Commander._set(ar[i++], max_stab_z_P);
		error += Commander._set(ar[i++], max_stab_z_M);
		error += Commander._set(ar[i++], Z_CF_SPEED);
		error += Commander._set(ar[i++], Z_CF_DIST);
		t = Z_FILTER;
		if ((error += Commander._set(ar[i++], t)) == 0)
			Z_FILTER=t;

		//resset_z();
		cout << "Stabilization Z set:\n";

		for (uint8_t ii = 0; ii < i; ii++){
			cout << ar[ii] << ",";
		}
		cout << ar[i] << endl;
	}
	if (error>0){
		cout << "Stab Z set Error\n";
	}
}


string StabilizationClass::get_xy_set(){
	ostringstream convert;
	convert<<\
	accXY_stabKP<<","<<pids[ACCX_SPEED].kP()<<","<<\
	pids[ACCX_SPEED].kI()<<","<<pids[ACCX_SPEED].imax()<<","<<\
	max_speed_xy<<","<<XY_KF_SPEED<<","<<XY_KF_DIST<<","<< XY_FILTER;
	string ret = convert.str();
	return string(ret);
}

void StabilizationClass::setXY(const float  *ar){

	int error = 1;

	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){

		error = 0;
		float t;
		uint8_t i = 0;

		error += Commander._set(ar[i++], accXY_stabKP);
		accXY_stabKP_Rep = 1.0f / accXY_stabKP;

		t = pids[ACCX_SPEED].kP();
		if ((error += Commander._set(ar[i++], t))==0)
			set_acc_xy_speed_kp(t);

		t = pids[ACCX_SPEED].kI();
		if ((error += Commander._set(ar[i++], t))==0)
			set_acc_xy_speed_kI(t);

		t = pids[ACCX_SPEED].imax();
		if ((error += Commander._set(ar[i++], t))==0)
			set_acc_xy_speed_imax(t);
		
		error += Commander._set(ar[i++], max_speed_xy);
		error += Commander._set(ar[i++], XY_KF_SPEED);
		error += Commander._set(ar[i++], XY_KF_DIST);
		t = XY_FILTER;
		if ((error += Commander._set(ar[i++], t))==0)
			XY_FILTER=t;

		//resset_xy_integrator();
		cout << "Stabilization XY set:\n";
		for (uint8_t ii = 0; ii < i; ii++){
			cout << ar[ii] << ",";
		}
		cout << ar[i] << endl;
	}
	if (error>0)
	{
		cout << "Stab XY set Error\n";
	}
}












StabilizationClass Stabilization;
