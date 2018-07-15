#include "stdafx.h"
#include "graph.h"
#include <stdint.h>
#include "KK.h"

#include "Pressure.h"
#include "GPS_Loger.h"
#include "Mpu.h"
#include "MyMath.h"
#include "Balance.h"
#include "Telemetry.h"
#include "Stab.h"
//#define LOG_FILE_NAME "d:/tel_log10011.log"





uint32_t control_bits = 0;
char *fname;
enum LOG {/* MPU, HMC, MS5, GpS, COMM, STABXY, STABZ, BAL , EMU, AUTO, TELE, MPU_M, */mMPU = 1, mHMC = 2, mMS5 = 4, mGpS = 8, mCOMM = 16, mSTABXY = 32, mSTABZ = 64, mBAL = 128
};







int findLog(char *buffer, int beg, int size) {
	for (int i = beg; i < size; i++) {
		if (buffer[i] == 0) {
			float dt = (double)buffer[i + 1] * 0.001f;
			float pitch = *(float*)(&buffer[i + 2]);
			float roll = *(float*)(&buffer[i + 6]);

			if (dt>0.008 && dt < 0.012 && abs(pitch) < 50 && abs(roll) < 50)
				return i;
		}
	}
	return -1;
}

#define NO_DATA 0
#define ANGK 0.1f
bool init_yaw = true;

uint16_t log_bank_size;

void Graph::filter(float src, int dataI, int elementi, float max) {

	if (isnan(src) || abs(src) > max)
		src = 0;
	sensors_data[dataI].sd[elementi] = (flags[FILTER] && kalman[elementi].initialized == 77777) ? kalman[elementi].update(src) : src;
}


#define SOME_K 0.9
int Graph::updateGPS(HDC hdc, RectF rect, double zoom, double pos) {

	Graphics g(hdc);

	g.Clear(Color(255, 255, 255, 255));

	p = lSize * pos;
	step = (double)(lSize - p)*zoom / rect.Width;
	double mull_y = SOME_K * (double)rect.Height / (gps_log.max_y - gps_log.min_y);
	double mull_x = SOME_K * (double)rect.Width / (gps_log.max_x - gps_log.min_x);
	mull_x = mull_y = min(mull_x, mull_y);
	int startX, startY;
	{
		int dy[2], dx[2];
		int y0 = 0;
		int x0 = 0;



		int ind = 1;

		if (p < gpsI)
			p = gpsI;
		startY = dy[0] = y0 + (sensors_data[(int)p].sd[Y] - gps_log.min_y) * mull_y;
		startX = dx[0] = x0 + (sensors_data[(int)p].sd[X] - gps_log.min_x) * mull_x;


		Pen pen(Color(255, 0, 0, 0));
		for (int x = rect.X + 1, i = 1; x < rect.X + rect.Width; x++, i++) {
			dy[ind & 1] = y0 + (sensors_data[(int)(p + (step*i))].sd[Y] - gps_log.min_y) * mull_y;
			dx[ind & 1] = x0 + (sensors_data[(int)(p + (step*i))].sd[X] - gps_log.min_x) * mull_x;

			int x1 = dx[(ind - 1) & 1];
			int x2 = dx[ind & 1];
			int y1 = dy[(ind - 1) & 1];
			int y2 = dy[ind & 1];
			//if ( sqrt((x1-x2)*(x1 - x2)+(y1-y2)*(y1 - y2))<10)
			g.DrawLine(&pen, x1, y1, x2, y2);
			ind++;


		}
	}
	if (flags[SX] || flags[SY]) {
		int dy[2], dx[2];
		int y0 = 0;
		int x0 = 0;


		int ind = 1;

		dy[0] = y0 + (sensors_data[(int)p].sd[SY] - gps_log.min_y) * mull_y;
		dx[0] = x0 + (sensors_data[(int)p].sd[SX] - gps_log.min_x) * mull_x;

		startX - dx[0];
		startY - dy[0];


		Pen pen(Color(255, 255, 0, 0));
		for (int x = rect.X + 1, i = 1; x < rect.X + rect.Width; x++, i++) {
			dy[ind & 1] = y0 + (sensors_data[(int)(p + (step*i))].sd[SY] - gps_log.min_y) * mull_y;
			dx[ind & 1] = x0 + (sensors_data[(int)(p + (step*i))].sd[SX] - gps_log.min_x) * mull_x;

			g.DrawLine(&pen, dx[(ind - 1) & 1], dy[(ind - 1) & 1], dx[ind & 1], dy[ind & 1]);
			ind++;


		}
	}



	return 0;
}










#define wrap_180(x) (x < -180 ? x+360 : (x > 180 ? x - 360: x))
float old_speedX = 0, old_speedY = 0;
boolean new_mode_ver = false;





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////











enum { MPU_EMU, MPU_SENS, HMC_BASE, HMC_SENS, HMC_EMU, GPS_SENS, TELE, COMM, EMU, AUTO, BAL, MS5611_SENS, XYSTAB, ZSTAB
};












int load_uint8_(byte buf[], int i) {
	int vall = buf[i];
	vall &= 255;
	return vall;
}

int16_t load_int16_(byte buf[], int i) {
	int16_t *ip = (int16_t*)&buf[i];
	return *ip;
}


int Graph::parser(byte buf[]) {
	int i = 0;
	int f_len = load_int16_(buf, i);
	i += 2;
	while (i<f_len) {
		int b = buf[i++];
		int len = load_uint8_(buf, i);
		i++;
		if (len == 0) {
			len = load_int16_(buf, i);
			i += 2;
		}
		switch (b) {

		case MPU_SENS: {
			if (len == 5)
				press.parser(buf,i);
			else
				mpu.parser(buf, i, len, flags[FILTER]);
			break;
		}
		case MS5611_SENS: {
			press.parser(buf, i);
			break;
		}
		case GPS_SENS: {
			
			gps_log.decode((SEND_I2C*)&buf[i]);
			break;
		}
		case HMC_BASE: {
			
			break;
		}
		case AUTO: {
			control_bits = *(uint32_t*)&buf[i];
			break;
		}
		case HMC_SENS: {

			break;
		}
		case TELE: {
			tel.parser(buf, i);
			break;
		}
		case ZSTAB:{
			stab.parser(buf, i, len);
			break;
		}
		case BAL: {
			bal.parser(buf, i);
			break;
		}
		}
		i += len;
	}

	return i;
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////
int indexes[] = { 0,0,0,0,0,0,0,0,0,0,0 };

int Graph::decode_Log() {
	/*
	gps_log.init();



	kk0 = KK(5, 0.7);
	bufff0I = 0;
	//создать функцию а потом ее отфильтровать

	double q = 0.96;
	float fr = 1.5;
	float k = 0.707;
	kk0 = KK(fr, q);
	kk1 = KK(fr*=k,q);
	kk2 = KK(fr*=k, q);
	kk3 = KK(fr *= k, q);
	kk4 = KK(fr *= k, q);
	kk5 = KK(fr *= k, q);








	modesI = 0;
	*/
	int i = 0;
	int j = 0;
	int n = 1;

	while (j < lSize) {

		i = 0;
		byte *buf = (byte*)&buffer[j];
		int f_len = i + load_int16_(buf, i);
		i += 2;
		while (i<f_len) {
			int b = buf[i++];
			int len = load_uint8_(buf, i);
			i++;
			if (len == 0) {
				len = load_int16_(buf, i);
				i += 2;
			}
			i += len;
		}
		n++;
		j += i;
	}

	sensors_data = (SensorsData*)malloc(sizeof(SensorsData)*n);


	j = 0;
	n = 0;
	while (j < lSize) {


		j+=parser((byte*)&buffer[j]);



		if ((control_bits&MOTORS_ON) && !(old_control_bits&MOTORS_ON)) {
			zero_alt = press.altitude;
			fly_at_altitude = press.altitude;
		}

		if ((control_bits&MOTORS_ON) && (control_bits&Z_STAB) && !(old_control_bits&Z_STAB)) {
			fly_at_altitude = press.altitude;

		}



		old_control_bits = control_bits;
		
		sensors_data[n].sd[GPS_Z] = gps_log.z;

		sensors_data[n].sd[TIME] = mpu.time;
		sensors_data[n].sd[DT] = mpu.dt;
		sensors_data[n].sd[PITCH] = mpu.pitch;
		sensors_data[n].sd[ROLL] = mpu.roll;
		sensors_data[n].sd[YAW] = mpu.yaw;
		sensors_data[n].sd[ACCX] = mpu.accX;
		sensors_data[n].sd[ACCY] = mpu.accY;
		sensors_data[n].sd[ACCZ] = mpu.accZ;
		sensors_data[n].sd[GYRO_PITCH] = mpu.gyroPitch;
		sensors_data[n].sd[GYRO_ROLL] = mpu.gyroRoll;
		sensors_data[n].sd[GYRO_YAW] = mpu.gyroYaw;

		sensors_data[n].sd[F0] = bal.f0;
		sensors_data[n].sd[F1] = bal.f1;
		sensors_data[n].sd[F2] = bal.f2;
		sensors_data[n].sd[F3] = bal.f3;
		sensors_data[n].sd[THROTTLE] = bal.thr;
		sensors_data[n].sd[C_PITCH] = bal.ap_pitch;
		sensors_data[n].sd[C_ROLL] = bal.ap_roll;
		sensors_data[n].sd[HEADING] = bal.ap_yaw;


	static float sZ = 0, speedZ = 0;

		float alt = press.altitude;
	
		sZ += 0.01*(speedZ + mpu.accZnF*0.01*0.5f);
		sZ += (alt - sZ)*0.03;

		speedZ += mpu.accZnF*0.01;
		speedZ += (press.speed - speedZ)*0.01;



		static float altt = 0;
		if (altt == 0)
			altt = press.altitude;
		altt += (press.altitude - altt)*0.001;

		sensors_data[n].sd[PRESSURE] = flags[FILTER]?sZ:press.altitude;// -fly_at_altitude;
		sensors_data[n].sd[PRESSURE_SPEED] = flags[FILTER]?speedZ:press.speed;// -fly_at_altitude;



		static double tacc_bar = 0;
		tacc_bar += (press.acc - tacc_bar)*0.01;

		sensors_data[n].sd[PRESSURE_ACC] = tacc_bar;


		sensors_data[n].sd[MI0] = tel.m_current[0];
		sensors_data[n].sd[MI1] = tel.m_current[1];
		sensors_data[n].sd[MI2] = tel.m_current[2];
		sensors_data[n].sd[MI3] = tel.m_current[3];
		sensors_data[n].sd[BAT_F] = tel.m_current[4];


		sensors_data[n].sd[STAB_SPEED_Z] = stab.speedZ;
		sensors_data[n].sd[F_Z] = stab.fZ;
		sensors_data[n].sd[STAB_Z] = stab.sZ;

		(*(uint32_t*)&sensors_data[n].sd[CONTROL_BITS]) = control_bits;



		static double pres_alt = 0;
		pres_alt += (sensors_data[n].sd[PRESSURE] - pres_alt)*0.0001;
	//	sensors_data[n].sd[PRESSURE] = pres_alt;
		n++;

	}


	/*

	log_bank_size = *(uint16_t*)buffer;
	i += 2;
	bool starPointOK = false;


	if (indexes[LOG::MPU] == 0) {
	do {
	//if (indexes[LOG::MPU] > 157000) { lSize = i; break; }
	switch (buffer[i]) {
	case LOG::MPU: {
	mpu.view(indexes[LOG::MPU], buffer, i);
	break;
	}
	case LOG::MPU_M: {
	i += 17;
	break;
	}
	case LOG::TELE: {
	i += 11;
	break;
	}
	case LOG::AUTO: {
	i += 5;
	break;
	}
	case LOG::HMC:
	indexes[LOG::HMC]++;
	i += 17;
	break;
	case LOG::MS5: {
	press.view(indexes[LOG::MS5], buffer, i);
	break;
	}
	case LOG::GpS:
	indexes[LOG::GpS]++;
	i += 1 + sizeof(SEND_I2C);
	i += 8 * 4;
	break;

	case LOG::COMM: {
	indexes[LOG::COMM]++;

	unsigned short len = *(unsigned short*)(buffer + i + 1);
	i += len + 3;
	break;
	}
	case LOG::STABXY: {
	indexes[LOG::STABXY]++;



	i += 17;
	break;
	}
	case LOG::STABZ: {
	indexes[LOG::STABZ]++;

	i += 9;
	break;
	}

	case  LOG::BAL: {
	indexes[LOG::BAL]++;
	bal.decode(buffer, i, false);
	//i += 43;
	log_bank_size = *(uint16_t*)(buffer + i);
	i += 2;
	break;
	}
	case LOG::EMU: {
	indexes[LOG::EMU]++;
	i += buffer[i + 1] + 2;
	break;
	}

	default: {

	int res = findLog(buffer, i, lSize);
	if (res == -1) {
	lSize = i - 2;
	break;
	}
	else
	i = res;
	}
	}

	} while (i < lSize);

	sensors_data = (SensorsData*)malloc(sizeof(SensorsData)*(indexes[LOG::MPU] + 1));

	}
	i = 2;
	int dataI = 0;

	modes[modesI].mode = def_mode;
	modes[modesI].index = dataI;
	modesI++;

	kalman[GACCX] = Kalman(300, 0);
	kalman[GACCY] = Kalman(300, 0);
	kalman[GACCZ] = Kalman(300, 0);
	kalman[F0] = Kalman(10, 0);
	kalman[F1] = Kalman(10, 0);
	kalman[F2] = Kalman(10, 0);
	kalman[F3] = Kalman(10, 0);

	kalman[MI0] = Kalman(10, 0);
	kalman[MI1] = Kalman(10, 0);
	kalman[MI2] = Kalman(10, 0);
	kalman[MI3] = Kalman(10, 0);
	kalman[BAT_F] = Kalman(10, 0);


	kalman[GYRO_PITCH] = Kalman(1, 0);
	//kalman[GACCX] = Kalman(100, 0);


	//	kalman[F3] = Kalman(10, 0);
	kalman[ACC] = Kalman(100, 0);
	kalman[GACC] = Kalman(100, 0);
	kalman[ANGLE] = Kalman(100, 0);

	kalman[ACCX] = Kalman(100, 0);
	kalman[ACCY] = Kalman(100, 0);
	kalman[ACCZ] = Kalman(100, 0);
	kalman[EXP2] = Kalman(100, 0);
	kalman[EXP3] = Kalman(100, 0);
	kalman[G_SPEED] = Kalman(100, 0);

	kalman[PRESSURE] = Kalman(30000, 0);
	kalman[PRESSURE_SPEED] = Kalman(30000, 0);
	kalman[PRESSURE_ACC] = Kalman(30000, 0);


	//	kalman[HEADING] = Kalman(10, 0);
	kalman[GYRO_PITCH] = Kalman(100, 0);
	kalman[GYRO_ROLL] = Kalman(100, 0);
	kalman[GYRO_YAW] = Kalman(100, 0);
	kalman[BAT_F] = Kalman(10, 12);

	kalman[R_PITCH] = Kalman(60, 0);
	kalman[R_ROLL] = Kalman(60, 0);
	kalman[THROTTLE] = Kalman(300, 0);
	mi[0] = mi[1] = mi[2] = mi[3] = 0;
	bat = 0;
	cf = flags[FILTER] ? 0.1 : 1;



	press.init(flags[FILTER], 1, 0.01, 0.01);


	FILE *klm = fopen("d:/klm.txt", "w");
	;
	do {
	if (i > 3050)
	i = i;
	switch (buffer[i]) {

	case LOG::MPU: {
	mpu.decode(buffer, i);
	break;
	}
	case LOG::MPU_M: {

	#ifdef _MPU_M
	#endif
	i += 17;
	break;
	}
	case LOG::TELE: {
	i += 11;
	break;
	}
	case LOG::AUTO: {
	new_mode_ver = true;
	def_mode = *(uint32_t*)(&buffer[i + 1]);
	modes[modesI].mode = def_mode;
	modes[modesI].index = dataI;
	if (def_mode&GO2HOME)
	def_mode = def_mode;
	modesI++;
	i += 5;
	break;
	}
	case LOG::HMC:
	i += 17;
	break;
	case LOG::MS5: {
	press.decode(buffer, i);
	break;
	}
	case LOG::GpS: {
	gps_log.decode(buffer, i);
	//i += 1 + sizeof(SEND_I2C);
	//i += 8 * 4;

	break;
	}

	case LOG::COMM: {


	unsigned short len = *(unsigned short*)(buffer + i + 1);
	i += len + 3;
	break;
	}
	case LOG::STABXY: {

	i += 17;
	break;
	}

	case LOG::STABZ: {

	filter(*(float*)(&buffer[i + 1]), dataI, SZ);
	filter(*(float*)(&buffer[i + 5]), dataI, SPEED_Z);

	i += 9;
	break;
	}
	case LOG::BAL: {


	//------------------------------------------------------------------------------------
	sensors_data[dataI].sd[TIME] = mpu.time;
	sensors_data[dataI].sd[DT] = mpu.dt;
	sensors_data[dataI].sd[PITCH] = mpu.f[mPITCH];
	sensors_data[dataI].sd[ROLL] = mpu.f[mROLL];
	sensors_data[dataI].sd[GYRO_PITCH] = mpu.f[mGYRO_PITCH];
	sensors_data[dataI].sd[F0] = bal.f[bF0];





	sensors_data[dataI].sd[MAXACC] = mpu.f[mMAXACC];


	sensors_data[dataI].sd[PRESSURE] = press.alt;
	sensors_data[dataI].sd[PRESSURE_SPEED] = press.speed;
	sensors_data[dataI].sd[PRESSURE_ACC] = press.acc;

	sensors_data[dataI].sd[X] = gps_log.gx2home;
	sensors_data[dataI].sd[Y] = gps_log.gy2home;
	sensors_data[dataI].sd[GPS_Z] = gps_log.z;




	i += 43;
	i += 2;

	dataI++;
	break;
	}

	case LOG::EMU: {

	i += buffer[i + 1]+2;
	break;
	}
	default: {

	int res = findLog(buffer, i, lSize);
	if (res == -1) {
	lSize = i - 2;
	break;
	}
	else
	i = res;
	}
	}

	} while (i < lSize);
	fclose(klm);
	lSize = dataI;
	free(buffer);

	*/


	//fclose(klm);
	lSize = n;

	return 0;
}




int Graph::readLog() {

	FILE * pFile;
	size_t result;


	pFile = fopen(fname, "rb");
	if (pFile == NULL) { fputs("File error", stderr); exit(1); }

	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);

	// allocate memory to contain the whole file:
	buffer = (char*)malloc(sizeof(char)*lSize);
	if (buffer == NULL) { fputs("Memory error", stderr); exit(2); }

	// copy the file into the buffer:
	result = fread(buffer, 1, lSize, pFile);
	if (result != lSize) { fputs("Reading error", stderr); exit(3); }

	/* the whole file is now loaded in the memory buffer. */

	// terminate
	fclose(pFile);
	//free(buffer);
	return decode_Log();


}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Graph::Graph(char*fn)
{


	fname = fn;
	readLog();

	for (int i = 0; i < ALL_ELEMENTS; i++)
		flags[i] = false;

	color[BAT_F] = Color(255, 200, 0, 0);
	name[BAT_F] = L"BAT";


	color[MI0] = Color(255, 200, 0, 0);
	name[MI0] = L"MI0";
	color[MI1] = Color(255, 0, 200, 0);
	name[MI1] = L"MI1";
	color[MI2] = Color(255, 100, 100, 0);
	name[MI2] = L"MI2";
	color[MI3] = Color(255, 0, 100, 200);
	name[MI3] = L"MI3";

	color[STAB_SPEED_Z] = Color(255, 100, 100, 0);
	name[STAB_SPEED_Z] = L"Stab_SpZ";

	color[F_Z] = Color(255, 100, 100, 0);
	name[F_Z] = L"stab_FZ";

	color[STAB_Z] = Color(255, 100, 100, 0);
	name[STAB_Z] = L"stab_Z";




	color[STAB_Z] = Color(255, 100, 100, 0);
	name[STAB_Z] = L"stab_Z";


	color[G_SPEED_X] = Color(255, 255, 0, 0);
	name[G_SPEED_X] = L"G_SPEEDX";
	color[G_SPEED_Y] = Color(255, 0, 180, 255);
	name[G_SPEED_Y] = L"G_SPEEDY";

	color[G_SPEED] = Color(255, 0, 180, 255);
	name[G_SPEED] = L"G_SPEED_k/h";


	color[F0] = Color(255, 255, 0, 0);
	name[F0] = L"F0";
	color[F1] = Color(255, 0, 180, 255);
	name[F1] = L"F1";
	color[F2] = Color(255, 0, 0, 255);
	name[F2] = L"F2";
	color[F3] = Color(255, 255, 180, 0);
	name[F3] = L"F3";


	color[PRESSURE] = Color(255, 255, 0, 0);
	name[PRESSURE] = L"alt pres";

	color[PRESSURE_ACC] = Color(255, 255, 0, 100);
	name[PRESSURE_ACC] = L"acc pres";

	color[PRESSURE_SPEED] = Color(255, 255, 0, 100);
	name[PRESSURE_SPEED] = L"speed pres";

	color[SZ] = Color(255, 155, 100, 0);
	name[SZ] = L"SZ    ";
	color[GPS_Z] = Color(255, 50, 180, 200);
	name[GPS_Z] = L"GPS Z"


		;
	color[SPEED_Z] = Color(255, 255, 0, 180);
	name[SPEED_Z] = L"speedZ    ";




	color[THROTTLE] = Color(255, 120, 180, 120);
	name[THROTTLE] = L"THR";

	color[FILTER] = Color(127, 127, 127, 127);
	name[FILTER] = L"filter";
	flags[FILTER] = false;

	color[ROTATE] = Color(127, 127, 127, 127);
	name[ROTATE] = L"rotate";
	flags[ROTATE] = false;


	//--------------------------------------------------------------------------------------------------------------------------------------------

	color[EMU_ROLL] = Color(100, 255, 0, 0);
	name[EMU_ROLL] = L"e_roll";
	color[EMU_PITCH] = Color(100, 0, 200, 0);
	name[EMU_PITCH] = L"e_pitch";


	color[MAXACC] = Color(100, 0, 0, 0);
	name[MAXACC] = L"e_maxacc";




	color[PITCH] = Color(255, 0, 200, 0);
	name[PITCH] = L"pitch";
	color[R_PITCH] = Color(250, 0, 200, 100);
	name[R_PITCH] = L"r_pitch";

	color[ANGLE] = Color(250, 0, 200, 0);
	name[ANGLE] = L"angle";
	color[ACC] = Color(250, 255, 0, 0);
	name[ACC] = L"eACC";
	color[GACC] = Color(150, 0, 0, 200);
	name[GACC] = L"gacc";


	color[ROLL] = Color(255, 255, 0, 0);
	name[ROLL] = L"roll";
	color[R_ROLL] = Color(250, 255, 100, 0);
	name[R_ROLL] = L"r_roll";


	color[C_PITCH] = Color(180, 0, 200, 0);
	name[C_PITCH] = L"c_pitch";
	//color[TC_PITCH] = Color(100, 0, 200, 0);
	//name[TC_PITCH] = L"t_c_pitch";



	color[C_ROLL] = Color(180, 255, 0, 0);
	name[C_ROLL] = L"c_roll";
	//color[TC_ROLL] = Color(100, 255, 0, 0);
	//name[TC_ROLL] = L"t_c_roll";

	color[ACCX] = Color(255, 0, 100, 0);
	name[ACCX] = L"accX";
	color[ACCY] = Color(180, 100, 0, 0);
	name[ACCY] = L"accY";
	color[ACCZ] = Color(100, 0, 0, 100);
	name[ACCZ] = L"accZ";


	color[GACCX] = Color(255, 0, 100, 0);
	name[GACCX] = L"gaccX";
	color[GACCY] = Color(180, 100, 0, 0);
	name[GACCY] = L"gaccY";
	color[GACCZ] = Color(100, 0, 0, 100);
	name[GACCZ] = L"gaccZ";

	color[HEADING] = Color(100, 0, 0, 0);
	name[HEADING] = L"Heading";

	color[YAW] = Color(100, 0, 0, 200);
	name[YAW] = L"Yaw";

	color[GYRO_PITCH] = Color(255, 0, 200, 0);
	name[GYRO_PITCH] = L"gyro pitch";
	color[GYRO_ROLL] = Color(255, 255, 0, 0);
	name[GYRO_ROLL] = L"gyro roll";
	color[GYRO_YAW] = Color(255, 0, 0, 255);
	name[GYRO_YAW] = L"gyro yaw";

	color[SX] = Color(255, 0, 200, 0);
	name[SX] = L"s_x";


	color[SPEED_X] = Color(100, 200, 0, 0);
	name[SPEED_X] = L"speed_x";
	color[SPEED_Y] = Color(100, 200, 0, 200);
	name[SPEED_Y] = L"speed_y";

	color[EXP0] = Color(100, 200, 0, 0);
	name[EXP0] = L"exp0";
	color[EXP1] = Color(100, 200, 0, 200);
	name[EXP1] = L"exp1";

	color[EXP2] = Color(100, 200, 0, 0);
	name[EXP2] = L"exp2";
	color[EXP3] = Color(100, 200, 0, 200);
	name[EXP3] = L"exp3";



	color[M_C_PITCH] = Color(100, 200, 100, 0);
	name[M_C_PITCH] = L"mcpitch";

	color[SY] = Color(255, 255, 0, 0);
	name[SY] = L"s_y";




	color[M_C_ROLL] = Color(100, 200, 100, 200);
	name[M_C_ROLL] = L"wspeed_y";

	color[I_THROTHLE] = Color(255, 255, 0, 255);
	name[I_THROTHLE] = L"i_thr";
	color[I_YAW] = Color(255, 255, 0, 255);
	name[I_YAW] = L"i_yaw";
	color[I_PITCH] = Color(255, 255, 30, 255);
	name[I_PITCH] = L"i_pitch";
	color[I_ROLL] = Color(255, 0, 20, 200);
	name[I_ROLL] = L"i_roll";




	//	color[ROLL] =  Color(180,  255,    0,    0);
	//color[YAW] =   Color(100,    0,    0,  255);


}


Graph::~Graph()
{
}


int Graph::drawText(HDC hdc, RectF rect, double pos) {
	if (isnan(pos))
		return 0;
	Graphics g(hdc);
	g.Clear(Color(255, 255, 255, 255));
	int _p = lSize * pos;
	if (_p >= lSize)
		return -1;
	Pen pen(Color(255, 255, 0, 0));

	Font myFont(L"Arial", 14);

	StringFormat format;
	format.SetAlignment(StringAlignmentNear);

	SolidBrush blackBrush(Color(0, 0, 0));


	
	uint32_t mode = *(uint32_t*)&sensors_data[_p].sd[CONTROL_BITS];

	std::wstring mode_mask = (mode&MOTORS_ON) ? L"M ON, " : L"";
	mode_mask += (mode&PROGRAM) ? L"PROG, " : L"";
	mode_mask += (mode&XY_STAB) ? L"sXY ," : L"";
	mode_mask += (mode&Z_STAB) ? L"sZ ," : L"";
	mode_mask += (mode&GO2HOME) ? L"G2H ," : L"";
	mode_mask += (mode&COMPASS_ON) ? L"C_ON ," : L"";
	mode_mask += (mode&HORIZONT_ON) ? L"H_ON ," : L"";



	std::wstring mode_ = L"mode=" + std::to_wstring(mode);
	g.DrawString((WCHAR*)mode_.c_str(), mode_.size(), &myFont, rect, &format, &blackBrush);
	rect.Y += 20;
	g.DrawString((WCHAR*)mode_mask.c_str(), mode_mask.size(), &myFont, rect, &format, &blackBrush);
	rect.Y += 20;
	std::wstring time = L"time: " + std::to_wstring(sensors_data[_p].sd[TIME]);
	g.DrawString((WCHAR*)time.c_str(), time.size(), &myFont, rect, &format, &blackBrush);
	rect.Y += 20;
	std::wstring wpos = L"pos: " + std::to_wstring(_p);
	g.DrawString((WCHAR*)wpos.c_str(), wpos.size(), &myFont, rect, &format, &blackBrush);


	rect.Y += 20;
	for (int i = 0; i < ROTATE; i++) {
		if (flags[i]) {
			SolidBrush coloredBrush(color[i]);
			std::wstring text = name[i] + L" " + std::to_wstring(sensors_data[_p].sd[i]);
			g.DrawString((WCHAR*)text.c_str(), text.size(), &myFont, rect, &format, &coloredBrush);
			rect.Y += 20;
		}
	}
	return 0;
}



int Graph::drawGPSmarkder(HDC hdc, RectF rect, double pos) {

	Graphics g(hdc);
	int _p = lSize * pos;

	if (_p >= lSize)
		return 0;



	if (_p < gpsI)
		_p = gpsI;





	Pen pen(Color(255, 0, 0, 0));
	double mull_y = SOME_K * (double)rect.Height / (gps_log.max_y - gps_log.min_y);
	double mull_x = SOME_K * (double)rect.Width / (gps_log.max_x - gps_log.min_x);
	mull_x = mull_y = min(mull_x, mull_y);
	int ind = 1;
	int y = (sensors_data[_p].sd[Y] - gps_log.min_y) * mull_y;
	int x = (sensors_data[_p].sd[X] - gps_log.min_x) * mull_x;

	g.DrawLine(&pen, x - 10, y, x + 10, y);
	g.DrawLine(&pen, x, y - 10, x, y + 10);


	y = (0 - gps_log.min_y) * mull_y;
	x = (0 - gps_log.min_x) * mull_x;

	Pen penr(Color(255, 0, 0, 255));
	penr.SetWidth(3);
	g.DrawLine(&penr, x - 5, y, x + 5, y);
	g.DrawLine(&penr, x, y - 5, x, y + 5);







	if (flags[SX] || flags[SY]) {
		Pen pen(Color(255, 255, 0, 0));
		int y = (sensors_data[_p].sd[SY] - gps_log.min_y) * mull_y;
		int x = (sensors_data[_p].sd[SX] - gps_log.min_x) * mull_x;

		g.DrawLine(&pen, x - 10, y, x + 10, y);
		g.DrawLine(&pen, x, y - 10, x, y + 10);
	}



	return 0;
}



LONG old_time = 0;
int Graph::update(HDC hdc, RectF rect, double zoom, double pos) {/////////////////////////////////////////////////////////////////////////////////////

	Graphics g(hdc);

	p = lSize * pos;
	step = (double)(lSize - p)*zoom / rect.Width;

	//graphics.Clear(Color(255, 255, 255, 255));
	RectF r;

	r.X = 200;

	

#define MUL_4_ANG 45



	int y0 = (rect.Y + rect.Height / 2);
	double mull = (double)rect.Height / 2 / 45;
#




	
	drawModes(g, rect);
	draw(g, rect, mpu._max[mPITCH], mpu._min[mPITCH], PITCH);
	draw(g, rect, mpu._max[mROLL], mpu._min[mROLL], ROLL);
	draw(g, rect, mpu._max[mYAW], mpu._min[mYAW], YAW);
	draw(g, rect, 4, -4, ACCX);
	draw(g, rect, 4, -4, ACCY);
	draw(g, rect, 4, -4, ACCZ);
	draw(g, rect, 180, -180, GYRO_PITCH);
	draw(g, rect, 180, -180, GYRO_ROLL);


	draw(g, rect, mpu._max[mGYRO_PITCH], mpu._min[mGYRO_PITCH], GYRO_PITCH);
	draw(g, rect, 10, 0, MI0);
	draw(g, rect, 10, 0, MI1);
	draw(g, rect, 10, 0, MI2);
	draw(g, rect, 10, 0, MI3);
	draw(g, rect, 1680, 1440, BAT_F);

	draw(g, rect, 5, -5, STAB_SPEED_Z);
	draw(g, rect, 2, -2, F_Z);
	draw(g, rect, 23, 0,  STAB_Z);

	draw(g, rect, 1, 0, F0);
	draw(g, rect, 1, 0, F1);
	draw(g, rect, 1, 0, F2);
	draw(g, rect, 1, 0, F3);
	draw(g, rect, 1, 0, THROTTLE);
	draw(g, rect, 20, -20, C_PITCH);
	draw(g, rect, 20, -20, C_ROLL);
	draw(g, rect, 20, -20, HEADING);


	draw(g, rect, gps_log.min_z+2, gps_log.min_z, GPS_Z);
//	draw(g, rect, mpu._max[mMAXACC], mpu._min[mMAXACC], MAXACC);
	//draw(g, rect, mpu._max[mPITCH], mpu._min[mPITCH], PITCH);





	draw(g, rect, press.max_alt,  press.min_alt,PRESSURE);
	//draw(g, rect, press.max_a, press.min_a, PRESSURE_ACC);
	draw(g, rect, 5, -5, PRESSURE_SPEED);

	draw(g, rect, 5, -5, PRESSURE_ACC);



	return 0;
}
void Graph::drawModes(Graphics &g, RectF rect) {
	int y0 = (rect.Y + rect.Height / 2);
	float base = 0;

	float mull = 1;

	if (true) {
		int dy[2];
		int index = 0;



		for (int x = rect.X + 1, i = 1; x < rect.X + rect.Width; x++, i++) {
			int in = (int)((p)+(step*i));
			if (in >= lSize)
				return;
			
			
			

			uint32_t mode = *(uint32_t*)&sensors_data[in].sd[CONTROL_BITS];



			int pow = ((mode&MOTORS_ON) ? 100 : 50);
			int R = pow + ((mode&Z_STAB) ? 20 : 0) + ((mode&XY_STAB) ? 40 : 0) + ((mode&COMPASS_ON) ? 80 : 0);
			int gg = pow + ((mode&PROGRAM) ? 20 : 0) + ((mode&GO2HOME) ? 40 : 0);
			int B = pow + ((mode&HORIZONT_ON) ? 50 : 0);

			



			g.DrawLine(&Pen(Color(100, R, gg, B)), x - 1, (int)rect.Y, x, (int)(rect.Y+rect.Height));
			index++;
		}
	}
}


void Graph::draw(Graphics &g, RectF rect, float max, float min, int sdi) {
	//if (true)return;
	//void Graph::draw_ang(Graphics &g, Pen &pen, RectF rect, double zoom, double pos, int sdi) {

	int y0 = (rect.Y + rect.Height / 2);
	float base = min + (max - min) / 2;

	float mull = (double)rect.Height / (max - min);

	if (flags[sdi]) {
		int dy[2];
		int index = 0;


		dy[0] = y0 - (sensors_data[(int)(p)].sd[sdi] - base) * mull;
		if (dy[0] > rect.Height)
			dy[0] = rect.Height;
		if (dy[0] < rect.Y)
			dy[0] = rect.Y;
		for (int x = rect.X + 1, i = 1; x < rect.X + rect.Width; x++, i++) {
			int in = (int)((p)+(step*i));
			if (in >= lSize)
				return;
			dy[index & 1] = y0 - (sensors_data[in].sd[sdi] - base) * mull;
			if (dy[index & 1] > rect.Height + rect.Y)
				dy[index & 1] = rect.Height + rect.Y;
			if (dy[index & 1] < rect.Y)
				dy[index & 1] = rect.Y;
			g.DrawLine(&Pen(color[sdi]), x - 1, dy[(index - 1) & 1], x, dy[index & 1]);
			index++;
		}
	}

}