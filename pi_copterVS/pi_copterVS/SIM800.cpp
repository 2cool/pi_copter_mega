
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>

//------------------------------------------------------
#include "SIM800.h"
#include "Autopilot.h"
#include "Telemetry.h"



string cyr[] = { 
	"A","B","V","G","D","E","ZH","Z","I","J","K","L","M","N","O","P","R","S","T","U","F","X","C","CH","SH","SHH","\"","Y","'","EH","YU","YA",
	"a", "b", "v", "g", "d", "e", "zh", "z", "i", "j", "k", "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "x", "c", "ch", "sh", "shh", "\"", "y", "'", "eh", "yu", "ya" };

string unicod2trasns(string text) {
	if (text.length() < 4)
		return "";
	int pos;
	string out = "";
	for (int p = 0; p < text.length(); p += 4) {
		if (text.substr(p, 2).compare("04")==0) {
			int c = stoi(text.substr(p + 1, 3), nullptr,16)-0x410;
			if (c < 0 || c >= 64) if (c < 0 || c >= 64) {
				//out += text.substr(p, 4);
				out += "?";
				continue;
			}
			out += cyr[c];
		}
		else if (text.substr(p, 2).compare("00") == 0) {
			char c = (char)(stoi(text.substr(p + 1, 3), nullptr,16)-0x400);
			out += c;
		}
	}
	return out;
}





static volatile int error = -1;
volatile uint32_t command = 0;
static int messages_counter = 0;

string sms_phone_number;
string sms_mes;

static const std::string head =  "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/";
static const std::string htext = "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/sendMessage?chat_id=241349746&text=";

int _sms_n;
bool _delite_sms_n, _just_do_it;


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

			//fprintf(Debug.out_stream, "%s", mes.c_str());


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

inline bool space(char c) {
	return std::isspace(c);
}

inline bool notspace(char c) {
	return !std::isspace(c);
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
		fprintf(Debug.out_stream, "%s\n", mes.c_str());
		mes = "AT+CMGS=\""+ sms_phone_number +"\"\r\n";
		res = send_command(mes, true);
		if (res == 0) {
			fprintf(Debug.out_stream, "%s ", mes.c_str());
			mes = mes2send + char(26);
			res = send_command(mes);
			fprintf(Debug.out_stream, "%s\n", mes.c_str());
			if (res == 0) {
				;// fprintf(Debug.out_stream, "%s\n", mes.c_str());

			}
			else
				fprintf(Debug.out_stream, "ERROR3\n");
		}
		else
			fprintf(Debug.out_stream, "ERROR2\n");
	}
	else
		fprintf(Debug.out_stream, "ERROR1\n");
	sim.sms_done = true;
}


string get_next_sms(string mes)
{
	static int pos = 0;
	if (pos == 0)
		pos = mes.find("+CMGL: ");
	if (pos == -1)
		return "";
	int next = mes.substr(pos+8).find("+CMGL: ")+pos+9;


	return mes;
}

void readallsms() {
	string mes = "AT+CMGF=1\r";
	int res = send_command(mes);
	if (res == 0) {
		mes = "AT+CMGL=\"ALL\"\r";
		res = send_command(mes);
		if (res == 0) {
			
			fprintf(Debug.out_stream, "%s\n", mes.c_str());

			if (_delite_sms_n) {
				mes = "AT+CMGDA=\"DEL ALL\"\r";
				res = send_command(mes);
				if (res == 0) {
					fprintf(Debug.out_stream, "%s\n", mes.c_str());
				}
			}
		}
	}
	sim.sms_done = true;


}

int parse_sms_msg(string mes) {
	int pos = mes.find("+CMGR:")+1;
	if (pos > 0) {
		int num_pos = mes.substr(pos).find("\",\"");
		if (num_pos >= 0) {
			int num_end= mes.substr(pos+num_pos+3).find("\",\"");
			if (num_end >= 0) {
				sms_phone_number = mes.substr(pos + num_pos + 3, num_end);
				fprintf(Debug.out_stream, "%s:\t",sms_phone_number.c_str());
				pos += num_pos + 3 + num_end;
				int mes_beg = mes.substr(pos).find("\n") + 1;
				if (mes_beg >= 1) {
					int end_beg = mes.substr(pos + mes_beg).find("\nOK") - 2;
					if (end_beg > 0) {
						sms_mes = mes.substr(pos + mes_beg, end_beg);
						if (sms_mes.length()>4 && sms_mes.c_str()[0] == '0' && (sms_mes.c_str()[1] == '0' || sms_mes.c_str()[1] == '4')) {
							string text = unicod2trasns(sms_mes);
							if (text.length() > 0 && sms_mes.length() / text.length() >= 3) {
								sms_mes = text;
								end_beg = text.length();
							}
						}
						fprintf(Debug.out_stream, "%s", sms_mes.c_str());
						return  end_beg;
					}
				}
			}
		}
	}
	sms_phone_number = "";
	sms_mes = "";
	return -1;
}
//-----------------------------------------------
string getLocation() {
	std::string req = "lat:"+std::to_string(GPS.loc.lat_) + " lon:" + std::to_string(GPS.loc.lon_) + " alt:" + std::to_string((int)GPS.loc.altitude) + " hor:" + std::to_string((int)GPS.loc.accuracy_hor_pos_);
	return req;
}


void add_stat(string &send) {
	if (Autopilot.motors_is_on()) {
		send += "m_on,";
		if (Autopilot.go2homeState())
			send += "go2home,";
		else if (Autopilot.progState())
			send += "prog,";
	}
	else
		send += "m_off,";
	send += "b" + std::to_string((int)Telemetry.get_voltage4one_cell()) + ",";
	send += getLocation() + ",";

}
string down_case(string &str) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] >= 65 && str[i] <= 90)
			str[i] += 32;
	}
	return str;
}
const static int  com_bit[] = { MOTORS_ON ,CONTROL_FALLING,REBOOT,SHUTDOWN,GIMBAL_PLUS,GIMBAL_MINUS,PROGRAM,Z_STAB,XY_STAB,COMPASS_ON,HORIZONT_ON,MPU_GYRO_CALIBR,COMPASS_CALIBR};
const static string str_com[] = { "motors_on","cntr_f","reboot","shutdown","gimb_p","gimb_m","prog","z_stab","xy_stab","compas_on","horizont_on","mpu_gyro_calibr","compass_calibr" };
const static int arr_size = sizeof(com_bit) / 4;
//-----------------------------------------------------------------------------
void parse_messages_(string message, string &send) {

	command = 0;
	message = down_case(message);
	const string in = send;
	const bool seccure_f = true;// (message.find("282496") != string::npos);
	if (seccure_f) {
		
		for (int i = 0; i < arr_size; i++) {

			if (message.find(str_com[i]) != string::npos) {
				command = com_bit[i];
				send += str_com[i];
				break;
			}
		
		}

		if (message.find("exit") != string::npos)
			Autopilot.exit();
		else if (message.find("stat") != string::npos){
			add_stat(send);
		}
		else if (message.find("m_off") != string::npos)	{
			Autopilot.off_throttle(true, "off");
			send += "m_off OK";
		}
		else if (message.find("cntr_f") != string::npos)		{
			Autopilot.off_throttle(false, "off");
			send += "cntr_f OK";
		}
		else if (message.find("help_all") != string::npos) {
			send += "xxxxxx,stat,exit, m_off,cntr_f,motors_on,cntr_f,reboot,shutdown,gimb_p,gimb_m,prog,z_stab,xy_stab,compas_on,horizont_on,mpu_gyro_calibr,compass_calibr";
		}
		else if (message.find("help") != string::npos)	{
				send += "xxxxxx,stat,exit, m_off,cntr_f";
		}



	}

	
	
	
	
	
	
	if (send.compare(in) == 0) {
		send = "";
		
	}
	fprintf(Debug.out_stream, "recived mess: %s\n", message.c_str());

}



//-----------------------------------------------------------------------------
void parse_sms_command() {
	if (_just_do_it) {
		mes2send = "";
		parse_messages_(sms_mes, mes2send);
		if (mes2send.length()>0)
			sendsms();
		_just_do_it = false;
		sms_phone_number = "";
		sms_mes = "";
	}
}

int readsms_n() {
	int ret = -1;
	string mes = "AT+CMGF=1\r";
	int res = send_command(mes);
	if (res == 0) {
		mes = "AT+CMGR=" + to_string(_sms_n) + "\r";
		res = send_command(mes);
		if (res == 0) {
			ret = parse_sms_msg(mes);
			//fprintf(Debug.out_stream, "%s\n", mes.c_str());

			if (ret != -1) {
				parse_sms_command();
				if (_delite_sms_n) {
					mes = "AT+CMGD=" + to_string(_sms_n) + "\r";
					res = send_command(mes);
					if (res == 0) {
						fprintf(Debug.out_stream, " - del");
						//fprintf(Debug.out_stream, "%s\n", mes.c_str());
					}
				}
				fprintf(Debug.out_stream, "\n");
			}

		}

	}
	sim.sms_done = true;
	return ret;
}

void readsms() {
	if (_sms_n > 0)
		readsms_n();
	else {//read all
		fprintf(Debug.out_stream, "\n");
		for (int i = 1; i <= 20; i++) {
			_sms_n = i;
			readsms_n();
		}
	}
}






void SIM800::sendSMS(string message) {
	if (sms_done) {
		sms_done = false;
		mes2send = message;
		thread t(sendsms);
		t.detach();
	}
}
void SIM800::readSMS(int n, bool and_del,  bool just_do_it) {
	if (sms_done) {
		sms_done = false;
		_sms_n = n;
		_delite_sms_n = and_del;
		_just_do_it = just_do_it;
		thread t(readsms);
		t.detach();
	}
}
void SIM800::readAllSMS(bool and_del) {
	if (sms_done) {
		sms_done = false;
		_delite_sms_n = and_del;
		thread t(readallsms);
		t.detach();
	}
}


int SIM800::get_error() { return error; }

uint32_t SIM800::get_commande() {
	const uint32_t com = command;
	command ^= command;
	return com; 
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
//	printf("pop rnet\n");
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
			//printf("upd\n");
			last_update = time;
			std::string upd = "" + exec(head + "getUpdates\"");
		//	printf(upd.c_str());
			//int res = upd.find("{\"ok\":true,\"result\":[]}");

			int dat_pos = 6 + upd.rfind("date\":");
			int mes_pos = upd.rfind(",\"text\":\"");

			if (dat_pos > 6 && mes_pos > 0  && dat_pos + mes_pos - dat_pos < upd.length()) {
				int data = std::stoi(upd.substr(dat_pos, mes_pos - dat_pos));

				if (old_message_data != data) {
					old_message_data = data;
					mes_pos += 9;


					const int finds = upd.rfind("\"}}");
					if (finds > 0 && mes_pos + (finds - mes_pos) < upd.length()) {
						if (messages_counter > 0) {
							std::string send = htext;
							std::string message = upd.substr(mes_pos, finds - mes_pos);

							parse_messages_(message, send);
							if (send.length()>0)
								send = exec(send + " \"");
						}
					}else
						fprintf(Debug.out_stream, "err2\n");
				}
			}else
				fprintf(Debug.out_stream, "err1\n");
			messages_counter ++;
		}
		
		//send location
		if (messages_counter <= 1) {
			//printf("snd1\n");
			std::string send = exec(htext + "copter bot started"+"\"");
			messages_counter = 2;
		}
		if (mes2send.length()>0 || (GPS.loc.accuracy_hor_pos_< 99 && (time - Autopilot.last_time_data_recived) > 1000))
		{
		
			if (
				GPS.loc.dist2home_2 - last_dist2home2 > max(625, GPS.loc.accuracy_hor_pos_*GPS.loc.accuracy_hor_pos_) || 
				abs(GPS.loc.altitude - last_alt) > max(10,GPS.loc.accuracy_ver_pos_) || 
				(time - last_time_loc_send) > 300000
				)
			{
				last_time_loc_send = time;
				last_dist2home2 = GPS.loc.dist2home_2;
				last_alt = GPS.loc.altitude;
				//printf("snd2\n");
				std::string send = exec(htext + getLocation() + "\"");
			}
		}
	}
#ifdef PPP_INET
	
	std:string _ret=exec("poff");
	fprintf(Debug.out_stream, "%s\n",_ret);
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
	//sendSMS("hi2all");
	//readSMS(1, true);
	readSMS(0, true, false);
	//readAllSMS();
	//string cyr=unicod2trasns("041204380020043A043E044004380441044204430454044204350441044C0020043E0431043C043504360435043D0438043C00200434043E044104420443043F043E043C00200434043E00200406043D044204350440043D043504420443002E00200414043B044F0020043F04560434043A043B044E04470435043D043D044F002004320441");
	//printf("%s\n", cyr.c_str());
	error = -1;
	command = 0;
	_loop = true;
#ifdef TELEGRAM_BOT_RUN
	thread t(loop_t);
	t.detach();
#endif
}


void SIM800::send_sos(string msg) {
	sos_msg = msg;
	mes2send = msg + ":";
	add_stat(mes2send);
	sms_phone_number = "+380973807646";
	sendsms();
	//..............
}
















SIM800 sim;