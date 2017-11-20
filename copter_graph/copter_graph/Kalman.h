#pragma once
#include "stdafx.h"

class Kalman
{
public:
	int initialized = 0;

	Kalman(float sigmaEta, float v0);
	~Kalman();
	Kalman() {};
	float update(float in);

private:
	int tt;
	float sigmaPsi = 1;
	float sigmaEta = 50;
	float xOpt[2];
	float z[2];
	float eOpt[2];
	float K[2];
};

