#include "stdafx.h"
#include "GPS_Loger.h"
#include "graph.h"

double z_cor = 101010;
FILE *klm;
void GPS_Loger::init() {
	startZ = 122222220;
	klm = fopen("d:/klm.txt", "w");
}








uint32_t def_mode = Z_STAB + XY_STAB;
int GPS_Loger::decode(char buffer[], int &i, bool rotate ) {

	

	//old_z = 0, gspeedZ, old_gspeeZ = 0
	static double first_alt = 0;

	SEND_I2C*p = (SEND_I2C*)&buffer[i + 1];
	z = 0.001*p->height;
	if (z_cor == 101010)
		z_cor = z;

//	z_cor += (z - z_cor)*0.01;
	z -= z_cor;

	if (first_alt == 0)
		first_alt = z;
//	z -= first_alt;
//	gspeedZ = (z - old_z) / 0.1;
//	old_z = z;
//	gaz = (gspeedZ - old_gspeeZ) / 0.1;
//	old_gspeeZ = gspeedZ;



	double n_lat = p->lat*0.0000001*GRAD2RAD;
	double n_lon = p->lon*0.0000001*GRAD2RAD;

	if (lat == 0 || lon == 0) {
		lat = n_lat;
		lon = n_lon;
	}

	double distance = mymath.distance_(n_lat, n_lon, lat, lon);
	double bearing = mymath.bearing_(n_lat, n_lon, lat, lon);

	static double old_distance = 0;

	gy = (distance - old_distance)*sin(bearing);
	old_distance = distance;
	gspeedY = (gy) / 0.1;


	gx = (distance - old_distance)*cos(bearing);
	gspeedX = (gx) / 0.1;


	//	lat = n_lat;
	//	lon = n_lon;






	if (startZ == 122222220)
		startZ = z;
	z -= startZ;
	i += 1 + sizeof(SEND_I2C);
	gx2home = *(float*)(&buffer[i]);
	gy2home = *(float*)(&buffer[i + 4]);
	gdX = *(float*)(&buffer[i + 8]);
	gdY = *(float*)(&buffer[i + 12]);
	//	gspeedX = *(float*)(&buffer[i + 16]);
	//	gspeedY = *(float*)(&buffer[i + 20]);
	gspeed = sqrt(gspeedX*gspeedX + gspeedY*gspeedY)*3.6;
	gax = *(float*)(&buffer[i + 24]);
	gay = *(float*)(&buffer[i + 28]);
	int vacc = p->vAcc;
	int hacc = p->hAcc;

	//if (dataI > 44467)
	{
		std::wstring str = L" " + std::to_wstring(p->lon*0.0000001) + L"," + std::to_wstring(p->lat * 0.0000001) + L"," + std::to_wstring(p->height * 0.001) + L" ";
		fwrite(str.c_str(), str.length(), 2, klm);
	}


	i += 8 * 4;


	gy2home = distance *sin(bearing);
	gx2home = distance*cos(bearing);


	max_x = max(max_x, gx2home);
	min_x = min(min_x, gx2home);
	max_y = max(max_y, gy2home);
	min_y = min(min_y, gy2home);









	return 0;
}
int GPS_Loger::view(int &indexes, char buffer[], int &i) {

	return 0;
}

GPS_Loger gps_log;