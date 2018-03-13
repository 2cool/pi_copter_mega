#include "stdafx.h"
#include "Balance.h"

int Balance::decode(char buffer[], int &i, bool rotate) {
	i += 9;
	f[bcPITCH]=*(float*)(&buffer[i]);
	i += 4;
	f[bcROLL] = *(float*)(&buffer[i]);
	i+= 4;
	f[bTHROTHLE] = *(float*)(&buffer[i]);
	i += 4;
	int n=*(int*)(&buffer[i]);
	i += 4;
	i += 2;
	f[bF0] = *(float*)(&buffer[i]);
	i += 4;
	f[bF1] = *(float*)(&buffer[i]);
	i += 4;
	f[bF2] = *(float*)(&buffer[i]);
	i += 4;
	f[bF3] = *(float*)(&buffer[i]);
	i += 4;

	return 0;
}
void Balance::init()
{
	for (int i = 0; i < bALL; i++) {
		_max[i] = -1000000;
		_min[i] =  1000000;
	}


}
Balance bal;


