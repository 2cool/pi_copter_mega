// 
// 
// 
#include <cstdio>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include "commander.h"
#include "mi2c.h"
#include "Autopilot.h"
#include "Telemetry.h"
#include "Balance.h"
//#include "Hmc.h"
//#include "mpu.h"
#include "Prog.h"
#include "define.h"
#include "Hmc.h"

#include "Stabilization.h"
#include "debug.h"

#include "Log.h"


void CommanderClass::init()
{
	init_shmPTR();





	//Out.println("COMMANDER INIT");
	vedeo_stream_client_addr = 0;
	ppp_inet = true;
	telegram_bot = false;

	yaw = yaw_offset = pitch = roll = throttle = 0;


}

//static private final int s_COUNTER = 0;
//static private final int s_BUTTONS_S = 2;
//static private final int s_HEADING = 3;
//static private final int s_HEIGHT = 5;
//static private final int s_ACCELEROMETER = 7;



#define r_COUNTER  0
#define r_BUTTONS_S  2
#define r_HEADING  4
#define r_HEIGHT 6
#define r_ACCELEROMETER  8




#define  MAX_bit_on 219
#define  OFF_bit_on  36
#define  START_STOP 170












short old_height_c = 0;


/*
M,COUNTER,HEIGHT,HEADING,X,Y,
A,COUNTER,LAT,LON,HEIGHT,HEADING, SPEED,TIME,


*/
//max angel = 20 градусов
//




#define ANGK 0.1f
uint8_t data_errors = 0;



static float ar_t333[SETTINGS_ARRAY_SIZE+1];
float * load(const string  buf, const uint8_t  * filds){

	ar_t333[SETTINGS_ARRAY_SIZE] = SETTINGS_IS_OK;
	for (int i = 0; i <SETTINGS_ARRAY_SIZE; i++){
		float val= (float)stod(buf.substr(filds[i], filds[i + 1]- filds[i] - 1));
		if (val != 0 || buf.substr(filds[i], filds[i + 1]- filds[i] - 1).find("0.0")==0)
			ar_t333[i] = val;
		else{
			//Out.println(buf.substr(filds[i], filds[i + 1]- filds[i] - 1));
			fprintf(Debug.out_stream, "%s\n",buf.substr(filds[i], filds[i + 1] - filds[i] - 1));
			ar_t333[10] = SETTINGS_ERROR;
			fprintf(Debug.out_stream,"%i\n",i);
			return ar_t333;
		}
	}

	return ar_t333;
}



uint8_t CommanderClass::_set(const float  val, float &set, bool secure){
	if (val == 0)
		return 1;
	if (secure == false){
		set = val;
	}else
		if (Autopilot.motors_is_on()){
			if (set*0.8f > val)
				set *= 0.8f;
			else
			if (set*1.2f < val)
				set *= 1.2f;
			else
				set = val;
		}else
			set = val;
	return 0;
}

bool CommanderClass::Settings(string buf){
	fprintf(Debug.out_stream,"settings\n");
	uint8_t filds[11];
	uint8_t fi = 0;
	uint8_t i = 0;

	i++;

	while (fi<10 && i<buf.length()){
		while (buf[i++] != ',');
		filds[fi++] = i++;

	}
	filds[10] = (uint8_t)buf.length();

	uint8_t n = (uint8_t)stoi(buf.substr(0, filds[0] - 1));
	switch (n){
	case 0:
		Balance.set(load(buf, filds));
		break;
	case 1:
		Stabilization.setZ(load(buf, filds));
		break;
	case 2:
		Stabilization.setXY(load(buf, filds));
		break;
	case 3:
		Autopilot.set(load(buf, filds));//secure
		break;
	case 4:
		Mpu.set(load(buf,filds));
		break;
	case 5:
		Hmc.set(load(buf, filds));
		break;
	case 6:
		set(load(buf, filds));
		
		break;
	default:
		return false;
	}
	return true;
	
}





/*

B,COUNTER,MAX,   full throttle
B,COUNTER,OFF,   off throttle
B,COUNTER,S_S,		start stop


*/






bool CommanderClass::ButtonMessage(string msg){

#ifdef DEBUG_MODE
	fprintf(Debug.out_stream,"<- $s\n",msg.c_str());
#endif

	bool command_correct = false;
/*
	
	if (msg.find(BEGIN_CONVERSATION) == 0){
		if (Autopilot.lost_conection_time > 0){
				
			Autopilot.lost_conection_time = 0;
			Telemetry.beginConversation();
			reset_old();
			res = true;
		}
		else{
			//Autopilot.lost_conection_time = millis();
		//	Autopilot.connectionLost_();
			return false;
		}

			
	}
	else */
	
	

	if (msg.find("UP") == 0 && msg[2] >= '0' && msg[2] <= '9'){
		int_fast8_t n = (int_fast8_t)(msg[2] - '0');
		//Out.println(msg);
		//Out.println(n);
		/////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		Telemetry.getSettings(n);
		command_correct = true;
	}

	if (command_correct == false)
		fprintf(Debug.out_stream,"WORNG MESSAGE\n");

	return command_correct;

}

float CommanderClass::getPitch(){ return Autopilot.horizont_onState() ? pitch : 0; }
float CommanderClass::getRoll(){ return Autopilot.horizont_onState() ? roll : 0; }



void CommanderClass::data_reset() {
	pitch = roll = 0;
	throttle = MIDDLE_POSITION;
}




int get32to8bMask(int v) {
	int mask = v & 255;
	mask ^= ((v >> 8) & 255);
	mask ^= ((v >> 16) & 255);
	mask ^= ((v >> 24) & 255);
	return mask;
}

int get16to8bMask(int v) {
	int mask = v & 255;
	mask ^= ((v >> 8) & 255);

	return mask;
}
float CommanderClass::get_yaw_minus_offset() {
	float y = yaw - yaw_offset;
	return wrap_180(y);
}
float CommanderClass::getYaw() {
	return yaw; 
}
bool CommanderClass::input(){


	if (shmPTR->connected==0 || shmPTR->wifibuffer_data_len_4_read ==  0)
		return false;
	
	if (Autopilot.busy()) {
		shmPTR->wifibuffer_data_len_4_read = 0;
		return true;
	}

	Autopilot.last_time_data_recivedd = Mpu.timed;
	uint8_t *buf = shmPTR->wifiRbuffer;
	if (shmPTR->wifibuffer_data_len_4_read >= 12) {

		if (Log.writeTelemetry) {
			Log.block_start(LOG::COMM,true);
			Log.loadMem(buf, shmPTR->wifibuffer_data_len_4_read,false);
			Log.block_end(true);
		}



		uint32_t mode = *(uint32_t*)buf;
		int sec_mask = mode >> 24;
			
		mode &= 0x00ffffff;
		int mask = get32to8bMask(mode);

		int i = 4;
		int16_t i_throttle = *(int16_t*)(buf + i);
		mask ^= get16to8bMask(i_throttle);
		i += 2;
			

		int16_t i_yaw = *(int16_t*)(buf + i);
		i += 2;
		mask ^= get16to8bMask(i_yaw);
			

		int i_yaw_offset = *(int16_t*)(buf + i);
		i += 2;
		mask ^= get16to8bMask(i_yaw_offset);
			

		int i_pitch = *(int16_t*)(buf + i);
		i += 2;
		mask ^= get16to8bMask(i_pitch);
			
		int i_roll = *(int16_t*)(buf + i);
		i += 2;
		mask ^= get16to8bMask(i_roll);


		if (mask == sec_mask) {
			Autopilot.set_control_bits(mode);
			throttle = 0.00003125f*(float)i_throttle;
			yaw = -ANGK*(float)i_yaw;
			yaw_offset = ANGK*(float)i_yaw_offset;
			pitch = ANGK*(float)i_pitch;
			roll = ANGK*(float)i_roll;
			if ((i + 3) < shmPTR->wifibuffer_data_len_4_read) {
				string msg = "";
				msg += *(buf + i++);
				msg += *(buf + i++);
				msg += *(buf + i++);
				if (msg.find(m_PROGRAM) == 0 && Autopilot.progState()==false) {
					Prog.add(buf + i);
				}
				else if (msg.find(m_SETTINGS) == 0) {
						Settings(string((char*)(buf+i)));
				}
				else if (msg.find(m_UPLOAD_SETTINGS) == 0) {
					Telemetry.getSettings(buf[i++]);
				}
				
			}
		}
		else {
			fprintf(Debug.out_stream,"COMMANDER ERROR\n");
		}
		shmPTR->wifibuffer_data_len_4_read = 0;
		return true;
	}
	else
	{
		shmPTR->wifibuffer_data_len_4_read = 0;
		return true;
	}
	

	return false;
}


void stop_stream() {

	int pid = get_pid("ffmpeg");
	if (pid != -1) {
		kill(pid, SIGQUIT);
		fprintf(Debug.out_stream, "stream STOP\n");
	}

}



string CommanderClass::get_set() {
	string s = std::to_string(vedeo_stream_client_addr);
	s += ",";
	s += (ppp_inet) ? "1" : "0";
	s += ",";
	s+= (telegram_bot) ? "1" : "0";
	s += ",";

	return s;
}

void CommanderClass::set(const float buf[]) {
	vedeo_stream_client_addr = buf[0];
	ppp_inet = buf[1] > 0;
	telegram_bot = buf[2] > 0;
	if (telegram_bot)
		ppp_inet = true;
	fprintf(Debug.out_stream, "trans adr %f\n", buf[0]);
	thread t(stop_stream);
	t.detach();

}






CommanderClass Commander;

