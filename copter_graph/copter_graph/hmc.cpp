#include "stdafx.h"
#include "hmc.h"


Hmc::Hmc()
{
}


Hmc::~Hmc()
{
}

//enum{X,Y,Z};

uint8_t buffer[6];
int16_t mx, my, mz,c_base[3];
float fmx, fmy, fmz,dx,dy,dz;
void Hmc::parser(byte buf[], int n) {

	memcpy(buffer, buf,  6);


	//if (mode == HMC5883L_MODE_SINGLE) writeByte(devAddr, HMC5883L_RA_MODE, HMC5883L_MODE_SINGLE << (HMC5883L_MODEREG_BIT - HMC5883L_MODEREG_LENGTH + 1));
	mx = (((int16_t)buffer[0]) << 8) | buffer[1];
	my = (((int16_t)buffer[4]) << 8) | buffer[5];
	mz = (((int16_t)buffer[2]) << 8) | buffer[3];
	//fmy = -(float)(mx - c_base[X])*dx;
	//fmx = -(float)(my - c_base[Y])*dy;
	//fmz = -(float)(mz - c_base[Z])*dz;

	// Tilt compensation
	//float Xh = fmx * mpu.cosPitch - fmz * mpu.sinPitch;
	//float Yh = fmx * mpu.sinRoll * Mpu.sinPitch + fmy * mpu.cosRoll - fmz * mpu.sinRoll * mpu.cosPitch;

	//heading = (float)atan2(Yh, Xh);
	//log_sens();
















}
Hmc hmc;