// 
// 
// 
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
//#include <thread>

#include "Location.h"
#include "define.h"
#include "debug.h"
#include "Stabilization.h"
#include "Autopilot.h"
#include "Log.h"

#define DELTA_ANGLE_C 0.001
#define DELTA_A_RAD (DELTA_ANGLE_C*GRAD2RAD)
#define DELTA_A_E7 (DELTA_ANGLE_C*10000000)


int fd_loc;



void LocationClass::fromLoc2Pos(long lat, long lon, float &x, float&y) {
	y = (float)form_lon2Y((double)(lon_zero - lon));
	x = (float)from_lat2X((double)(lat_zero - lat));
}



void LocationClass::xy(){


	double t = form_lon2Y((double)(lon_zero - lon_));
	double tspeedY = (t - dY) *rdt;
	dY = t;
	shmPTR->speedY=speedY = constrain(tspeedY,-25,25);
	t = from_lat2X((double)(lat_zero - lat_));
	double tspeedX = (t - dX) * rdt;
	dX = t;
	shmPTR->speedX=speedX=constrain(tspeedX,-25,25);
	//update z
	float tsz = (altitude - old_alt)*rdt;
	old_alt = altitude;
	shmPTR->speedZ=speedZ = tsz;
	//accZ = (accZ, -2, 6);


		//
	
//	dY = form_lon2Y((double)(lon_zero - (double)lon_)) + (speedY*0.5);
//	dX = from_lat2X((double)(lat_zero - (double)lat_)) + (speedX*0.5f);
	
//	set_cos_sin_dir();


#ifdef XY_SAFE_AREA
	if (Autopilot.motors_is_on() && sqrt(x2home*x2home+y2home*y2home)>XY_SAFE_AREA)
		Autopilot.control_falling(e_OUT_OF_PER_H);
#endif
}
void LocationClass::update(){

	
	double bearing, distance;

#ifdef DEBUG_MODE
	//printf("upd\n");
	//Debug.dump(lat_, lon_, 0, 0);
#endif
	double lat = 1.74532925199433e-9 * (double)lat_;  //radians
	double lon = 1.74532925199433e-9 * (double)lon_;

	bearing = bearing_(lat, lon, lat + DELTA_A_RAD, lon + DELTA_A_RAD);
	distance = distance_(lat, lon, lat + DELTA_A_RAD, lon + DELTA_A_RAD);
//	Debug.dump(lat_, lon_, lat + 0.01*GRAD2RAD, lon + 0.01*GRAD2RAD);
//	Debug.dump(lat, lon, distance, bearing);
	double y = distance*sin(bearing);
	double x = distance*cos(bearing);
	
	//kd_lat = -x *0.00001;
	//kd_lon = -y *0.00001;

	kd_lat_ = x / (DELTA_A_E7);
	r_kd_lat = 1.0f / kd_lat_;
	kd_lon_ = y / (DELTA_A_E7);
	r_kd_lon = 1.0f / kd_lon_;

}
#define MAX_DIST2UPDATE 1000000
void LocationClass::updateXY(){

	double dx = (dX - x_from_zero_2_home);
	double dy = (dY - y_from_zero_2_home);
	shmPTR->dist2home_2 =dx*dx + dy*dy;
	dist2zero_2 = dX * dX + dY * dY;
	//Out.println(dist2home_2);
	if (fabs(dist2zero_2 - oldDist_2) > MAX_DIST2UPDATE){
		oldDist_2 = dist2zero_2;
		update();
	}

	xy();
	//Out.printf("N  "); Out.printf(x2home); Out.printf(" "); Out.println(y2home);



}


//////////////////////////////////////////////////////////////
void LocationClass::proceed(SEND_I2C *d) {
	last_gps_data_timed = Mpu.timed;
	dt = last_gps_data_timed - old_iTOWd;
	dt = (dt < 1.6) ? 0.1 : 0.2;
	old_iTOWd = last_gps_data_timed;

#define REAL_GPS
#ifdef REAL_GPS
	shmPTR->accuracy_hor_pos_ = accuracy_hor_pos_ = (accuracy_hor_pos_ > 99) ? 99 : d->hAcc;
	shmPTR->accuracy_ver_pos_ = accuracy_ver_pos_ = (accuracy_ver_pos_ > 99) ? 99 : d->vAcc;
	if (accuracy_hor_pos_ < MIN_ACUR_HOR_POS_4_JAMM)
		last_gps_accurasy_okd = Mpu.timed;
	shmPTR->gps_altitude_ = d->height;
	altitude = 0.001*(double)d->height;
	shmPTR->lat_ = lat_ = d->lat;
	shmPTR->lon_ = lon_ = d->lon;
#else
	shmPTR->accuracy_hor_pos_ = accuracy_hor_pos_ = 1;
	shmPTR->accuracy_ver_pos_ = accuracy_ver_pos_ = 1;
	shmPTR->gps_altitude_ = 15000;
	altitude = 0.001 * 15000;

	shmPTR->lat_ = lat_ = 479079700;shmPTR->lon_ = lon_ = 333320180; //
	

	///shmPTR->lat_ = lat_ = 479079060;shmPTR->lon_ = lon_ = 333321560; //dvor


#endif









	if (lat_zero == 0 && lon_zero == 0 && accuracy_hor_pos_ < MIN_ACUR_HOR_POS_2_START) {
		lat_zero = lat_;
		lon_zero = lon_;
		alt_zero = altitude;
	}

	if (lat_zero != 0 || lon_zero != 0)
		updateXY();


	if (Log.writeTelemetry) {
		Log.block_start(LOG::GPS_SENS);
		Log.loadSEND_I2C(d);
		Log.loadFloat((float)dX);
		Log.loadFloat((float)speedX);
		Log.loadFloat((float)dY);
		Log.loadFloat((float)speedY);
		Log.block_end();
	}


}
/*
void LocationClass::add2NeedLoc(const double speedX, const double speedY, const double dt){
	//double t = (add_lat_need+= from_X2Lat(speedX*dt));
	//add_lat_need -= t;


	lat_needR_ -= from_X2Lat(speedX*dt);
	lat_needV_ = lat_needR_ - from_X2Lat(Stabilization.getDist_XY(speedX));
	//t = (add_lon_need += from_Y2Lon(speedY*dt));
	//add_lon_need -= t;
	lon_needR_ -= from_Y2Lon(speedY*dt);
	lon_needV_ = lon_needR_ - from_Y2Lon(Stabilization.getDist_XY(speedY));
}
*/


//lat 0.001 ������ = 111.2 �����
//lon 0.001 ������ = 74.6 �����

//lat 899.2805755396
//lon 1340.482573727

int LocationClass::init(){
#ifndef FLY_EMULATOR

	
#endif
	mspeedx =  mspeedy = 0;
	old_iTOWd = 0;
	oldDist_2 = MAX_DIST2UPDATE + MAX_DIST2UPDATE;

	dt = 0.1f;
	add_lat_need = add_lon_need = 0;
	//kd_lon = 0;// -0.000812690982;
	//kd_lat = 0;// -0.001112000712;

	speedX = speedY = 0;
	lon_zero = lat_zero = 0;
	accuracy_hor_pos_ = 99;
	accuracy_ver_pos_ = 99;
	altitude = 0;
	lat_ = 0;  //radians
	lon_ = 0;
	dt = 0.1f;
	rdt = 10;
	speedX = speedY = 0;
	last_gps_data_timed = 0;
	last_gps_accurasy_okd = 0;
	
	cout << "loc init\n";
}

void LocationClass::setHomeLoc(){
	x_from_zero_2_home = dX;
	y_from_zero_2_home = dY;
	shmPTR->lat_home = lat_;// lat_home;
	shmPTR->lon_home = lon_;// lon_home;
	old_alt=startAlt = altitude;

	//Debug.dump(lat_, lon_, 0, 0);
//	dX=dY=speedZ=speedX = speedY = x2home = y2home = accX=accY=accZ=0;
//	lat_needR_ = lat_needV_ = (double)lat_home;
//	lon_needR_ = lon_needV_ = (double)lon_home;

	//setNeedLoc2HomeLoc();
}

/*
void LocationClass::setNeedLoc(long lat, long lon){
	lat_needR_ = lat_needV_ = (double)lat;
	lon_needR_ = lon_needV_ = (double)lon;
	xy(false);
	//set_cos_sin_dir();

}


void LocationClass::setNeedLoc2HomeLoc(){
	//setNeedLoc(lat_home, lon_home);
	lat_needR_ = lat_needV_ = (double)lat_home;
	lon_needR_ = lon_needV_ = (double)lon_home;
	xy(false);
}
*/
double LocationClass::set_cos_sin_dir(){

//	double angle = atan2(dY, dX);

//	dir_angle_GRAD = angle*RAD2GRAD;
	//ErrorLog.println(angle*RAD2GRAD);
//	cosDirection = fabs(cos(angle));
//	sinDirection = fabs(sin(angle));
	
}

double LocationClass::bearing_(const double lat, const double lon, const double lat2, const double lon2){
	double x, y;
	sin_cos(x, y, lat, lon, lat2, lon2);
	return atan2(y, x);  //minus � ��� � �����

}
void LocationClass::sin_cos(double &x, double &y, const double lat, const double lon, const double lat2, const double lon2){
	double rll = (lon2 - lon);
	double rlat = (lat);
	double rlat2 = (lat2);

	y = (sin(rll)*cos(rlat2));
	x = (cos(rlat)*sin(rlat2) - sin(rlat)*cos(rlat2)*cos(rll));

}
double LocationClass::distance_(const double lat, const double lon, const double lat2, const double lon2){
	double R = 6371000;
	double f1 = (lat);
	double f2 = (lat2);
	double df = (lat2 - lat);
	double dq = (lon2 - lon);

	double a = (sin(df / 2)*sin(df / 2) + cos(f1)*cos(f2)*sin(dq / 2)*sin(dq / 2));
	return R * 2 * atan2(sqrt(a), sqrt(1 - a));

}



