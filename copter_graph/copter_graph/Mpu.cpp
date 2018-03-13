#include "stdafx.h"
#include "Mpu.h"
#include "Pressure.h"
#include "GPS_Loger.h"

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
#define DRAG_K 0.0052
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








void Mpu::init() {
	time = gdt = 0;
	for (int i = 0; i < mALL_E; i++) {
		_max[i] = -1000;
		_min[i] = 1000;
	}
}



int Mpu::decode(char buffer[], int &i, bool rotate) {

	static float t_pitch = 0, t_roll = 0;

	dt = (double)buffer[i + 1] * 0.01;
	time += dt;
	gdt += dt;

	press.dt += dt;

	int j;
	for (j = 0; j < mALL_E-1; j++) {

		f[j] = *(float*)(&buffer[i + 2+j*4]);
		_max[j] = max(_max[j], f[j]);
		_min[j] = min(_min[j], f[j]);

	}
	const float n122 = 1.220740379e-4;
	j = j * 4;
	float ax= n122 * *(int16_t*)(&buffer[i + 2 + j]);
	float ay = n122 * *(int16_t*)(&buffer[i + 2 + j +2]);
	float az = n122 * *(int16_t*)(&buffer[i + 2 + j + 4]);

	f[mMAXACC]=sqrt(ax*ax + ay * ay + az * az);

	if (f[mMAXACC] > 1.2) {
		t_pitch += f[mGYRO_PITCH]*0.01;
		t_roll += f[mGYRO_ROLL] * 0.01;
		//f[mPITCH] = t_pitch;
		//f[mROLL] = t_roll;

	}
	else {
		t_pitch = f[mPITCH];
		t_roll = f[mROLL];
	}


	_max[mMAXACC] = max(_max[mMAXACC], f[mMAXACC]);
	_min[mMAXACC] = min(_min[mMAXACC], f[mMAXACC]);


	if (rotate) {
		cosYaw = cos(f[mYAW]*GRAD2RAD);
		sinYaw = sin(f[mYAW] *GRAD2RAD);
	}
	else {
		cosYaw = 1;
		sinYaw = 0;
	}
	//------------------------------------------------------------------

#define _MPU_M

#ifndef _MPU_M	
	do_magic();
#endif

	i += 52;
	return 0;
}
int Mpu::view(int &indexes, char buffer[], int &i) {
	indexes++;
	i += 52;
	return 0;
}
Mpu mpu;