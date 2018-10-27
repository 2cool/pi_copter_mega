#include "mpu.h"
#include "define.h"
#include "Settings.h"
#include "Hmc.h"
#include "Autopilot.h"
#include "Telemetry.h"
#include "Balance.h"
#include "debug.h"
#include "Stabilization.h"
#include "GPS.h"
#include "Log.h"


/*

пользоватся только гироскопом если акселерометр показивает меньший угол. (но временно) и акселерометр и гироскоп должни бить отфильтровані
при снижении резком. надо тоже обдумть  логику.


*/





//#include "Mem.h"


#define delay_ms(a)    usleep(a*1000)






#define RESTRICT_PITCH // Comment out to restrict roll to ±M_2PIdeg instead - please read: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf

//2g
//	0.00006103515625
//4g

//3G

static const float f_constrain(const float v, const float min, const float max){
	return constrain(v, min, max);
}
#define DIM 3
//WiFiClass wi_fi;

float MpuClass::get_yaw() { return yaw; }
//-----------------------------------------------------
void  MpuClass::initYaw(const float angle){
	yaw = angle;
}
//-----------------------------------------------------
float MpuClass::get_pitch() { return pitch; }
float MpuClass::get_roll() { return roll; }



void MpuClass::do_magic4Z() {
static float aK = 19.6;
static float thr = 0.5;
	if (Balance.get_true_throttle() < 0.3) {
		e_accZ = e_speedZ = w_accZ =  0;
		thr = HOVER_THROTHLE;
		hower_thr = HOVER_THROTHLE;
		min_thr = MIN_THROTTLE_;
		fall_thr = FALLING_THROTTLE;
	}
	else {
		//Telemetry.get_full_power();
		e_accZ = 0;
		/*
		
		thr += (Balance.get_true_throttle() - thr)*0.2;
		e_accZ = thr*aK - 9.8 - e_speedZ*abs(e_speedZ)*0.232;
	//	e_accZ += (accZ - e_accZ)*0.01;//?
		e_speedZ += e_accZ*dt;
		e_speedZ += (MS5611.speed - e_speedZ)*0.01;
		if (Autopilot.z_stabState() && Commander.getThrottle() == HOVER_THROTHLE) {
			aK += (e_accZ + 9.8 / thr - aK)*0.002;
			aK = constrain(aK, 16, 25);
			hower_thr = 9.8 / aK;
			min_thr = hower_thr*0.8;
			fall_thr = hower_thr*0.9;
		//	cout << "%f\n", aK);
		}
		*/
	}
}

//float DRAG_K =0.022;
void MpuClass::do_magic() {
	if (Autopilot.motors_is_on() == false) {
		e_accX = e_accY = e_speedX = e_speedY = w_accX = w_accY = m7_accX=m7_accY=0;
		f_pitch = pitch;
		f_roll = roll;
		return;
	}
	//---calc acceleration on angels------
	

#define WIND_SPEED_X sqrt(abs(w_accX / DRAG_K))*((w_accX>=0)?1:-1)
#define WIND_SPEED_Y sqrt(abs(w_accY / DRAG_K))*((w_accY>=0)?1:-1)

//	float windX = e_speedX + WIND_SPEED_X;
//	float windY = e_speedY + WIND_SPEED_Y;

	const float _p = sinPitch / cosPitch;
	const float _r = sinRoll / cosRoll;

	const float GP = G + e_accZ;

	e_accX = -GP*(-cosYaw*_p - sinYaw*_r) - e_speedX*abs(e_speedX)*DRAG_K-w_accX;
	e_accX = constrain(e_accX, -MAX_ACC, MAX_ACC);
	e_accY = GP*(-cosYaw*_r + sinYaw*_p) - e_speedY*abs(e_speedY)*DRAG_K-w_accY;
	e_accY = constrain(e_accY, -MAX_ACC, MAX_ACC);
	w_accX += (e_accX - GPS.loc.accX - w_accX)*0.01;
	w_accY += (e_accY - GPS.loc.accY - w_accY)*0.01;

	e_speedX += e_accX*dt;
	e_speedX += (GPS.loc.speedX - e_speedX)*0.1;

	e_speedY += e_accY*dt;
	e_speedY += (GPS.loc.speedY - e_speedY)*0.1;

	//-----calc real angels------
	m7_accX += ((cosYaw*e_accX + sinYaw*e_accY) - m7_accX)*_0007;// 0.007;
	m7_accX = constrain(m7_accX, -MAX_ACC / 2, MAX_ACC / 2);
	m7_accY += ((cosYaw*e_accY - sinYaw*e_accX) - m7_accY)*_0007;// 0.007;
	m7_accY = constrain(m7_accY, -MAX_ACC / 2, MAX_ACC / 2);

	f_pitch = pitch;
	f_roll = roll;

	pitch = atan2((sinPitch + m7_accX*cosPitch / G), cosPitch);// +abs(gaccX*sinPitch));
	roll = atan2((sinRoll - m7_accY*cosRoll / G), cosRoll);// +abs(gaccY*sinRoll));


}

//-----------------------------------------------------

void MpuClass::log_sens() {
	if (Log.writeTelemetry) {
		Log.block_start(LOG::MPU_SENS);

		Log.loaduint64t(oldmpuTimed*1000);//new
		Log.loadMem((uint8_t*)g, 6, false);
		Log.loadMem((uint8_t*)a, 6, false);
		Log.loadMem((uint8_t*)_q, 16, false);



		Log.loadFloat(f_pitch * RAD2GRAD);
		Log.loadFloat(f_roll * RAD2GRAD);
		Log.loadFloat(pitch);
		Log.loadFloat(roll);
		Log.loadFloat(yaw);
		Log.loadFloat(gyroPitch);
		Log.loadFloat(gyroRoll);
		Log.loadFloat(gyroYaw);
		Log.loadFloat(accX);
		Log.loadFloat(accY);
		Log.loadFloat(accZ);

		Log.block_end();
	}
}
void MpuClass::log_emu() {
	if (Log.writeTelemetry) {
		Log.block_start(LOG::MPU_EMU);

		Log.loadByte((uint8_t)(dt * 1000));
		Log.loadFloat(f_pitch * RAD2GRAD);
		Log.loadFloat(f_roll * RAD2GRAD);
		Log.loadFloat(pitch);
		Log.loadFloat(roll);
		Log.loadFloat(yaw);
		Log.loadFloat(gyroPitch);
		Log.loadFloat(gyroRoll);
		Log.loadFloat(gyroYaw);
		Log.loadFloat(accX);
		Log.loadFloat(accY);
		Log.loadFloat(accZ);

		Log.block_end();
	}


}
//-----------------------------------------------------
int MpuClass::ms_open() {
	dmpReady = 1;
	initialized = 0;
	for (int i = 0; i<DIM; i++) {
		lastval[i] = 10;
	}

	// initialize device
	cout << "Initializing MPU...\n";
	if (mpu_init(NULL) != 0) {
		cout << "MPU init failed!\n";
		return -1;
	}
	cout << "Setting MPU sensors...\n";
	if (mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL) != 0) {
		cout << "Failed to set sensors!\n";
		return -1;
	}
	cout << "Setting GYRO sensitivity...\n";
	if (mpu_set_gyro_fsr(2000) != 0) {
		cout << "Failed to set gyro sensitivity!\n";
		return -1;
	}
	cout << "Setting ACCEL sensitivity...\n";
	if (mpu_set_accel_fsr(8) != 0) {
		cout << "Failed to set accel sensitivity!\n";
		return -1;
	}
	// verify connection
	cout << "Powering up MPU...\n";
	mpu_get_power_state(&devStatus);
	if (devStatus)
		cout << "MPU6050 connection successful\n";
	else
		cout << "MPU6050 connection failed " << devStatus << "\n";


	//fifo config
	cout << "Setting MPU fifo...\n";
	if (mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL) != 0) {
		cout << "Failed to initialize MPU fifo!\n";
		return -1;
	}

	// load and configure the DMP
	cout << "Loading DMP firmware...\n";
	if (dmp_load_motion_driver_firmware() != 0) {
		cout << "Failed to enable DMP!\n";
		return -1;
	}

	cout << "Activating DMP...\n";
	if (mpu_set_dmp_state(1) != 0) {
		cout << "Failed to enable DMP!\n";
		return -1;
	}

	//dmp_set_orientation()
	//if (dmp_enable_feature(DMP_FEATURE_LP_QUAT|DMP_FEATURE_SEND_RAW_GYRO)!=0) {
	cout << "Configuring DMP...\n";
	if (dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL) != 0) {
		cout << "Failed to enable DMP features!\n";
		return -1;
	}


	cout << "Setting DMP fifo rate...\n";
	if (dmp_set_fifo_rate(rate) != 0) {
		cout << "Failed to set dmp fifo rate!\n";
		return -1;
	}
	cout << "Resetting fifo queue...\n";
	if (mpu_reset_fifo() != 0) {
		cout << "Failed to reset fifo!\n";
		return -1;
	}

	cout << "Checking... ";
	do {
		delay_ms(1000 / rate);  //dmp will habve 4 (5-1) packets based on the fifo_rate
		r = dmp_read_fifo(g, a, _q, &sensors, &fifoCount);
	} while (r != 0 || fifoCount<5); //packtets!!!
	cout << "Done.\n";

	initialized = 1;
	return 0;
}
//-----------------------------------------------------
void MpuClass::init()
{
	hower_thr = HOVER_THROTHLE;
	min_thr = MIN_THROTTLE_;
	fall_thr = FALLING_THROTTLE;
	DRAG_K = 0.0052;
	//DRAG_K = 0.022;
	_0007=0.007;
	gaccX =  gaccY =0;
	acc_callibr_timed = 0;
	rate = 100;
	tiltPower_CF = 0.05;

	f_pitch = f_roll = 0;

	windFX = windFY = e_speedX = e_speedY = e_accX=e_accY=m7_accX=m7_accY=w_accX=w_accY=0;
	yaw_off = 0;
	max_g_cnt = 0;
	cosYaw = 1;
	sinYaw = 0;
	temp_deb = 6;
	fx = fy = fz = 0;

	faccX = faccY = faccZ = 0;
	oldmpuTimed = 0;
	yaw_offset = yaw = pitch = roll = gyroPitch = gyroRoll = gyroYaw = accX = accY = accZ = 0;
	sinPitch = sinRoll = 0;
	tiltPower = cosPitch = cosRoll = 1;
	timed = 0;
	//COMP_FILTR = 0;// 0.003;
	vibration = 0;
	cout << "Initializing MPU6050\n";

#ifndef FALSE_WIRE








	ms_open();
	
	writeWord(104, MPU6050_RA_XA_OFFS_H, -535);//-5525);
	writeWord(104, MPU6050_RA_YA_OFFS_H, 219);// -1349);
	writeWord(104, MPU6050_RA_ZA_OFFS_H, 1214);// 1291);
	writeWord(104, MPU6050_RA_XG_OFFS_USRH, 165);// -43);
	writeWord(104, MPU6050_RA_YG_OFFS_USRH, -39);// 36);
	writeWord(104, MPU6050_RA_ZG_OFFS_USRH, 16);// -49);
		
	

	
	


#ifdef GYRO_CALIBR
	gyro_calibratioan = false;
#else
	gyro_calibratioan = true;
#endif

#ifdef DEBUG_MODE
		//for (int i = 0; i < 6; i++)
		//	Out.println(offset_[i]);
#endif
#ifndef WORK_WITH_WIFI
		//calibrated = false;
#endif
	//}
//	else {
	//	cout << "MPU NOT CALIBRATED !!!\n");
	//}




#else
	

#endif

}
//-----------------------------------------------------
string MpuClass::get_set(){
	
	
	ostringstream convert;
	convert<<
		DRAG_K <<","<< _0007<<","<< tiltPower_CF;
	
	string ret = convert.str();
	return string(ret);
	
}
//-----------------------------------------------------
void MpuClass::set(const float  *ar){

	int i = 0;
	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){
		int error = 0;

		float t;

		t = DRAG_K;
		if (error += Commander._set(ar[i++], t) == 0)
			DRAG_K = t;
		t = _0007;
		if (error += Commander._set(ar[i++], t) == 0)
			_0007 = t;
		t = tiltPower_CF;
		if (error += Commander._set(ar[i++], t) == 0)
			tiltPower_CF = t;
		cout << "mpu set:\n";
		//int ii;
		if (error == 0){
			//for (ii = 0; ii < i; ii++){
			//	Out.cout << ar[ii]); Out.cout << ",");
			//}
			//Out.println(ar[ii]);
			cout << "OK\n";
		}
		else{
			cout << "ERROR to big or small. P=" << error << endl;
		}
	}
	else{
		cout << "ERROR\n";
	}
}
//-----------------------------------------------------
int16_t MpuClass::getGX(){
	int16_t x, y, z;
	accelgyro.getAcceleration(&x, &y, &z);
	return x;
}
//-----------------------------------------------------
const float n003 = 0.030517578f;
const float n006 =  0.061035156f;
//4g
const float n122 = 1.220740379e-4;
//2g
//const float n604 = 0.00006103515625f;



const float to_98g = 0.0005981445312f;

#ifdef FALSE_WIRE





float real_pitch = 0, real_roll = 0;

//-----------------------------------------------------
void MpuClass::calc_corrected_ang(){

	double raccX = cosYaw*GPS.loc.accX + sinYaw*GPS.loc.accY;
	double raccY = cosYaw*GPS.loc.accY - sinYaw*GPS.loc.accX;






}

///////////////////////////////////////////////////////////////////

bool MpuClass::loop(){




	timed = (0.000001*(double)micros());
	double ___dt = (float)(timed - oldmpuTimed);// *div;
	if (___dt < 0.01)
		return false;

	dt = 0.01;
	rdt = 1.0 / dt;
	oldmpuTimed = timed;
	if (dt > 0.02)
		dt = 0.01;


	pitch=Emu.get_pitch();
	roll = Emu.get_roll();



	gyroPitch = Emu.get_gyroPitch();
	gyroRoll = Emu.get_gyroRoll();
	gyroYaw = Emu.get_gyroYaw();
	accX = Emu.get_raccX();
	accY = Emu.get_raccY();
	accZ = Emu.get_accZ();




	float head = Hmc.heading;

	float g_yaw = Emu.get_yaw();

	yaw_offset += (wrap_PI(g_yaw - head) - yaw_offset)*0.0031f;

	yaw = wrap_PI(g_yaw - yaw_offset);


	sin_cos(pitch, sinPitch, cosPitch);
	sin_cos(roll, sinRoll, cosRoll);


	tiltPower = cosPitch*cosRoll;
	cosYaw = cos(yaw);
	sinYaw = sin(yaw);

#ifdef MPU_MAGIC
	do_magic4Z();
	do_magic();
#endif

	yaw *= RAD2GRAD;
	pitch *= RAD2GRAD;
	roll *= RAD2GRAD;

	
	//r_pitch = RAD2GRAD*pitch;
	//r_roll = RAD2GRAD*roll;

	delay(4);
	gyro_calibratioan = true;

	log_emu();

	shmPTR->pitch = pitch;
	shmPTR->roll = roll;
	shmPTR->yaw = yaw;

	return true;
}

#else


/////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t GetGravity(VectorFloat *v, Quaternion *q) {
	v->x = 2 * (q->x*q->z - q->w*q->y);
	v->y = 2 * (q->w*q->x + q->y*q->z);
	v->z = q->w*q->w - q->x*q->x - q->y*q->y + q->z*q->z;
	return 0;
}


#define ROLL_COMPENSATION_IN_YAW_ROTTATION 0.02
#define PITCH_COMPENSATION_IN_YAW_ROTTATION 0.025

float agpitch = 0, agroll = 0, agyaw = 0;

uint64_t maxG_firs_time = 0;


bool compas_flip = false;


#define _2PI 6.283185307179586476925286766559


bool pitch_flag;
bool set_yaw_flag = true;




static void toEulerianAngle(const Quaternion& q, float& roll, float& pitch, float& yaw)
{
	// roll (x-axis rotation)
	double sinr = +2.0 * (q.w * q.x + q.y * q.z);
	double cosr = +1.0 - 2.0 * (q.x * q.x + q.y * q.y);

	roll = atan2(sinr, cosr);

	// pitch (y-axis rotation)
	double sinp = +2.0 * (q.w * q.y - q.z * q.x);
	if (fabs(sinp) >= 1)
		pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	else
		pitch = asin(sinp);

	// yaw (z-axis rotation)
	double siny = +2.0 * (q.w * q.z + q.x * q.y);
	double cosy = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);
	yaw = atan2(siny, cosy);
}

void MpuClass::test_vibration( float x,  float y,  float z){
	static float lx = 0, ly = 0, lz = 0;
	lx += (x - lx)*0.1;
	ly += (y - ly)*0.1;
	lz += (z - lz)*0.1;
	x -= lx;
	y -= ly;
	z -= lz;
	const float vibr = sqrt(x * x + y * y + z * z);
	vibration += (vibr - vibration)*0.01;
	
}

static int mpu_false = 0;
bool MpuClass::loop() {//-------------------------------------------------L O O P-------------------------------------------------------------
	//написать тайм менеджер
	double old_tied = timed;
	timed = 0.000001*(double)micros();
	if (dmp_read_fifo(g, a, _q, &sensors, &fifoCount) != 0) { //gyro and accel can be null because of being disabled in the efeatures
		mpu_false++;
		return false;
	}

	
	dt = (timed - oldmpuTimed);// *div;
	

	mpu_timed = 0.000001*(double)micros();
	static uint cnt2l = 0;
	if (dt > 0.02) {
		if (cnt2l++) {
			cout << mpu_false << endl;
			
			cout << "MPU DT too long " << endl;// << dt << ":" << dt << ":" << timed << endl;

			cout << endl << ms5611_timed- old_tied << " " << hmc_timed - old_tied << " " << gps_timed - old_tied <<
				" " << telem_timed - old_tied << " " << com_timed - old_tied <<" "<< autopilot_timed - old_tied <<" "<< mpu_timed - old_tied << endl;


			mega_i2c.beep_code(B_MPU_TOO_LONG);
		}

	}
mpu_false = 0;

	//if (dt < 0.008)
	//	cout << "MPU DT too short "<<dt<<"\n";
	oldmpuTimed = timed;
	dt = 0.01;
	rdt = 100;// 1.0 / dt;
	

	q = _q;
	q.x *= 1.5259e-5;
	q.y *= 1.5259e-5;
	q.z *= 1.5259e-5;
	q.w *= 1.5259e-5;

	float g_yaw;
	toEulerianAngle(q, roll, pitch, g_yaw);
	pitch = -pitch;
	g_yaw = -g_yaw;

	if (set_yaw_flag) {
		yaw_off = g_yaw-Hmc.heading;
		set_yaw_flag = false;
	}

	if (abs(roll) >90*GRAD2RAD) {
		yaw = -Autopilot.get_yaw();
		set_yaw_flag = true;

		roll = (roll > 0) ? 180*GRAD2RAD-roll : -180*GRAD2RAD-roll;

		if (abs(pitch) > 35 * GRAD2RAD) {
			pitch_flag = true;
			if (abs(roll) > abs(pitch))
				pitch_flag = false;
		}
		else
			if (abs(roll) > 35 * GRAD2RAD) {
				pitch_flag = false;
				if (abs(pitch) > abs(roll))
					pitch_flag = true;
			}
		if (pitch_flag)
			pitch = ((pitch > 0) ? M_PI : -M_PI) - pitch;
		else
			roll = ((roll > 0) ? M_PI : -M_PI) - roll;
	}
	else
		yaw_off += ( wrap_PI( wrap_PI(g_yaw - Hmc.heading) - yaw_off) )*0.0031f;

	yaw = wrap_PI(g_yaw - yaw_off);

	sin_cos(yaw, sinYaw, cosYaw);
	sin_cos(pitch, sinPitch, cosPitch);
	sin_cos(roll, sinRoll, cosRoll);

#ifdef MPU_MAGIC
	if (abs(pitch) <= 65 * GRAD2RAD && abs(roll) <= 65 * GRAD2RAD) {
		do_magic4Z();
		do_magic();
	}
#endif
	tiltPower += (constrain(cosPitch*cosRoll, 0.5f, 1) - tiltPower)*tiltPower_CF;
	gyroPitch = -n006 * (float)g[1] - agpitch;
	gyroRoll = n006 * (float)g[0] - agroll;
	gyroYaw = -n006 * (float)g[2] - agyaw;

	float x = n122*2 * (float)a[0];
	float y = -n122*2 * (float)a[1];
	float z = n122*2 * (float)a[2];

	


	static float low_accZ = 0;

	accZ = z*cosPitch + sinPitch*x;
	accZ = 9.8f*(accZ*cosRoll - sinRoll*y - 1);
	/////////////////////////////////////////////////
	low_accZ += (accZ - low_accZ)*0.001;
	accZ -= low_accZ+MS5611.acc;
	/////////////////////////////////////////////////

	accX = 9.8f*(x*cosPitch - z*sinPitch);
	accY = 9.8f*(y*cosRoll + z*sinRoll);

	test_vibration(accX, accY, accZ);

	//Debug.load(0, accZ, accX);
	//Debug.dump();

	if (Autopilot.motors_is_on() == false) {
		if (timed > 30 && acc_callibr_timed > timed) {
			agpitch += (gyroPitch - agpitch)*0.01;
			agroll += (gyroRoll - agroll)*0.01;
			agyaw += (gyroYaw - agyaw) *0.01;
			
		}
	}
	shmPTR->pitch = pitch *= RAD2GRAD;
	shmPTR->roll = roll *= RAD2GRAD;
	shmPTR->yaw = yaw*=RAD2GRAD;
	log_sens();
	return true;
}


#endif


void MpuClass::setDLPFMode_(uint8_t bandwidth){
	gLPF = bandwidth;
	accelgyro.setDLPFMode(bandwidth);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MpuClass::new_calibration(const bool onlyGyro){

	acc_callibr_timed = timed+10;
	gyro_calibratioan = true;
}


MpuClass Mpu;



