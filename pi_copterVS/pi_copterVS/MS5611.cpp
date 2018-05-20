
#include "MS5611.h"
#include "Telemetry.h"
#include "Autopilot.h"
#include "debug.h"
#include "Log.h"
#include "GPS.h"

static int fd4S;
unsigned int PROM_read(int DA, char PROM_CMD)
{
	uint16_t ret = 0;
	uint8_t r8b[] = { 0, 0 };

	if (write(DA, &PROM_CMD, 1) != 1){
		fprintf(Debug.out_stream,"read set reg Failed to write to the i2c bus.\n");
	}

	if (read(DA, r8b, 2) != 2){
		fprintf(Debug.out_stream,"Failed to read from the i2c bus.\n");
	}

	ret = r8b[0] * 256 + r8b[1];

	return ret;
}


char RESET = 0x1E;
#define ALT_NOT_SET 0



int MS5611Class::error(int e) {
	return -1;
}


int MS5611Class::writeReg(char bar_zero) {
	if (write(fd4S, &bar_zero, 1) != 1) {
		fprintf(Debug.out_stream, "write reset 8 bit Failed to write to the i2c bus.\n");
		bar_task = 0;
		return error(2);
	}
}


float MS5611Class::altitude() {
	return altitude_ - altitude_error;
}
#ifndef WORK_WITH_WIFI
int cntssdde = 0;
#endif
float fspeed = 0;
void MS5611Class::copterStarted(){
//	if (altitude_error == ALT_NOT_SET){
		altitude_error = altitude_;
//	}
}



#define MAX_D_PRESSURE 300.0

bool MS5611Class::fault() {
	return wrong_altitude_cnt > MAX_BAROMETR_ERRORS;
}
double MS5611Class::getAltitude(const double pressure) {
	double alt = (44330.0f * (1.0f - pow(pressure / PRESSURE_AT_0, 0.1902949f)));//Где блядь проверка 4.4.2018-вот она
	
	if (fault()) {
			return GPS.loc.altitude - gps_barometr_alt_dif - GPS_ALT_MAX_ERROR;
	}
	else {
		shmPTR->altitude = altitude_ = alt;
		shmPTR->pressure = pressure;
		gps_barometr_alt_dif += (GPS.loc.altitude - alt - gps_barometr_alt_dif)*0.1;
	}
	
	return alt;
}


void MS5611Class::log_sens() {
	if (Log.writeTelemetry) {
		Log.block_start(LOG::MPU_SENS);
		Log.loadByte(i_readTemperature);
		Log.loadFloat(pressure);
		Log.block_end();
	}
}

//--------------------------------------------------
#ifdef FALSE_WIRE

#include "GPS.h"


#include "Balance.h"


long ttimet = millis();
uint8_t MS5611Class::loop(){
	if (millis() - ttimet < 200)
		return 0;


	const float dt = 0.2;// (millis() - timet)*0.001;
	ttimet = millis();
		const float new_altitude = Emu.get_alt();

	speed = (new_altitude - altitude_) / dt;
	altitude_ = new_altitude;
	pressure = get_pressure(altitude_);



	i_readTemperature = 20;

	//speed = Emu.get_speedZ();
#ifdef Z_SAFE_AREA
	if (Autopilot.motors_is_on() && (altitude_ - altitude_error) > Z_SAFE_AREA) {
		Autopilot.control_falling(i_CONTROL_FALL);
	}
#endif




	
	log_sens();

	powerK = PRESSURE_AT_0 / pressure;

	if (powerK>1.4)
		powerK = 1.4;

}

#else

uint8_t MS5611Class::loop(){
	
	switch (bar_task)
	{
	case 0:
		phase0();
		break;
	case 1:
		phase1();
		break;
	default:
		phase2();
	}

	return 0;
}

#endif

//----------------------------------------------------




void MS5611Class::phase0() {
	bar_D[0] = bar_D[1] = bar_D[2] = 0;

	writeReg(CONV_D2_4096);
	b_timeDelay = micros() + ct;
	bar_task = 1;
}
#define fc C
void MS5611Class::phase1()
{
	if (micros()  > b_timeDelay) {
		bar_task = 2;
		bar_zero = 0;
		writeReg(bar_zero);
		bar_h = read(fd4S, &bar_D, 3);
		if (bar_h != 3) {
			fprintf(Debug.out_stream, "Failed to read from the i2c bus %d.\n", bar_h);
			error(3);
			return;
		}

		D2 = ((int32_t)bar_D[0] << 16) | ((int32_t)bar_D[1] << 8) | bar_D[2];
		int32_t dT = D2 - (uint32_t)fc[4] * 256;
		int32_t TEMP = 2000 + ((int64_t)dT * fc[5]) / 8388608;
		int32_t TEMP2 = 0;
		if (compensation)
		{
			if (TEMP < 2000)
			{
				TEMP2 = (dT * dT) / (2 << 30);
			}
		}
		TEMP = TEMP - TEMP2;
		i_readTemperature = ((int8_t)(TEMP * 0.01));
		writeReg(CONV_D1_4096);
		b_timeDelay = micros() + ct;
		
	}
}

void MS5611Class::phase2() {
	if (micros()  > b_timeDelay)
	{
		bar_task = 0;
		//openDev();
		bar_zero = 0;
		writeReg(bar_zero);

		bar_h = read(fd4S, &bar_D, 3);

		if (bar_h != 3)
			fprintf(Debug.out_stream, "Failed to read from the i2c bus %d.\n", bar_h);

		D1 = ((int32_t)bar_D[0] << 16) | ((int32_t)bar_D[1] << 8) | bar_D[2];
		int32_t dT = D2 - (uint32_t)fc[4] * 256;
		int64_t OFF = (int64_t)fc[1] * 65536 + (int64_t)fc[3] * dT / 128;
		int64_t SENS = (int64_t)fc[0] * 32768 + (int64_t)fc[2] * dT / 256;

		if (compensation)
		{
			int32_t TEMP = 2000 + ((int64_t)dT * fc[5]) / 8388608;
			int64_t OFF2 = 0;
			int64_t SENS2 = 0;
			if (TEMP < 2000)
			{
				OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;
				SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;
			}
			if (TEMP < -1500)
			{
				OFF2 = OFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500));
				SENS2 = SENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
			}

			OFF = OFF - OFF2;
			SENS = SENS - SENS2;
		}





		//float pr = (float)(D1 * SENS / 2097152 - OFF) * 0.000030517578125;

		int32_t tP = ((((int64_t)D1*SENS) / 2097152 - OFF) / 32768);
		if (tP < 80000) {
			fprintf(Debug.out_stream, "PRESSURE ERROR %i\n",tP);
			wrong_altitude_cnt++;
		}
		else {
			wrong_altitude_cnt = 0;
			P = tP;
		}
		const double dt = (Mpu.timed - old_timed);
		old_timed = Mpu.timed;
		
		if (pressure == PRESSURE_AT_0) {
			pressure = P;
			altitude_ = getAltitude(pressure);
		}

		pressure += ((double)P - pressure)*0.3;
		log_sens();
		powerK = constrain(PRESSURE_AT_0 / pressure, 1, 1.2);
		const float new_altitude = getAltitude(pressure);
		speed = (new_altitude - altitude_) / dt;
		altitude_ = new_altitude;


#ifdef Z_SAFE_AREA
		if (Autopilot.motors_is_on() && (altitude_ - altitude_error) > Z_SAFE_AREA) {
			Autopilot.control_falling(e_OUT_OF_PER_V);
		}
#endif

		
	}
}

void MS5611Class::update(){}


float MS5611Class::get_pressure(float h) {
	return PRESSURE_AT_0 * pow(1 - h*2.25577e-5, 5.25588);
}

//-------------------------------------------init-------------------------
int MS5611Class::init() {
	oldAltt = 100000;
	gps_barometr_alt_dif = 0;
	old_timed = 0;
	bar_task = 0;
	bar_zero = 0x0;
	ct = 10000;
	

	wrong_altitude_cnt = 0;
	speed = altitude_ = 0;
	altitude_error = ALT_NOT_SET;

	powerK = 1;

#ifndef FALSE_WIRE

	fprintf(Debug.out_stream, "Initialize High resolution: MS5611\n");


#endif
	pressure = PRESSURE_AT_0;
	ms5611_count = 0;

#ifndef FALSE_WIRE
	compensation = true;

	if ((fd4S = open("/dev/i2c-0", O_RDWR)) < 0) {
		fprintf(Debug.out_stream, "Failed to open the bus.\n");
		return error(0);
	}

	if (ioctl(fd4S, I2C_SLAVE, MS5611_ADDRESS) < 0) {
		fprintf(Debug.out_stream, "Failed to acquire bus access and/or talk to slave.\n");
		return error(1);
	}

	writeReg(RESET);

	usleep(100000);
	for (i = 0; i < 6; i++) {
		C[i] = PROM_read(fd4S, 0xA2 + (i * 2));
	}

#endif
	return 0;
}
MS5611Class MS5611;
