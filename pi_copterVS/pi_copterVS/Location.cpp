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

void LocationClass::xy(bool update_speed){

	
	
	//------------------------------------------------------------
	
	
	if (lon_home == 0){
		return;
	}

	if (update_speed) {

		double t = form_lon2Y((double)(lon_home - lon_));
		double tspeedY = (t - y2home) *rdt;
		y2home = t;
		accY = (tspeedY - speedY)*rdt;
		accY = constrain(accY, -MAX_ACC, MAX_ACC);
		speedY = constrain(tspeedY,-25,25);
		
	
		t = from_lat2X((double)(lat_home - lat_));
		double tspeedX = (t - x2home) * rdt;
		x2home = t;
		accX = (tspeedX - speedX)*rdt;
		accX = constrain(accX, -MAX_ACC, MAX_ACC);
		speedX=constrain(tspeedX,-25,25);
	}
	dY = form_lon2Y((double)(lon_needV_ - (double)lon_)) + (speedY*0.5);
	dX = from_lat2X((double)(lat_needV_ - (double)lat_)) + (speedX*0.5f);
	
	set_cos_sin_dir();


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



	//Debug.load(0, x, y);
	//Debug.dump();
	//Out.fprintf(Debug.out_stream,"3UPD "); Out.fprintf(Debug.out_stream,x); Out.fprintf(Debug.out_stream," "); Out.println(y);
}
#define MAX_DIST2UPDATE 1000000
void LocationClass::updateXY(){
	dist2home_2 = x2home*x2home + y2home*y2home;
	//Out.println(dist2home_2);
	if (abs(dist2home_2 - oldDist) > MAX_DIST2UPDATE){
		oldDist = dist2home_2;
		
		update();
		
	}

	xy(true);
	//Out.fprintf(Debug.out_stream,"N  "); Out.fprintf(Debug.out_stream,x2home); Out.fprintf(Debug.out_stream," "); Out.println(y2home);



}


//////////////////////////////////////////////////////////////
void LocationClass::proceed(SEND_I2C *d) {
	last_gps_data_time = micros();
	accuracy_hor_pos_ = d->hAcc;
	if (accuracy_hor_pos_ > 99)accuracy_hor_pos_ = 99;
	accuracy_ver_pos_ = d->vAcc;
	if (accuracy_ver_pos_ > 99)accuracy_ver_pos_ = 99;


	if (old_iTOW == 0)
		old_iTOW = last_gps_data_time - 100;
	dt = DELTA_ANGLE_C*(double)(last_gps_data_time - old_iTOW);
	if (dt< 0.16) {
		dt = 0.1;
		rdt = 10;
	}
	else {
		//fprintf(Debug.out_stream, "\ngps dt error: %f, time= %i\n", dt, last_gps_data_time / 100);
		dt = 0.2;
		rdt = 5;
	}
	old_iTOW = last_gps_data_time;

	lat_ = d->lat;
	lon_ = d->lon;

	updateXY();


	if (Log.writeTelemetry) {
		Log.loadByte(LOG::GpS);
		Log.loadSEND_I2C(d);
		Log.loadFloat((float)x2home);
		Log.loadFloat((float)y2home);
		Log.loadFloat((float)dX);
		Log.loadFloat((float)dY);
		Log.loadFloat((float)speedX);
		Log.loadFloat((float)speedY);
		Log.loadFloat((float)accX);
		Log.loadFloat((float)accY);
	}


}



#ifdef OLDPROCGPS




int available_loc = 0, all_available_loc,buf_ind_loc;
char buf_loc[72];

bool LocationClass::processGPS_1() {
	buf_ind_loc = 0;
	const int payloadSize = sizeof(NAV_POSLLH);
	static unsigned char checksum[2];
	static int fpos = 0;
	while (available_loc) {

		//fprintf(Debug.out_stream,"%#.2X,", c);
		if (fpos < 2) {
			if (buf_loc[buf_ind_loc] == UBX_HEADER[fpos])
				fpos++;
			else
				fpos = 0;
		}
		else {
			if ((fpos - 2) < payloadSize)
				((unsigned char*)(&posllh))[fpos - 2] = buf_loc[buf_ind_loc];
			fpos++;
			if (fpos == (payloadSize + 2)) {
				calcChecksum(checksum);
			}
			else if (fpos == (payloadSize + 3)) {
				if (buf_loc[buf_ind_loc] != checksum[0]) {
					fpos = 0;
					
				}
			}
			else if (fpos == (payloadSize + 4)) {
				fpos = 0;
				if (buf_loc[buf_ind_loc] == checksum[1]) {

					

					accuracy_hor_pos_ = DELTA_ANGLE_C*(double)posllh.hAcc;
					if (accuracy_hor_pos_ > 99)accuracy_hor_pos_ = 99;
					accuracy_ver_pos_ = DELTA_ANGLE_C*(double)posllh.vAcc;
					if (accuracy_ver_pos_ > 99)accuracy_ver_pos_ = 99;

					if (Log.writeTelemetry) {
						Log.loadByte(LOG::GpS);
						Log.loadGPS_full(&posllh);
						Log.loadFloat((float)x2home);
						Log.loadFloat((float)y2home);
						Log.loadFloat((float)dX);
						Log.loadFloat((float)dY);
						Log.loadFloat((float)speedX);
						Log.loadFloat((float)speedY);
						Log.loadFloat((float)accX);
						Log.loadFloat((float)accY);


					}

					mseconds = posllh.iTOW;
					if (old_iTOW == 0)
						old_iTOW = posllh.iTOW - 100;
					dt = DELTA_ANGLE_C*(double)(posllh.iTOW - old_iTOW);
					if (dt > 0.18 || dt < 0.06)
						fprintf(Debug.out_stream,"\ngps dt error: %f, time= %i\n", dt, millis() / 1000);
					dt = constrain(dt, 0.1, 0.2);
					rdt = 1.0 / dt;
					old_iTOW = posllh.iTOW;
					last_gps_data_time = micros();

					lat_ = posllh.lat;
					lon_ = posllh.lon;

					//Debug.load(0, lat_, lon_);
					//Debug.dump();

					buf_ind_loc++;
					available_loc--;
					
				//	fprintf(Debug.out_stream,"\t\tOK\n");
					return true;
				}

			}
			else if (fpos > (payloadSize + 4)) {
				fpos = 0;
			}
		}
		available_loc--;
		buf_ind_loc++;
		//ioctl(fd_loc, FIONREAD, &available_loc);
	}
	//fprintf(Debug.out_stream,"________error\n");
	return false;
}

bool LocationClass::processGPS() {

	bool ret = false;

	ioctl(fd_loc, FIONREAD, &all_available_loc);

	if ((available_loc+all_available_loc) < 36){
		return false;
	}

	if (all_available_loc)
		available_loc+=read(fd_loc, &buf_loc[available_loc], 72 - available_loc);

	int t_available_loc = available_loc;
	ret =  processGPS_1();

	if (available_loc) {
		memmove(buf_loc, &buf_loc[buf_ind_loc], available_loc);
	}
	return ret;
}


#endif


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



//lat 0.001 ������ = 111.2 �����
//lon 0.001 ������ = 74.6 �����

//lat 899.2805755396
//lon 1340.482573727

int LocationClass::init(){
#ifndef FALSE_WIRE

	
#endif
	mspeedx =  mspeedy = 0;
	old_iTOW = 0;
	oldDist = MAX_DIST2UPDATE + MAX_DIST2UPDATE;

	dt = 0.1f;
	add_lat_need = add_lon_need = 0;
	//kd_lon = 0;// -0.000812690982;
	//kd_lat = 0;// -0.001112000712;

	x2home = y2home = speedX = speedY = 0;
	lon_home = lat_home = 0;
	accuracy_hor_pos_ = 99;
	accuracy_ver_pos_ = 99;
	altitude = 0;
	lat_ = 0;  //radians
	lon_ = 0;
	dt = 0.1f;
	rdt = 10;
	speedX = speedY = 0;
	last_gps_data_time = micros();
	
	fprintf(Debug.out_stream,"loc init\n");
}

void LocationClass::setHomeLoc(){
	lat_home = lat_;
	lon_home = lon_;
	//Debug.dump(lat_, lon_, 0, 0);
	speedX = speedY = x2home = y2home = 0;
	setNeedLoc2HomeLoc();
}
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
	xy(true);




}

double LocationClass::set_cos_sin_dir(){

	if (dX == 0){
		cosDirection = 0;
		sinDirection = 1;
		dir_angle_GRAD = 0;
	}
	else{
		double angle =  atan(dY / dX);
		dir_angle_GRAD = angle*RAD2GRAD;
		//ErrorLog.println(angle*RAD2GRAD);
		cosDirection = abs(cos(angle));
		sinDirection = abs(sin(angle));
	}
	
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

LocationClass Location;

