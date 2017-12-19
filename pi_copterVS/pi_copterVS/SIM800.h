#pragma once
#include "define.h"
#include "WProgram.h"
#include "GPS.h"
#include "debug.h"

class SIM800
{
private:
	
public:
	bool pppstoped();
	string sms;
	volatile bool sms_done;
	void sendSMS(string message);
	void readSMS(int n, bool and_del=false);



	volatile bool _loop;
	void start();
	void stop();
	int get_error();
	uint32_t get_commande();
};
extern SIM800 sim;
