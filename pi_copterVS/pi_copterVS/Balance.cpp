
#include "Balance.h"
#include "MPU6050.h"
#include "define.h"
#include "Stabilization.h"
#include "Telemetry.h"
#include "debug.h"
#include "GPS.h"
#include "Log.h"
#include "mpu.h"







class Pendulum {
private:
	float possition, speed;
	float mass, stiff;
	float k;
public:
	
	Pendulum(float m, float s, float _k) {
		k = 0.1;
		possition = 1;
		mass = 1;
		speed = 10;
		stiff = 1;
	}
	void loop(float pos) {
		const float dt = 0.01;
		float force = possition * stiff + (pos-possition)*k;
		float a = force / mass;
		speed -= a * dt;
		

	}


};






void correct(float & f){
	if (f < 0)
		f = 0;
	else if (f>1)
		f = 1;
}




/*



если скорость радиан в секунду то 0.11

63 грама

2.24-3.01 за 108.088 милесекунды

*/




/*

0         1

+

2    +    3

*/






float MAX_ANGLE__ = 35;

float old[2];
float force[2];
float balanceForce[2];

float dTime[2];
float nTime[2];
#define MIN_SPEED 0.2f
float const predTime = 0.009f;



//float dt = 0.009f;

long oldtttttttttttt = 0;
int cntttttttttt = 0;

//bool tempp1 = false;
//uint32_t taim0;
static const float f_constrain(const float v, const float min, const float max){
	return constrain(v, min, max);
}
void BalanceClass::set_pitch_roll_pids(const float kp, const float ki, const float imax) {
	pids[PID_PITCH_RATE].kP(kp);
	pids[PID_PITCH_RATE].kI(ki);
	pids[PID_PITCH_RATE].imax(imax);

	pids[PID_ROLL_RATE].kP(kp);
	pids[PID_ROLL_RATE].kI(ki);
	pids[PID_ROLL_RATE].imax(imax);
}
void BalanceClass::init()
{
//f/speed^2/0.5=cS;
	//speed^2*0.5*cS=f
	//speed = sqrt(2f / cS)
	//cS = 0.00536;//15 град 
//	0.00357

	min_thr = MIN_THROTTLE_;
	f_[0] = f_[1] = f_[2] = f_[3] = 0;
	cout << "BALANCE INIT\n";
	
	c_pitch = c_roll = 0;
	

	Stabilization.init();
	true_throttle = throttle = 0;
	_max_angle_= MAX_ANGLE_;


	pitch_roll_stabKP = 2;
	
	propeller_lost[0]= propeller_lost[1] = propeller_lost[2] = propeller_lost[3] = false;
	
	set_pitch_roll_pids(0.001, 0.001, 0.2); 

	yaw_stabKP = 2;

	pids[PID_YAW_RATE].kP(0.0017f);
	pids[PID_YAW_RATE].kI(0.0017f);
	pids[PID_YAW_RATE].imax(0.1);

	delay(1500);

	Mpu.init();
	Hmc.init();
	Hmc.loop();
//	Mpu.initYaw(Hmc.heading*RAD2GRAD);
	mid_powerK = 1;
	power_K = 0.1;  //Єто теперь не повер к.
#ifdef DEBUG_MODE
	printf( "Heading :%i\n", (int)Hmc.get_headingGrad());
	
#endif

}


string BalanceClass::get_set(int n){
	
	ostringstream convert;
	if (n == 0) {
		convert << \
			pids[PID_PITCH_RATE].kP() << "," << \
			pids[PID_PITCH_RATE].kI() << "," << \
			pids[PID_PITCH_RATE].imax() << "," << \
			pitch_roll_stabKP << "," << \
			pids[PID_YAW_RATE].kP() << "," << \
			pids[PID_YAW_RATE].kI() << "," << \
			pids[PID_YAW_RATE].imax() << "," << \
			yaw_stabKP << "," << \
			_max_angle_ << "," << \
			power_K << ",";
	}
	else {
		
	}
	string ret = convert.str();
	return string(ret);
}



void BalanceClass::set(const float *ar, int n){
	int i = 0;
	if (ar[SETTINGS_ARRAY_SIZE] == SETTINGS_IS_OK){
		int error=0;
		
		float t;
		if (n == 0) {
			t = pids[PID_PITCH_RATE].kP();
			if ((error += Commander._set(ar[i++], t)) == 0) {
				pids[PID_PITCH_RATE].kP(t);
				pids[PID_ROLL_RATE].kP(t);
			}
			t = pids[PID_PITCH_RATE].kI();
			if ((error += Commander._set(ar[i++], t)) == 0) {
				pids[PID_PITCH_RATE].kI(t);
				pids[PID_ROLL_RATE].kI(t);
			}
			t = pids[PID_PITCH_RATE].imax();
			if ((error += Commander._set(ar[i++], t)) == 0) {
				pids[PID_PITCH_RATE].imax(t);
				pids[PID_ROLL_RATE].imax(t);
			}

			error += Commander._set(ar[i++], pitch_roll_stabKP);

			t = pids[PID_YAW_RATE].kP();
			if ((error += Commander._set(ar[i++], t)) == 0) {
				pids[PID_YAW_RATE].kP(t);
			}
			t = pids[PID_YAW_RATE].kI();
			if ((error += Commander._set(ar[i++], t)) == 0) {
				pids[PID_YAW_RATE].kI(t);
			}
			t = pids[PID_YAW_RATE].imax();
			if ((error += Commander._set(ar[i++], t)) == 0) {
				pids[PID_YAW_RATE].imax(t);
			}
			t = yaw_stabKP;
			if ((error += Commander._set(ar[i++], t)) == 0) {
				yaw_stabKP = t;
			}
			t = _max_angle_;
			if ((error += Commander._set(ar[i++], t)) == 0) {
				_max_angle_ = constrain(t, 15, 35);
			}

			t = power_K;
			if ((error += Commander._set(ar[i++], t,false)) == 0) {

				//power_K = constrain(t, 1, 1.2);
				power_K = t;
			}
			//error += Commander._set(ar[i++], yaw_stabKP);

			//error += Commander._set(ar[i], _max_angle_);

			//	error += Commander._set(ar[i], stop_throttle);

			cout << "balance set:\n";
		}
		else {
			
			}
		if (error == 0){
			//for (ii = 0; ii < i; ii++){
			//	Out.printf(ar[ii]); Out.printf(",");
			//}
			//Out.println(ar[ii]);
			cout << "OK\n";
		}
		else{
			cout << "ERROR to big or small. P="<<error;
		}
	}
	else{
		cout << "ERROR\n";
	}
}

float BalanceClass::powerK(){
	mid_powerK +=(Telemetry.powerK*MS5611.powerK- mid_powerK)*0.001;
	return mid_powerK;
}

void BalanceClass::log() {
	if (Log.writeTelemetry) {
		Log.block_start(LOG::BAL);

		Log.write_bank_cnt();
		Log.loadMem((uint8_t*)f_, 16, false);
		Log.loadInt16t((int)c_roll * 16);
		Log.loadInt16t((int)c_pitch * 16);
		Log.loadInt16t((int)Autopilot.get_yaw() * 16);

		//при лагах в связи c_pitch,c_roll обнуляются.


		Log.block_end();
		Log.end();
	}
}
void BalanceClass::reset() {
	pids[PID_PITCH_RATE].reset_I();
	pids[PID_ROLL_RATE].reset_I();
	pids[PID_YAW_RATE].reset_I();
	c_pitch = c_roll = 0;
	Stabilization.resset_xy_integrator();
	Stabilization.resset_z();
}
/////////////////////////////////////////////////////////////////////////////////////////

#define MAX_ANGLE_SPEED 360
#define MAX_YAW_SPEED 180
//#define MAX_POWER_K_IF_MAX_ANGLE_30 1.12



bool BalanceClass::loop()
{
	
	//double c_timed = Mpu.timed;
	if (!Mpu.loop()) {
		MS5611.loop();
		GPS.loop();
		return false;
	}
	else {
		if (Autopilot.motors_is_on()) { 

			float pK = powerK();
			const float min_throttle = min_thr*pK;
			const float max_throttle = constrain(MAX_THROTTLE_*pK, MAX_THROTTLE_,0.9);

			maxAngle = _max_angle_;
			if (Autopilot.z_stabState()) {
				true_throttle=Stabilization.Z();
				throttle = pK*true_throttle;
				throttle = constrain(throttle, min_throttle, max_throttle);

				const float thr = throttle / Mpu.tiltPower;
				if (thr > max_throttle) {
					maxAngle = RAD2GRAD*acos(throttle / max_throttle);
					maxAngle = constrain(maxAngle,MIN_ANGLE, _max_angle_);
					throttle = max_throttle;
				}
				else {
					throttle = thr;
				}
			}
			else {

				//Stabilization.Z(true);
				true_throttle = Autopilot.get_throttle();
				throttle = true_throttle;
				throttle /= Mpu.tiltPower;
				throttle = constrain(throttle, MIN_THROTTLE_, max_throttle);
				//	Debug.load(0, throttle, f_[0]);

			}

			if (Autopilot.xy_stabState()) {
				Stabilization.XY(c_pitch, c_roll);
			}
			else {
				//Stabilization.XY(c_pitch, c_roll, true);
				c_pitch = Autopilot.get_Pitch();
				c_roll = Autopilot.get_Roll();
			}

			c_pitch = constrain(c_pitch, -maxAngle, maxAngle);
			c_roll = constrain(c_roll, -maxAngle, maxAngle);

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


			static float pitch_stab_output =0;
			static float roll_stab_output = 0;
			static float yaw_stab_output = 0;

#define BAL_F 0.33f

			pitch_stab_output += (f_constrain(pitch_roll_stabKP*(wrap_180(Mpu.get_pitch() - c_pitch)), -MAX_ANGLE_SPEED, MAX_ANGLE_SPEED)-pitch_stab_output)*BAL_F;
			roll_stab_output += (f_constrain(pitch_roll_stabKP*(wrap_180(Mpu.get_roll() - c_roll)), -MAX_ANGLE_SPEED, MAX_ANGLE_SPEED)-roll_stab_output)*BAL_F;
			yaw_stab_output += (f_constrain(yaw_stabKP*wrap_180(-Autopilot.get_yaw() - Mpu.get_yaw()), -MAX_YAW_SPEED, MAX_YAW_SPEED)-yaw_stab_output)*BAL_F;

			//float pitch_gk = min(abs(pitch_stab_output*power_K), 1);
			//float roll_gk = min(abs(roll_stab_output*power_K), 1);
			//заменить на подавление частоти раскачивания.
			// rate PIDS
		//	уменьшить макс дельту думаю из за слишком большой происходит раскачивание
		//		или по крайней мере динамическую дельту надо уменьшить.
			const float max_delta = 0.3;// 0.5;// (throttle < 0.6f) ? 0.3f : MAX_DELTA;

			//напоминание
			//----------------------------------------------------
			static float correction = 1;
			//correction += (0.5 / min(throttle,0.5) - correction)*0.2;
		
			float pitch_output = pK*pids[PID_PITCH_RATE].get_pid(correction*(pitch_stab_output + Mpu.gyroPitch), Mpu.dt);
			pitch_output = constrain(pitch_output, -max_delta, max_delta);
			float roll_output = pK*pids[PID_ROLL_RATE].get_pid(correction*(roll_stab_output + Mpu.gyroRoll), Mpu.dt);
			roll_output = constrain(roll_output, -max_delta, max_delta);
			float yaw_output = pK*pids[PID_YAW_RATE].get_pid(correction*(yaw_stab_output - Mpu.gyroYaw), Mpu.dt);
			yaw_output = constrain(yaw_output, -0.1f, 0.1f);

#ifdef YAW_OFF
			yaw_output = 0;
			pitch_output=0;
#endif

			float m_yaw_output = -yaw_output;  //антираскачивание при низкой мощности на плече
			if ((throttle + yaw_output) < min_throttle)
				yaw_output = min_throttle - throttle;//???????????????????????????????????????????????????
			if ((throttle + m_yaw_output) < min_throttle)
				m_yaw_output = min_throttle - throttle;

			f_[3] = f_constrain((throttle + roll_output + pitch_output + m_yaw_output), STOP_THROTTLE_, FULL_THROTTLE_);
			f_[1] = f_constrain((throttle + roll_output - pitch_output + yaw_output), STOP_THROTTLE_, FULL_THROTTLE_);
			f_[2] = f_constrain((throttle - roll_output + pitch_output + yaw_output), STOP_THROTTLE_, FULL_THROTTLE_);
			f_[0] = f_constrain((throttle - roll_output - pitch_output + m_yaw_output), STOP_THROTTLE_, FULL_THROTTLE_);

			if (Hmc.do_compass_motors_calibr) {
				f_[0] = f_[1] = f_[2] = f_[3] = 0;
				f_[Hmc.motor_index] = 0.5;
			}
			else {
#ifndef FALSE_WIRE
				if (Mpu.timed - Autopilot.time_at_startd < 5 || Autopilot.time_at_startd - Autopilot.old_time_at_startd > 8) {
					f_[0] = f_[1] = f_[2] = f_[3] = throttle = true_throttle = 0.3;//
					//if (Mpu.vibration > 3)
						//Autopilot.off_throttle(true, "VBR");

				}
#endif
				if (throttle < MIN_THROTTLE_) {
					reset();
				}

			}

		}
		else {
			reset();
		}
		
#define MOTORS_OFF
#ifdef MOTORS_OFF
		//f_[0] = f_[1] = f_[2] = f_[3] = 0;
#endif
		//f_[0] = f_[1] = f_[2] = f_[3] = 0.51;
		//f_[0] = f_[1] = 0.502;
		//отключить двигатели при слабом токе
		//if (propeller_lost[0] || propeller_lost[3]) 	f_[0]=f_[3] = 0;
		//if (propeller_lost[1] || propeller_lost[2]) 	f_[1] = f_[2] = 0;
	//	if (f_[0]>=0.4 || f_[1]>0.4 || f_[2]>0.4 || f_[3]>0.4)	f_[0] = f_[1] = f_[2] = f_[3] = 0.3;

		mega_i2c.throttle(f_);  //670 micros
		log();
	}
	Mpu.balance_timed = 0.000001*(double)micros();
	return true;
}



void BalanceClass::set_off_th_() { 
	f_[0] = f_[1] = f_[2] = f_[3] = 0; 
	throttle = true_throttle = 0;
	mega_i2c.throttle(f_);
}









BalanceClass Balance;

