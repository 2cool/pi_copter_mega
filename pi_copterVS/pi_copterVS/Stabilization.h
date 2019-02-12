// Stabilization.h

#ifndef _STABILIZATION_h
#define _STABILIZATION_h


#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif



#include "AP_PID.h"
#include "define.h"
#include "mpu.h"
class StabilizationClass{


#define ACCX_SPEED 0
#define ACCY_SPEED 1
#define ACCZ_PID 2

private:
	
	void max_speed_limiter(float &x, float &y);
	void dist2speed(float &x, float &y);
	void speed2dist(float &x, float &y);

	


	float mc_z,Z_FILTER,XY_FILTER,mc_pitch,mc_roll;
	float accXY_stabKP, accXY_stabKP_Rep;

	float dalt2speedZ, dalt2speedZ_Rep;
	float dspeedZ2accZ;

	float accxy_stab(float dist, float maxA, float timeL);
	float accxy_stab_rep(float speed, float maxA, float timeL);
	//float throttle;
#define STAB_PIDS 3
	AP_PID pids[STAB_PIDS];
	float max_z_integrator;
	//float  sX, sY, speedX, speedY;
	
	//unsigned long gps_sec;
	//float speedZ, sZ;
	float Z_CF_SPEED,Z_CF_DIST;

	float needXR, needYR, needXV, needYV;

	void set_acc_xy_speed_kp(const float f){ pids[ACCX_SPEED].kP(f);	pids[ACCY_SPEED].kP(f); }
	void set_acc_xy_speed_kI(const float f){ pids[ACCX_SPEED].kI(f);	pids[ACCY_SPEED].kI(f); }
	void set_acc_xy_speed_kD(const float f){ pids[ACCX_SPEED].kD(f,3);	pids[ACCY_SPEED].kD(f,3); }
	void set_acc_xy_speed_imax(const float f){ pids[ACCX_SPEED].imax(f);	pids[ACCY_SPEED].imax(f); }
	
	
public:
	
	void setNeedPos2Home();
	void add2NeedPos(float speedX, float speedY, float dt);
	void setNeedPos(float x, float y);
	void setNeedLoc(long lat, long lon, float &x, float &y);
	void fromLoc2Pos(long lat, long lon, float &x, float &y);
	float get_dist2goal();
	//void set_XY_2_GPS_XY();
	void  resset_z();
	void  resset_xy_integrator();
	//float getAltitude() { return sZ; }
	float getSpeed_Z(float dist){
		return dist*dalt2speedZ;
	}

	float getDist_Z(float speed){
		return speed*dalt2speedZ_Rep;
	}

	float getSpeed_XY(float dist){		
		return dist*accXY_stabKP;
	}
	float getDist_XY(float speed){	
		return speed*accXY_stabKP_Rep;
	}




	void setDefaultMaxSpeeds();
	
	//float get_dalt2speedZ_Rep(){ return dalt2speedZ_Rep; }
//	float getDist2SpeedXYKP_Rep(){ return accXY_stabKP_Rep; }
//	float getDist2SpeedXYKP(){ return accXY_stabKP; }
	//long get_sX_mul_100(){return (long)(sX * 100); 	}
	//long get_sY_mul_100(){ return (long)(sY * 100); }
	//void init_XY(const float sx, const float sy);
	
	float XY_KF_SPEED, XY_KF_DIST;
	
	

	float max_stab_z_P,max_stab_z_M,max_speed_xy;
	float last_accZ;
	float Z();
	float Zgps();
	void XY(float &xF, float&yF);
	void horizont(const uint8_t i, const float dt, const float accX, const float accY, const float F);

	void init();
	string get_z_set();
	string get_xy_set();
	void setZ(const float  *ar);
	void setXY(const float  *ar);


};
extern StabilizationClass Stabilization;
#endif

