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
	int last_update, last_dist2home, last_alt,command,old_message_data;
	void start();
	void stop();
	int getCommand();
};
extern SIM800 sim;
