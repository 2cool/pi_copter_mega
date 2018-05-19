#include <cstdio>
#include <string>
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
#include <string.h>
#include <iostream>
#include <string>
#include <math.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <math.h>
#include <ctime>
#include <thread>


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
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

using namespace std;





//#define FORTEST

#ifdef FORTEST
#include "../../../../repos/pi_copter_mega/pi_copterVS/pi_copterVS/glob_header.h"

#else
#include "../pi_copterVS/glob_header.h"
#endif



#define PPP_INET
#define TELEGRAM_BOT_RUN
#define LOGER_RUN




std::string exec(const std::string cmd) {
	//printf(cmd.c_str());
	//printf("\n");
	char buffer[128];
	std::string result = "";
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		//throw std::runtime_error("popen() failed!");
		printf("pipe brock\n");
		return "";
	}
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return result;
}











string sos_msg;

key_t          ShmKEY;
int            ShmID;
struct Memory *shmPTR;

int init_shmPTR() {
	if (shmPTR == 0) {


		ShmKEY = ftok(SHMKEY, 'x');
		ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
		if (ShmID < 0) {
			printf("*** shmget error (server) ***\n");
			return 1;
		}
		shmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////
uint32_t start_seconds = 0;
uint32_t millis() {
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	uint32_t ret;
	if (start_seconds == 0)
		start_seconds = t.tv_sec;
	ret = ((t.tv_sec - start_seconds) * 1000) + (t.tv_nsec / 1000000);
	return ret;
}

void delay(unsigned long t) {
	usleep(t * 1000);
}
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))


//////////////////////////////////////////////////////////////////////////
int stop_ppp();

string cyr[] = {
	"A","B","V","G","D","E","ZH","Z","I","J","K","L","M","N","O","P","R","S","T","U","F","X","C","CH","SH","SHH","\"","Y","'","EH","YU","YA",
	"a", "b", "v", "g", "d", "e", "zh", "z", "i", "j", "k", "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "x", "c", "ch", "sh", "shh", "\"", "y", "'", "eh", "yu", "ya" };

string unicod2trasns(string text) {
	if (text.length() < 4)
		return "";
	int pos;
	string out = "";
	for (int p = 0; p < text.length(); p += 4) {
		if (text.substr(p, 2).compare("04") == 0) {
			int c = stoi(text.substr(p + 1, 3), nullptr, 16) - 0x410;
			if (c < 0 || c >= 64) if (c < 0 || c >= 64) {
				//out += text.substr(p, 4);
				out += "?";
				continue;
			}
			out += cyr[c];
		}
		else if (text.substr(p, 2).compare("00") == 0) {
			char c = (char)(stoi(text.substr(p + 1, 3), nullptr, 16) - 0x400);
			out += c;
		}
	}
	return out;
}







static unsigned int messages_counter = 0;

string sms_phone_number;
string sms_mes;

static const std::string head = "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/";
static const std::string htext = "curl -k -s \"https://api.telegram.org/bot272046998:AAESv6nbLLWWm1nGaYPRc9Etr04XhY3aUww/sendMessage?chat_id=241349746&text=";

int _sms_n;
bool _delite_sms_n, _just_do_it;


int send_command(string &mes, bool resp_more = false, int timeout = 5000) {
	int fd_in = open("/dev/tnt0", O_RDWR | O_NOCTTY | O_SYNC);
	if (fd_in < 0)
	{
		printf( "error %d opening /dev/tnt0: %s", errno, strerror(errno));
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

			//printf( "%s", mes.c_str());


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
			if (ok >= 0 || err >= 0) {

				for (int i = 0; i < mes.length(); i++)
					if (mes[i] == '\r')// || mes[i] == '\n')
						mes[i] = ' ';
				//printf("%s\n",mes.c_str());
				break;
			}
		}
	}


	close(fd_in);
	return (ok >= 0) ? 0 : -1;
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
		printf( "%s\n", mes.c_str());
		mes = "AT+CMGS=\"" + sms_phone_number + "\"\r\n";
		res = send_command(mes, true);
		if (res == 0) {
			printf( "%s ", mes.c_str());
			mes = mes2send + char(26);
			res = send_command(mes);
			printf( "%s\n", mes.c_str());
			if (res == 0) {
				;// printf( "%s\n", mes.c_str());

			}
			else
				printf( "ERROR3\n");
		}
		else
			printf( "ERROR2\n");
	}
	else
		printf( "ERROR1\n");

}


string get_next_sms(string mes)
{
	static int pos = 0;
	if (pos == 0)
		pos = mes.find("+CMGL: ");
	if (pos == -1)
		return "";
	int next = mes.substr(pos + 8).find("+CMGL: ") + pos + 9;


	return mes;
}

void readallsms() {
	string mes = "AT+CMGF=1\r";
	int res = send_command(mes);
	if (res == 0) {
		mes = "AT+CMGL=\"ALL\"\r";
		res = send_command(mes);
		if (res == 0) {

			printf( "%s\n", mes.c_str());

			if (_delite_sms_n) {
				mes = "AT+CMGDA=\"DEL ALL\"\r";
				res = send_command(mes);
				if (res == 0) {
					printf( "%s\n", mes.c_str());
				}
			}
		}
	}

}

int parse_sms_msg(string mes) {
	int pos = mes.find("+CMGR:") + 1;
	if (pos > 0) {
		int num_pos = mes.substr(pos).find("\",\"");
		if (num_pos >= 0) {
			int num_end = mes.substr(pos + num_pos + 3).find("\",\"");
			if (num_end >= 0) {
				sms_phone_number = mes.substr(pos + num_pos + 3, num_end);



				printf( "phone number %s:\t", sms_phone_number.c_str());

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
						printf( "%s", sms_mes.c_str());
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
	std::string req = "lat:" + std::to_string(shmPTR->lat_) + " lon:" + std::to_string(shmPTR->lon_) + " alt:" + std::to_string((int)shmPTR->altitude) + " hor:" + std::to_string((int)shmPTR->accuracy_hor_pos_);
	return req;
}


void add_stat(string &send) {
	if (shmPTR->control_bits & MOTORS_ON){
		send += "m_on,";
		if (shmPTR->control_bits & GO2HOME)
			send += "go2home,";
		else if (shmPTR->control_bits & PROGRAM) 
			send += "prog,";
	}
	else
		send += "m_off,";
	send += "b" + std::to_string((int)shmPTR->voltage) + ",";
	send += getLocation() + ",";

}
string down_case(string &str) {
	for (int i = 0; i < str.length(); i++) {
		if (str[i] >= 65 && str[i] <= 90)
			str[i] += 32;
	}
	return str;
}
const static int  com_bit[] = { MOTORS_ON ,GO2HOME,CONTROL_FALLING,REBOOT,SHUTDOWN,GIMBAL_PLUS,GIMBAL_MINUS,PROGRAM,Z_STAB,XY_STAB,COMPASS_ON,HORIZONT_ON,MPU_GYRO_CALIBR,COMPASS_CALIBR };
const static string str_com[] = { "motorson","go2home","cntrf","reboot","shutdown","gimbp","gimbm","prog","zstab","xystab","compason","horizonton","mpugyrocalibr","compasscalibr","stat", "help", "help_all" };
const static int arr_size = sizeof(com_bit) / 4;
//-----------------------------------------------------------------------------
void parse_messages_(string message, string &send) {

	message = down_case(message);
	const string in = send;
	const bool seccure_f = true;// (message.find("282496") != string::npos);
	if (seccure_f) {

		for (int i = 0; i < arr_size; i++) {

			if (message.find(str_com[i]) != string::npos) {
				shmPTR->control_bits_4_do = com_bit[i];
				send += str_com[i];
				break;
			}

		}
		if (message.find(str_com[14]) != string::npos) {
			add_stat(send);
		}
		else if (message.find(str_com[15]) != string::npos) {
			for (int i = 0; i < 17; i++)
				send += str_com[i] + ",";
		}
	}
	if (send.compare(in) == 0) {
		send = "";

	}
	printf( "recived mess: %s\n", message.c_str());

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
			//printf( "%s\n", mes.c_str());

			if (ret != -1) {
				parse_sms_command();
				if (_delite_sms_n) {
					mes = "AT+CMGD=" + to_string(_sms_n) + "\r";
					res = send_command(mes);
					if (res == 0) {
						printf( " - del");
						//printf( "%s\n", mes.c_str());
					}
				}
				printf( "\n");
			}

		}

	}
	return ret;
}

void readsms() {
	if (_sms_n > 0)
		readsms_n();
	else {//read all
		printf( "\n");
		for (int i = 1; i <= 20; i++) {
			_sms_n = i;
			if (readsms_n() == -1)
				break;
		}
	}
}
void sendSMS(string message) {

	mes2send = message;
	shmPTR->sms_at_work = 2;

}
void readSMS(int n, bool and_del, bool just_do_it) {
	_sms_n = n;
	_delite_sms_n = and_del;
	_just_do_it = just_do_it;
	shmPTR->sms_at_work = 1;
}

void send_sos(string msg) {
	sos_msg = msg;
	mes2send = msg + ":";
	add_stat(mes2send);
	sms_phone_number = "+380973807646";
	sendsms();
	//..............
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////////////////////

void _stop_ppp_read_sms_start_ppp() { // external

	shmPTR->ppp_run = false;
	printf("ppp_run stop\n");
	while (shmPTR->inet_ok) {
		delay(100);
		shmPTR->internet_cnt++;
	}

	printf("inet off\n");
	_sms_n = 0;
	_delite_sms_n = true;
	_just_do_it = true;
	shmPTR->sms_at_work = 1;

	while (shmPTR->sms_at_work) {
		delay(100);
		shmPTR->internet_cnt++;
	}
	printf("sms_done\n");
	delay(100);
	shmPTR->ppp_run = true;

}




void sms_loop() {
	while (true) {
		while (shmPTR->sms_at_work == 0)
			delay(100);
		if (shmPTR->sms_at_work == 1) {
			readsms();
			stop_ppp();
		}
		if (shmPTR->sms_at_work == 2) {
			stop_ppp();
			sendsms();
		}

		shmPTR->sms_at_work = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
volatile bool telegram_run;
void telegram_loop() {
	static int old_message_len = 0;
	static int last_alt = 0;
	static uint32_t last_update = millis(), last_time_loc_send = 0;
	static double last_dist2home2 = 0;






	telegram_run = true;

	while (true) {


		while (/*GPS.loc.lon_home == 0 || GPS.loc.lat_home == 0 || WiFi.connectedF() ||*/ shmPTR->inet_ok == false || telegram_run == false) {
			if (messages_counter > 0)
				printf( "telegram bot stoped\n");
			delay(100);
			messages_counter = 0;
			last_alt = 0;
			last_dist2home2 = 0;
		}

		if (messages_counter == 0)
			printf( "telegram bot started\n");

		uint32_t time = millis();
		//commander
		if (time - last_update > 5000) {
			//printf("upd\n");
			last_update = time;
			std::string upd = "" + exec(head + "getUpdates\"");
			//printf("%s\n", upd.c_str());
			if (old_message_len == 0 || old_message_len > upd.length())
				old_message_len = upd.length();
			else
				if (old_message_len < upd.length()) {
					old_message_len = upd.length();
					//int dat_pos = 6 + upd.rfind("date\":");
					int mes_pos = upd.rfind(",\"text\":\"");
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
			if (messages_counter == 0) {
				std::string send = exec(htext + "copter bot started" + "\"");
				messages_counter++;
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


	((unsigned int*)tb)[0] = (unsigned int)((double)shmPTR->lat_*0.18);
	com[i++] = tb[3];
	com[i++] = tb[2];
	com[i++] = tb[1];
	com[i++] = tb[0];

	((unsigned int*)tb)[0] = (unsigned int)((double)shmPTR->lon_*0.18);

	com[i++] = tb[3];
	com[i++] = tb[2];
	com[i++] = tb[1];
	com[i++] = tb[0];


	com[i++] = (unsigned char)(3.6*sqrt(shmPTR->speedX*shmPTR->speedX + shmPTR->speedY*shmPTR->speedY));

	float course = shmPTR->yaw;
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
	shmPTR->loger_run = true;

	while (true) {
		delay(1000);

		while (shmPTR->inet_ok == false || shmPTR->loger_run == false) {
			delay(100);
			if (serial_n > 1)
				printf( "loger stoped\n");
			serial_n = 1;
		}
		if (serial_n <= 1)
			printf( "loger started\n");

		if (false)//GPS.loc.accuracy_hor_pos_>MIN_ACUR_HOR_POS_4_JAMM || abs(GPS.loc.dist2home_2 - last_dist2home2) < max(625, GPS.loc.accuracy_hor_pos_*GPS.loc.accuracy_hor_pos_))
			continue;
		last_dist2home2 = shmPTR->dist2home_2;
		if (serial_n == 1) {
			struct sockaddr_in serv_addr;
			struct hostent *server;
			int portno = 3335;
			sockfd_loger = socket(AF_INET, SOCK_STREAM, 0);

			if (sockfd_loger < 0) {
				//printf( "ERROR opening socket\n");
				serial_n = 1;
				continue;
			}

			server = gethostbyname("srv1.livegpstracks.com");
			if (server == NULL) {
				//printf( "ERROR, no such host\n");
				serial_n = 1;
				continue;
			}

			bzero((char *)&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
			serv_addr.sin_port = htons(portno);
			if (connect(sockfd_loger, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
				//printf( "ERROR connecting");
				serial_n = 1;
				continue;
			}
			//------------------------------------------------------
		}

		getCommunication();
		int n = write(sockfd_loger, com, 42);
		if (n < 0) {
			//printf( "ERROR writing to socket 42");
			serial_n = 1;
			continue;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int start_ppp() {
	if (shmPTR->inet_ok == true)
		return 0;
	//delay(15000);
	shmPTR->ppp_run = true;
#ifdef PPP_INET 

	while (shmPTR->sim800_reset_time > 0)
		delay(100);

	printf( "starting ppp...\n");
	string ret = exec("pon rnet");
	if (ret.length()) {
		printf( "pop %s\n", ret.c_str());
		return -1;
	}
	int cnt = 0;
	delay(5000);

	//printf( "route add default dev ppp0\n");
	ret = exec("route add default dev ppp0");  // if not "SIOCADDRT: No such device"
	if (ret.length()) {
		printf( "route %s\n", ret.c_str());
		return -1;
	}
	else {

		return 0;
	}
	delay(1000);
#endif
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////
static int no_inet_errors = 0;
int stop_ppp() {

	if (shmPTR->inet_ok == false)
		return 0;



	system("route add default dev wlan0");
	system("poff -a");
	printf( "ppp OFF\n");
	shmPTR->inet_ok = false;

}


void test_ppp_inet_and_local_loop() {
	int n = 2;
	int ln = 2;
	while (shmPTR->ppp_run) {
		delay(5000);
		string ret = exec("ping -w 5 -c 1 8.8.8.8");
		if (ret.find("1 received") == string::npos) {
			if (--n <= 0) {
				printf( "no inet %s %i\n", ret.c_str(), n);
				no_inet_errors++;
				break;
			}
		}
		else {
			if (shmPTR->inet_ok == false) {
				printf( "internet OK\n");
				shmPTR->inet_ok = true;
			}

			delay(10000);
			n = 2;
			no_inet_errors = 0;
		}

		ret = exec("ping -w 1 -c 1 192.168.43.1");
		if (ret.find("1 received") == string::npos) {
			if (--ln <= 0) {
				//printf( "%s %i\n", ret.c_str(),n);
				ret = exec("nmcli dev wifi | grep 2coolzNET");
				if (ret.find(" 2coolzNET ") != string::npos) {
					exec("ifconfig wlan0 down");
					delay(1000);
					exec("ifconfig wlan0 up");
					delay(5000);
					ln = 2;
				}
			}
		}
		else
			ln = 2;
	}
}
/////////////////////////////////////////////////////////////////////////////
void ppp_loop() {



	while (true) {
		while (shmPTR->sms_at_work )
			delay(100);

		stop_ppp();
		start_ppp();
		test_ppp_inet_and_local_loop();
		stop_ppp();
		if (no_inet_errors >= 3) {
			no_inet_errors = 0;
			shmPTR->sim800_reset = true;
		}



		while (shmPTR->ppp_run == false)
			delay(100);

	}
}
//sendSMS("hi2all");
//readSMS(1, true);

//readAllSMS();
//string cyr=unicod2trasns("041204380020043A043E044004380441044204430454044204350441044C0020043E0431043C043504360435043D0438043C00200434043E044104420443043F043E043C00200434043E00200406043D044204350440043D043504420443002E00200414043B044F0020043F04560434043A043B044E04470435043D043D044F002004320441");
//printf("%s\n", cyr.c_str());


//////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])//lat,lon,.......
{
	static bool stop_start = false;
	if (init_shmPTR())
		return 0;

	readSMS(0, true, false);

	thread tsms(sms_loop);
	tsms.detach();

	thread tppp(ppp_loop);
	tppp.detach();


	thread tl(loger_loop);
	tl.detach();

	thread tg(telegram_loop);
	tg.detach();


	while (true) {
		if ( stop_start == false && shmPTR->stop_ppp_read_sms_start_ppp ) {
			stop_start = true;
			_stop_ppp_read_sms_start_ppp();
			shmPTR->stop_ppp_read_sms_start_ppp = stop_start = false;
		}
		else {
			usleep(100000);
			shmPTR->internet_cnt++;
		}
	}

    return 0;
}