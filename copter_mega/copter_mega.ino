




#include <stdint.h>
#include <Wire.h>
#include "Adafruit_NeoPixel-master\Adafruit_NeoPixel.h"
#ifdef __AVR__
#include <avr/power.h>
#endif


#define PIN           27

#define NUMPIXELS      8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


#include "gps.h"


#define _16MHZ CS10
#define _8MHZ CS11

#define MAX90_THROTTLE 30400
#define pwm_MIN_THROTTLE 17600
#define pwm_MAX_THROTTLE 32000
#define pwm_OFF_THROTTLE 16000

#define SIM800 Serial2
#define BUZZER 30
#define RING 31


#define OCR0 OCR4C
#define OCR1 OCR5A
#define OCR2 OCR3A
#define OCR3 OCR4A
#define OCR_GP OCR3C
#define OCR_GR OCR5C

volatile uint8_t beep_code = 0;

uint8_t beeps_coder[] = { 0, B00001000,B00001001,B00001010,B00001011,B00001100,B00001101,B00001110,B00001111,B00000001,B00000010,B00000011,B00000100,B00000101,B00000110,B00000111 };//4 beeps if 0 short 1 long beep


#define b10 {100,0,0}
#define b01 {0,100,0}
#define b00 {0,0,0}
#define b11 {100,100,0}
uint8_t beeps_led[][2][3]{ {b00,b00}, { b10,b00},{b10,b01},{ b10,b01 },{b10,b11},{b11,b00},{b11,b01},{b11,b10},{b11,b11},{b00,b01,},{b00,b11},{b01,b00},{b01,b01},{b01,b10},{b01,b11} };

volatile bool ring=false;





volatile uint8_t pi_copter_color[8][3]={ {0,1,0},{ 0,1,0 },{ 0,1,0 },{ 0,1,0 },{ 0,1,0 },{ 0,1,0 },{ 0,1,0 },{ 0,1,0 } };



enum {eNO_CON,eRING};
volatile uint8_t col[][3] = { { 1,0,0 },{255,0,0} };
volatile bool  do_sound = true;




volatile uint8_t new_colors_i = 0;
bool ring_was = false;

#define PAUSE_TIME 100
#define LONG_BEEP 600
#define SHORT_BEEP 200

uint8_t beep_bit_n = 0;
uint32_t beep_time = 0;
uint8_t old_beep_code = 255;
void beep() {

	if (old_beep_code != beep_code)
	{
		old_beep_code = beep_code;
		for (int i = 0; i < 8; i += 2)
		{
			pixels.setPixelColor(i, pixels.Color(beeps_led[beep_code][0][0], beeps_led[beep_code][0][1], beeps_led[beep_code][0][2]));
			pixels.setPixelColor(i + 1, pixels.Color(beeps_led[beep_code][1][0], beeps_led[beep_code][1][1], beeps_led[beep_code][1][2]));
		}
		pixels.show();

	}
	if (beep_time == 0) {
		beep_time = millis() + ((beeps_coder[beep_code] & 1) ? LONG_BEEP : SHORT_BEEP);
		beep_bit_n = 0;
		digitalWrite(BUZZER, HIGH&do_sound);
	}
	else {
		if (beep_bit_n & 1) { //pause
			if (millis() > beep_time) {
				digitalWrite(BUZZER, HIGH&do_sound);
				beep_bit_n++;
				beep_time = millis() + (((beeps_coder[beep_code] >> (beep_bit_n >> 1)) & 1) ? LONG_BEEP : SHORT_BEEP);
			}
		}
		else {	//beep
			if (millis() > beep_time) {
				digitalWrite(BUZZER, LOW);
				beep_bit_n++;
				if (beep_bit_n >= 8) {
					beep_code = 0;
					beep_time = 0;
					old_beep_code = 255;
					for (int i = 0; i<8; i++)
						pixels.setPixelColor(i, pixels.Color(pi_copter_color[i][0], pi_copter_color[i][1], pi_copter_color[i][2]));
					pixels.show();
				}
				else
					beep_time = millis() + PAUSE_TIME;
			}
		}
	}

}

void stop_motors() {
	OCR0 = pwm_OFF_THROTTLE;
	OCR1 = pwm_OFF_THROTTLE;
	OCR2 = pwm_OFF_THROTTLE;
	OCR3 = pwm_OFF_THROTTLE;
}

void on(const uint16_t COUNTER, const uint16_t throthle)
{
	//old_g_pitch = 1000;

	//timer1		A 11,B 12
	//timer3		A 5,B 2,C 3
	//timer4		A 6,B 7,C o8
	//timer5		A44,B45,C46
	pinMode(46, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(8, OUTPUT);

	pinMode(44, OUTPUT);
	//pinMode(45, OUTPUT);

//	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);

	TCCR3A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM13) | _BV(WGM12) | _BV(WGM11);
	TCCR3B = _BV(_16MHZ) | _BV(WGM12) | _BV(WGM13);
	TCCR4A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM13) | _BV(WGM12) | _BV(WGM11);
	TCCR4B = _BV(_16MHZ) | _BV(WGM12) | _BV(WGM13);
	TCCR5A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM13) | _BV(WGM12) | _BV(WGM11);
	TCCR5B = _BV(_16MHZ) | _BV(WGM12) | _BV(WGM13);
	ICR3 = ICR4 = ICR5 = COUNTER;
	OCR_GP = pwm_OFF_THROTTLE + (pwm_OFF_THROTTLE / 2);
	OCR_GR = pwm_OFF_THROTTLE + (pwm_OFF_THROTTLE / 2);
	OCR0 = OCR1 = OCR2 = OCR3 =  throthle;
	//OCR4B = 0;// 35000;
}

void throttle_0(const float n) {
	OCR4C = pwm_OFF_THROTTLE + (uint16_t)(n*pwm_OFF_THROTTLE);//3
}//8
void throttle_1(const float n) {
	OCR5A = pwm_OFF_THROTTLE + (uint16_t)(n*pwm_OFF_THROTTLE);//0
}//46 

void throttle_2(const float n) {
	OCR3A = pwm_OFF_THROTTLE + (uint16_t)(n*pwm_OFF_THROTTLE); //1
}//5
void throttle_3(const float n) {
	OCR4A = pwm_OFF_THROTTLE + (uint16_t)(n*pwm_OFF_THROTTLE);//2
}//6


enum COMMANDS_BIT { SOUND_ON = 1, BEEP_CODE = 30 };



volatile uint8_t cnt = 0;
volatile bool beep_on = false;


uint8_t old_cnt = 0;
uint16_t err = 0;
int temp;
#define writePWM8(n)					\
		temp = 126*Wire.read();			\
		n=constrain(temp,pwm_OFF_THROTTLE,pwm_MAX_THROTTLE);\

#define writePWM16(n)						\
		temp = Wire.read()+Wire.read()*256;	\
		n=constrain(temp,pwm_OFF_THROTTLE,pwm_MAX_THROTTLE);\



char inBuf[32];
volatile uint8_t reg=15;
void receiveEvent(int countToRead) {
	if (countToRead == 1) {
		reg = Wire.read();
		return;
	}


	//int av = Wire.available();
	Wire.readBytes(inBuf, countToRead);
	
	switch (inBuf[0] & 7)
	{
	case 0:
	{
		uint8_t len = inBuf[0] >> 3;
		uint16_t temp = *((uint16_t*)&inBuf[1]);
		if (len == 0) {
			OCR0 = constrain(temp, pwm_OFF_THROTTLE, pwm_MAX_THROTTLE);
			temp = *((uint16_t*)&inBuf[3]);
			OCR1 = constrain(temp, pwm_OFF_THROTTLE, pwm_MAX_THROTTLE);
			temp = *((uint16_t*)&inBuf[5]);
			OCR2 = constrain(temp, pwm_OFF_THROTTLE, pwm_MAX_THROTTLE);
			temp = *((uint16_t*)&inBuf[7]);
			OCR3 = constrain(temp, pwm_OFF_THROTTLE, pwm_MAX_THROTTLE);
		}
		else {
			OCR_GP = constrain(temp, pwm_OFF_THROTTLE, pwm_MAX_THROTTLE);
			temp = *((uint16_t*)&inBuf[3]);
			OCR_GR = constrain(temp, pwm_OFF_THROTTLE, pwm_MAX_THROTTLE);
			//Serial.print(OCR_GR); Serial.print(" "); Serial.println(OCR_GP);

		}
		break;
	}
	case 1:
	{
		beep_code=inBuf[1];
		//Serial.println(beep_code);
		break;
	}
	case 2: 
	{
		uint8_t len = countToRead-1;
		SIM800.write(&inBuf[1], len);
		//Serial.println(2);
		break;
	}
	case 3: 
	{   new_colors_i = inBuf[0] >> 3;
		pi_copter_color[new_colors_i - 1][0] = *(uint8_t*)&inBuf[1];
		pi_copter_color[new_colors_i - 1][1] = *(uint8_t*)&inBuf[2];
		pi_copter_color[new_colors_i - 1][2] = *(uint8_t*)&inBuf[3];
	}
	default: //setup
	{
		if (countToRead == 7)
		{
			enum { eNO_CON, eRING };
			uint8_t mask = inBuf[0] >> 3;
			do_sound = (mask & 1);
			memcpy((uint8_t*)col, (uint8_t*)&inBuf[1], 6);
		}
	}
	}
	cnt++;
}

//	A0	-	3v from balance
//	A1	-	6v form balance
//	A2	-	9v form power


volatile float fb[5];

uint16_t readBuffer[5];
char buf[64];

byte gps_buf[12];

uint8_t old_gps_c = 100;



volatile char ret = 0;
char simbuf[16];
void requestEvent() {



	cnt++;


	switch (reg) {
	case 0: 
	{
		readBuffer[0] = fb[0];
		readBuffer[1] = fb[1];
		readBuffer[2] = fb[2];
		readBuffer[3] = fb[3];
		readBuffer[4] = fb[4];
		Wire.write((char*)readBuffer, 10);
		break;
	}
	case 1: 
	{
		ret = ring;
		while (gps.available()) {
			uint8_t r = processGPS();
			if (r>0 && r!=old_gps_c) {
				old_gps_c = r;
				//ret = sizeof(SEND_I2C);
				ret |= 2;
				//Serial.println(ret);
				reg = 2;
				break;
			}
		}
	
		Wire.write(sizeof(SEND_I2C));
		break;
	}
	case 2: 
	//	Serial.println(gps_data()->lon);
	//	Serial.println("send");
		Wire.write((char*)gps_data(), sizeof(SEND_I2C));
		break;
	case 3:
	{
		if (ret = SIM800.available()) {
			if (ret > 16)
				ret = 16;
			Wire.write(ret);
			reg = 4;
			//Serial.print("sim ");
			//Serial.println((int)ret);
		}
		break;
	}
	case 4: 
	{

		SIM800.readBytes(simbuf, ret);
		Wire.write(simbuf, ret);
		
		//Serial.println("sended");
	}
	

	}
}


//#define ESC_CALIBR
void setup()
{
#ifdef ESC_CALIBR
	on(48000, pwm_MAX_THROTTLE);
	delay(3000);
	OCR0 = OCR1 = OCR2 = OCR3 = pwm_OFF_THROTTLE;
#else
	on(48000, pwm_OFF_THROTTLE);
#endif
	//Serial.begin(9600);
	//while (!Serial);
	SIM800.begin(9600);
	gps_setup();
	pinMode(BUZZER, OUTPUT);
	pinMode(RING, INPUT);
	digitalWrite(BUZZER, LOW);
	analogReference(INTERNAL2V56);

	Wire.begin(9);
	Wire.onRequest(requestEvent); // data request to slave
	Wire.onReceive(receiveEvent); // data slave received
	pixels.begin(); // This initializes the NeoPixel library.



	for (int i=0; i<8; i++)
		pixels.setPixelColor(i, pixels.Color(0, 1, 0)); // Moderately bright green pi_copter_color.
	pixels.show();


}
///thr0 = m1
//thr1 = m0
//thr2 =
//thr3 =
float thr;
float i[5] = { 0,0,0,0,0 };


#define MI0 PIN_A2
#define MI1 PIN_A0
#define MI2 PIN_A6
#define MI3 PIN_A4
#define BAT PIN_A1

void loop()
{
	

	const float CF = 0.01;

	fb[0] += ((float)(analogRead(MI0)) - fb[0])*CF;
	fb[1] += ((float)(analogRead(MI1)) - fb[1])*CF;
	fb[2] += ((float)(analogRead(MI2)) - fb[2])*CF;
	fb[3] += ((float)(analogRead(MI3)) - fb[3])*CF;
	fb[4] += ((float)(analogRead(BAT)) - fb[4])*CF;

	if (beep_code)
		beep();
	else {

		ring = digitalRead(31) == LOW;

		if (millis() > 5000 && ring) {
			bool puls = (micros() & (unsigned long)65536) == 0;
			for (int i = 0; i<8; i++)
				if (puls)
					pixels.setPixelColor(i, pixels.Color(col[eRING][0], col[eRING][1], col[eRING][2])); // Moderately bright green pi_copter_color.
				else
					pixels.setPixelColor(i, pixels.Color(col[eRING][1],col[eRING][0],  col[eRING][2])); // Moderately bright green pi_copter_color.
			pixels.show();
			if (do_sound)
				digitalWrite(BUZZER, puls);
			ring_was = true;
		}
		else 
		{
			bool alarm = (fb[4] > (1210.0 / 1.725) && fb[4] < (1320.0 / 1.725));
			digitalWrite(BUZZER, alarm);

		}
	}
	//3.3 - alarm ;3.6 red

	//Serial.print(fb[0]); Serial.print(" "); Serial.print(fb[1]); Serial.print(" "); Serial.print(fb[2]); Serial.print(" "); Serial.print(fb[3]); Serial.print(" "); Serial.println(fb[4]);


	if (cnt != old_cnt) {
		old_cnt = cnt;
		err = 0;

	}
	else {
		err++;
		if (err > 300) {
			stop_motors();
			for (int i = 0; i<8; i++)
				pixels.setPixelColor(i, pixels.Color(col[eNO_CON][0], col[eNO_CON][1], col[eNO_CON][2]));
			pixels.show();
		}
	}

	if (ring_was && !ring) {
		for (int i=0; i<8; i++)
			pixels.setPixelColor(i, pixels.Color(pi_copter_color[i][0], pi_copter_color[i][1], pi_copter_color[i][2]));
		pixels.show();
		ring_was = false;
	}

	if (new_colors_i) {
		new_colors_i--;
		pixels.setPixelColor(new_colors_i, pixels.Color(pi_copter_color[new_colors_i][0], pi_copter_color[new_colors_i][1], pi_copter_color[new_colors_i][2]));
		pixels.show();
		
		new_colors_i = 0;
	}

	

	


	//------------------------------------------------------------------------------------------------------------------------------

}
