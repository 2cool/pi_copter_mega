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
#include "../../../../repos/pi_copter_mega/pi_copterVS/pi_copterVS/glob_header.h"

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

bool stopServer();

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

void error(const char *msg)
{
	perror(msg);
	exit(1);
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
		cout << "ERROR opening socket/n";
		wifi_connections--;
		return -1;
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = 9876;
	serv_addr.sin_family = AF_INET;

	serv_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(adr.c_str());
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		cout << "ERROR on binding/n";
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

bool run = true;
bool stopServer() {
	run = false;
	mclose();

}
uint32_t wifiold_t = 0;




enum {REDY_FOR_READ_DATA=1,READY_4_WRITE_DATA=2};

void server() {
	//delay(5000);
	
	new_server();
	if (wite_connection())
		return;
	while (flag==0 && shmPTR->run_main) {

		// bzero(buffer,256);
		const uint32_t t = millis();
		const uint32_t dt = t - wifiold_t;
		wifiold_t = t;

		if (dt < 33)
			delay(33 - dt);

		// if (dt > 35)
		//  printf("too long %i\n", dt);


		while (shmPTR->wifibuffer_data_len_4_read != 0)
			usleep(10000);
				
		shmPTR->wifibuffer_data_len_4_read = read(newsockfd, shmPTR->wifiRbuffer, TELEMETRY_BUF_SIZE);

		if (shmPTR->wifibuffer_data_len_4_read > 0) {
			if (shmPTR->connected == 0)
				shmPTR->client_addr = cli_addr.sin_addr.s_addr;
			shmPTR->connected++;
		}
		else {
			if (shmPTR->connected) {

				cout << "ERROR reading from socket\n";
				
			}
			if (wite_connection())
				return;
			
			continue;
		}
		
		while (shmPTR->wifibuffer_data_len_4_write == 0)
			usleep(10000);

		
		n = write(newsockfd, shmPTR->wifiWbuffer, shmPTR->wifibuffer_data_len_4_write);
		if (n > 0) {
			
			shmPTR->wifibuffer_data_len_4_write = 0;
		}
		else{
			if (shmPTR->connected) {

				cout << "ERROR reading from socket\n";
				
			}
			if (wite_connection())
				return;
			
		}
	}
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void watch_d() {

	while (true) {
		shmPTR->wifi_cnt++;
		delay(100);
	}
}

std::ofstream out;
std::streambuf *coutbuf;// ������ �����

int main(int argc, char *argv[])
{
	init_shmPTR();
	if (shmPTR->run_main == false)
		return 0;
	thread tl(watch_d);
	tl.detach();


	if (signal(SIGINT, handler) == SIG_ERR) {
		return EXIT_FAILURE;
	}
	if (signal(SIGPIPE, pipe_handler) == SIG_ERR) {
		return EXIT_FAILURE;
	}


	if (argc == 2) {
		out = std::ofstream(argv[1]); //������� ���� ��� �����
		coutbuf = std::cout.rdbuf(); //�������� ������ �����
		std::cout.rdbuf(out.rdbuf()); //� ������ ��� ����� � ����!
		std::cerr.rdbuf(out.rdbuf());
	}

	
	//cout << "  wifi started...\n";

	

	shmPTR->connected = 0;
	shmPTR->client_addr = 0;
	shmPTR->wifibuffer_data_len_4_read = 0;
	shmPTR->wifibuffer_data_len_4_write = 0;

	//cout << "server started...\n";
	server();


	shmdt((void *)shmPTR);
	cout << "   wifi exits...\n";
	out.close();
    return 0;
}




