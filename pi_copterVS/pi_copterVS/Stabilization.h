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


#define SPEED_X_SPEED 0
#define SPEED_Y_SPEED 1
#define SPEED_Z_PID 2

private:
	
	void max_speed_limiter(float &x, float &y);
	void dist2speed(float &x, float &y);
	void speed2dist(float &x, float &y);

	
	float SPEED_Z_CF, SPEED_XY_CF;

	float mc_z,mc_x,mc_y,d_speedX,d_speedY;
	float dist2speed_XY;
	float alt2speedZ;
	float accxy_stab(float dist, float maxA, float timeL);
	float accxy_stab_rep(float speed, float maxA, float timeL);
	//float throttle;
#define STAB_PIDS 3
	AP_PID pids[STAB_PIDS];
	float max_z_integrator;

	float needXR, needYR, needXV, needYV;

	void set_acc_xy_speed_kp(const float f){ pids[SPEED_X_SPEED].kP(f);	pids[SPEED_Y_SPEED].kP(f); }
	void set_acc_xy_speed_kI(const float f){ pids[SPEED_X_SPEED].kI(f);	pids[SPEED_Y_SPEED].kI(f); }
	void set_acc_xy_speed_kD(const float f){ pids[SPEED_X_SPEED].kD(f,3);	pids[SPEED_Y_SPEED].kD(f,3); }
	void set_acc_xy_speed_imax(const float f){ pids[SPEED_X_SPEED].imax(-f,f);	pids[SPEED_Y_SPEED].imax(-f,f); }
	
	
public:
	void setMaxAng();
	void setMinMaxI_Thr();
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
	float getSpeed_Z(const float dist){
		return constrain(dist*alt2speedZ, max_speedZ_M, max_speedZ_P);
	}

	float getDist_Z(float speed){
		return speed/alt2speedZ;
	}

	float getSpeed_XY(float dist){		
		return dist* dist2speed_XY;
	}
	float getDist_XY(float speed){	
		return speed/ dist2speed_XY;
	}




	void setDefaultMaxSpeeds();
	


	float max_speedZ_P,max_speedZ_M,max_speed_xy;
	float Z();
	void XY(float &xF, float&yF);

	void init();
	string get_z_set();
	string get_xy_set();
	void setZ(const float  *ar);
	void setXY(const float  *ar);


};
extern StabilizationClass Stabilization;
#endif

