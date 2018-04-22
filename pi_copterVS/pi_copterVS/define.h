



#include "WProgram.h"

#ifndef DEFINE_H

#define DEFINE_H






//------------------------------------------------
#define MPU_MAGIC
//#define FALSE_WIRE
//#define MOTORS_OFF


//#define XY_SAFE_AREA 200
//#define Z_SAFE_AREA 60


#define MAX_ACC 7
//#define DEBUG_MODE
#define GPS_ALT_MAX_ERROR 30
#define MAX_BAROMETR_ERRORS 10
#define MAX_BAROMETR_ERROR 100
//#define OFF_MOTOR_IF_LOST_CONNECTION  
//#define PLUS_CAMERA
#define WORK_WITH_WIFI
#define LED_ON
#define LOST_BEEP
#define pwm_MAX_THROTTLE 32000
#define pwm_OFF_THROTTLE 16000
#define  ARDUINO_ADDR 9
#define  DO_SOUND 0



//#define FASLE_GPS_STILL

//#define NO_BATTERY

#ifdef FALSE_WIRE

//#define FASLE_GPS_STILL
#define MOTORS_OFF
#define COMPAS_MOTORS_OFF  
#define FALSE_ALTITUDE 30
#else
#define BUZZER_R
//#define GYRO_CALIBR
//#define ON_MAX_G_MOTORS_OFF
#endif

#define FALL_IF_STRONG_WIND

#ifdef FALL_IF_STRONG_WIND
#define MAX_DIST_ERROR_TO_FALL 200.0f
#define e_TOO_STRONG_WIND "TSW"
#endif

#define NO_GPS_DATA 10000000

#define MIN_ACUR_HOR_POS_2_START 5
#define MIN_ACUR_HOR_POS_4_JAMM 20


#define MAX_ANGLE_ 35
#define MIN_ANGLE 15
#define COS_MIN_ANGLE 0.966f

#define HIGHT_TO_LIFT_ON_TO_FLY_TO_HOME 50
#define FAST_DESENDING_TO_HIGH 20

#define MAX_HOR_SPEED 10
#define MAX_VER_SPEED_PLUS 5
#define MAX_VER_SPEED_MINUS -3

#define ACCURACY_XY 3
#define ACCURACY_Z 3
#define HOWER_TIME 60

#define MAX_DELTA 0.2f
#define MAX_YAW_DELTA 0.1f
#define FULL_THROTTLE_ 1.0f
#define MAX_THROTTLE_ (FULL_THROTTLE_-MAX_DELTA)



#define CONNECTION_LOST_TIMEOUT 3000
#define TIMEOUT_LAG 500

#define MIDDLE_POSITION 0.5f
#define HOVER_THROTHLE 0.5
#define MIN_THROTTLE_ 0.4
#define FALLING_THROTTLE 0.45

#define STOP_THROTTLE_ 0.2f


#define PRESSURE_AT_0 101325
#define MAX_G 32760  
#define RAD2GRAD 57.29578
#define GRAD2RAD 0.0174533
#define G 9.8
#define TELEMETRY_BUF_SIZE 16384
#define wrap_180(x) (x < -180 ? x+360 : (x > 180 ? x - 360: x))
#define wrap_PI(x) (x < -PI ? x+TWO_PI : (x > PI ? x - TWO_PI: x))


#define SETTINGS_ARRAY_SIZE 10
#define SETTINGS_IS_OK 1
#define SETTINGS_ERROR 0
#define BEGIN_CONVERSATION "GET"
#define e_OUT_OF_PER_H   "TFR"
#define e_OUT_OF_PER_V   "THG"
#define e_NO_WIFI_2_LONG "NWF"
#define e_LOW_VOLTAGE    "LWV"
#define e_GPS_ERROR      "GPE"
#define e_NO_GPS_2_LONG  "NGP"
#define e_MAX_ACCELERATION    "MXG"
#define e_GPS_ERRORS_M_50 "GER"
#define e_GPS_NO_UPDATE  "GRR"
#define e_VOLTAGE_ERROR "TEE"
#define e_VOLT_MON_ERROR "VME"
#define e_PRESURE_DEV_ZER "PRE"
#define e_BATERY_OFF_GO_2_HOME "BOH"
#define e_CONNECTION_TIME_LAG "LAG"
#define e_LOST_CONNECTION "LST"
#define e_SYSTEM_MALFUNCTION "WDT"
#define e_ESK_ERROR			"ESK"
#define e_CALIBRATING		"CLB"
#define e_BARROMETR_FAULT   "BFT"
#define e_BARROMETR_FAULT_COPTER_AT_HOME "BCH"
#define i_OFF_MOTORS     "MD0"
#define i_CONTROL_FALL   "CNF"
#define i_MAX_THR        "MXT"

#define m_START_STOP  "S_S"
#define m_HOLD_HIGHT  "AHD"
#define m_SMART_CNTR  "SCT"
#define m_GO_2_HOME   "THM"
#define m_MPU_GYRO_CAL "STS"
#define m_COMPAS_CAL  "CMC"
#define m_MAX_THR     "MAX"
#define m_OFF_THR     "OFF"
#define m_MPU_NEW_CAL "HOR"

#define m_GIMBAL_PA   "CDN"
#define m_GIMBAL_PS   "CUP"
#define m_DIRECTION_C "CMP"
#define m_XY_CONTROL  "HRT"
#define m_MOTOR_COMP_C "MCC"
#define m_START_PROG   "SRP"
#define m_SETTINGS	   "SET"
#define m_UPLOAD_SETTINGS "UPS"
#define m_PROGRAM	   "PRG"

enum { X, Y, Z };

enum {
	MOTORS_ON = 1, CONTROL_FALLING = 2, Z_STAB = 4, XY_STAB = 8, GO2HOME = 0x10, PROGRAM = 0x20, COMPASS_ON = 0x40, HORIZONT_ON = 0x80,
	MPU_ACC_CALIBR = 0x100, MPU_GYRO_CALIBR = 0x200, COMPASS_CALIBR = 0x400, COMPASS_MOTOR_CALIBR = 0x800, SHUTDOWN = 0x1000, GIMBAL_PLUS = 0x2000, 
	GIMBAL_MINUS = 0x4000, REBOOT = 0x8000,M_ON_AND_GO2HOME=10000,M_ON_AND_PROG_START=20000
};

struct SEND_I2C {

	int32_t lon;
	int32_t lat;
	int32_t height;
	uint8_t hAcc;
	uint8_t vAcc;
};







#endif
