#pragma once

#include "graph.h"

class Pressure
{
private:
	double t_alt, t_sp;

	
public:
double temp, pressure,altitude;

	bool f;
	double cf_alt, cf_sp, cf_acc;
	double min_alt = 10000, max_alt = -10000, acc = 0, speed = 0, dt = 0, alt = 0;
	double min_sp = 10000, max_sp = -100000;
	double min_a = 10000, max_a = -100000;
	void init(bool filter, double cf_alt, double cf_sp, double cf_a);
	int decode(char buffer[], int &i);
	int view(int &indexes, char buffer[], int &i);
	void parser(byte buf[], int n);
};
extern Pressure press;
