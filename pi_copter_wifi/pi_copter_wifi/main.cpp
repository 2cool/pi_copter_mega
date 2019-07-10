#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
//#define FORTEST
#include <thread>
#ifdef FORTEST
#include"C:/Users/2coolz/pi_copter_mega/pi_copterVS/pi_copterVS/glob_header.h"
//#include "../../../../repos/pi_copter_mega/pi_copterVS/pi_copterVS/glob_header.h"

#else
#include "../pi_copterVS/glob_header.h"
#endif

key_t          ShmKEY;
int            ShmID;
struct Memory *shmPTR;

int init_shmPTR() {
	if (shmPTR == 0) {


		ShmKEY = ftok(SHMKEY, 'x');
		ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
		if (ShmID < 0) {
			cout << "*** shmget error (wifi) ***\n";
			return 1;
		}
		shmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	}
	return 0;
}

//thread t;


uint64_t offline_time = 0;
bool newConnection_;
bool is_connected(void) { return shmPTR->connected>0; }
string get_client_addres();




//http://www.cprogramming.com/tutorial/lesson10.html
volatile sig_atomic_t flag = 0;
void handler(int sig) { // can be called asynchronously
	flag = 1; // set flag
}
void pipe_handler(int sig) {
	cout << "wifi pipe error\n";
}




uint32_t start_seconds = 0;
uint32_t millis_g() {
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	uint32_t ret;

	ret = ((t.tv_sec) * 1000) + (t.tv_nsec / 1000000);
	return ret;
}
uint32_t millis() {
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	uint32_t ret;
	if (start_seconds == 0)
		start_seconds = t.tv_sec;
	ret = ((t.tv_sec - start_seconds) * 1000) + (t.tv_nsec / 1000000);
	return ret;
}



int64_t micros(void) {
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	int64_t ret;
	if (start_seconds == 0)
		start_seconds = t.tv_sec;
	ret = ((int64_t)(t.tv_sec - start_seconds) * 1000000) + (t.tv_nsec / 1000);
	return ret;
}
void delay(unsigned long t) {
	usleep(t * 1000);
}

int wifi_connections = 0;
int sockfd, newsockfd, portno;
socklen_t clilen;

struct sockaddr_in serv_addr, cli_addr;
int n;

#define uchar unsigned char


string get_client_addres() {
	int adr = cli_addr.sin_addr.s_addr;
	string s = to_string((uchar)(adr & 255)) + "." + to_string((uchar)(255 & (adr >> 8))) + "." + to_string((uchar)(255 & (adr >> 16))) + "." + to_string((uchar)(adr >> 24));
	return s;
}



string log_fname;

void mclose() {

	
	cout << "server stoped\n";
	wifi_connections--;
	close(newsockfd);
	close(sockfd);
	cout << "WIFI closed\n";
}




int new_server() {
	if (wifi_connections>0)
		return 0;
	wifi_connections++;
	/*
	string adr;
	do {
	delay(1000);
	adr = get_my_ip_addres();
	} while ( adr.length() == 0);
	*/

	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		//cout << "ERROR opening socket/n";
		wifi_connections--;
		return -1;
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = 9876;
	serv_addr.sin_family = AF_INET;

	serv_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(adr.c_str());
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		//cout << "ERROR on binding/n";
		wifi_connections--;
		return -1;
	}
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
}

bool wite_connection() {
	shmPTR->connected = 0;
	shmPTR->client_addr = 0;
	shmPTR->wifibuffer_data_len_4_read = 0;
	shmPTR->wifibuffer_data_len_4_write = 0;
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) {
		cout << "ERROR on accept\n";
		wifi_connections--;
		return true;
	}


	return false;
}


uint32_t wifiold_t = 0;




enum {REDY_FOR_READ_DATA=1,READY_4_WRITE_DATA=2};

void server() {
	//delay(5000);
	uint8_t len_buf_0_cnt = 0;
	new_server();
	if (wite_connection())
		return;
	while (flag==0 && shmPTR->run_main) {

		// bzero(buffer,256);
		const uint32_t t = millis();
		const uint32_t dt = t - wifiold_t;
		wifiold_t = t;

	//	if (dt < 33)
	//		delay(33 - dt);

		// if (dt > 35)
		//  printf("too long %i\n", dt);


		while (flag==0 && shmPTR->wifibuffer_data_len_4_read != 0)
			usleep(20000);
				
		int len = read(newsockfd, shmPTR->wifiRbuffer, TELEMETRY_BUF_SIZE);
		if (len <= 0) {
			//cout << "ra\n";
			len = read(newsockfd, shmPTR->wifiRbuffer, TELEMETRY_BUF_SIZE);
		}
		shmPTR->wifibuffer_data_len_4_read = len;

		if (len > 0) {
			len_buf_0_cnt = 0;
			if (shmPTR->connected == 0)
				shmPTR->client_addr = cli_addr.sin_addr.s_addr;
			shmPTR->connected++;
			if (shmPTR->connected == 1) {
				cout << "ONLINE\n";
				offline_time = 0;
			}
		}
		else {
			if (len_buf_0_cnt++ >= 2) {
				if (shmPTR->connected) {
					if (offline_time == 0)
						offline_time = millis();
					cout << "OFFLINE\n";//ERROR reading from socket\n";

				}
				if (wite_connection())
					return;
			}
			continue;
		}
		
		while (flag==0 && shmPTR->wifibuffer_data_len_4_write == 0)
			usleep(20000);

		
		n = write(newsockfd, shmPTR->wifiWbuffer, shmPTR->wifibuffer_data_len_4_write);
		if (n <= 0) {
			//cout << "wa\n";
			n = write(newsockfd, shmPTR->wifiWbuffer, shmPTR->wifibuffer_data_len_4_write);

		}
		if (n > 0) {
			
			shmPTR->wifibuffer_data_len_4_write = 0;
		}
		else{
			if (shmPTR->connected) {

				cout << "ERROR writing to socket\n";
				
			}
			if (wite_connection())
				return;
			
		}
	}
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::string exec(const std::string cmd) {
	//printf(cmd.c_str());
	//printf("\n");
	char buffer[128];
	std::string result = "";
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		//throw std::runtime_error("popen() failed!");
		cout << "inet pipe brock\n";
		return "";
	}
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	//cout << "close pipe\n";
	pclose(pipe);
	return result;
}


//"*  2coolzNET      Infra  11    54 Mbit/s  93      ▂▄▆█  WPA1 WPA2"
void get_signal_strong() {
	string ret = exec("nmcli dev wifi | grep 2coolzNET");
	if (ret.length() >= 20) {
		string strong = ret.substr(2 + ret.find("/s"), 5);
		int signal = atoi(strong.c_str());
		shmPTR->status = signal;
	}
}



//----------------------------------------------------------
void test_wifi() {
	int ln = 2;
	cout << "test wifi work\n";
	string ret = exec("ifconfig wlan0");
	int ip;
	string myIP = "";
	ip = ret.find("192.168.");
	if (ip >= 0)
		myIP = ret.substr(ip, 8 + ret.substr(ip + 8).find(".")) + ".1";
	if (myIP.length() > 0)
		ret = exec("ping -w 1 -c 1 " + myIP);
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
	else {
		ln = 2;
		cout << "test is OK\n";
	}
}
void watch_d() {
	static int cnt_wifi_strong = 0;
	static int errors = 0;
	shmPTR->wifi_run = true;
	uint old_main_cnt = shmPTR->main_cnt - 1;
	while (true) {
		if (shmPTR->wifi_run) {
			shmPTR->wifi_cnt++;
			if (shmPTR->main_cnt == old_main_cnt) {
				if (++errors >= 3) {
					flag = 1;
					cout << "main dont update cnt! EXIT\n";
					return;
				}
			}
			else {
				errors = 0;
				old_main_cnt = shmPTR->main_cnt;
			}

			if (offline_time && millis() - offline_time > 15000) {
				offline_time = millis();
				test_wifi();
			}

			delay(100);
			cnt_wifi_strong++;
			cnt_wifi_strong &= 15;
			if (cnt_wifi_strong == 1)
			{
				get_signal_strong();
			}
		}
		else {
			flag = 1;
			cout << "wifi: recived EXIT command\n";
			return;
		}
	}

}

std::ofstream out;
std::streambuf *coutbuf;// старый буфер

int main(int argc, char *argv[])
{
	init_shmPTR();

	int tmp = shmPTR->wifi_cnt;
	delay(1000);
	if (tmp != shmPTR->wifi_cnt) {
		cout << "wifi_clone\n";
		return -1;
	}
	
	thread tl(watch_d);
	tl.detach();
	delay(100);
	if (flag == 1)
		return -1;
	//if (shmPTR->run_main == false)
		//return 0;

	if (signal(SIGINT, handler) == SIG_ERR) {
		return EXIT_FAILURE;
	}
	if (signal(SIGPIPE, pipe_handler) == SIG_ERR) {
		return EXIT_FAILURE;
	}


	if (argc == 2) {
		out = std::ofstream(argv[1]); //откроем файл для вывод
		coutbuf = std::cout.rdbuf(); //запомним старый буфер
		std::cout.rdbuf(out.rdbuf()); //и теперь все будет в файл!
		std::cerr.rdbuf(out.rdbuf());
	}

	
	//cout << "  wifi started...\n";

	

	shmPTR->connected = 0;
	shmPTR->client_addr = 0;
	shmPTR->wifibuffer_data_len_4_read = 0;
	shmPTR->wifibuffer_data_len_4_write = 0;

	//cout << "server started...\n";
	delay(400);
	if (flag==0)
		server();


	shmdt((void *)shmPTR);
	cout << "   wifi exits...\n";
	out.close();
    return 0;
}




