#pragma once


class Pressure
{
public:
	double max_alt = -10000,acc = 0,speed = 0,	dt = 0,alt = 0,start_alt=10101010;
	void init();
	int decode(char buffer[], int &i);
	int view(int &indexes, char buffer[], int &i);
};
extern Pressure press;
