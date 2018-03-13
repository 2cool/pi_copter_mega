#pragma once
#include "graph.h"
enum { mPITCH, mROLL, mrPITCH, mrROLL, mYAW, mGYRO_PITCH, mGYRO_ROLL, mGYRO_YAW, mACCX, mACCY, mACCZ,mMAXACC ,mALL_E };
class Mpu
{
private:
	void do_magic();
public:

	double dt, time, gdt;

	double f[mALL_E];
	double _max[mALL_E];
	double _min[mALL_E];

	double cosYaw, sinYaw;
	float e_speedX, e_speedY;
	float e_accX, e_accY, _sinYaw, _cosYaw;
	float m7_accX, m7_accY;
	float f_pitch, f_roll;
	float w_accX, w_accY;
	float sinPitch, cosPitch, sinRoll, cosRoll;

	void init();
	int decode(char buffer[], int &i, bool rotate = false);
	int view(int &indexes, char buffer[], int &i);
};
extern Mpu mpu;
