
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>

//------------------------------------------------------
#include "SIM800.h"
#include "Autopilot.h"
#include "Telemetry.h"

#define SIM_UPD_P 5000
//#define PPP_INET


static volatile int error = -1;
volatile uint32_t command = 0;

static bool first_false_command = true;

static const std::string head =  "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/";
static const std::string htext = "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/sendMessage?chat_id=241349746&text=";


int _sms_n;
bool _delite_sms_n;


int send_command(string &mes, bool resp_more=false, int timeout = 5000) {
	int fd_in = open("/dev/tnt0", O_RDWR | O_NOCTTY | O_SYNC);
	if (fd_in < 0)
	{
		fprintf(Debug.out_stream, "error %d opening /dev/tnt0: %s", errno, strerror(errno));
		mes = "error";
		return -1;
	}
	write(fd_in, mes.c_str(), mes.length());
	mes = "";
	int tcur;
	int tlast_upd = tcur = millis();
	int ok;
	while (tcur - tlast_upd < timeout) {
		ok = -1;
		int a_in;
		delay(10);
		tcur = millis();
		ioctl(fd_in, FIONREAD, &a_in);
		if (a_in) {
			tlast_upd = tcur;
			char buf[256];
			int av = read(fd_in, &buf, a_in);

			

			mes += string(buf, av);
			int  err = ok = -1;
			if (resp_more) {
				int pos = max(0, mes.length() - 3);
				ok = mes.find("> ", pos);
			}
			else {
				int pos = max(0, mes.length() - 5);
				ok = mes.find("OK\r\n", pos);
				pos = max(0, mes.length() - 8);
				err = mes.find("ERROR\r\n", pos);
			}
			if ( ok>=0 || err >=0) {
				
				for (int i = 0; i < mes.length(); i++)
				if (mes[i] == '\r')// || mes[i] == '\n')
					mes[i] = ' ';
				//printf("%s\n",mes.c_str());
				break;
			}
		}
	}


	close(fd_in);
	return (ok>=0)?0:-1;
}

void readsms() {
	string mes = "AT+CMGF=1\r";
	int res=send_command(mes);
	mes = "AT+CMGR=" + to_string(_sms_n) + "\r";
	res=send_command(mes);
	printf("%s\n", mes.c_str());
	sim.sms_done = true;
}

string mes2send;
string telNumber;

void sendsms() {
/*
	AT+CMGF=1
	AT+CMGS="+380661140320\r"
	>
	"hello world"+char(26)
	+CMGS: 62
	*/
	string mes = "AT+CMGF=1\r";
	int res = send_command(mes);
	if (res == 0) {
		printf("%s\n", mes.c_str());
		mes = "AT+CMGS=\"+380661140320\"\r\n";
		res = send_command(mes, true);
		if (res == 0) {
			printf("%s ", mes.c_str());
			mes = mes2send + char(26);
			res = send_command(mes);
			if (res == 0) {
				printf("%s\n", mes.c_str());
				
			}else
				printf("ERROR\n");
		}else
			printf("ERROR\n");
	}
	else
		printf("ERROR\n");
	sim.sms_done = true;
}
void SIM800::sendSMS(string message) {
	if (sms_done) {
		sms_done = false;
		mes2send = message;
		thread t(sendsms);
		t.detach();
	}
}
void SIM800::readSMS(int n, bool and_del) {
	if (sms_done) {
		sms_done = false;
		_sms_n = n;
		_delite_sms_n = and_del;
		thread t(readsms);
		t.detach();
	}
}



int SIM800::get_error() { return error; }

uint32_t SIM800::get_commande() {
	const uint32_t com = command;
	command ^= command;
	return com; }

//-----------------------------------------------
string getLocation() {
	std::string req = 	std::to_string(GPS.loc.lat_) + "," + std::to_string(GPS.loc.lon_) + "," + std::to_string((int)GPS.loc.altitude) ;
	return req;
}
//-----------------------------------------------

volatile bool ppp_stoped = true;
bool SIM800::pppstoped() { return ppp_stoped; }
void loop_t()
{
	static int old_message_data=0,last_alt=0;
	static uint32_t last_update = millis(), last_time_loc_send=0;
	static double last_dist2home2 = 0;

	//poff - off ppp0
	std::string ret;
	ppp_stoped = true;
#ifdef PPP_INET
	usleep(10000000);
//	fprintf(Debug.out_stream, "pop rnet\n");
	ret = exec("pon rnet");
	if (ret.length()) {
		fprintf(Debug.out_stream, "%s\n",ret.c_str());
		error = 1;
		return;
	}
	ppp_stoped = false;
	usleep(5000000);
	fprintf(Debug.out_stream, "ifconfig | grep ppp0\n");
	ret = exec("ifconfig | grep ppp0");  //ppp0      Link encap : Point - to - Point Protocol
	if (ret.length() == 0) {
		fprintf(Debug.out_stream, "ERROR no ppp0\n");
		error = 2;
		return;
	}
	//fprintf(Debug.out_stream, "route add default dev ppp0\n");
	ret = exec("route add default dev ppp0");  // if not "SIOCADDRT: No such device"
	if (ret.length()) {
		fprintf(Debug.out_stream, "%s\n", ret.c_str());
		error = 3;
		return;
	}
#endif
	delay(5000);
	int n = 4;
	do {
		//fprintf(Debug.out_stream, "ping -c 1 8.8.8.8\n");
		ret = exec("ping -c 1 8.8.8.8");
		if (ret.find("Unreachable")!= -1) {
			fprintf(Debug.out_stream, "%s\n", ret.c_str());
			if (--n < 0) {
				error = 4;
				return;
			}
		}
		else
			break;
	} while (true);
	fprintf(Debug.out_stream, "internet OK!\n");
	error = 0;
	//-------------------------------------------
	while (sim._loop) {
		delay(1000);
		
		uint32_t time = millis();
		//commander
		if (time - last_update > SIM_UPD_P) {
			last_update = time;
			std::string upd = exec(head + "getUpdates\"");

			int res = upd.find("{\"ok\":true,\"result\":[]}");
			if (res != 0) {
				int dat_pos = 6 + upd.rfind("date\":");
				int mes_pos = upd.rfind(",\"text\":\"");
				int data = std::stoi(upd.substr(dat_pos, mes_pos - dat_pos));
				if (old_message_data != data) {
					old_message_data = data;
					mes_pos += 9;
					if (first_false_command == false) {
						std::string send = htext;
						std::string message = upd.substr(mes_pos, upd.rfind("\"}}") - mes_pos);
						
						if (message.find("go2home") == 0) {
							if (Autopilot.go2homeState() == false) {
								command = GO2HOME;
								fprintf(Debug.out_stream, "recived mess - go2home\n");
								send += "go2home OK";
							}else
								send += "already on the way to home";
						}else if (message.find("stat") == 0) 
						{
							if (Autopilot.motors_is_on()) {
								send += "m_on,";
								if (Autopilot.go2homeState())
									send += "go2home,";
								else if (Autopilot.progState())
									send += "prog,";
							}
							else
								send += "m_off,";
							send += "b" + std::to_string((int)Telemetry.get_voltage4one_cell())+",";
							send += getLocation() + ",";
						}
						else {
							send += "?";
							fprintf(Debug.out_stream, "recived mess: %s\n", message.c_str());
						}
						send = exec(send+" \"");
					}
				}
			}
			first_false_command = false;
		}
		//send location
		if ((time - Autopilot.last_time_data_recived) > 1000) 
		{
			if (GPS.loc.dist2home_2 - last_dist2home2 > 625 || abs(GPS.loc.altitude - last_alt) > 10 || (time - last_time_loc_send) > 300000) 
			{
				last_time_loc_send = time;
				last_dist2home2 = GPS.loc.dist2home_2;
				last_alt = GPS.loc.altitude;

				std::string send = exec(htext + getLocation() + "\"");
			}
		}
	}
#ifdef PPP_INET
	
	std:string ret=exec("poff");
	fprintf(Debug.out_stream, "%s\n",ret);
#endif
	delay(1000);
	ppp_stoped = true;
}


void SIM800::stop() {
	_loop = false;
}

void SIM800::start()
{
	sms_done = true;
	sendSMS("hi2all");
	//readSMS(1, false);
	//readSMS(2, false);



	error = -1;
	command = 0;
	_loop = true;
	//thread t(loop_t);
	//t.detach();

}


















SIM800 sim;