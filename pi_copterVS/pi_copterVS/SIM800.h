#pragma once
#include "define.h"
#include "WProgram.h"
#include "GPS.h"
#include "debug.h"

class SIM800
{
private:
	
public:
	
	volatile bool _loop;
	void start();
	void stop();
	int get_error();
	uint32_t get_commande();
};
extern SIM800 sim;
