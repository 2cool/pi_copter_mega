#include <sys/types.h>
#include "WProgram.h"



uint32_t start_seconds = 0;

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
		printf("pipe brock\n");
		return "";
	}
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
		pclose(pipe);
	}
	catch (...) {
		pclose(pipe);
		printf("pipe brock\n");
		//throw;
		return "";
	}
	
	
	return result;
}