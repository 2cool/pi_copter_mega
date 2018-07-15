#include "stdafx.h"
#include "Mpu.h"
#include "Pressure.h"
#include "GPS_Loger.h"







#define ROLL_COMPENSATION_IN_YAW_ROTTATION 0.02
#define PITCH_COMPENSATION_IN_YAW_ROTTATION 0.025

float ac_accX = 0, ac_accY = 0, ac_accZ = 1.15504527;
float agpitch = 0, agroll = 0, agyaw = 0;

uint64_t maxG_firs_time = 0;


bool compas_flip = false;


#define _2PI 6.283185307179586476925286766559


bool pitch_flag;
bool set_yaw_flag = true;

inline void sin_cos(const float a, float &s, float &c) {
	s = (float)sin(a);
	c = (float)cos(a);
	/*
	const double ss = s*s;
	c = (float)sqrt(1 - min(1.0f, ss));
	//30.7.2017 corected
	if (abs(a) > 90)
	c = -c;
	*/
}



/*
void Mpu::do_magic() {
	sin_cos(f[mYAW] *GRAD2RAD, _sinYaw, _cosYaw);
	//---calc acceleration on angels------
	sin_cos(f[mPITCH] *GRAD2RAD, sinPitch, cosPitch);
	sin_cos(f[mROLL] *GRAD2RAD, sinRoll, cosRoll);

#define WIND_SPEED_X sqrt(abs(w_accX / DRAG_K))*((w_accX>=0)?1:-1)
#define WIND_SPEED_Y sqrt(abs(w_accY / DRAG_K))*((w_accY>=0)?1:-1)

	//	float windX = e_speedX;/// +WIND_SPEED_X;
	//	float windY = e_speedY;// +WIND_SPEED_Y;


	e_accX = -G*(-_cosYaw*sinPitch - _sinYaw*sinRoll) - e_speedX*abs(e_speedX)*DRAG_K - w_accX;
	e_accY = G*(-_cosYaw*sinRoll + _sinYaw*sinPitch) - e_speedY*abs(e_speedY)*DRAG_K - w_accY;

	w_accX += (e_accX - gps_log.gax - w_accX)*0.01;
	w_accY += (e_accY - gps_log.gay - w_accY)*0.01;

	e_speedX += e_accX*dt;
	e_speedX += (gps_log.gspeedX - e_speedX)*0.1;

	e_speedY += e_accY*dt;
	e_speedY += (gps_log.gspeedY - e_speedY)*0.1;

	//-----calc real angels------
	m7_accX += ((_cosYaw*e_accX + _sinYaw*e_accY) - m7_accX)*0.007;
	m7_accX = constrain(m7_accX, -MAX_ACC / 2, MAX_ACC / 2);
	m7_accY += ((_cosYaw*e_accY - _sinYaw*e_accX) - m7_accY)*0.007;
	m7_accY = constrain(m7_accY, -MAX_ACC / 2, MAX_ACC / 2);

	f_pitch = f[mPITCH];
	f_roll = f[mROLL];

	f[mPITCH] = RAD2GRAD*atan2((sinPitch + m7_accX*cosPitch / G), cosPitch);// +abs(gaccX*sinPitch));
	f[mROLL] = RAD2GRAD*atan2((sinRoll - m7_accY*cosRoll / G), cosRoll);// +abs(gaccY*sinRoll));

}


*/



//////////////////////////////////////////////////////////////////////////////////////
int load_uint8(byte buf[], int i) {
	int vall = buf[i];
	vall &= 255;
	//if (vall<0)
	//	vall=0-vall;
	return vall;
}

uint64_t loaduint64t(byte buf[], int i) {

	uint64_t  *ip = (uint64_t*)&buf[i];

	return *ip;

}




int32_t load_int32(byte buf[], int i) {

	int32_t *ip = (int32_t*)&buf[i];

	return *ip;
}
int16_t load_int16(byte buf[], int i) {

	int16_t *ip = (int16_t*)&buf[i];
	return *ip;
}


const float n003 = 0.030517578f;
const float n006 = 0.061035156f;
//4g
const float n122 = 1.220740379e-4;
double qw, qx, qy, qz,g_yaw;

void Mpu::toEulerianAngle()
{
	// roll (x-axis rotation)
	double sinr = +2.0 * (qw * qx + qy * qz);
	double cosr = +1.0 - 2.0 * (qx * qx + qy * qy);

	roll = RAD2GRAD * atan2(sinr, cosr);

	// pitch (y-axis rotation)
	double sinp = +2.0 * (qw * qy - qz * qx);
	if (abs(sinp) >= 1)
		pitch = RAD2GRAD * copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	else
		pitch = RAD2GRAD * asin(sinp);

	// yaw (z-axis rotation)
	double siny = +2.0 * (qw * qz + qx * qy);
	double cosy = +1.0 - 2.0 * (qy * qy + qz * qz);
	g_yaw = RAD2GRAD * atan2(siny, cosy);
}

void Mpu::loadmax_min(const int n, const double val, bool simetric) {



	if (_max_minC[n] == 0) {
		_max[n] = _min[n] = val;
		_max_minC[n]++;
	}
	else {
		_max[n] = max(mpu._max[n], val);
		_min[n] = min(mpu._min[n], val);
		if (simetric) {
			_max[n] = max(mpu._max[n], -val);
			_min[n] = min(mpu._min[n], -val);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Mpu::parser(byte buf[], int j, int len, bool filter) {
	static double old_time = 0;
	
	uint64_t itime = loaduint64t(buf, j);
	time = (double)itime *0.001;

	if (time > 80)
		time--;
	dt = time - old_time;
	old_time = time;

	j += 8;

	int g[3];
	int a[3];
	int q[4];


	g[0] = load_int16(buf, j);
	j += 2;
	g[1] = load_int16(buf, j);
	j += 2;
	g[2] = load_int16(buf, j);
	j += 2;

	a[0] = load_int16(buf, j);
	j += 2;
	a[1] = load_int16(buf, j);
	j += 2;
	a[2] = load_int16(buf, j);
	j += 2;


	q[0] = load_int32(buf, j);
	j += 4;
	q[1] = load_int32(buf, j);
	j += 4;
	q[2] = load_int32(buf, j);
	j += 4;
	q[3] = load_int32(buf, j);
	j += 4;





	f_pitch = *(float*)&buf[j]; j += 4;
	f_roll = *(float*)&buf[j]; j += 4;
	pitch= *(float*)&buf[j]; j += 4;
	roll= *(float*)&buf[j]; j += 4;
	yaw = *(float*)&buf[j]; j += 4;;
	gyroPitch = *(float*)&buf[j]; j += 4;
	gyroRoll = *(float*)&buf[j]; j += 4;
	gyroYaw = *(float*)&buf[j]; j += 4;



#define ACC_CF 0.007



	accX += ((*(float*)&buf[j] - accX)*(filter ? ACC_CF : 1));
	j += 4;
	accY += ((*(float*)&buf[j] - accY)*(filter ? ACC_CF :1));
	j += 4;
	accZnF = *(float*)&buf[j];
	j += 4;
	accZ += ((accZnF -accZ)*(filter? ACC_CF :1));
	//accZ = *(float*)&buf[j];


	

	/*
	qw = 1.5259e-5f*(float)q[0] / 16384.0f;
	qx = 1.5259e-5f*(float)q[1] / 16384.0f;
	qy = 1.5259e-5f*(float)q[2] / 16384.0f;
	qz = 1.5259e-5f*(float)q[3] / 16384.0f;

	toEulerianAngle();


	pitch = -pitch;
	g_yaw = -g_yaw;


	sin_cos(yaw, sinYaw, cosYaw);
	sin_cos(pitch, sinPitch, cosPitch);
	sin_cos(roll, sinRoll, cosRoll);

	tiltPower += (constrain(cosPitch*cosRoll, 0.5f, 1) - tiltPower)*tiltPower_CF;
	gyroPitch = -n006 * (float)g[1] - agpitch;
	gyroRoll = n006 * (float)g[0] - agroll;
	gyroYaw = -n006 * (float)g[2] - agyaw;

	float x = n122 * (float)a[0];
	float y = -n122 * (float)a[1];
	float z = n122 * (float)a[2];


	accZ = z * cosPitch + sinPitch * x;
	accZ = 9.8f*(accZ*cosRoll - sinRoll * y - 1) - ac_accZ;

	accY = 9.8f*(x*cosPitch - z * sinPitch) - ac_accX;
	//accY = 9.8f*(y*cosRoll + z * sinRoll) - ac_accY;

	*/

	loadmax_min(mPITCH, pitch,true);
	loadmax_min(mROLL, roll, true);
	loadmax_min(mYAW, yaw, true);

	loadmax_min(mACCX, accX, true);
	loadmax_min(mACCY, accY, true);
	loadmax_min(mACCZ, accZ, true);




}





void Mpu::init() {

	time = rdt = 0;
	for (int i = 0; i < mALL_E; i++) {
		_max[i] = -1000;
		_min[i] = 1000;
	}

		//hower_thr = HOVER_THROTHLE;
		//min_thr = MIN_THROTTLE_;
		//fall_thr = FALLING_THROTTLE;
		DRAG_K = 0.0052;
		//DRAG_K = 0.022;
		_0007 = 0.007;
		gaccX = gaccY = 0;
		acc_callibr_timed = 0;
		rate = 100;
		tiltPower_CF = 0.05;

		f_pitch = f_roll = 0;

		windFX = windFY = e_speedX = e_speedY = e_accX = e_accY = m7_accX = m7_accY = w_accX = w_accY = 0;
		yaw_off = 0;
		maccX = maccY = maccZ = 0;
		max_g_cnt = 0;
		cosYaw = 1;
		sinYaw = 0;
		temp_deb = 6;
		fx = fy = fz = 0;

		faccX = faccY = faccZ = 0;
		oldmpuTimed = 0;
		yaw_offset = yaw = pitch = roll = gyroPitch = gyroRoll = gyroYaw = accX = accY = accZ = 0;
		sinPitch = sinRoll = 0;
		tiltPower = cosPitch = cosRoll = 1;
		timed = 0;
}





Mpu mpu;