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
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 

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
#include <fstream>
#include <arpa/inet.h>
using namespace std;

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
	pclose(pipe);
	return result;
}



//#define FORTEST
#ifdef FORTEST
#include "C:/Users/Igor/pi_copter_mega/pi_copterVS/pi_copterVS/glob_header.h"
//#include "C:/Users/2coolz/pi_copter_mega/pi_copterVS/pi_copterVS/glob_header.h"

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




bool is_connected(void) { return shmPTR->connected > 0; }


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


string wlan_fpv = "wlx7cdd901e13d5";



void video_stream() {
	while (true) {
		//ffmpeg - rtsp_transport udp - i "rtsp://192.168.42.1:554/live" - c copy - f h264 udp ://android_phone_address:5544
		delay(6000);
		if (shmPTR->fpv_adr == 0)
			continue;

#define uchar unsigned char
		int iadr = shmPTR->client_addr;
		if (iadr == 0)
			return;
		string adr = to_string((uchar)(iadr & 255)) + "." + to_string((uchar)(255 & (iadr >> 8))) + "." + to_string((uchar)(255 & (iadr >> 16))) + "." + to_string((uchar)(iadr >> 24));



		int last_dot = adr.find_last_of(".");
		adr = adr.substr(0, last_dot + 1) += std::to_string(shmPTR->fpv_adr);
		//printf("%s\n", adr.c_str());
		string ret = exec("ping -c 1 " + adr);
		if (ret.find("1 received") != string::npos) {
			ret = exec("ping -c 1 192.168.42.1");
			if (ret.find("1 received") != string::npos) {
				//printf( "try stream to %s\n", adr.c_str());
				cout << "try stream to " << adr << endl;

				string s = "ffmpeg -rtsp_transport udp -i \"rtsp://192.168.42.1:554/live\" -c copy -f h264 udp://" + adr + ":554 > /dev/null 2>&1";
				system(s.c_str());
				cout << "stream stoped\n";
			}
		}

		/*
		ffmpeg -rtsp_transport udp -i "rtsp://192.168.42.1:554/live" -c copy -f h264 udp://192.168.0.104:554

		*/

		//printf( "%s\n", ret.c_str());

	}
}


struct sockaddr_in address;
int sock = 0, valread;
struct sockaddr_in serv_addr;
 
int open_socket() {



	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout<<"\n Socket creation error \n";
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(7878);

	// Convert IPv4 and IPv6 addresses from text to binary form 
	if (inet_pton(AF_INET, "192.168.42.1", &serv_addr.sin_addr) <= 0)
	{
		cout<<"\nInvalid address/ Address not supported \n";
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		cout<<"\nConnection Failed \n";
		return -1;
	}
	return 0;
}

int camera_video_stream(){
	//-------------------------------------------------------
	string tosend = "{\"msg_id\":257,\"token\":0}";
	char buffer[1024] = { 0 };
	int cnt = 0;
	do {
		send(sock, tosend.c_str(), strlen(tosend.c_str()), 0);
		valread = read(sock, buffer, 1024);
		string str = buffer;
		int rval_i = str.find("rval");
		if (rval_i >= 0) {
			cout << str << endl;
			int beg = 9 + str.find("\"param\": ");
			int len = str.substr(beg).find(" }");
			string stoken = str.substr(beg, len);
			int token = stoi(stoken);
			//tosend = '{"msg_id":259,"token":%s,"param":"none_force"}' %token
			tosend = "{\"msg_id\":259,\"token\":" + stoken +",\"param\":\"none_force\"";
			send(sock, tosend.c_str(), strlen(tosend.c_str()), 0);
			read(sock, buffer, 1024);
			cout << "Live webcam stream is now available.\n";
			return 0;
		}
	} while (cnt++ < 3 );
	return -1;
	

	





	
	printf("%s\n", buffer);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////


int zoom;

int get_pid(const char* name) {

	FILE *in;
	char buff[512];

	if (!(in = popen("ps -e", "r"))) {
		return 1;
	}

	while (fgets(buff, sizeof(buff), in) != NULL) {
		//	cout << buff;
		string s = string(buff);
		if (s.find(name) != -1) {
			cout << s;
			int pid = stoi(s.substr(0, 5));
			fclose(in);
			return pid;

		}
	}
	pclose(in);
	return -1;
}
void stop_ffmpeg_stream() {
	system("pkill ffmpeg");
	

}
string intIP2strIP(uint32_t ip) {

	string sip = to_string(ip & 255) + "." + to_string((ip >> 8) & 255) + "." + to_string((ip >> 16) & 255)+"."+ to_string(ip >> 24);
	return sip;
}







int main()
{
	//wlx7cdd901e13d5



	init_shmPTR();
	int old_main_cnt = shmPTR->main_cnt;
	
	while (true) {
		while (shmPTR->fpv_zoom == 0) {
			delay(200);
			if (shmPTR->main_cnt == old_main_cnt) 
				return 0;
			old_main_cnt = shmPTR->main_cnt;
		}
		
		//zoom = shmPTR->fpv_zoom;



		//open_socket();

		//camera_video_stream();
		uint32_t ip = (shmPTR->client_addr & 0x00ffffff) | shmPTR->fpv_adr<<24;
	//	string s = "ffmpeg -rtsp_transport udp -i rtsp://192.168.42.1:554/live -c copy -f h264 udp://192.168.1.102:5544";// > / dev / null 2 > & 1";
		string s = "ffmpeg -rtsp_transport udp -i \"rtsp://192.168.42.1:554/live\" -c copy -f h264 udp://" + intIP2strIP(ip) + ":" + to_string(shmPTR->fpv_port) + " > /dev/null 2>&1";
		cout << s << endl;
		//system(s.c_str());

		while (shmPTR->fpv_zoom > 0) {
			delay(200);
			if (shmPTR->main_cnt == old_main_cnt) {
				stop_ffmpeg_stream();
				return 0;
			}
			old_main_cnt = shmPTR->main_cnt;
		}

		stop_ffmpeg_stream();
	}

  
    return 0;
}