// Autopilot.h

#ifndef _AUTOPILOT_h
#define _AUTOPILOT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "mi2c.h"
#include "MS5611.h"
#include "Location.h"
#include "AP_PID.h"


// in dicimetr
#define MAX_HEIGHT 15


#include "commander.h"
#include "mpu.h"
class AutopilotClass
{



 protected:

	 float dist2home_at_begin2;
#ifdef FALL_IF_GPS_ALT_NOT_MATCH_BAROM_ALT_
	 float gps_alt_at_begin, bar_alt_at_begin;
#endif

	 bool howeAt2HOME;
	 float sens_z, sens_xy;

	 bool newData;
	 double controlDeltaTimed;
	 uint8_t go2homeIndex;
	 float f_go2homeTimer;
 
	 
	 float flyAtAltitude;

	// bool motors_on, smart_ctrl;

	 float aPitch, aRoll,aYaw_;

	 uint32_t old_control_bits;
	 float throttle;
	 
	 uint32_t control_bits;
	 float tflyAtAltitude;

	 void log();

	 void smart_commander(const float dt);
	// void setNextGeoDot();
	 //void prog_loop();
	// float get_dist();
	 float lowest_height;
	 float fall_thr;
	

	 
 public:

	 bool not_start_motors_if_gps_error;
	 int reboot();
	 int shutdown();
	 int exit();
	 double  time_at_startd,old_time_at_startd;
	 void gimBalRollCorrection();
	 bool busy() { return (control_bits & (MPU_ACC_CALIBR | MPU_GYRO_CALIBR | COMPASS_CALIBR)); }
	 volatile double last_time_data_recived;
	 void setYaw(const float yaw){aYaw_ = yaw;}
	 float getGimbalPitch(){ return gimbalPitch; }
	 void control_falling(const string msg);
	 void gimBalPitchADD(const float add);
	 void set_gimBalPitch(const float angle);

	 float corectedAltitude4tel();
	
	 float start_timed;
	void reset_compas_motors_calibr_bit() {control_bits &= (~COMPASS_MOTOR_CALIBR);}

	bool motors_onState(){ return control_bits&MOTORS_ON; }
	bool z_stabState(){ return control_bits&Z_STAB; }
	bool xy_stabState(){ return control_bits&XY_STAB; }
	 bool go2homeState(){ return control_bits&GO2HOME; }
	 bool progState(){ return control_bits&PROGRAM; }
	 bool control_fallingState(){ return control_bits & CONTROL_FALLING; }
	 bool program_is_loaded() { return control_bits & PROGRAM_LOADED; }
	 void program_is_loaded(bool set);
	 bool set_control_bits(uint32_t bits);

	 float fly_at_altitude() { return flyAtAltitude; }
	 uint32_t get_control_bits(){ return control_bits; }
	 //uint8_t mod;  //����� ������ 
	// bool falling(){ return ctrl_flag == CNTR_FALLING; }


	 float get_throttle(){ return throttle; }
	 float get_yaw(){ return aYaw_; }

	 
	 void add_2_need_altitude(float speed, const float dt);
	 void add_2_need_yaw(float speed, const float dt);
	// bool manualZ;
	
	string get_set();
	 void set(const float buf[]);

	
	 void clearSpeedCoreection(){ flyAtAltitude = tflyAtAltitude; }

	// bool get_smart_cntr_flag(){ return smart_ctrl; }

	 float gimBalPitchZero, gimBalRollZero, gimbalPitch;
	 float height_to_lift_to_fly_to_home;

	 bool going2HomeON(const bool hower);
	 bool going2HomeStartStop(const bool hower);
	 bool go2HomeProc(const float dt);


	 float get_Roll(){ return aRoll; }
	 float get_Pitch(){ return aPitch; }


	 
	 bool holdLocationStartStop();
	 bool holdLocation(const long lat, const long lon);
	 
	 bool holdAltitudeStartStop();
	 bool holdAltitude(float alt);
	 void	 set_new_altitude(float alt);
	 void init();
	 void loop();
	// uint8_t ctrl_flag;
	 bool motors_is_on(){ return control_bits & MOTORS_ON; }

	 bool start_stop_program(const bool stopHere);

	void connectionLost_();


	void calibration(); 

	//void set_height(const short h);
	bool off_throttle(const bool force,const string msg);
	bool motors_do_on(const bool start,const string msg);



private:

};

extern AutopilotClass Autopilot;

#endif

