#include "stdafx.h"
#include "kalman.h"
#include "math.h"

float Kalman::update(float in) {

	xOpt[0] += (in - xOpt[0]) / (1+sigmaEta);
	return xOpt[0];

	/*
	int t = tt & 1;
	int t1 = t^1;
	z[t] = in;
	eOpt[t] = sqrt((sigmaEta*sigmaEta)*(eOpt[t1] * eOpt[t1] + sigmaPsi*sigmaPsi) / (sigmaEta *sigmaEta + eOpt[t1] * eOpt[t1] + sigmaPsi*sigmaPsi));
	K[t] = (eOpt[t] * eOpt[t]) / (sigmaEta * sigmaEta);
	xOpt[t] = (xOpt[t1])*(1 - K[t]) + K[t] * z[t];
	tt++;
	return xOpt[t];
	*/
}

Kalman::Kalman(float sigmaEta_, float v0)
{
	initialized = 77777;
	sigmaPsi = 1;
	sigmaEta = sigmaEta_;
	tt = 0;
	z[0] = v0;
	xOpt[0] = z[0];
	eOpt[0] = sigmaEta;
}


Kalman::~Kalman()
{
}
