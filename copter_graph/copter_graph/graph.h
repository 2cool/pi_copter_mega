#pragma once

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <stdio.h>
#include <math.h>
#include <iostream> 
#include <string> 
#include "MyMath.h"
#include "Kalman.h"
#include "KK.h"
#include "MYfILTER.h"

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define RAD2GRAD 57.29578f
#define GRAD2RAD 0.0174533f

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

enum {
	MOTORS_ON = 1, CONTROL_FALLING = 2, Z_STAB = 4, XY_STAB = 8, GO2HOME = 0x10, PROGRAM = 0x20, COMPASS_ON = 0x40, HORIZONT_ON = 0x80,
	MPU_ACC_CALIBR = 0x100, MPU_GYRO_CALIBR = 0x200, COMPASS_CALIBR = 0x400, COMPASS_MOTOR_CALIBR = 0x800, RESETING = 0x1000, GIMBAL_PLUS = 0x2000, GIMBAL_MINUS = 0x4000
};


enum {
	PITCH,ROLL,R_PITCH,R_ROLL,YAW,HEADING,C_PITCH,C_ROLL,EMU_PITCH,EMU_ROLL,
	GYRO_PITCH,GYRO_ROLL,GYRO_YAW, MAXACC,MI0,MI1,MI2,MI3,BAT_F,ACC,ACCX,ANGLE,GACC,ACCY,ACCZ,GACCX,GACCY,GACCZ,PRESSURE,PRESSURE_SPEED,PRESSURE_ACC,SX,SY,X2HOME,Y2HOME,G_SPEED_X,G_SPEED_Y,G_SPEED,GX,GY,dX,SPEED_X,dY,SPEED_Y,
	SZ,GPS_Z,SPEED_Z,THROTTLE,F0,F1,F2,F3,I_THROTHLE,I_YAW,I_YAW_OFFSET,I_PITCH,I_ROLL, Y,X,TIME,DT,M_C_PITCH,M_C_ROLL,EXP0,EXP1,EXP2,EXP3, ROTATE, FILTER, CONTROL_BITS, ALL_ELEMENTS
};





struct SensorsData {
	double sd[ALL_ELEMENTS];
	
};

struct MODE {
	int mode, index;
};
#define G 9.8
#define MAX_ACC 7










struct SEND_I2C {

	int32_t lon;
	int32_t lat;
	int32_t height;
	uint8_t hAcc;
	uint8_t vAcc;
};



class Graph
	
{
private:

	


	void correct_c_pitch_c_roll(int p);

	uint16_t bat;
	uint16_t mi[4];

	int time_index;
	int gpsI= -1;
	void filter(float src, int dataI, int elementi,float max=4000);
	float cf;
	SensorsData *sensors_data;
	Kalman kalman[ALL_ELEMENTS];
	MYfILTER maccX, maccY;
	MYfILTER mgroll;

	int decode_Log();
	char * buffer;
	int  sensorsData;
	long lSize;


	void drawModes(Graphics &g, RectF rect);
	void draw(Graphics &g, RectF rect, float max, float min, int sdi);
	MODE modes[100];
	uint32_t def_mode =  Z_STAB + XY_STAB;
	int modesI = 0;
	double p, step;
	double gdt = 0;
	double press_dt = 0;


/*
	float pitch, roll, sinPitch, sinRoll, cosPitch, cosRoll,yaw;
	float e_speedX, e_speedY;
	float e_accX, e_accY,_sinYaw,_cosYaw;
	float m7_accX, m7_accY;
	float f_pitch, f_roll;
	float w_accX, w_accY;*/
	void do_magic();
	

public:
	int readLog();
	bool flags[ALL_ELEMENTS];
	Color color[ALL_ELEMENTS];
	std::wstring name[ALL_ELEMENTS];
	KK kk0,kk1,kk2,kk3,kk4,kk5;
	float bufkk0[128];
	int bufff0I = 0;

	Graph(char*);
	~Graph();
	int update(HDC hdc, RectF rect, double zoom, double pos);
	int updateGPS(HDC hdc, RectF rect, double zoom, double pos);
	int drawGPSmarkder(HDC hdc, RectF rect,  double pos);
	int drawText(HDC hdc, RectF rect, double pos);
};


