// 
// 
// 
/*

4.20 � �� 100%
3.95 � �� 75%
3.85 � �� 50%
3.73 � �� 25%
3.50 � �� 5%
2.75 � �� 0%


*/

#include "Telemetry.h"
#include "GPS.h"
#include "Hmc.h"
#include "mpu.h"
#include "MS5611.h"
#include "commander.h"
#include "Balance.h"
#include "Stabilization.h"

#include "debug.h"
#include "mpu_umulator.h"
#include "Log.h"

#define BALANCE_DELAY 120
#define MAX_FLY_TIME 600
#define BAT_ZERO 350.0f
#define BAT_50P 385.0f
#define BAT_timeout 0.05
#define BAT_timeoutRep  2
//#define BAT_100P 422
#define MAX_UPD_COUNTER 100
#define MAX_VOLTAGE_AT_START 406
#define BAT_Ampere_hour 3.5

static float  f_current = 0;

void TelemetryClass::addMessage(const string msg, bool and2sms){

	cout << msg << "\t"<<Mpu.timed << endl;;
	if (message.length() < msg.length() || message.compare(msg) == -1) {
		message += msg;
	}

}

void TelemetryClass::getSettings(int n){
	
	if (n > 7 || n < 0)
		return;
	cout << "up set: "<<n<<"\n";
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
		message += Commander.get_set();
		break;
	}
	
/*	uint8_t o = (message.length()+1) % 3;  //����� ����� ��������� ���� ������� ����.
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
	
	
	init_shmPTR();

	buf = shmPTR->telemetry_buf;
	uint32_t power_on_time = 0;
	
	
	powerK = 1;
	minimumTelemetry = false;
	lov_voltage_cnt = 0;
	//inner_clock_old_sec = millis() >> 10;
	low_voltage = voltage50P=false;
	message = "";
	next_battery_test_timed = BAT_timeout;
	update_voltage();
	SN = ((voltage < 1200) ? 3 : 4);
		

	newGPSData = false;
	//Out.println("TELEMETRY INIT");
	voltage_at_start = 0;
	full_power = 0;
	consumed_charge = 0;



	battery_charge = BAT_Ampere_hour * 3600*  ((voltage- (BAT_ZERO*SN))/(70*SN));
}

uint16_t data[5];

void TelemetryClass::loop()
{
	
	if (next_battery_test_timed<Mpu.timed){
		next_battery_test_timed = Mpu.timed + BAT_timeout;
		testBatteryVoltage();
		uint16_t time_left = check_time_left_if_go_to_home();
		if (Autopilot.progState() && time_left < 60 && ++no_time_cnt>3){ 
			cout << "too far from HOME!" << "\t"<<Mpu.timed << endl;
			addMessage(e_BATERY_OFF_GO_2_HOME);
			Autopilot.going2HomeStartStop(false);
		}	

		static uint8_t log_cnt = 0;
		if (Log.writeTelemetry) {
			Log.block_start(LOG::TELE);
			Log.loadMem((uint8_t*)data, 10, false);
			if (++log_cnt > 50) {
				Log.loadInt16t(time_left);
				//cout << time_left << endl;
			}
			Log.block_end();
		}
		
	}
	update_buf();
	message = "";
}

int TelemetryClass::get_voltage4one_cell() { return (int)(voltage / SN); }
int TelemetryClass::fly_time_left() {
	int fly_time=fmax(0,  battery_charge-consumed_charge);
	fly_time = fmin(MAX_FLY_TIME, fly_time);
	return fly_time;
}
int TelemetryClass::check_time_left_if_go_to_home(){
	float time_left = fly_time_left();

	if (Autopilot.motors_is_on()) {
		const float dist2home = (float)sqrt(Mpu.dist2home_2());
		const float time2home = dist2home * (1.0f / MAX_HOR_SPEED);
		const float time2down = fabs((Mpu.get_Est_Alt()) * (1.0f / MAX_VER_SPEED_MINUS));
		time_left -= (time2home + time2down);
	}

	return (int)time_left;

}


#ifdef FLY_EMULATOR
#define FULL_FW
#endif

void TelemetryClass::update_voltage() {
	
#ifdef FULL_FW
	Emu.battery(m_current,voltage);
#else

	if (mega_i2c.getiiiiv((char*)data)==-1)
		return;

#define max_V 1022
#define current_k 0.01953125
//#define current_k 1


/*
No-Load Current (Io/10V) 0.45 Amps
Motor Resistance (Rm) 0.117 Ohms
Max Continuous Current 14 Amps
Max Continuous Power 220 Watts
*/

/*

	shmPTR->m_current[0] = m_current[0] = 0.027 *(float)(1004- data[0]);
	shmPTR->m_current[1] = m_current[1] = 0.027 *(float)(995 - data[1]);
	shmPTR->m_current[2] = m_current[2] = 0.022 *(float)(999 - data[2]);
	shmPTR->m_current[3] = m_current[3] = 0.025 *(float)(996 - data[3]);
*/	
	

#define CUR_K 51.15
	shmPTR->m_current[0] = m_current[0] = 1.024 *( 20 - (float)(data[0]-24) / CUR_K);
	shmPTR->m_current[1] = m_current[1] = 1.024 * (20 - (float)(data[1]-24) / CUR_K);
	shmPTR->m_current[2] = m_current[2] = 1.024 * (20 - (float)(data[2]-24) / CUR_K);
	shmPTR->m_current[3] = m_current[3] = 1.024 * (20 - (float)(data[3]-24) / CUR_K);



	/*/////////////
	static float mc0 = 0, mc1 = 0, mc2 = 0, mc3 = 0;
	mc0 += (m_current[0] - mc0)*0.03;
	mc1 += (m_current[1] - mc1)*0.03;
	mc2 += (m_current[2] - mc2)*0.03;
	mc3 += (m_current[3] - mc3)*0.03;
	Debug.dump(mc0, mc1, mc2, mc3);

	/////////////



	static float d[4] = { 1000,1000,1000,1000 };
	for (int i=0; i<4; i++)
		d[i] += (data[i] - d[i]) * 1;

*/

	//Debug.dump((float)d[0], d[1], d[2], d[3]);
	//Debug.dump(m_current[0], m_current[1], m_current[2], m_current[3]);


/*

#define WORK_I 1.5

	if (Autopilot.motors_is_on() && Autopilot.get_throttle()>Balance.get_min_throttle() && (m_current[0] > WORK_I || m_current[1] > WORK_I || m_current[2] > WORK_I || m_current[3] > WORK_I)) {
		Balance.propeller_lost[0] = (m_current[0] < WORK_I);
		Balance.propeller_lost[1] = (m_current[1] < WORK_I);
		Balance.propeller_lost[2] = (m_current[2] < WORK_I);
		Balance.propeller_lost[3] = (m_current[3] < WORK_I);
		//printf("propeller lost\n");
	}

*/


//	Debug.dump(m_current[0], m_current[1], m_current[2], m_current[3]);
	shmPTR->voltage = voltage = 1.725*(float)(data[4]);
	full_power += ( (m_current[0] + m_current[1] + m_current[2] + m_current[3]) * voltage - full_power)*0.2;  //152 ����  - 274, 9.24 amper
	
#endif
}


void TelemetryClass::testBatteryVoltage(){
	static double old_timed = 0;
	update_voltage();
	//const double time_nowd = Mpu.timed;
	double dt = Mpu.timed - old_timed;
	if (dt > 1)
		dt = 1;
	old_timed = Mpu.timed;
	float current = (m_current[0] + m_current[1] + m_current[2] + m_current[3] + 0.64);

	f_current += (current - f_current)*0.03;
	consumed_charge += current *dt;

	//printf("charge=%f, cons ch=%f, bat ch=%f\n", current,consumed_charge, battery_charge);

	if (!Autopilot.motors_is_on())
		shmPTR->voltage_at_start = voltage_at_start = voltage;
	

	if (voltage < BAT_ZERO*SN)
		lov_voltage_cnt++;
	else
		lov_voltage_cnt = 0;

	low_voltage = lov_voltage_cnt > 3;
	voltage50P = voltage < BAT_50P*SN;

	//if (low_voltage)
	//	addMessage(e_VOLT_MON_ERROR);

	float t_powerK = (MAX_VOLTAGE_AT_START * SN) / voltage;
	powerK += (constrain(t_powerK, 1, 1.35f) - powerK)*0.001;
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



uint32_t last_update_time=0;
void TelemetryClass::update_buf() {
	if (shmPTR->connected == 0 || shmPTR->telemetry_buf_len > 0)
		return;
	if (Autopilot.busy()) {
		shmPTR->telemetry_buf_len=4;
		return;
	}

	//bzero(buf, 32);
	//delay(1000);
	int i = 0;
	uint32_t mod = Autopilot.get_control_bits();
//	printf("out <- %i\n", mod);
	loadBUF32(i, mod);
	//printf("message=", message.c_str());
	loadBUF(i, 1000 + (Balance.get_throttle() * 1000));
	loadBUF32(i, GPS.loc.lat_);
	loadBUF32(i, GPS.loc.lon_);

	buf[i++] = (byte)GPS.loc.accuracy_hor_pos_;
	buf[i++] = (byte)GPS.loc.accuracy_ver_pos_;

	loadBUF(i, 10.0f*Autopilot.corectedAltitude4tel());// -Autopilot.startAltitude));
	//Out.printf(t_old_alt); Out.printf(" "); Out.println(MS5611.altitude);// -Autopilot.startAltitude);
	loadBUF8(i, -Mpu.get_pitch());
	loadBUF8(i, Mpu.get_roll());
	loadBUF8(i, Balance.c_pitch);
	loadBUF8(i, Balance.c_roll);
	loadBUF16(i, voltage/SN*4);



	//----
	loadBUF16(i, consumed_charge);
	loadBUF16(i, f_current *1000);
	loadBUF16(i, Mpu.vibration * 1000);

	buf[i++] = (int8_t)Autopilot.getGimbalPitch();

	


	float yaw=Mpu.get_yaw();
	loadBUF16(i, (int16_t)(yaw * 182.0));
	loadBUF32(i, shmPTR->status);

	if (message.length() && i + message.length() + 2 < TELEMETRY_BUF_SIZE) {
		loadBUF16(i, message.length());
		memcpy(&buf[i], message.c_str(), message.length());
		i += message.length();
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
	shmPTR->telemetry_buf_len = i;
}
//nado echo peredat koordinaty starta i visoti ili luche ih androis socharanaet

TelemetryClass Telemetry;

