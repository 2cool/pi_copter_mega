#pragma once
#include "graph.h"

class Mpu
{
private:
	void do_magic();
public:

	double dt, time, gdt;
	double pitch, roll, yaw, accX, accY, gyro_pitch, gyro_roll, rPitch, rRoll;
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
