#include "define.h"


// Hmc.h

#ifndef _HMC_h
#define _HMC_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "I2Cdev.h"


#define HMC5883L_ADDRESS            0x1E // this device only has one address
#define HMC5883L_DEFAULT_ADDRESS    0x1E

#define HMC5883L_RA_CONFIG_A        0x00
#define HMC5883L_RA_CONFIG_B        0x01
#define HMC5883L_RA_MODE            0x02
#define HMC5883L_RA_DATAX_H         0x03
#define HMC5883L_RA_DATAX_L         0x04
#define HMC5883L_RA_DATAZ_H         0x05
#define HMC5883L_RA_DATAZ_L         0x06
#define HMC5883L_RA_DATAY_H         0x07
#define HMC5883L_RA_DATAY_L         0x08
#define HMC5883L_RA_STATUS          0x09
#define HMC5883L_RA_ID_A            0x0A
#define HMC5883L_RA_ID_B            0x0B
#define HMC5883L_RA_ID_C            0x0C

#define HMC5883L_CRA_AVERAGE_BIT    6
#define HMC5883L_CRA_AVERAGE_LENGTH 2
#define HMC5883L_CRA_RATE_BIT       4
#define HMC5883L_CRA_RATE_LENGTH    3
#define HMC5883L_CRA_BIAS_BIT       1
#define HMC5883L_CRA_BIAS_LENGTH    2

#define HMC5883L_AVERAGING_1        0x00
#define HMC5883L_AVERAGING_2        0x01
#define HMC5883L_AVERAGING_4        0x02
#define HMC5883L_AVERAGING_8        0x03

#define HMC5883L_RATE_0P75          0x00
#define HMC5883L_RATE_1P5           0x01
#define HMC5883L_RATE_3             0x02
#define HMC5883L_RATE_7P5           0x03
#define HMC5883L_RATE_15            0x04
#define HMC5883L_RATE_30            0x05
#define HMC5883L_RATE_75            0x06

#define HMC5883L_BIAS_NORMAL        0x00
#define HMC5883L_BIAS_POSITIVE      0x01
#define HMC5883L_BIAS_NEGATIVE      0x02

#define HMC5883L_CRB_GAIN_BIT       7
#define HMC5883L_CRB_GAIN_LENGTH    3

#define HMC5883L_GAIN_1370          0x00
#define HMC5883L_GAIN_1090          0x01
#define HMC5883L_GAIN_820           0x02
#define HMC5883L_GAIN_660           0x03
#define HMC5883L_GAIN_440           0x04
#define HMC5883L_GAIN_390           0x05
#define HMC5883L_GAIN_330           0x06
#define HMC5883L_GAIN_220           0x07

#define HMC5883L_MODEREG_BIT        1
#define HMC5883L_MODEREG_LENGTH     2

#define HMC5883L_MODE_CONTINUOUS    0x00
#define HMC5883L_MODE_SINGLE        0x01
#define HMC5883L_MODE_IDLE          0x02

#define HMC5883L_STATUS_LOCK_BIT    1
#define HMC5883L_STATUS_READY_BIT   0



class HmcClass
{
 protected:
	 void log();
	 void motTest(const float, const float, const float);
	 int16_t baseX, baseY, baseZ;
	 
	float   dx, dy, dz;


 public:
	 void start_motor_compas_calibr();
	 unsigned long startTime;
	 int motor_index;
	 bool compas_motors_calibr;
	 float base[12];
	 bool calibrated,motors_calibrated;
	 string get_set();
	 void set(const float buf[]);
	// int16_t mx, my, mz;
	 bool motors_power_on;
	 float heading;
	void init();
	void loop();
	bool ok;
	bool calibration(const bool newc=false);
	bool selfTest();

	float get_headingGrad(){ return heading*RAD2GRAD; }



	HmcClass();
	HmcClass(uint8_t address);

	void initialize();
	bool testConnection();

	// CONFIG_A register
	uint8_t getSampleAveraging();
	void setSampleAveraging(uint8_t averaging);
	uint8_t getDataRate();
	void setDataRate(uint8_t rate);
	uint8_t getMeasurementBias();
	void setMeasurementBias(uint8_t bias);

	// CONFIG_B register
	uint8_t getGain();
	void setGain(uint8_t gain);

	// MODE register
	uint8_t getMode();
	void setMode(uint8_t mode);

	// DATA* registers
	void getHeading(int16_t *x, int16_t *y, int16_t *z);
	int16_t getHeadingX();
	int16_t getHeadingY();
	int16_t getHeadingZ();

	// STATUS register
	bool getLockStatus();
	bool getReadyStatus();

	// ID_* registers
	uint8_t getIDA();
	uint8_t getIDB();
	uint8_t getIDC();

private:
	float old_heading;
	void newCalibration(int16_t sh[]);
	uint8_t devAddr;
	uint8_t buffer[6];
	uint8_t mode;




};

extern HmcClass Hmc;

#endif
