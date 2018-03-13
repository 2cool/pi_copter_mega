#pragma once

enum {bSOM1,bSOM2,bcPITCH,bcROLL,bTHROTHLE,bLOGBANK,bF0,bF1,bF2,bF3,bALL};

class Balance
{
	
public:

	float f[bALL];
	float _max[bALL], _min[bALL];
	void init();
	int decode(char buffer[], int &i, bool rotate);


};
extern Balance bal;
