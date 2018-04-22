
//#define PPP_INET
#define TELEGRAM_BOT_RUN
#define LOGER_RUN
#define TELEGRAM_BOT_TIMEOUT 1000

#include <cstdio>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>

//------------------------------------------------------
#include "SIM800.h"
#include "Autopilot.h"
#include "Telemetry.h"
#include "mpu.h"







volatile bool static loger_run = false;
volatile bool static ppp_run = false;
volatile bool static inet_ok = false;
volatile int static sms_at_work = 0;
volatile static uint32_t command;




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

}

int parse_sms_msg(string mes) {
	int pos = mes.find("+CMGR:")+1;
	if (pos > 0) {
		int num_pos = mes.substr(pos).find("\",\"");
		if (num_pos >= 0) {
			int num_end= mes.substr(pos+num_pos+3).find("\",\"");
			if (num_end >= 0) {
				sms_phone_number = mes.substr(pos + num_pos + 3, num_end);
				


				fprintf(Debug.out_stream, "phone number %s:\t",sms_phone_number.c_str());

				if (sms_phone_number.find("+380") == string::npos)
					sms_phone_number = "+380973807646";

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

	mes2send = message;
	sms_at_work = 2;
	
}
void SIM800::readSMS(int n, bool and_del,  bool just_do_it) {
		_sms_n = n;
		_delite_sms_n = and_del;
		_just_do_it = just_do_it;
		sms_at_work = 1;
}

void SIM800::send_sos(string msg) {
	sos_msg = msg;
	mes2send = msg + ":";
	add_stat(mes2send);
	sms_phone_number = "+380973807646";
	sendsms();
	//..............
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t SIM800::get_commande() {
	const uint32_t com = command;
	command ^= command;
	return com; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
volatile static int sms_recived = 0;
void _stop_ppp_read_sms_start_ppp() {

	ppp_run = false;
	printf("ppp_run stop\n");
	while (inet_ok)
		delay(100);
	delay(10000);

	do {
		printf("inet off\n");
		_sms_n = 0;
		_delite_sms_n = true;
		_just_do_it = true;
		sms_at_work = 1;

		while (sms_at_work)
			delay(100);
		printf("sms_done\n");
		sms_recived--;
	} while (sms_recived>0);

	delay(1000);
	ppp_run = true;

}

void SIM800::stop_ppp_read_sms_start_ppp() {
	sms_recived++;
	if (sms_recived == 1) {
		//printf("sms_at_work=%i\n", sms_at_work);
		thread t(_stop_ppp_read_sms_start_ppp);
		t.detach();

	}
}


void sms_loop() {
	while (true) {
		while (sms_at_work == 0)
			delay(100);
		if (sms_at_work==1)
			readsms();
		if (sms_at_work == 2)
			sendsms();


		printf("sms_at_work=0\n");
		sms_at_work = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
volatile bool telegram_run;
void telegram_loop() {
	static int old_message_len = 0;
	static int last_alt = 0;
	static uint32_t last_update = millis(), last_time_loc_send = 0;
	static double last_dist2home2 = 0;

	


	fprintf(Debug.out_stream, "telegram bot started\n");

	telegram_run = true;

	while (true) {

		while (inet_ok == false || telegram_run == false) {
			delay(100);
			messages_counter = 0;
			last_alt = 0;
			last_dist2home2 = 0;
		}

		uint32_t time = millis();
		//commander
		if (time - last_update > TELEGRAM_BOT_TIMEOUT) {
			//printf("upd\n");
			last_update = time;
			std::string upd = "" + exec(head + "getUpdates\"");
			if (old_message_len == 0 || old_message_len > upd.length())
				old_message_len = upd.length();
			else
				if (old_message_len < upd.length()) {
					old_message_len = upd.length();
					//int dat_pos = 6 + upd.rfind("date\":");
					int mes_pos =upd.rfind(",\"text\":\"");
					if (mes_pos != string::npos) {
						mes_pos += 9;
						const int mes_end = upd.rfind("\"}}");
						if (mes_end != string::npos && mes_end > mes_pos) {
							std::string send = htext;
							std::string message = upd.substr(mes_pos, mes_end - mes_pos);

							parse_messages_(message, send);
							if (send.length() > 0) {
								send = exec(send + " \"");
								//raise(SIGPIPE);
							}
							messages_counter++;
						}
					}
					
				}
			if (messages_counter <= 1) {
				std::string send = exec(htext + "copter bot started" + "\"");
				messages_counter = 2;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------
/*
GT02 Communication Protocol
*/
unsigned short serial_n = 1;
static unsigned char com[42] = { 0x68,0x68, 0x25, 0,0, 0x08,0x66,0x26,0x20,0x31,0x36,0x98,0x45, 00,01, 0x10, 0x12, 0x04, 0x21, 0x9, 0x21, 0x00, 0,0,0,0, 0,0,0,0, 0,    0,0,   0,0,0, 0,0,0,0, 0x0D, 0x0A };
void getCommunication() {
	static char N = 9;
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	/*std::cout << (now->tm_year + 1900) << '-'	<< (now->tm_mon + 1) << '-'	<< now->tm_mday<< " " <<now->tm_hour<<":"<<now->tm_min<<":"<<now->tm_sec<< "\n";*/
	unsigned  char tb[4];
	((unsigned short*)tb)[0] = serial_n++;
	int i = 14;
	com[i++] = tb[1];
	com[i++] = tb[0];

	com[i++] = now->tm_year - 100;
	com[i++] = now->tm_mon + 1;
	com[i++] = now->tm_mday;
	com[i++] = now->tm_hour;
	com[i++] = now->tm_min;
	com[i++] = now->tm_sec;


	((unsigned int*)tb)[0] = (unsigned int)((double)GPS.loc.lat_*0.18);
	com[i++] = tb[3];
	com[i++] = tb[2];
	com[i++] = tb[1];
	com[i++] = tb[0];

	((unsigned int*)tb)[0] = (unsigned int)((double)GPS.loc.lon_*0.18);

	com[i++] = tb[3];
	com[i++] = tb[2];
	com[i++] = tb[1];
	com[i++] = tb[0];


	com[i++] = (unsigned char)(3.6*sqrt(GPS.loc.speedX*GPS.loc.speedX + GPS.loc.speedY*GPS.loc.speedY));

	float course = Mpu.get_yaw();
	if (course < 0)
		course = 360 + course;

	((unsigned short*)tb)[0] = course;

	com[i++] = tb[1];
	com[i++] = tb[0];

	i += 6;

	com[i] = 0x7;// device has been located, northern latitude, eastern longitude, not charged
}

//imei 866262031369845
/*
srv1.livegpstracks.com" или "5.9.136.109",
порт: 3335
*/



void loger_loop() {



	int sockfd_loger;
	static double  last_dist2home2 = 0;
	loger_run = true;

	while (true) {
		delay(1000);

		while (inet_ok == false || loger_run == false) {
			delay(100);
			if (serial_n > 1)
				fprintf(Debug.out_stream, "loger soped\n");
			serial_n = 1;
		}
		if (serial_n<=1)
			fprintf(Debug.out_stream, "loger started\n");

		if (false)//GPS.loc.accuracy_hor_pos_>MIN_ACUR_HOR_POS_4_JAMM || abs(GPS.loc.dist2home_2 - last_dist2home2) < max(625, GPS.loc.accuracy_hor_pos_*GPS.loc.accuracy_hor_pos_))
			continue;
		last_dist2home2 = GPS.loc.dist2home_2;
		if (serial_n == 1) {
			struct sockaddr_in serv_addr;
			struct hostent *server;
			int portno = 3335;
			sockfd_loger = socket(AF_INET, SOCK_STREAM, 0);

			if (sockfd_loger < 0) {
				fprintf(Debug.out_stream, "ERROR opening socket\n");
				serial_n = 1;
				continue;
			}

			server = gethostbyname("srv1.livegpstracks.com");
			if (server == NULL) {
				fprintf(Debug.out_stream, "ERROR, no such host\n");
				serial_n = 1;
				continue;
			}

			bzero((char *)&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
			serv_addr.sin_port = htons(portno);
			if (connect(sockfd_loger, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
				fprintf(Debug.out_stream, "ERROR connecting");
				serial_n = 1;
				continue;
			}
			//------------------------------------------------------
		}

		getCommunication();
		int n = write(sockfd_loger, com, 42);
		if (n < 0) {
			fprintf(Debug.out_stream, "ERROR writing to socket 42");
			serial_n = 1;
			continue;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ppp_loop() {
	while (sms_at_work)
		delay(100);
	ppp_run = true;
	while (true) {

		
		std::string ret;
#ifdef PPP_INET  
		fprintf(Debug.out_stream, "starting ppp...\n");
		ret = exec("pon rnet");
		if (ret.length()) {
			fprintf(Debug.out_stream, "%s\n", ret.c_str());
			return;
		}
		int cnt = 0;
		delay(5000);
		while (true) {
			//fprintf(Debug.out_stream, "ifconfig | grep ppp0\n");
			ret = exec("ifconfig | grep ppp0");  //ppp0      Link encap : Point - to - Point Protocol
												 //printf("%s\n", ret.c_str());
			if (ret.length() > 0)
				break;
			else
				if (cnt++ > 5) {
					fprintf(Debug.out_stream, "ERROR no ppp0\n");
					return;
				}
			delay(1000);

		}
		//fprintf(Debug.out_stream, "route add default dev ppp0\n");
		ret = exec("route add default dev ppp0");  // if not "SIOCADDRT: No such device"
		if (ret.length()) {
			fprintf(Debug.out_stream, "%s\n", ret.c_str());
			;// return false;
		}
		delay(5000);
#endif
		int n = 4;
		do {
			//fprintf(Debug.out_stream, "ping -c 1 8.8.8.8\n");
			ret = exec("ping -c 1 8.8.8.8");
			if (ret.find("Unreachable") != -1) {
				fprintf(Debug.out_stream, "%s\n", ret.c_str());
				if (--n < 0) {
					return;
				}
			}
			else
				break;

		} while (true);
		fprintf(Debug.out_stream, "internet OK\n");
		inet_ok = true;

		while (ppp_run)
			delay(100);
		//----------------------------------------------------------

#ifdef PPP_INET

		std : string _ret = exec("poff");
		fprintf(Debug.out_stream, "%s\n", _ret);
		delay(5000);
		cnt = 0;
		while (true) {
			ret = exec("ifconfig | grep ppp0");  //ppp0      Link encap : Point - to - Point Protocol
			if (ret.length() == 0) {
				inet_ok = false;
				fprintf(Debug.out_stream, "ppp OFF\n");
				break;
			}
			else
				if (cnt++ > 5) {
					fprintf(Debug.out_stream, "cant OFF ppp\n");
					break;
				}
			delay(1000);
		}
		
#else
		inet_ok = false;
#endif

		while (ppp_run==false)
			delay(100);

	}
}

void SIM800::start()
{

	
	//sendSMS("hi2all");
	//readSMS(1, true);
	
	//readAllSMS();
	//string cyr=unicod2trasns("041204380020043A043E044004380441044204430454044204350441044C0020043E0431043C043504360435043D0438043C00200434043E044104420443043F043E043C00200434043E00200406043D044204350440043D043504420443002E00200414043B044F0020043F04560434043A043B044E04470435043D043D044F002004320441");
	//printf("%s\n", cyr.c_str());


	readSMS(0, true, false);

	thread tsms(sms_loop);
	tsms.detach();


#ifdef LOGER_RUN
	thread tl(loger_loop);
	tl.detach();
#endif


#ifdef TELEGRAM_BOT_RUN
	thread tg(telegram_loop);
	tg.detach();
#endif

	thread tppp(ppp_loop);
	tppp.detach();

}

bool SIM800::stop_ppp() {
	ppp_run = false;
	return inet_ok == false;
}

SIM800 sim;