#include "stdafx.h"
#include "graph.h"
#include <stdint.h>
#include "KK.h"
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#include "MyMath.h"
//#define LOG_FILE_NAME "d:/tel_log10011.log"
char *fname;
enum LOG { MPU, HMC, MS5, GpS, COMM, STABXY, STABZ, BAL , EMU, AUTO, TELE, MPU_M, mMPU = 1, mHMC = 2, mMS5 = 4, mGpS = 8, mCOMM = 16, mSTABXY = 32, mSTABZ = 64, mBAL = 128
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




double cosYaw = 1, sinYaw = 0;

void rotate(double &sx, double &sy) {
	double x = (cosYaw*sx - sinYaw*sy);
	double y = (cosYaw*sy + sinYaw*sx);


	sx = x;
	sy = y;
}

void rotateR(double &sx, double &sy, float yaw) {

	double cosYaw = cos(yaw);
	double sinYaw = sin(yaw);
	double x = (cosYaw*sx + sinYaw*sy);
	double y = (cosYaw*sy - sinYaw*sx);


	sx = x;
	sy = y;
}





float maxAngle = 35;
float c_pitch, c_roll;

float total_ax = 0, total_ay = 0;
float speedX = 0, speedY = 0;

inline void sin_cos(const float a, float &s, float &c) {
	s = (float)sin(a);
	c = (float)cos(a);
	/*
	const double ss = s*s;
	c = (float)sqrt(1 - min(1.0f, ss));
	//30.7.2017 corected
	if (abs(a) > 90)
	c = -c;
	*/
}

#define NEED_ANGLE_4_SPEED_10_MS 20.0f
float cS = (float)tan(NEED_ANGLE_4_SPEED_10_MS * GRAD2RAD)*0.02f;








//сделать чтоби нераскачивлся а именно попробовать усиливать при низкой мощности. типпа k=0.5/0.4 а пото перемножать питч рол и яв


#define DRAG_K 0.0052
//#define DRAG_K 6.2353829072479582566988068294211e-4






void Graph::do_magic() {
	sin_cos(yaw*GRAD2RAD, _sinYaw, _cosYaw);
	//---calc acceleration on angels------
	sin_cos(pitch*GRAD2RAD, sinPitch, cosPitch);
	sin_cos(roll*GRAD2RAD, sinRoll, cosRoll);

#define WIND_SPEED_X sqrt(abs(w_accX / DRAG_K))*((w_accX>=0)?1:-1)
#define WIND_SPEED_Y sqrt(abs(w_accY / DRAG_K))*((w_accY>=0)?1:-1)

//	float windX = e_speedX;/// +WIND_SPEED_X;
//	float windY = e_speedY;// +WIND_SPEED_Y;


	e_accX = -G*(-_cosYaw*sinPitch - _sinYaw*sinRoll) - e_speedX*abs(e_speedX)*DRAG_K-w_accX;
	e_accY = G*(-_cosYaw*sinRoll + _sinYaw*sinPitch) - e_speedY*abs(e_speedY)*DRAG_K-w_accY;

	w_accX += (e_accX - gax - w_accX)*0.01;
	w_accY += (e_accY - gay - w_accY)*0.01;

	e_speedX += e_accX*dt;
	e_speedX += (gspeedX - e_speedX)*0.1;

	e_speedY += e_accY*dt;
	e_speedY += (gspeedY - e_speedY)*0.1;

	//-----calc real angels------
	m7_accX += ((_cosYaw*e_accX + _sinYaw*e_accY) - m7_accX)*0.007;
	m7_accX = constrain(m7_accX, -MAX_ACC / 2, MAX_ACC / 2);
	m7_accY += ((_cosYaw*e_accY - _sinYaw*e_accX) - m7_accY)*0.007;
	m7_accY = constrain(m7_accY, -MAX_ACC / 2, MAX_ACC / 2);

	f_pitch = pitch;
	f_roll = roll;

	pitch = RAD2GRAD*atan2((sinPitch + m7_accX*cosPitch / G), cosPitch);// +abs(gaccX*sinPitch));
	roll = RAD2GRAD*atan2((sinRoll - m7_accY*cosRoll / G), cosRoll);// +abs(gaccY*sinRoll));

}












void Graph::correct_c_pitch_c_roll(int p) {


	float _cosYaw = cos(GRAD2RAD*sensors_data[p].sd[YAW]);
	float _sinYaw = sin(GRAD2RAD*sensors_data[p].sd[YAW]);
	float dt = sensors_data[p].sd[DT];


	//c_pitch = constrain(sensors_data[p].sd[TC_PITCH], -maxAngle, maxAngle);
//	c_roll = constrain(sensors_data[p].sd[TC_ROLL], -maxAngle, maxAngle);
	const float maxAngle07 = maxAngle*0.7f;
	if (abs(c_pitch) > maxAngle07 || abs(c_roll) > maxAngle07) {
		//	c_pitch = constrain(c_pitch, -maxAngle, maxAngle);
		//c_roll = constrain(c_roll, -maxAngle, maxAngle);
		float k = (float)(RAD2GRAD*acos(cos(c_pitch*GRAD2RAD)*cos(c_roll*GRAD2RAD)));
		if (k == 0)
			k = maxAngle;
		if (k > maxAngle) {
			k = maxAngle / k;
			c_pitch *= k;
			c_roll *= k;
		}
	}

	if (true) {
		float c_cosPitch, c_sinPitch, c_cosRoll, c_sinRoll;
		sin_cos(c_pitch*GRAD2RAD, c_sinPitch, c_cosPitch);
		sin_cos(c_roll*GRAD2RAD, c_sinRoll, c_cosRoll);
		if (c_cosRoll == 0 || c_cosPitch == 0)
			return;

#ifndef MOTORS_OFF
		speedY = sensors_data[p].sd[SPEED_Y];
		speedX = sensors_data[p].sd[SPEED_X];

		float rspeedX = _cosYaw*speedX - _sinYaw*speedY;
		float rspeedY = _cosYaw*speedY + _sinYaw*speedX;

#define CF 0.007f

		//c_pitch
		float break_fx = 0.5f*abs(rspeedX)*rspeedX*(cS + cS*abs(c_sinPitch));
		float force_ax = c_sinPitch / c_cosPitch - break_fx;
		total_ax += ((force_ax*c_cosPitch) - total_ax)*CF;

		const float false_pitch = RAD2GRAD*(float)atan((total_ax - c_sinPitch) / c_cosPitch);
		c_pitch = -false_pitch;

		//c_roll
		float break_fy = 0.5f*abs(rspeedY)*rspeedY*(cS + cS*abs(c_sinRoll));
		float force_ay = c_sinRoll / c_cosRoll - break_fy;
		total_ay += ((force_ay*c_cosRoll) - total_ay)*CF;

		const float false_roll = RAD2GRAD*(float)atan((total_ay - c_sinRoll) / c_cosRoll);
		c_roll = -false_roll;



		
#endif

	}
	

	c_pitch = constrain(c_pitch, -maxAngle, maxAngle);
	c_roll = constrain(c_roll, -maxAngle, maxAngle);

		sensors_data[p].sd[M_C_PITCH] = c_pitch;
		sensors_data[p].sd[M_C_ROLL] = c_roll;

}










float startZ = 0;
void Graph::filter(float src, int dataI, int elementi,float max) {
	
	if (isnan(src) || abs(src) > max)
		src = 0;
	sensors_data[dataI].sd[elementi] = (flags[FILTER] && kalman[elementi].initialized == 77777) ? kalman[elementi].update(src):src;
}

#define wrap_180(x) (x < -180 ? x+360 : (x > 180 ? x - 360: x))
float old_speedX = 0, old_speedY = 0;
boolean new_mode_ver = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int indexes[] = { 0,0,0,0,0,0,0,0,0,0,0 };

int Graph::decode_Log() {

	
	//maccY=  MYfILTER(0.14);

	pitch = roll = sinPitch = sinRoll = yaw=_sinYaw= 0;
	cosPitch = cosRoll = _cosYaw =1;
	e_speedX = e_speedY = e_accX = e_accY = m7_accX = m7_accY = f_pitch = f_roll = w_accX = w_accY = 0;


	gspeedX = gspeedY = 0;






	




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
	
	long i = 0;
	log_bank_size = *(uint16_t*)buffer;
	i += 2;
	bool starPointOK = false;
	gax = gay = gaz = 0;
	if (indexes[LOG::MPU] == 0) {
		do {

			switch (buffer[i]) {
			case LOG::MPU: {
				indexes[LOG::MPU]++;

				dt = (double)buffer[i + 1] * 0.001f;
				pitch = *(float*)(&buffer[i + 2]);
				roll = *(float*)(&buffer[i + 6]);
				float к_pitch = *(float*)(&buffer[i + 10]);
				float к_roll = *(float*)(&buffer[i + 14]);


				yaw = *(float*)(&buffer[i + 18]);
				float gyroPitch = *(float*)(&buffer[i + 22]);
				float gyroRoll = *(float*)(&buffer[i + 26]);
				float gyroYaw = *(float*)(&buffer[i + 30]);
				float accX = *(float*)(&buffer[i + 34]);
				float accY = *(float*)(&buffer[i + 38]);
				float accZ = *(float*)(&buffer[i + 42]);
				i += 46;
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
				indexes[LOG::MS5]++;

				float pressur = *(float*)(&buffer[i + 1]);
				i += 5;
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

				float sX = *(float*)(&buffer[i + 1]);
				float speedX = *(float*)(&buffer[i + 5]);
				float sY = *(float*)(&buffer[i + 9]);
				float speedY = *(float*)(&buffer[i + 13]);

				i += 17;
				break;
			}
			case LOG::STABZ: {
				indexes[LOG::STABZ]++;
				float sZ = *(float*)(&buffer[i + 1]);
				float speedZ = *(float*)(&buffer[i + 5]);
				i += 9;
				break;
			}

			case  LOG::BAL: {
				indexes[LOG::BAL]++;

				float t_c_pitch = *(float*)(&buffer[i + 1]);
				float t_c_roll = *(float*)(&buffer[i + 5]);
				float c_pitch = *(float*)(&buffer[i + 9]);
				float c_roll = *(float*)(&buffer[i + 13]);
				float throttle = *(float*)(&buffer[i + 17]);

				//	time_index = *(int*)(&buffer[i + 23]);
				float f0 = *(float*)(&buffer[i + 27]);
				float f1 = *(float*)(&buffer[i + 31]);
				float f2 = *(float*)(&buffer[i + 35]);
				float f3 = *(float*)(&buffer[i + 39]);

				i += 43;
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
	


	max_x =  -20000000000000;
	min_x =  20000000000000;
	max_y =  -20000000000000;
	min_y =  20000000000000;



	modes[modesI].mode = def_mode;
	modes[modesI].index = dataI;
	modesI++;

	float accXC = 0, accYC = 0, accZC = 0;
	float gpitch = 0, groll = 0, gyaw = 0;
	

	

	float f[] = { 0.5, 0.5, 0.5, 0.5 };
	float thr = 0.5;
	float pressure_alt=100000;
	float pressure_speed=0;
	float pressure_acc = 0;
	float start_alt = -1;
	float start_gps_alt = -1;
	float time = 0;





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
	kalman[PRESSURE] = Kalman(30, 0);
	kalman[PRESSURE_SPEED] = Kalman(30, 0);
	kalman[PRESSURE_ACC] = Kalman(300, 0);


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


	FILE *klm = fopen("d:/klm.txt", "w");
	lat = lon =gx=gy=0;
	do {

		switch (buffer[i]) {
		
		case LOG::MPU: {
			
			dt = (float)buffer[i+1] * 0.001;
			time += dt;
			gdt += dt;
			press_dt += dt;
			sensors_data[dataI].sd[TIME] = time;
			sensors_data[dataI].sd[DT] = dt;
			
			pitch = *(float*)(&buffer[i + 2]);
			roll = *(float*)(&buffer[i + 6]);
			//yaw = (*(float*)(&buffer[i + 18]));
			float rpitch, rroll;
			filter(*(float*)(&buffer[i +2]), dataI, PITCH);
			filter(*(float*)(&buffer[i+6]), dataI, ROLL);
			filter(rpitch=*(float*)(&buffer[i +10]), dataI, R_PITCH);
			filter(rroll=*(float*)(&buffer[i +14]), dataI, R_ROLL);
			filter(acos(cos(rpitch*GRAD2RAD)*cos(rroll*GRAD2RAD))*RAD2GRAD, dataI, ANGLE);

			float rPitch = *(float*)(&buffer[i + 10]);
			float rRoll = *(float*)(&buffer[i + 14]);
		//	filter(acos(cos(rPitch*GRAD2RAD)*cos(rRoll*GRAD2RAD))*RAD2GRAD, dataI, PITCH);

			

			float yaw;
			filter(yaw= (*(float*)(&buffer[i +18])), dataI, YAW);
			float accX, accY;


			//sensors_data[dataI].sd[GYRO_ROLL]=mgroll.update(*(float*)(&buffer[i + 26]));
			//sensors_data[dataI].sd[GYRO_PITCH] = *(float*)(&buffer[i + 26]);

			filter(*(float*)(&buffer[i +22]), dataI, GYRO_PITCH);
			filter(*(float*)(&buffer[i +26]), dataI, GYRO_ROLL);



			filter(*(float*)(&buffer[i +30]), dataI, GYRO_YAW);
			accX = *(float*)(&buffer[i + 34]);
			accY = *(float*)(&buffer[i + 38]);


			filter(accX, dataI, ACCX);
			filter(accY, dataI, ACCY);
			//sensors_data[dataI].sd[ACCX] = mccX.update(accX);
			filter(accX, dataI, ACCY);



			filter(*(float*)(&buffer[i +42]), dataI, ACCZ);
			



			i += 46;
			if (flags[ROTATE]) {
				cosYaw = cos(yaw*GRAD2RAD);
				sinYaw = sin(yaw*GRAD2RAD);
			}
			else {
				cosYaw = 1;
				sinYaw = 0;
			}
			//------------------------------------------------------------------
//#define _MPU_M

#ifndef _MPU_M	
			do_magic();

			filter(e_speedX, dataI, EXP0);
			filter(e_speedY, dataI, EXP1);
			filter(e_accX, dataI, EXP2);
			filter(e_accY, dataI, EXP3);
			
			filter(pitch, dataI, R_PITCH);
			filter(roll, dataI, R_ROLL);
#endif
		//	filter(-spx, dataI, EXP0);
			//filter(-spy, dataI, EXP1);


			//rotate(ax, ay);filter(-ax, dataI, EXP2);filter(-ay, dataI, EXP3);

		//	rotate(expAX, expAY);filter(expAX, dataI, EXP2);filter(expAY, dataI, EXP3);

			//------------------------------------------------------------------
			
			break;
		}
		case LOG::MPU_M: {

#ifdef _MPU_M
			e_accX= -*(float*)(&buffer[i + 1]);
			e_accY =- *(float*)(&buffer[i + 5]);
			e_speedX= *(float*)(&buffer[i + 9]);
			//e_speedX = sqrt(abs(e_speedX) / 0.022)*(e_speedX>=0?1:-1);
			e_speedY = *(float*)(&buffer[i + 13]);
			//e_speedY = sqrt(abs(e_speedY) / 0.022)*(e_speedY >= 0 ? 1 : -1);
		//	e_accX += e_speedX;
		//	e_accY += e_speedY;
			filter(e_speedX, dataI, EXP0);
			filter(e_speedY, dataI, EXP1);
			filter(e_accX, dataI, EXP2);
			filter(e_accY, dataI, EXP3);
		//	filter(sqrt(e_accX*e_accX + e_accY*e_accY), dataI, ACC);
#endif
			
			i += 17;
			break;
		}
		case LOG::TELE: {
			
			mi[0] = *(uint16_t*)(&buffer[i + 1]);
			mi[1]= *(uint16_t*)(&buffer[i + 3]);
			mi[2] = *(uint16_t*)(&buffer[i + 5]);
			mi[3] = *(uint16_t*)(&buffer[i + 7]);
			bat= *(uint16_t*)(&buffer[i + 9]);
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
			head= *(float*)(&buffer[i + 13]);
			head *= RAD2GRAD;
		
			i += 17;
			break;
		case LOG::MS5: {

			if (dataI > 24802)
				i = i;
			float pressure = *(float*)(&buffer[i + 1]);
#define PRESSURE_AT_0 101325
			double tpressure_alt = (44330.0f * (1.0f - pow((double)pressure / PRESSURE_AT_0, 0.1902949f)));
			if (start_alt == -1 && def_mode&MOTORS_ON)
				start_alt = tpressure_alt;
			tpressure_alt -= start_alt;

			float tpressure_speed = (tpressure_alt - pressure_alt) / press_dt;
			pressure_alt = tpressure_alt;
			pressure_acc = (tpressure_speed - pressure_speed) / press_dt;
			pressure_speed = tpressure_speed;



			press_dt = 0;

			i += 5;
			break;
		}
		case LOG::GpS: {

			//old_z = 0, gspeedZ, old_gspeeZ = 0


			SEND_I2C*p = (SEND_I2C*)&buffer[i+1];
			z = 0.001*p->height;

			gspeedZ = (z - old_z) / 0.1;
			old_z = z;
			gaz = (gspeedZ - old_gspeeZ) / 0.1;
			old_gspeeZ =gspeedZ;

			if (dataI > 24463)
				i = i;

			double n_lat = p->lat*0.0000001*GRAD2RAD;
			double n_lon = p->lon*0.0000001*GRAD2RAD;

			if (lat == 0 || lon == 0) {
				lat = n_lat;
				lon = n_lon;
			}

			double distance = mymath.distance_(n_lat, n_lon, lat, lon);
			double bearing = mymath.bearing_(n_lat, n_lon, lat, lon);

			gy = distance*sin(bearing);
			gspeedY = (gy)/0.1;
			

			gx = distance*cos(bearing);
			gspeedX = (gx) / 0.1;
			

			lat = n_lat;
			lon = n_lon;






			if (startZ == 0 && def_mode&MOTORS_ON)
				startZ = z;
			z -= startZ;
			i += 1 + sizeof(SEND_I2C);
			gx2home = *(float*)(&buffer[i]);
			gy2home = *(float*)(&buffer[i+4]);
			gdX= *(float*)(&buffer[i + 8]);
			gdY = *(float*)(&buffer[i + 12]);
		//	gspeedX = *(float*)(&buffer[i + 16]);
		//	gspeedY = *(float*)(&buffer[i + 20]);
			gspeed = sqrt(gspeedX*gspeedX + gspeedY*gspeedY)*3.6;
			gax= *(float*)(&buffer[i + 24]);
			gay = *(float*)(&buffer[i + 28]);
			int vacc = p->vAcc;
			int hacc = p->hAcc;

			if (dataI > 44467)
			{
				std::wstring str = L" " + std::to_wstring(p->lon*0.0000001) + L"," + std::to_wstring(p->lat * 0.0000001) + L"," + std::to_wstring(p->height * 0.001) + L" ";
				fwrite(str.c_str(), str.length(), 2, klm);
			}

			/*

			double angle = GRAD2RAD*(180-((dataI / 4) % 360));
			gax =  sin(angle) * 1;



	

			
			double tttt = (1 - 2 * (double)rand() / RAND_MAX);
			gax += constrain(tttt,-2,2);

			*/
	//	gay = gax;	
		//gay = maccY.update(gay);
		//	gax=maccX.update(gax);

			//gay= *(float*)(&buffer[i + 28]);


			//double ang=atan2(gax, gay);


		
			//rotate(gspeedX, gspeedY);
			
			//rotate(gax, gay);
			i += 8 * 4;



			max_x = max(max_x, gx2home);
			min_x = min(min_x, gx2home);
			max_y = max(max_y, gy2home);
			min_y = min(min_y, gy2home);

			break;
		}

		case LOG::COMM: {

			//int mode_mask = *(uint32_t*)(&buffer[i + 3]);
			//mode_mask &= 0x00ffffff;
			int16_t t = *(int16_t*)(&buffer[i + 7]);
			i_throthle = 0.00003125f*(double)t;
			t = *(int16_t*)(&buffer[i + 9]);
			if (def_mode&(COMPASS_ON |  GO2HOME) == COMPASS_ON)
				i_yaw = ANGK*(double)t;
			else
				i_yaw = 0;
			t = *(int16_t*)(&buffer[i + 11]);
			i_yaw_offset = ANGK*(double)t;
			t = *(int16_t*)(&buffer[i + 13]);


			if ((def_mode&(HORIZONT_ON |   GO2HOME)) == HORIZONT_ON) {
				i_pitch = ANGK*(double)t;
				t = *(int16_t*)(&buffer[i + 15]);
				i_roll = ANGK*(double)t;
			}
			else
				i_pitch = i_roll = 0;

			//отключить со следующие версии.
		/*	if (new_mode_ver==false)
				if (mode_mask) {
					def_mode ^= mode_mask;
					modes[modesI].mode = def_mode;
					modes[modesI].index = dataI;
					modesI++;
				}
				*/
			unsigned short len = *(unsigned short*)(buffer + i + 1);
			i += len + 3;
			break;
		}
		case LOG::STABXY: {
			float speedX, speedY;

//filter(gx2home,dataI,SX);
//filter(gy2home,dataI,SY);


			filter(*(float*)(&buffer[i + 1]),dataI,SX);
			filter(speedX=*(float*)(&buffer[i + 5]),dataI,SPEED_X);
			filter(*(float*)(&buffer[i + 9]),dataI,SY);
			filter(speedY=*(float*)(&buffer[i + 13]),dataI,SPEED_Y);












			rotate(sensors_data[dataI].sd[SX], sensors_data[dataI].sd[SY]);
			rotate(sensors_data[dataI].sd[SPEED_X], sensors_data[dataI].sd[SPEED_Y]);
		//	gax = (speedX - old_speedX) / dt;
			old_speedX = speedX;


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
				filter(i_throthle, dataI, I_THROTHLE);

				sensors_data[dataI].sd[I_YAW] = i_yaw;
				sensors_data[dataI].sd[I_YAW_OFFSET] = i_yaw_offset;
				filter(i_pitch, dataI, I_PITCH);
				filter(i_roll, dataI, I_ROLL);
				//sensors_data[dataI].sd[HEADING] = head;
				filter(head, dataI, HEADING);
				if (pressure_alt!=100000)
					filter(pressure_alt, dataI, PRESSURE);
				else
					filter(0, dataI, PRESSURE);

				filter(pressure_acc, dataI, PRESSURE_ACC);

				filter(pressure_speed, dataI, PRESSURE_SPEED);

			

				sensors_data[dataI].sd[X] = gx2home;
				sensors_data[dataI].sd[Y] = gy2home;
				sensors_data[dataI].sd[GPS_Z] = z;

				
				filter((float)bat *0.01725, dataI, BAT_F);



				filter(0.01953125 * (1024.0 - mi[0]), dataI, MI0);
				filter(0.01953125 * (1024.0 - mi[1]), dataI, MI1);
				filter(0.01953125 * (1024.0 - mi[2]), dataI, MI2);
				filter(0.01953125 * (1024.0 - mi[3]), dataI, MI3);
	




			//	gax = ((dataI % 200) > 99) ? 2 : -2;

			//	gax += ((dataI % 10) > 4) ? 2 : -2;


/*

				bufkk0[bufff0I & 127] = -gax;
				int shift = 4;
				if (bufff0I >= shift) {

					filter(bufkk0[(bufff0I - shift) & 127], dataI, GACCY);

				}else
					filter(0, dataI, GACCY);
				
				
				//filter(-gax, dataI, GACCY);
				float res=0;
				

				//if (dataI<10)
				//	res=kk0.update(gax, 0.01);
				//else
				//	res = kk0.update(0, 0.01);

			//	bufkk0[bufff0I&127]=kk0.update(-gax, 0.01);
				
				float kk = 0.16;
				res += kk0.update(-gax*kk, 0.01);
				res += kk1.update(-gax*kk, 0.01);
				res += kk2.update(-gax*kk, 0.01);
				res += kk3.update(-gax*kk, 0.01);
				res += kk4.update(-gax*kk, 0.01);
				res += kk5.update(-gax*kk, 0.01);
				*/
			//	sensors_data[dataI].sd[GACCX] = res;// +kk0.update(-gax, 0.01) - bufkk0[(bufff0I - shift) & 127];
			//	filter(sensors_data[dataI].sd[GACCX], dataI, GACCX);


			//	bufff0I++;


				//filter(res, dataI, GACCX);

				//tgaccx += (gax - tgaccx)*0.1;
				//sensors_data[dataI].sd[GACCX] = tgaccx;
				filter(gax, dataI, GACCX);
				filter(gay, dataI, GACCY);

				



				//filter(sqrt(gax*gax + gay*gay), dataI, GACC);

				filter(gaz, dataI, GACCZ);

				filter(gspeedX, dataI, G_SPEED_X);
				filter(gspeedY, dataI, G_SPEED_Y);
				filter(gspeed, dataI, G_SPEED);



				float c_pitch = *(float*)(&buffer[i + 1]);
				float c_roll = *(float*)(&buffer[i + 5]);

				float maxAngle = 35;
				c_pitch = constrain(c_pitch, -maxAngle, maxAngle);
				c_roll = constrain(c_roll, -maxAngle, maxAngle);
				const float maxAngle07 = maxAngle*0.7f;
				if (abs(c_pitch) > maxAngle07 || abs(c_roll) > maxAngle07) {

					float k = (float)(RAD2GRAD*acos(cos(c_pitch*GRAD2RAD)*cos(c_roll*GRAD2RAD)));
					if (k == 0)
						k = maxAngle;
					if (k > maxAngle) {
						k = maxAngle / k;
						c_pitch *= k;
						c_roll *= k;
					}
				}

			//	filter(c_pitch, dataI, TC_PITCH);
			//	filter(c_roll, dataI, TC_ROLL);
				mc_pitch += (*(float*)(&buffer[i + 9]) - mc_pitch)*1;

				mc_roll += (*(float*)(&buffer[i + 13]) - mc_roll)*1;


				filter(mc_pitch, dataI, C_PITCH);
				filter(mc_roll, dataI, C_ROLL);

				filter(*(float*)(&buffer[i + 17]), dataI, THROTTLE);

				filter(*(float*)(&buffer[i + 27]), dataI, F0);
				filter(*(float*)(&buffer[i + 31]), dataI, F1);
				filter(*(float*)(&buffer[i + 35]), dataI, F2);
				filter(*(float*)(&buffer[i + 39]), dataI, F3);

				correct_c_pitch_c_roll(dataI);
				//filter(sensors_data[dataI].sd[M_C_PITCH], dataI, M_C_PITCH);
				//filter(sensors_data[dataI].sd[M_C_ROLL], dataI, M_C_ROLL);

				//rotate(sensors_data[dataI].sd[M_C_PITCH], sensors_data[dataI].sd[M_C_ROLL]);
				//	rotate(sensors_data[dataI].sd[M_C_ROLL], sensors_data[dataI].sd[M_C_ROLL]);



				




				i += 43;
				i += 2;

				dataI++;
				break;
		}




		case LOG::EMU: {
			filter(*(float*)(&buffer[i + 2]), dataI, EMU_PITCH);
			filter(*(float*)(&buffer[i + 6]), dataI, EMU_ROLL);
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
	
	
	return 0;
}




int Graph::readLog(){

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

	color[PITCH] = Color(255, 0,200, 0);
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
	int _p = lSize*pos;
	if (_p >= lSize)
		return -1;
	Pen pen(Color(255, 255, 0, 0));

	Font myFont(L"Arial", 14);

	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	
	SolidBrush blackBrush(Color(0,0,0));


	int mode = -1;
	for (int i = 0; i < modesI; i++) {
		if (_p > modes[i].index)
			mode = modes[i].mode;
	}
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
	g.DrawString((WCHAR*)time.c_str(),time.size(),&myFont,rect,&format,&blackBrush);
	rect.Y += 20;
	std::wstring wpos = L"pos: " + std::to_wstring(_p);
	g.DrawString((WCHAR*)wpos.c_str(), wpos.size(), &myFont, rect, &format, &blackBrush);


	rect.Y += 20;
	for (int i = 0; i < ROTATE; i++) {
		if ( flags[i]) {
			SolidBrush coloredBrush(color[i]);
			std::wstring text = name[i] + L" " + std::to_wstring(sensors_data[_p].sd[i]);
			g.DrawString((WCHAR*)text.c_str(), text.size(), &myFont, rect, &format, &coloredBrush);
			rect.Y += 20;
		}
	}
	return 0;
}


#define SOME_K 0.9
int Graph::drawGPSmarkder(HDC hdc, RectF rect, double pos) {

	Graphics g(hdc);
	int _p = lSize*pos;

	if (_p >= lSize)
		return 0;



	if (_p < gpsI)
		_p = gpsI;




	
	Pen pen(Color(255, 0, 0, 0));
	double mull_y = SOME_K* (double)rect.Height / (max_y - min_y);
	double mull_x = SOME_K*(double)rect.Width / (max_x - min_x);
	mull_x = mull_y = min(mull_x, mull_y);
	int ind = 1;
	int y = (sensors_data[_p].sd[Y] - min_y) * mull_y;
	int x = (sensors_data[_p].sd[X] - min_x) * mull_x;

	g.DrawLine(&pen, x - 10, y, x + 10, y);
	g.DrawLine(&pen, x, y - 10, x, y + 10);


	y = (startPointY - min_y) * mull_y;
	x = (startPointX - min_x) * mull_x;

	Pen penr(Color(255, 0, 0, 255));
	penr.SetWidth(3);
	g.DrawLine(&penr, x - 5, y, x + 5, y);
	g.DrawLine(&penr, x, y - 5, x, y + 5);






	
	if (flags[SX] || flags[SY]) {
		Pen pen(Color(255, 255, 0, 0));
		int y = (sensors_data[_p].sd[SY] - min_y) * mull_y;
		int x = (sensors_data[_p].sd[SX] - min_x) * mull_x;

		g.DrawLine(&pen, x - 10, y, x + 10, y);
		g.DrawLine(&pen, x, y - 10, x, y + 10);
	}



	return 0;
}

int Graph::updateGPS(HDC hdc, RectF rect, double zoom, double pos) {
	Graphics g(hdc);

	g.Clear(Color(255, 255, 255, 255));

	p = lSize*pos;
	step = (double)(lSize - p)*zoom / rect.Width;
	double mull_y = SOME_K* (double)rect.Height / (max_y - min_y);
	double mull_x = SOME_K*(double)rect.Width / (max_x - min_x);
	mull_x = mull_y = min(mull_x, mull_y);
	int startX, startY;
	{
		int dy[2], dx[2];
		int y0 = 0;
		int x0 = 0;

		
		
		int ind = 1;

		if (p < gpsI)
			p = gpsI;
		startY=dy[0] = y0 + (sensors_data[(int)p].sd[Y] - min_y) * mull_y;
		startX=dx[0] = x0 + (sensors_data[(int)p].sd[X] - min_x) * mull_x;


		Pen pen(Color(255, 0, 0, 0));
		for (int x = rect.X + 1, i = 1; x < rect.X + rect.Width; x++, i++) {
			dy[ind & 1] = y0 + (sensors_data[(int)(p + (step*i))].sd[Y] - min_y) * mull_y;
			dx[ind & 1] = x0 + (sensors_data[(int)(p + (step*i))].sd[X] - min_x) * mull_x;

			g.DrawLine(&pen, dx[(ind - 1) & 1], dy[(ind - 1) & 1], dx[ind & 1], dy[ind & 1]);
			ind++;


		}
	}
	if (flags[SX] || flags[SY]) {
		int dy[2], dx[2];
		int y0 = 0;
		int x0 = 0;

		
		int ind = 1;

		dy[0] = y0 + (sensors_data[(int)p].sd[SY] - min_y) * mull_y;
		dx[0] = x0 + (sensors_data[(int)p].sd[SX] - min_x) * mull_x;

		startX - dx[0];
		startY - dy[0];

	
		Pen pen(Color(255, 255, 0, 0));
		for (int x = rect.X + 1, i = 1; x < rect.X + rect.Width; x++, i++) {
			dy[ind & 1] = y0 + (sensors_data[(int)(p + (step*i))].sd[SY] - min_y) * mull_y;
			dx[ind & 1] = x0 + (sensors_data[(int)(p + (step*i))].sd[SX] - min_x) * mull_x;

			g.DrawLine(&pen, dx[(ind - 1) & 1], dy[(ind - 1) & 1], dx[ind & 1], dy[ind & 1]);
			ind++;


		}
	}



	return 0;
}





LONG old_time = 0;
int Graph::update(HDC hdc, RectF rect,double zoom, double pos) {/////////////////////////////////////////////////////////////////////////////////////

	Graphics g(hdc);

	p = lSize*pos;
	step = (double)(lSize - p)*zoom / rect.Width;

	//graphics.Clear(Color(255, 255, 255, 255));
	RectF r;

	r.X = 200;
	
	for (int i = 0; i < modesI; i++) {
		int pow = ((modes[i].mode&MOTORS_ON) ? 100 : 50);
		int R = pow +  ((modes[i].mode&Z_STAB)?20 : 0) + ((modes[i].mode&XY_STAB)?40 : 0) + ((modes[i].mode&COMPASS_ON)?80:0);
		int gg = pow + ((modes[i].mode&PROGRAM) ? 20 : 0) + ((modes[i].mode&GO2HOME) ? 40 : 0);
		int B = pow + ((modes[i].mode&HORIZONT_ON)?50:0);





			SolidBrush coloredBrush(Color(100,R, gg, B));
			double t0 = (modes[i].index - p)/ step;
			double t1 = (i+1==modesI)?rect.Width:(modes[i+1].index - p) / step;
			if (t1 < 0)
				continue;

			if (t0 < 0) {
				r.X = rect.X;
				t1 += t0;
			}else
				r.X=rect.X + t0;
			
			r.Y = rect.Y + 2;
			r.Width = t1 - t0;

			if (r.Width + r.X > rect.X + rect.Width) {
				r.Width = rect.X + rect.Width - r.X;
			}
			r.Height = rect.Height - 4;
			g.FillRectangle(&coloredBrush, r);
			
	}
	
#define MUL_4_ANG 45
	


	int y0 = (rect.Y + rect.Height / 2);
	double mull = (double)rect.Height / 2 / 45;

	draw(g, rect, y0,mull, PITCH);
	draw(g,  rect, y0, mull, ROLL);
	draw(g, rect, y0, mull, R_PITCH);
	draw(g, rect, y0, mull, R_ROLL);
	draw(g, rect, y0, mull, ANGLE);

	draw(g,  rect, y0, mull, C_PITCH);
	//draw(g,  rect, y0, mull, TC_PITCH);
	draw(g, rect, y0, mull, EMU_PITCH);
	draw(g, rect, y0, mull, EMU_ROLL);

	
	draw(g,  rect, y0, mull, C_ROLL);
	//draw(g,  rect, y0, mull, TC_ROLL);

	mull = (double)rect.Height / 2 / 5;





	draw(g,  rect, y0, mull, ACCX);
	draw(g,  rect, y0, mull, ACCY);
	draw(g,  rect, y0, mull, ACCZ);
	draw(g, rect, y0, mull, GACCX);
	draw(g, rect, y0, mull, GACCY);
	draw(g, rect, y0, mull, GACCZ);
	draw(g, rect, y0, mull, EXP2);
	draw(g, rect, y0, mull, EXP3);
	draw(g, rect, y0, mull, ACC);
	draw(g, rect, y0, mull, GACC);


	mull= (double)rect.Height / 2 / 360;
	draw(g,  rect, y0, mull, HEADING);
	draw(g,  rect, y0, mull, YAW);

	mull = (double)rect.Height / 2 / 300;
	draw(g,  rect, y0, mull, GYRO_PITCH);
	draw(g,  rect, y0, mull, GYRO_ROLL);
	draw(g,  rect, y0, mull, GYRO_YAW);

	mull = (double)rect.Height / 2 / 20;
	draw(g,  rect, y0, mull, SPEED_X);
	draw(g,  rect, y0, mull, SPEED_Y);
	draw(g, rect, y0, mull, G_SPEED_X);
	draw(g, rect, y0, mull, G_SPEED_Y);

	draw(g, rect, y0, mull, EXP0);
	draw(g, rect, y0, mull, EXP1);



	mull = (double)rect.Height / 2 / 40;
	draw(g, rect, y0, mull, G_SPEED);


	mull = (double)rect.Height / 2 / 45;
	draw(g, rect, y0, mull, M_C_PITCH);
	draw(g, rect, y0, mull, M_C_ROLL);



	mull = (double)rect.Height / 2 / (max_x-min_x);
	draw(g, rect, y0, mull, SX);
	mull = (double)rect.Height / 2 / (max_y - min_y);
	draw(g,  rect, y0, mull, SY);


	mull = (double)rect.Height / 2 / 200;
	draw(g, rect, y0, mull, I_YAW);

	mull = (double)rect.Height / 2 / MUL_4_ANG;
	draw(g, rect, y0, mull, I_PITCH);
	draw(g, rect, y0, mull, I_ROLL);

	y0 = (rect.Y + rect.Height);
	

	mull = (double)rect.Height ;
	draw(g,  rect, y0, mull, F0);
	draw(g, rect, y0, mull, F1);
	draw(g, rect, y0, mull, F2);
	draw(g, rect, y0, mull, F3);
	draw(g, rect, y0, mull, THROTTLE);
	draw(g, rect, y0, mull, I_THROTHLE);

	y0 = (rect.Y + rect.Height);
	mull = (double)rect.Height/150;
	y0 -= mull * 0.5;
	draw(g, rect, y0, mull, SZ);
	draw(g, rect, y0, mull, PRESSURE);
	draw(g, rect, y0, mull, GPS_Z);

	mull = (double)rect.Height / 2 / 300; 
	y0 = (rect.Y + rect.Height / 2);
	draw(g, rect, y0, mull, PRESSURE_ACC);
	mull = (double)rect.Height / 2 / 5;
	draw(g, rect, y0, mull, PRESSURE_SPEED);

	mull = (double)rect.Height / 3;
//	y0 = (rect.Y + rect.Height / 2);
	draw(g, rect, y0, mull, SPEED_Z);


	y0 = (rect.Y + rect.Height / 2)+1100;
	mull = (double)rect.Height / 2 / 2;
	draw(g, rect, y0, mull, BAT_F);


	y0 = (rect.Y + rect.Height / 2);
	mull = (double)rect.Height / 2 / 10;
	draw(g, rect, y0, mull, MI0);
	draw(g, rect, y0, mull, MI1);
	draw(g, rect, y0, mull, MI2);
	draw(g, rect, y0, mull, MI3);








	return 0;
}

void Graph::draw(Graphics &g, RectF rect, int y0, double mull, int sdi) {
	//void Graph::draw_ang(Graphics &g, Pen &pen, RectF rect, double zoom, double pos, int sdi) {


	if (flags[sdi]) {
		int dy[2];
		int index = 0;


		dy[0] = y0 - sensors_data[(int)(p)].sd[sdi] * mull;
		if (dy[0] > rect.Height)
			dy[0] = rect.Height;
		if (dy[0] < rect.Y)
			dy[0] = rect.Y;
		for (int x = rect.X + 1, i = 1; x < rect.X + rect.Width; x++, i++) {
			int in = (int)((p)+(step*i));
			if (in >= lSize)
				return;
			dy[index & 1] = y0 - sensors_data[in].sd[sdi] * mull;
			if (dy[index & 1] > rect.Height + rect.Y)
				dy[index & 1] = rect.Height + rect.Y;
			if (dy[index & 1] < rect.Y)
				dy[index & 1] = rect.Y;
			g.DrawLine(&Pen(color[sdi]), x - 1, dy[(index - 1) & 1], x, dy[index & 1]);
			index++;
		}
	}
	
}