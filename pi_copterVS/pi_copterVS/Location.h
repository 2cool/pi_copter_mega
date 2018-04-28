// Location.h

#ifndef _LOCATION_h
#define _LOCATION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "define.h"

const unsigned char UBX_HEADER[] = { 0xB5, 0x62 };

class LocationClass
{


public:

	double cosDirection, sinDirection;
	float dir_angle_GRAD;
	double dt, rdt;
	double add_lat_need, add_lon_need;
	long lat_, lon_, lat_home, lon_home;
	double accuracy_hor_pos_, altitude;
	double accuracy_ver_pos_;
	unsigned long mseconds;
	void setSpeedZero(){ lat_needV_ = lat_needR_; lon_needV_ = lon_needR_; }
	int init();
	void setNeedLoc2HomeLoc();
	
	void setNeedLoc(const long lat, const long lon);
	void setHomeLoc();
	void add2NeedLoc(const double speedX, const double speedY, const double dt);


	void updateXY();
	//void bearing_dist(double &bearing, double & distance);
	
	double x2home, y2home, dX, dY, speedX, speedY,accX,accY,accZ,speedZ,startAlt,old_alt;
	double dist2home_2;
	//---------------
	double last_gps_data_timed;
	double last_gps_accurasy_okd;
	
	double bearing_(const double lat, const double lon, const double lat2, const double lon2);
	void sin_cos(double &x, double &y, const double lat, const double lon, const double lat2, const double lon2);

	double distance_(const double lat, const double lon, const double lat2, const double lon2);
	//---------------
	void proceed(SEND_I2C *d);
	void clearSpeedCorrection(){ lat_needV_ = lat_needR_; lon_needV_ = lon_needR_; }

	double from_lat2X(const double lat){
		return lat*kd_lat_;
	}
	double from_X2Lat(const double x){ 
		return x *r_kd_lat; 
	}

	double form_lon2Y(const double lon){
		return lon*kd_lon_;
	}
	double from_Y2Lon(const double y){
		return y * r_kd_lon;
	}

private:
	double set_cos_sin_dir();
	void xy(bool update_speed);
	double lat_needV_, lon_needV_, lat_needR_, lon_needR_;
	double old_iTOWd;
	double mspeedx, mspeedy;


	double oldDist;
	void update();
	double kd_lon_, kd_lat_;
	double r_kd_lon, r_kd_lat;



};


#endif

