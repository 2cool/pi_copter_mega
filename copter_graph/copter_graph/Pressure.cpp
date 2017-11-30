#include "stdafx.h"
#include "Pressure.h"
#include <math.h>
void Pressure::init()
{
	max_alt = -10000;
	start_alt = 10101010;
	acc = 0;
	speed = 0;
	dt = 0;
	alt = 0;
}
int Pressure::view(int &indexes, char buffer[], int &i) {
	indexes++;

	//float pressur = *(float*)(&buffer[i + 1]);
	i += 5;

	return 0;
}

int Pressure::decode(char buffer[], int &i)
{
	float pressure = *(float*)(&buffer[i + 1]);
	
#define PRESSURE_AT_0 101325
	double t_alt = (44330.0f * (1.0f - pow((double)pressure / PRESSURE_AT_0, 0.1902949f)));
	if (start_alt == 10101010)
		start_alt = t_alt;
	t_alt -= start_alt;
	max_alt = max(max_alt, t_alt);
	float tpressure_speed = (t_alt - alt) / dt;
	alt = t_alt;
	acc = (tpressure_speed - speed) / dt;
	speed = tpressure_speed;

	dt = 0;
	i += 5;
	return 0;
}
Pressure press;