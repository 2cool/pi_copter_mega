#define PROG_VERSION "ver 2.170926_magic \n"

//#define ONLY_ONE_RUN


#include <cstdio>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include "Log.h"
//#include "KalmanFilterVector.h"
#include "Filter.h"

#include "Filter.h"
#include "define.h"
#include "debug.h"



#include "WProgram.h"
#include "Settings.h"
#include "Prog.h"
#include "Location.h"

#include "GPS.h"
#include "Telemetry.h"
#include "commander.h"
#include "Wi_Fi.h"

#include "Autopilot.h"

#include "mi2c.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Balance.h"
#include "SIM800.h"

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








int semid;
bool is_clone(char *argv0) {
	
	struct sembuf my_sembuf;
	key_t IPC_key = ftok(argv0, 0);
	if (0 > IPC_key)
	{
		printf("Can\'t generate IPC key\n");
		std::cout << argv0 << std::endl;
		std::cout << IPC_key << std::endl;
		
	}
	semid = semget(IPC_key, 1, 0666 | IPC_CREAT);
	if (0 > semid)
	{
		printf("Can\'t get semid\n");
	}
	my_sembuf.sem_op = 1;
	my_sembuf.sem_flg = 0;
	my_sembuf.sem_num = 0;
	if (0 > semop(semid, &my_sembuf, 1))
	{
		printf("Error increment semaphore!\n");
	}
	int sem_value = semctl(semid, 0, GETVAL, 0);
	if (-1 != sem_value)
	{
		//std::cout << sem_value << std::endl;
	}
	else
	{
		std::cout << " Error get senafor value! " << std::endl;
	}
	if (sem_value > 1)
	{
		//std::cout << " Error! 2 COPY! " << std::endl;
		return true;
	}

	
	return false;
}




int setup(int cnt) {////--------------------------------------------- SETUP ------------------------------
	Log.init(cnt);
	

	Settings.read();


	fprintf(Debug.out_stream,"___setup___\n");
	

#ifdef WORK_WITH_WIFI
	fprintf(Debug.out_stream,"wifi init...\n");
	if (WiFi.init())
		return -1;
#endif

	fprintf(Debug.out_stream,"commander init...\n");
	Commander.init();
	fprintf(Debug.out_stream,"Autopilot init...\n");
	Autopilot.init();
	Telemetry.init_();
	Telemetry.testBatteryVoltage();
	fprintf(Debug.out_stream,"telemetry init OK \n");
	//mega_i2c.beep_code(BEEPS_ON);
	GPS.init();

	//sim.start();

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
				fprintf(Debug.out_stream,"%i\n",maxTT); \
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
#endif
		Commander.input();
		Autopilot.loop();
		mega_i2c.gsm_loop();
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

int printHelp() {
	printf("<-help> for this help\n");
	printf(" <fly at start at hight in sm > <lower hight in sm> <f=write stdout to file > <log com and tel y/n> \n");
	printf("example to write in log file : pi_copter 300 100 f n \n");
	printf("example to write in stdout   : pi_copter 300 100 s n\n");
	return -1;
}




int main(int argc, char *argv[]) {
	string fname;
	//0xbefffcf7 "/home/igor/projects/pi_copterVS/bin/ARM/Debug/pi_copterVS.out"




	printf(PROG_VERSION);

#ifdef ONLY_ONE_RUN
	if (is_clone(argv[0]) == true) {
		printf("clone\n");
		if (-1 == semctl(semid, 0, IPC_RMID, 0))
		{
			printf("Error delete!\n");
		}
		return 0;
	}
#endif	
	
	Debug.fly_at_start = 3;
	Debug.lowest_altitude_to_fly = 1.6f;
	Debug.n_debug = 0;
	int counter = 0;

	if (argc >= 2) {
		int tt = string(argv[1]).compare("-help");
		if (tt==0) {

		
			return printHelp();

		}

		if (argc >= 6) {
			int t = atoi(argv[1]);
			t = constrain(t, 300, 300);/////
			Debug.fly_at_start = 0.01f*(float)t;

			t=atoi(argv[2]);
			Debug.lowest_altitude_to_fly = 0.01f*(float)t;
			if (Debug.lowest_altitude_to_fly > Debug.fly_at_start)
				Debug.lowest_altitude_to_fly = Debug.fly_at_start;
#define LOG_COUNTER_NAME "/home/igor/logs/logCounter.txt"
			
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
			else
				return 0;
			
			set = fopen(LOG_COUNTER_NAME, "w+");
			fprintf(set, "%i\n", counter + 1);
			fclose(set);
			if (argv[3][0] == 'f' || argv[3][0] == 'F') {
				

				ostringstream convert;
				convert << "/home/igor/logs/log_out" << counter << ".txt";
				fname = convert.str();

				Debug.out_stream = fopen(fname.c_str(), "w+");
			}else	
				Debug.out_stream = stdout;

			
			
			

		}
		
	}
	else
		return printHelp();

	fprintf(Debug.out_stream, PROG_VERSION);
	fprintf(Debug.out_stream, "picopter par: %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4]);

	if (signal(SIGINT, handler) == SIG_ERR) {
		return EXIT_FAILURE;
	}


	mega_i2c.init();
	string str = string(argv[0]);
	str = str.substr(str.length() - 4, str.length());


	if (str.compare("pter") == 0) {
		
	}

	if (setup(counter) == 0) {
		

		old_time4loop = micros();

		Debug.run_main = true;
		Debug.reboot = false;
		while (Debug.run_main && flag == 0) {

			if (loop()) {
				//usleep(5400);
			//	int ttt = micros();
			//	dfr += ((1000000 / (ttt - old_time4loop)) - dfr)*0.01;
			//	Debug.load(0, dfr, 0);
			//	old_time4loop = ttt;
				int64_t t = micros();
				int32_t time_past = (int32_t)(t - old_time4loop);
				old_time4loop = t;
				//if (time_past > 15000)
				//	fprintf(Debug.out_stream,"too long %i\n",time_past);

				//Debug.load(0, time_past, 0);
				//Debug.dump();
			}

		}
	}
	
	WiFi.stopServer();
	if (Debug.run_main==false)
		fprintf(Debug.out_stream, "\n exit\n");
	if (flag!=0)
		fprintf(Debug.out_stream, "\n main Signal caught!\n");
	Settings.write();

	fflush(Debug.out_stream);
	fclose(Debug.out_stream);
	Log.close();
	usleep(3000000);

	system(Debug.reboot?"reboot":"shutdown now");


#ifdef ONLY_ONE_RUN

	if (-1 == semctl(semid, 0, IPC_RMID, 0))
	{
		printf("Error delete!\n");
	}
#endif
	return 0;

}
