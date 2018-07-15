#pragma once


#include "graph.h"
#include "stdafx.h"

#include <stdint.h>

enum {bSOM1,bSOM2,bcPITCH,bcROLL,bTHROTHLE,bLOGBANK,bF0,bF1,bF2,bF3,bALL};

class Balance
{
	
public:
	double ap_roll = 0, ap_pitch = 0,ap_yaw=0,f0=0,f1=0,f2=0,f3=0,thr=0;
	float f[bALL];
	float _max[bALL], _min[bALL];
	void init();
	int decode(char buffer[], int &i, bool rotate);
	void parser(byte buf[], int i);

};
extern Balance bal;
