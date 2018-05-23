#include <sys/types.h>
#include "WProgram.h"
#include "define.h"

#include <sys/shm.h>
#include <sys/ipc.h>

uint32_t start_seconds = 0;
uint32_t millis_g() {
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	uint32_t ret;
	
	ret = ((t.tv_sec ) * 1000) + (t.tv_nsec / 1000000);
	return ret;
}
uint32_t millis(){
	timespec t;
	clock_gettime(CLOCK_REALTIME,&t);
	uint32_t ret;
	if (start_seconds == 0)
		start_seconds = t.tv_sec;
	ret=((t.tv_sec-start_seconds)*1000)+(t.tv_nsec/1000000);
	return ret;
}



int64_t micros(void){
	timespec t;
	clock_gettime(CLOCK_REALTIME,&t);
	int64_t ret;
	if (start_seconds == 0)
		start_seconds = t.tv_sec;
	ret=((int64_t)(t.tv_sec-start_seconds)*1000000)+(t.tv_nsec/1000);
	return ret;
}



void delay(unsigned long t){
	usleep(t*1000);
}

std::string exec(const std::string cmd) {
	//printf(cmd.c_str());
	//printf("\n");
	char buffer[128];
	std::string result = "";
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		//throw std::runtime_error("popen() failed!");
		cout << "pipe brock\n";
		return "";
	}
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return result;
}

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

key_t          ShmKEY;
int            ShmID;
struct Memory *shmPTR;

int init_shmPTR() {
	if (shmPTR == 0) {


		ShmKEY = ftok(SHMKEY, 'x');
	ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
		if (ShmID < 0) {
			cout << "*** shmget error (server) ***\n";
			return 1;
		}
		shmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	}
	return 0;
}

void close_shmPTR() {
	cout << "Server has detected the completion of its child...\n";
	shmdt((void *)shmPTR);
	cout << "Server has detached its shared memory...\n";
	shmctl(ShmID, IPC_RMID, NULL);
	cout << "Server has removed its shared memory...\n";
	cout << "Server exits...\n";
}
