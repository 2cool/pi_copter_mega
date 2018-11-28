//убрать нафиг ограничения на висоту спуска
//поставить мпу на старое место под орандж пи
//разобратся с переменкой на двигателях.

//сравнить показания барометрами  с показаниями на старой конфигурации. походу старая конфигурация била куда более гладкой может из за єтого вся хуита
//также напомню что надо перекомпилить ардуину єту и инет

/*

Виставить максимальние показатели
Some boards allow to adjust CPU speed.

nano /etc/default/cpufrequtils
Alter min_speed or max_speed variable.

service cpufrequtils restart








*/

#define PROG_VERSION "ver 3.180718\n"


#define SIM800_F

//при стартре замерять вибрацию после чего делать корекцию или вообще запрещать полет при сильной вибрации

#include <sys/sem.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>

#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include <cstdio>
#include <signal.h>
#include  <stdio.h>
#include  <stdlib.h>
#include <sys/types.h>

#include <stdio.h>

#include "define.h"

#include "Log.h"
//#include "KalmanFilterVector.h"
#include "Filter.h"


#include "debug.h"



#include "WProgram.h"/// not change
#include "Settings.h"
#include "Prog.h"
#include "Location.h"

#include "GPS.h"
#include "Telemetry.h"
#include "commander.h"


#include "Autopilot.h"

#include "mi2c.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Balance.h"


bool loop();



// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.


// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO

//#include <avr/io.h> //  can be omitted
//#include <avr/interrupt.h> // can be omitted


int zzz = 1;







void video_stream1() {
#ifdef SKSKSKSKSKSK
	while (true) {
		//ffmpeg - rtsp_transport udp - i "rtsp://192.168.42.1:554/live" - c copy - f h264 udp ://android_phone_address:5544
		delay(6000);
		if (Commander.vedeo_stream_client_addr == 0)
			continue;

#define uchar unsigned char
		int iadr = shmPTR->client_addr;
		if (iadr == 0)
			return;
		string adr = to_string((uchar)(iadr & 255)) + "." + to_string((uchar)(255 & (iadr >> 8))) + "." + to_string((uchar)(255 & (iadr >> 16))) + "." + to_string((uchar)(iadr >> 24));

		

		int last_dot = adr.find_last_of(".");
		adr = adr.substr(0, last_dot + 1) += std::to_string(Commander.vedeo_stream_client_addr);
		//printf("%s\n", adr.c_str());
		string ret = exec("ping -c 1 " + adr);
		if (ret.find("1 received") != string::npos) {
			ret = exec("ping -c 1 192.168.42.1");
			if (ret.find("1 received") != string::npos) {
				//printf( "try stream to %s\n", adr.c_str());
				cout << "try stream to " << adr << endl;
				
				string s = "ffmpeg -rtsp_transport udp -i \"rtsp://192.168.42.1:554/live\" -c copy -f h264 udp://" + adr + ":554 > /dev/null 2>&1";
				system(s.c_str());
				cout<< "stream stoped\n";
			}
		}

		/*
		ffmpeg -rtsp_transport udp -i "rtsp://192.168.42.1:554/live" -c copy -f h264 udp://192.168.0.104:554

		*/

		//printf( "%s\n", ret.c_str());
			
	}
#endif
}


int setup(int cnt) {////--------------------------------------------- SETUP ------------------------------
	Log.init(cnt);
	

	Settings.read();


	cout << "___setup___\n";

#ifdef WORK_WITH_WIFI
//	printf("wifi init...\n");
//	if (WiFi.init())
//		return -1;
#endif

	cout << "commander init...\n";
	Commander.init();
	cout << "Autopilot init...\n";
	Autopilot.init();
	Telemetry.init_();
	Telemetry.testBatteryVoltage();
	cout << "telemetry init OK \n";

	GPS.init();

#ifdef SIM800_F

#endif
	return 0;

}


uint8_t teil = 0, maxTeilN = 3;



uint64_t tt, maxTT = 0; \
void print_time() {
	\
		uint64_t d = micros() - tt; \
		if (d > maxTT) {
			\
				maxTT = d; \
				cout << maxTT<<endl; \
		}\
}
#ifndef WORK_WITH_WIFI
bool foo_flag = false;
uint32_t ttiimer = 0;
#endif


int cccccc_ss = 0;

uint64_t old_time4loop;
bool temp4test = true;


int ok_cnt = 0;
int ok_ccc = 0;
int er_cnt = 0;
long dt_sum=0;
int max_dt = 0;
int old_debug = 0;
bool loop()
{
	usleep(5000);
#ifndef WORK_WITH_WIFI
	if (temp4test && Autopilot.motors_is_on() == false && millis() > 2000) {
		//Autopilot.motors_do_on(true, "FALSE WIFI");
		//Mpu.new_calibration(false);
		temp4test = false;
	}


#endif


	if (Balance.loop()) {
		

#ifdef WORK_WITH_WIFI
		Telemetry.loop();
		Mpu.telem_timed = 0.000001*(double)micros();
#endif
		Commander.input();
		Mpu.com_timed = 0.000001*(double)micros();
		Autopilot.loop();
		Mpu.autopilot_timed = 0.000001*(double)micros();
		//mega_i2c.gsm_loop();
		if (shmPTR->sim800_reset_time > 0 && shmPTR->sim800_reset_time + 40000 < millis())
			shmPTR->sim800_reset_time = 0;

#ifdef FALSE_WIRE
		usleep(3000);
#endif
		return true;
	}
	else
		return false;

	
}

//http://www.cprogramming.com/tutorial/lesson10.html
volatile sig_atomic_t flag = 0;
void handler(int sig) { // can be called asynchronously
	flag = 1; // set flag
}
void pipe_handler(int sig) {
	cout << "pipe error" << "\t"<<Mpu.timed << endl;
}




int printHelp() {
	cout << "<-help> for this help\n";
	cout << " <fly at start at hight in sm > <lower hight in sm> <f=write stdout to file > <log com and tel y> <start wifi> <start sms> <start log> <start telegram>\n";
	cout << "example to write in log file : pi_copter 300 100 f n y y y y\n";
	cout << "example to write in stdout   : pi_copter 300 100 s n y y y y\n";
	return -1;
}
double last_wifi_reloaded = 0;
string stdout_file_ext = "";
int inet_start_cnt = 0, wifi_start_cnt = 0;

bool start_wifi = false, start_inet = false, start_loger = false, start_telegram = false;;
void watch_dog() {
	delay(3000);
	while (shmPTR->run_main) { 
		
		uint8_t wifi_cnt = shmPTR->wifi_cnt;
		uint8_t internet_cnt = shmPTR->internet_cnt;
		uint8_t fpv_cnt = shmPTR->fpv_cnt;
		delay(2000);
		

		system("/root/projects/fpv &");
		if (fpv_cnt == shmPTR->fpv_cnt  && Mpu.timed>15) {
			system("pkill fpv");
			system("/root/projects/fpv &");
		}
		if (start_wifi)
			if (wifi_cnt == shmPTR->wifi_cnt  || ( Mpu.timed - Autopilot.last_time_data_recivedd > 5 && Mpu.timed - last_wifi_reloaded > 30)) {
				last_wifi_reloaded = Mpu.timed;
				cout << "--------------wifi killed:\t"<<Mpu.timed<<endl;
				system("pkill wifi_p");
				delay(1000);
				cout << "--------------wifi started:\t" << Mpu.timed << endl;;
				string t = "/root/projects/wifi_p ";
				//if (stdout_file_ext.length()) {
			//		t += stdout_file_ext + "wifi_"+to_string(wifi_start_cnt++);
				//}
				t += " &";
				int ret=system(t.c_str());

			}
		if (start_inet)
			if (internet_cnt == shmPTR->internet_cnt) {

				cout << "--------------ppp starting" << "\t"<<Mpu.timed << endl;
				system("pkill ppp_p");
				string t = "/root/projects/ppp_p ";
				if (start_loger)
					t += "y";
				else
					t += "n";
				t += " ";
				if (start_telegram)
					t += "y";
				else
					t += "n";
				t+=" ";
				if (stdout_file_ext.length()) {
					t += stdout_file_ext + "i"+to_string(inet_start_cnt++)+".txt";
				}
				t += " &";
				int ret = system(t.c_str());
			
			}
	}
}
std::ofstream out;
std::streambuf *coutbuf;// старый буфер




int main(int argc, char *argv[]) {

cout << PROG_VERSION << endl;


	if (init_shmPTR())
		return 0;

	{
		uint8_t temp = shmPTR->main_cnt;

		usleep(51123);
		if (temp != shmPTR->main_cnt) {
			cout << "clone\n";
			return 0;
		}
	}

	init_millis_micros();
	if (millis() > 31000)
		return 0;

	shmPTR->in_fly = (shmPTR->control_bits&MOTORS_ON);
	shmPTR->wifi_cnt = 0;
	shmPTR->run_main = true;
	shmPTR->inet_ok = false;
	shmPTR->fpv_zoom = 0;
	//shmPTR->internet_run = false;
	thread tl(watch_dog);
	tl.detach();

	string fname;
	

		shmPTR->connected = 0;
		shmPTR->fly_at_start = 3;
		shmPTR->lowest_altitude_to_fly = 1.6f;
		Debug.n_debug = 0;

		int counter = 0;

		if (argc >= 2) {
			int tt = string(argv[1]).compare("-help");
			if (tt == 0) {
				return printHelp();
			}
			if (argc >= 9) {
				int t = atoi(argv[1]);
				t = constrain(t, 300, 300);/////
				shmPTR->fly_at_start = 0.01f*(float)t;

				t = atoi(argv[2]);
				shmPTR->lowest_altitude_to_fly = 0.01f*(float)t;
				if (shmPTR->lowest_altitude_to_fly > shmPTR->fly_at_start)
					shmPTR->lowest_altitude_to_fly = shmPTR->fly_at_start;
#define LOG_COUNTER_NAME "/home/igor/logs/logCounter"

				FILE *set = fopen(LOG_COUNTER_NAME, "r");
				if (set) {
					fscanf(set, "%i", &counter);

					fclose(set);
					usleep(500);
					if (counter < 9999)
					{
						FILE *in;
						char buff[512];

						if (!(in = popen("ls /home/igor/logs", "r"))) {
							return 1;
						}
						counter = 10000;
						while (fgets(buff, sizeof(buff), in) != NULL) {
							string s = string(buff);
							int b = s.find_first_of("0123456789");
							int e = s.find_first_of(".");
							if (b > 0 && e + 4 > b) {
								int cnt = stoi(s.substr(b, e));
								if (cnt > counter)
									counter = cnt;
							}
						}
						fclose(in);
					}
					remove(LOG_COUNTER_NAME);
				}
				else {
					cout << "no counter file";
					return 0;
				}

				set = fopen(LOG_COUNTER_NAME, "w+");
				fprintf(set, "%i\n", counter + 1);
				fclose(set);
				if (argv[3][0] == 'f' || argv[3][0] == 'F') {


					stdout_file_ext = "/home/igor/logs/log_out" + to_string(counter);
					fname = stdout_file_ext+".txt";


					out = std::ofstream(fname.c_str()); //откроем файл для вывод
					coutbuf = std::cout.rdbuf(); //запомним старый буфер
					std::cout.rdbuf(out.rdbuf()); //и теперь все будет в файл!
				}
				Log.writeTelemetry = (argv[4][0] == 'y' || argv[4][0] == 'Y');
				start_wifi = (argv[5][0] == 'y' || argv[5][0] == 'Y');
				start_inet = (argv[6][0] == 'y' || argv[6][0] == 'Y');
				start_inet |= start_loger=(argv[7][0] == 'y' || argv[7][0] == 'Y');
				start_inet |= start_telegram=(argv[8][0] == 'y' || argv[8][0] == 'Y');
			}

		}
		else
			return printHelp();

		cout << PROG_VERSION << endl;
		cout << "picopter par: " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
	
	if (signal(SIGINT, handler) == SIG_ERR) {
		return EXIT_FAILURE;
	}
	if (signal(SIGPIPE, pipe_handler) == SIG_ERR) {
		return EXIT_FAILURE;
	}

	mega_i2c.init();
	string str = string(argv[0]);
	str = str.substr(str.length() - 4, str.length());


	if (str.compare("pter") == 0) {
		
	}

	if (setup(counter) == 0) {
		

		old_time4loop = micros();

		
		shmPTR->reboot = 0;


		//video_stream();
		//thread t(video_stream);
		//t.detach();



		static uint32_t ppp_delay=0;
		while (shmPTR->run_main){
			if (loop()) {
				shmPTR->main_cnt++;
				//usleep(5400);
			//	int ttt = micros();
			//	dfr += ((1000000 / (ttt - old_time4loop)) - dfr)*0.01;
			//	Debug.load(0, dfr, 0);
			//	old_time4loop = ttt;
				int64_t t = micros();
				int32_t time_past = (int32_t)(t - old_time4loop);
				old_time4loop = t;
				//if (time_past > 15000)
				//	printf("too long %i\n",time_past);

				//Debug.load(0, time_past, 0);
				//Debug.dump();
			}
			if (flag)
				shmPTR->run_main = false;
		}
	}


	if (flag!=0)
		cout<< "\n main Signal caught!" << "\t"<<Mpu.timed << endl;
	//WiFi.stopServer();
	Settings.write();
	Log.close();
	sleep(3);

	if (shmPTR->run_main==false)
		cout<< "\n exit\n";

	
	if (shmPTR->reboot) {
		switch (shmPTR->reboot) {
		case 1:
			if (string(argv[0]).find("out") == -1)
				system("reboot");
			break;
		case 2:
			system("shutdown now");
			break;

		}
	}
	
	//close_shmPTR();
	out.close();

#ifdef ONLY_ONE_RUN

	if (-1 == semctl(semid, 0, IPC_RMID, 0))
	{
		printf("Error delete!\n");
	}
#endif
	return 0;

}

