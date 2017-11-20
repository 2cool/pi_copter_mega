#pragma once
class KK
{
public:
	KK();
	~KK();
	KK(double frec, double quality);
	double update(double force, double dt);
private:
	double v;
	double m;
	double k;
	double x;

	double quality;



	
};

