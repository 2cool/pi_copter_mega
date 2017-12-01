#include "SIM800.h"

//------------------------------------------------------
#define SIM_UPD_P 5000
enum { sim_GO2HOME = 1 };


static volatile int error = 0;
int last_update = 0;
std::string head = "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/";

void loop_t()
{
	static int last_time_loc_send = 0;
	//poff - off ppp0
	std::string ret;
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

	while (sim._loop) {
		int time = millis();
		if (time - last_update > SIM_UPD_P) {
			last_update = time;
			std::string upd = exec(head + "getUpdates\"");

			int res = upd.find("{\"ok\":true,\"result\":[]}");
			if (res != 0) {
				int dat_pos = 6 + upd.rfind("date\":");
				int mes_pos = upd.rfind(",\"text\":\"");
				int data = std::stoi(upd.substr(dat_pos, mes_pos - dat_pos));
				if (sim.old_message_data != data) {
					sim.old_message_data = data;
					mes_pos += 9;
					std::string message = upd.substr(mes_pos, upd.rfind("\"}}") - mes_pos);
					if (message.find("go2home") == 0) {
						sim.command = sim_GO2HOME;
						fprintf(Debug.out_stream, "recived mess - go2home\n");
						std::string send = exec(head + "sendMessage?chat_id=241349746&text=go2home OK\"");
					}
					else
						std::string send = exec(head + "sendMessage?chat_id=241349746&text=?\"");
					fprintf(Debug.out_stream, "recived mess: %s\n", message.c_str());
				}
			}
			if (GPS.loc.dist2home_2 - sim.last_dist2home > 625 || abs(GPS.loc.altitude - sim.last_alt) > 10 || (time - last_time_loc_send) > 30000) {
				last_time_loc_send = time;
				sim.last_dist2home = GPS.loc.dist2home_2;
				sim.last_alt = GPS.loc.altitude;

				std::string req = head + "sendMessage?chat_id=241349746&text=" + \
					std::to_string(GPS.loc.lat_) + " " + std::to_string(GPS.loc.lon_) + " " + std::to_string((int)GPS.loc.altitude) + "\"";
				std::string send = exec(req.c_str());
				
			}
			

		}
		delay(1000);
	}

}

int SIM800::getCommand() {
	if (command == -1)
		return -1;
	else {
		int c = command;
		command = -1;
		return c;
	}
}
void SIM800::stop() {
	_loop = false;
}

void SIM800::start()
{
	error = -1;
	last_update = millis();
	last_dist2home = -100;
	last_alt = -1000;
	command = -1;
	old_message_data = 0;


	_loop = true;
	thread t(loop_t);
	t.detach();

}


















SIM800 sim;