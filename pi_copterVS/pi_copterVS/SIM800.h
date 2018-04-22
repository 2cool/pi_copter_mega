#pragma once
#include "define.h"
#include "WProgram.h"
#include "GPS.h"
#include "debug.h"

class SIM800
{
private:
	void send_sos(string msg);

public:
	void stop_ppp_read_sms_start_ppp();



	string sms;

	void sendSMS(string message);
	void readSMS(int n,bool and_del=false, bool just_do_it=true);
	

	string sos_msg;

	void start();
	bool stop_ppp();

	uint32_t get_commande();
};
extern SIM800 sim;
