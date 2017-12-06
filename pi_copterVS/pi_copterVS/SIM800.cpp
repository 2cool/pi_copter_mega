
#define PPP_INET


//------------------------------------------------------
#include "SIM800.h"
#include "Autopilot.h"
#include "Telemetry.h"

#define SIM_UPD_P 5000

static volatile int error = -1;
volatile uint32_t command = 0;

static bool first_false_command = true;

static const std::string head =  "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/";
static const std::string htext = "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/sendMessage?chat_id=241349746&text=";

int SIM800::get_error() { return error; }

uint32_t SIM800::get_commande() {
	const uint32_t com = command;
	command ^= command;
	return com; }


void loop_t()
{
	static int old_message_data=0,last_alt=0;
	static uint32_t last_update = millis(), last_time_loc_send=0;
	static double last_dist2home2 = 0;

	//poff - off ppp0
	std::string ret;
#ifdef PPP_INET
	
	usleep(10000000);
//	fprintf(Debug.out_stream, "pop rnet\n");
	ret = exec("pon rnet");
	if (ret.length()) {
		fprintf(Debug.out_stream, "%s\n",ret.c_str());
		error = 1;
		return;
	}
	usleep(10000000);
	//fprintf(Debug.out_stream, "ifconfig | grep ppp0\n");
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
								send += "go2home OK\"";
							}else
								send += "already on the way to home\"";
						}else if (message.find("stat") == 0) {
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
		if ((time - Autopilot.last_time_data_recived) > 1000) {
			if (GPS.loc.dist2home_2 - last_dist2home2 > 625 || abs(GPS.loc.altitude - last_alt) > 10 || (time - last_time_loc_send) > 30000) {
				last_time_loc_send = time;
				last_dist2home2 = GPS.loc.dist2home_2;
				last_alt = GPS.loc.altitude;

				std::string req = htext + \
					std::to_string(GPS.loc.lat_) + "," + std::to_string(GPS.loc.lon_) + "," + std::to_string((int)GPS.loc.altitude) + "\"";
				std::string send = exec(req.c_str());
			}
		}
	}

}


void SIM800::stop() {
	_loop = false;
}

void SIM800::start()
{
	error = -1;
	command = 0;
	_loop = true;
	thread t(loop_t);
	t.detach();

}


















SIM800 sim;