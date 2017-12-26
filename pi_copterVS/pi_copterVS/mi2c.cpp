
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>

#include "debug.h"
#include "define.h"
#include "mi2c.h"
#include "mpu.h"
#include "SIM800.h"

#define pwm_MAX_THROTTLE 32000
#define pwm_OFF_THROTTLE 16000

#define  ARDUINO_ADDR 9
#define  DO_SOUND 1

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

int Megai2c::send2sim(const char *str, int len) {
	gsm_send_buf[0] = 2;
	memcpy(gsm_send_buf + 1, str, len);
	//usleep(50);
	write(fd, gsm_send_buf, len + 1);
	//	for (int i = 1; i<len+1; i++)
	//	printf("%c", gsm_send_buf[i]);
}

	/*
	+CMTI: "SM",1
	+CMTI: "SM",2
	...

	+CMTI: "ME",21
	приходят смски

	RING

	NO CARRIER
	*/
void Megai2c::m_parser(char *str, int len) {
const static char no_carrier[] = "NO CARRIER";
const static char ring[] = "RING";
const static char sms[] = "+CMTI: \"SM\",";
static int smsi = 0, ringi=0, no_carrieri=0;
static int smsN = 0;
static int sms_received = 0;
	for (int i = 0; i < len; i++) {


		if (sms_received == -1) {
			if (str[i] >= '0' && str[i] <= '9') {
				smsN = smsN * 10 + str[i] - '0';
			}
			else {
				sms_received = smsN;
				fprintf(Debug.out_stream, "SMS %i\n", sms_received);
				smsN ^= smsN;
				sim.readSMS(sms_received, true, true);
			}
		}
		else {
			if (str[i] == sms[smsi++]) {
				if (smsi == sizeof(sms) - 1) {
					sms_received = -1;
					smsi ^= smsi;
				}
			}
			else
				smsi ^= smsi;

			if (ring_received == false) {
				if (str[i] == ring[ringi++]) {
					if (ringi == sizeof(ring) - 1) {
						ring_received = true;
						fprintf(Debug.out_stream, "RING\n");
						ringi ^= ringi;
					}
				}
				else
					ringi ^= ringi;
			}
			else {
				if (str[i] == no_carrier[no_carrieri++]) {
					if (no_carrieri == sizeof(no_carrier) - 1) {
						ring_received = false;
						fprintf(Debug.out_stream, "NO CARRIER\n");
						no_carrieri ^= no_carrieri;
					}
				}
				else
					no_carrieri ^= no_carrieri;
			}
		}
	}
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
/*
		printf("<-");
		for (int i = 0; i < av; i++)
			printf("%c (%i) ", (char)gsm_in_buf[i],(int)gsm_in_buf[i]);
		printf("\n");
	*/	
		send2sim(gsm_in_buf, a_in);
	}
	int res = getsim(gsm_in_buf);


	

	if (res) {
		//if no ppp
		m_parser(gsm_in_buf, res);
		/*
		printf("->");
		for (int i = 0; i < res; i++)
			printf("%c (%i) ", (char)gsm_in_buf[i], (int)gsm_in_buf[i]);
		printf("\n");
		*/
		write(fd_in, gsm_in_buf, res);
	}

	return 0;

}


bool Megai2c::ppp(bool f) {
	ppp_on = true;
	return ppp_on;
}

int Megai2c::init()
{
	_ring_bit_high = false;
	ring_received = false;
	ppp_on = false;

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
		fprintf(Debug.out_stream, "error %d opening /dev/tnt1: %s", errno, strerror(errno));
		return -1;
	}
	//--------------------------------init sound & colors 
	char buf[7];

	buf[0] = 7 + (DO_SOUND << 3);
	// no connection RGB
	buf[1] = 1;
	buf[2] = 0;
	buf[3] = 0;
	// ring RGB
	buf[4] = 255;
	buf[5] = 0;
	buf[6] = 0;

	write(fd, buf, 7);

	return 0;


}

void Megai2c::beep_code(uint8_t c) {

	char chBuf[] = { 1,c };
	write(fd, chBuf, 2);

}



void Megai2c::Buzzer(const bool on) {

}


uint16_t Megai2c::correct(const float n) {    //0-это

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

void Megai2c::set_led_color(uint8_t n, uint8_t r, uint8_t g, uint8_t b) {
	char buf[4];
	buf[0] = 3 + (n << 3);
	buf[1] = *(char*)&r;
	buf[2] = *(char*)&g;
	buf[3] = *(char*)&b;
	write(fd, buf, 4);

}

//0.35555555555555555555555555555556 = 1град
bool Megai2c::gimagl(float pitch, float roll) {  // добавить поворот вмесете с коптером пра опред обст
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
	char bit_field;
	write(fd, &reg, 1);
	int res = read(fd, &bit_field, 1);



	if (bit_field & 1) {
		if (_ring_bit_high == false) {
			_ring_bit_high = true;
			fprintf(Debug.out_stream, "RINGk_BIT\n");
		}
	}else
		_ring_bit_high = false;

	if (bit_field & 2) {
		res = read(fd, (char*)gps_d, sizeof(SEND_I2C));
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

