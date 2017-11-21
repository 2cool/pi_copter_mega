
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>

#include "debug.h"
#include "define.h"
#include "mi2c.h"
#include "mpu.h"

#define pwm_MAX_THROTTLE 32000
#define pwm_OFF_THROTTLE 16000

#define  ARDUINO_ADDR 9

int fd, fd_in;

int Megai2c::getsim(char * str) {
	char reg = 3;
	char sim_count;
	write(fd, &reg, 1);
	int ret = read(fd, &sim_count, 1);
	int res = 0;
	if (ret == 1 && sim_count > 0) {
		res = read(fd, str, sim_count);
		//for (int i = 0; i < sim_count; i++)
		//	printf("%c", str[i+ shift]);
	}
	return res;
}

char gsm_send_buf[32];

int Megai2c::send2sim(char *str, int len) {
	gsm_send_buf[0] = 2;
	memcpy(gsm_send_buf + 1, str, len);
	//usleep(50);
	write(fd, gsm_send_buf, len + 1);
	//	for (int i = 1; i<len+1; i++)
	//	printf("%c", gsm_send_buf[i]);
}

int Megai2c::gsm_loop()
{
	char gsm_in_buf[18];
	int a_in;
	ioctl(fd_in, FIONREAD, &a_in);
	if (a_in) {
		//printf("----\n");
		if (a_in > 16)
			a_in = 16;

		int av = read(fd_in, &gsm_in_buf, a_in);
		send2sim(gsm_in_buf, a_in);
	}
	int res = getsim(gsm_in_buf);
	if (res) {
		write(fd_in, gsm_in_buf, res);
	}

	return 0;

}

int Megai2c::init()
{
	if ((fd = open("/dev/i2c-0", O_RDWR)) < 0) {
		fprintf(Debug.out_stream, "Failed to open /dev/i2c-0\n");
		return -1;
	}
	if (ioctl(fd, I2C_SLAVE, ARDUINO_ADDR) < 0) {
		fprintf(Debug.out_stream, "Failed to acquire /dev/i2c-0 access and/or talk to slave.\n");
		return -1;
	}

	fd_in = open("/dev/tnt1", O_RDWR | O_NOCTTY | O_SYNC);
	if (fd_in < 0)
	{
		printf("error %d opening /dev/tnt0: %s", errno, strerror(errno));
		return -1;
	}



	return 0;


}

void Megai2c::beep_code(uint8_t c) {
	char chBuf[] = { 1,c };
	write(fd, chBuf, 2);
}



void Megai2c::Buzzer(const bool on) {

}


uint16_t Megai2c::correct(const float n) {    //0-���

	return (uint16_t)(pwm_OFF_THROTTLE + (uint16_t)(n*pwm_OFF_THROTTLE));

}

void Megai2c::throttle(const float n0, const float n1, const float n2, const float n3) {
	uint16_t pwm_out[5];
	pwm_out[0] = 0;
	pwm_out[1] = pwm_OFF_THROTTLE + n0*pwm_OFF_THROTTLE;
	pwm_out[2] = pwm_OFF_THROTTLE + n1*pwm_OFF_THROTTLE;
	pwm_out[3] = pwm_OFF_THROTTLE + n2*pwm_OFF_THROTTLE;
	pwm_out[4] = pwm_OFF_THROTTLE + n3*pwm_OFF_THROTTLE;
	char *chBuf = (char*)pwm_out;
	write(fd, chBuf + 1, 9);
}


//0.35555555555555555555555555555556 = 1����
bool Megai2c::gimagl(float pitch, float roll) {  // �������� ������� ������� � �������� ��� ����� ����
	if (pitch <= 90 && pitch >= -45) {
		//Serial.fprintf(Debug.out_stream,"camAng="); Serial.println(angle);
		pitch = pwm_OFF_THROTTLE + (180 - pitch)*44.444444;
		roll = pwm_OFF_THROTTLE + (180 + roll)*44.4444;
		char buf[6];

		buf[1] = 8;
		((uint16_t*)buf)[1] = (uint16_t)(pitch);
		((uint16_t*)buf)[2] = (uint16_t)roll;
		write(fd, buf + 1, 5);
		return true;
	}
	else
		return false;

}



int Megai2c::get_gps(SEND_I2C *gps_d) {

	char reg = 1;
	char gps_count;
	write(fd, &reg, 1);
	int res = read(fd, &gps_count, 1);
	if (res>0 && gps_count == 14) {
		res = read(fd, (char*)gps_d, gps_count);
		return res;
	}
	else {
		return 0;
	}

}

void Megai2c::getiiiiv(char *iiiiv) {
	char reg = 0;
	write(fd, &reg, 1);
	read(fd, (char*)iiiiv, 10);
}


void Megai2c::sound(const float f) {
#ifdef BUZZER_R
	//OCR4B = 32000-(int16_t)f; 
#endif

}

Megai2c mega_i2c;

