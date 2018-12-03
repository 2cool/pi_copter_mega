#pragma once

#include <stdint.h>
#define SHMKEY "/home/igor/logs/"
#define TELEMETRY_BUF_SIZE 16384

enum { X, Y, Z };

enum {
	MOTORS_ON = 1, CONTROL_FALLING = 2, Z_STAB = 4, XY_STAB = 8, GO2HOME = 0x10, PROGRAM = 0x20, COMPASS_ON = 0x40, HORIZONT_ON = 0x80,
	MPU_ACC_CALIBR = 0x100, MPU_GYRO_CALIBR = 0x200, COMPASS_CALIBR = 0x400, COMPASS_MOTOR_CALIBR = 0x800, SHUTDOWN = 0x1000, GIMBAL_PLUS = 0x2000,
	GIMBAL_MINUS = 0x4000, REBOOT = 0x8000, PROGRAM_LOADED= 0x10000, M_ON_AND_GO2HOME = 0x20000, M_ON_AND_PROG_START = 0x40000
};


#define DEFAULT_STATE (Z_STAB|XY_STAB)

struct Memory {

	bool internet_run,fpv_run,wifi_run;
	bool in_fly;
	uint8_t main_cnt, internet_cnt, wifi_cnt,fpv_cnt;


	uint32_t status;
	uint32_t control_bits;
	uint32_t control_bits_4_do;

	float pitch, roll, yaw;
	uint8_t accuracy_hor_pos_, accuracy_ver_pos_;
	long lat_, lon_, lat_home, lon_home;
	float  speedX, speedY, speedZ, dist2home_2;
	int32_t gps_altitude_, altitude_;
	uint32_t pressure;
	float voltage, voltage_at_start;
	float m_current[4];


	uint8_t sms_at_work;
	bool inet_ok, ppp_run, sim800_reset,telegram_run, loger_run, stop_ppp_read_sms_start_ppp;
	uint32_t sim800_reset_time;




	float fly_at_start, lowest_altitude_to_fly;

	
	bool run_main;
	uint8_t reboot;










	int wifibuffer_data_len_4_read, wifibuffer_data_len_4_write;
	uint32_t connected;
	uint32_t client_addr;
	uint8_t wifiRbuffer[TELEMETRY_BUF_SIZE];
	uint8_t wifiWbuffer[TELEMETRY_BUF_SIZE];

	uint8_t fpv_adr;
	uint16_t fpv_port;
	uint8_t fpv_zoom; //if 0 then fpv off;
	uint16_t fpv_code; // code for control xiaomi
};
