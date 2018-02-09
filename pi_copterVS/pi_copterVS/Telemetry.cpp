// 
// 
// 

#include "Telemetry.h"
#include "GPS.h"
#include "Hmc.h"
#include "mpu.h"
#include "MS5611.h"
#include "commander.h"
#include "Balance.h"
#include "Stabilization.h"
#include "Wi_Fi.h"
#include "debug.h"
#include "mpu_umulator.h"
#include "Log.h"

#define BALANCE_DELAY 120
#define MAX_FLY_TIME 1200.0f
#define BAT_ZERO 360.0f
#define BAT_50P 391.0f
#define BAT_timeout 1000
#define BAT_timeoutRep  2
//#define BAT_100P 422
#define MAX_UPD_COUNTER 100
#define MAX_VOLTAGE_AT_START 406



void TelemetryClass::addMessage(const string msg){
	fprintf(Debug.out_stream,"%s\n", msg.c_str());
	if (message.length() + msg.length() >= TELEMETRY_BUF_SIZE)
		return;

	if (message.length() < msg.length() || message.compare(msg)==-1)
		message += msg;

}

void TelemetryClass::getSettings(int n){
	fprintf(Debug.out_stream,"up set\n");
	if (n > 7 || n < 0)
		return;

	ostringstream convert;
	convert << "UPS" << n <<",";
	message += convert.str();

	switch (n){
	case 0:
		message += Balance.get_set();
		break;
	case 1:
		message += Stabilization.get_z_set();
		break;
	case 2:
		message += Stabilization.get_xy_set();
		break;
	case 3:
		message += Autopilot.get_set();
		break;
	case 4:
		message += Mpu.get_set();
		break;
	case 5:
		message += Hmc.get_set();
		break;		
	case 6:
		message += Balance.get_set(1);
		break;
	}
	
/*	uint8_t o = (message.length()+1) % 3;  //чтобы длина сообщения была кратная трем.
	if (o == 1){
		message += "  ";
	}
	else if (o==2)
		message += " ";*/
	message += ",";
	//Out.println(message);

}


void TelemetryClass::init_()
{
	uint32_t power_on_time = 0;
	buffer_size = 0;
	powerK = 1;
	minimumTelemetry = false;
	lov_voltage_cnt = 0;
	//inner_clock_old_sec = millis() >> 10;
	low_voltage = voltage50P=false;
	message = "";
	next_battery_test_time = millis()+BAT_timeout;
	update_voltage();
	SN = ((voltage < 1200) ? 3 : 4);
		

	newGPSData = false;
	//Out.println("TELEMETRY INIT");
	voltage_at_start = 0;
	full_power = 0;
}


void TelemetryClass::loop()
{
	
	if (next_battery_test_time<millis()){
		next_battery_test_time = millis() + BAT_timeout;
		testBatteryVoltage();

		if (Autopilot.progState() && check_time_left_if_go_to_home() < 60 && ++no_time_cnt>3){ // на тестах ошибся на 5 минут.  
			fprintf(Debug.out_stream,"too far from HOME!\n");
			addMessage(e_BATERY_OFF_GO_2_HOME);
			Autopilot.going2HomeStartStop(false);
		}	
	}
	update_buf();
}

int TelemetryClass::get_voltage4one_cell() { return (int)(voltage / SN); }

int TelemetryClass::check_time_left_if_go_to_home(){
	float max_fly_time=0;
	if (voltage_at_start > 0){
		float work_time = 0.001f*(float)(millis() - Autopilot.time_at_start);
		if (work_time > BALANCE_DELAY && voltage_at_start > voltage){
			max_fly_time = ((voltage - BAT_ZERO*SN)*work_time / (voltage_at_start - voltage));
		}
		else{
			max_fly_time = MAX_FLY_TIME - work_time;
			no_time_cnt = 0;
		}
		const float dist2home = (float)sqrt(GPS.loc.dist2home_2);
		const float time2home = dist2home *(1.0f / MAX_HOR_SPEED);
		const float time2down = abs((MS5611.altitude())*(1.0f / MAX_VER_SPEED_MINUS));
	//	Debug.dump(max_fly_time, time2home + time2down, voltage, 0);
		return (int)(max_fly_time - time2home - time2down);
	}
	else
		return (int)max_fly_time;

}

void TelemetryClass::update_voltage() {
#ifdef FALSE_WIRE
	voltage = Emu.battery();
#else
	uint16_t data[5];
	mega_i2c.getiiiiv((char*)data);

	m_current[0] = 0.01953125*(float)(1024 - data[0]);
	m_current[1] = 0.01953125*(float)(1024 - data[1]);
	m_current[2] = 0.01953125*(float)(1024 - data[2]);
	m_current[3] = 0.01953125*(float)(1024 - data[3]);
	voltage = 1.725*(float)(data[4]);
	full_power += ( (m_current[0] + m_current[1] + m_current[2] + m_current[3]) * voltage - full_power)*0.2;  //152 вата  - 274, 9.24 amper
	if (Log.writeTelemetry) {
		Log.loadByte(LOG::TELE);
		Log.loadMem((uint8_t*)data, 10, false);
	}
#endif
}


void TelemetryClass::testBatteryVoltage(){

	update_voltage();

	if (!Autopilot.motors_is_on())
		voltage_at_start = voltage;
	

	if (voltage < BAT_ZERO*SN)
		lov_voltage_cnt++;
	else
		lov_voltage_cnt = 0;

	low_voltage = lov_voltage_cnt > 3;
	voltage50P = voltage < BAT_50P*SN;

	if (low_voltage)
		addMessage(e_VOLT_MON_ERROR);

	powerK = (MAX_VOLTAGE_AT_START * SN) / voltage;
	powerK = constrain(powerK, 1, 1.35f);
}

bool newGPSData = false;




void TelemetryClass::loadBUF32(int &i,  int32_t val)
{
	buf[i++] = ((byte*)&val)[0];
	buf[i++] = ((byte*)&val)[1];
	buf[i++] = ((byte*)&val)[2];
	buf[i++] = ((byte*)&val)[3];

}

void TelemetryClass::loadBUF16(int &i, int16_t val)
{
	buf[i++] = ((byte*)&val)[0];
	buf[i++] = ((byte*)&val)[1];


}



void TelemetryClass::loadBUF8(int &i,  const float val){
	int8_t t = (int8_t)(val);
	buf[i++] = ((byte*)&t)[0];
}
void TelemetryClass::loadBUF(int &i, const float fval)
{
	int16_t t = (int16_t)(fval);
	buf[i++] = ((byte*)&t)[0];
	buf[i++] = ((byte*)&t)[1];
	 
}


 /////////////////////////////////////////////////////////////////////////////////////////////////
 //uint8_t telemetry_cnt = 0;

bool gps_or_acuracy = false;


int TelemetryClass::read_buf(byte *buffer) {
	if (Autopilot.busy())
		return 4;
	else {
		while (buffer_size == 0) {
			usleep(10000);
		}
		int size = buffer_size;
		memcpy(buffer, buf, size);
		buffer_size = 0;
		return size;
	}


}
uint32_t last_update_time=0;
void TelemetryClass::update_buf() {
	if (buffer_size > 0)
		return;
	//bzero(buf, 32);
	//delay(1000);
	int i = 0;
	uint32_t mod = Autopilot.get_control_bits();
//	fprintf(Debug.out_stream,"out <- %i\n", mod);
	loadBUF32(i, mod);
	//fprintf(Debug.out_stream,"message=", message.c_str());
	loadBUF(i, 1000 + (Balance.get_throttle() * 1000));
	loadBUF32(i, GPS.loc.lat_);
	loadBUF32(i, GPS.loc.lon_);

	buf[i++] = (byte)GPS.loc.accuracy_hor_pos_;
	buf[i++] = (byte)GPS.loc.accuracy_ver_pos_;

	loadBUF(i, 10.0f*Autopilot.corectedAltitude4tel());// -Autopilot.startAltitude));
	//Out.fprintf(Debug.out_stream,t_old_alt); Out.fprintf(Debug.out_stream," "); Out.println(MS5611.altitude);// -Autopilot.startAltitude);
	loadBUF8(i, -Mpu.get_pitch());
	loadBUF8(i, Mpu.get_roll());
	loadBUF8(i, Balance.c_pitch);
	loadBUF8(i, Balance.c_roll);
	loadBUF16(i, voltage/SN*4);

	buf[i++] = (int8_t)Autopilot.getGimbalPitch();

	


	float yaw=Mpu.get_yaw();



	loadBUF8(i, yaw * 0.705555555555f);

	if (message.length() && i + message.length() + 2 < TELEMETRY_BUF_SIZE) {
		loadBUF16(i, message.length());
		memcpy(&buf[i], message.c_str(), message.length());
		i += message.length();
		message = "";
	}
	else {
		buf[i++] = 0;
		buf[i++] = 0;
	}

	uint8_t *b;
	int len;

	do {
		b = Log.getNext(len);
		if (len == 0) {
			buf[i++] = 0;
			buf[i++] = 0;
			break;
		}
		if (i + len + 6 < TELEMETRY_BUF_SIZE) {
			loadBUF16(i, len);
			memcpy(&buf[i], b, len);
			i += len;
		}
		else
			break;

	} while (true);

	buffer_size = i;
}
//nado echo peredat koordinaty starta i visoti ili luche ih androis socharanaet

TelemetryClass Telemetry;

