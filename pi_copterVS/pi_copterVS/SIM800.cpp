#include "SIM800.h"

//------------------------------------------------------
#define SIM_UPD_P 5000
enum { sim_GO2HOME = 1 };

int last_update = 0;
std::string head = "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/";

void loop_t()
{
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
			if (GPS.loc.dist2home_2 - sim.last_dist2home > 625 || abs(GPS.loc.altitude - sim.last_alt) > 10) {
				sim.last_dist2home = GPS.loc.dist2home_2;
				sim.last_alt = GPS.loc.altitude;

				std::string req = head + "sendMessage?chat_id=241349746&text=" + \
					std::to_string(GPS.loc.lat_) + " " + std::to_string(GPS.loc.lon_) + " " + std::to_string(GPS.loc.altitude) + "\"";
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