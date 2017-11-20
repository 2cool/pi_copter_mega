#include "stdafx.h"
#include "KK.h"


KK::KK()
{
}


KK::~KK()
{
}
#define PI 3.1415926535897932384626433832795
KK::KK(double frec, double quality_) {
	k = 1;
	
	m = 1;
	double T = 1.0 / frec;
	m = k*T*T / (4 * PI*PI);


	//k = 4 * PI*PI*m / (T*T);
	v = 0;

	//k = frec*frec;
	quality = quality_;
	x = 0;

	
}


/*
double velocity;
double mass;
double springFK;
double s;
double quality;
*/
double KK::update(double force, double dt) {
	if (force != 0)
		force = force;
	double f=(force-k*x);
	double a = f / m;
	v += a*dt;
	v *=quality;
	x += v*dt;
	return x;
}