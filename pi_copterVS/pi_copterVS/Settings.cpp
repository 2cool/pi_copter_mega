// 
// 
// 

#include "Settings.h"

#include "Hmc.h"
#include "Balance.h"
#include "Stabilization.h"
#include "commander.h"

#define EEPROM_SIZE 256
char EEPROM_MEM[EEPROM_SIZE];

template <class T> int writeAnything(int ee, const T& value)
{
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		Settings.write(ee++, *p++);
	return i;
}

template <class T> int readAnything(int ee, T& value)
{
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = Settings.read(ee++);
	return i;
}



uint32_t get_hash(const char *mem, const int len) {

	// Hashing
	uint32_t magic = 5381;
	for (int i = 4; i < len; i++)
		magic = ((magic << 5) + magic) + mem[i]; // magic * 33 + c		
	return magic;
}


void SettingsClass::init()
{


}


uint32_t SettingsClass::writeBuf(uint8_t adr, float buf[], uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		writeAnything(i * 4 + adr, buf[i]);
	}
}
uint32_t SettingsClass::readBuf(uint8_t adr, float buf[], uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		readAnything(i * 4 + adr, buf[i]);
	}
}
uint32_t SettingsClass::writeBuf(uint8_t adr, int16_t buf[], uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		writeAnything(i * 2 + adr, buf[i]);
	}
}
uint32_t SettingsClass::readBuf(uint8_t adr, int16_t buf[], uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		readAnything(i * 2 + adr, buf[i]);
	}
}

bool  SettingsClass::saveCompasMotorSettings(float base[]) {

	writeBuf(MOTOR_COMPAS, base, 12);
	uint32_t hash = get_hash((char*)base, 12);
	*(uint32_t*)(EEPROM_MEM + MOTOR_COMPAS_HASH) = hash;
	return write()!=-1;
	
}

bool  SettingsClass::saveCompssSettings(int16_t sh[]) {

	writeBuf(HMC_CALIBR, sh, 6);
	uint32_t hash = get_hash((char*)sh, 6);
	*(uint32_t*)(EEPROM_MEM + HCM_HASH) = hash;
	write();
	return true;
}

bool  SettingsClass::readCompassSettings(int16_t sh[]) {
	readBuf(HMC_CALIBR, sh, 6);
	uint32_t hash=get_hash((char*)sh, 6);
	bool ret = *(uint32_t*)(EEPROM_MEM + HCM_HASH) == hash;
	return ret;
}

bool  SettingsClass::readCompasMotorSettings(float base[]) {
	readBuf(MOTOR_COMPAS, base, 12);
	uint32_t hash = get_hash((char*)base, 12);
	bool ret = *(uint32_t*)(EEPROM_MEM + MOTOR_COMPAS_HASH) == hash;
	return ret;

}








int SettingsClass::read() {

	ifstream f;
	f.open("/home/igor/eeprom.set");
	if (f.is_open()) {
		f.read(EEPROM_MEM, EEPROM_SIZE);
		f.close();
		return 0;
	}
	cout << "\n can't read settings \n";
	return -1;
}


int SettingsClass::read_all() {





	FILE *f = fopen("/home/igor/copter_set.txt", "r");
	if (f == NULL)
	{
		cout << "Error opening file!\n";
		return -1;
	}
	char buf[1000];
////	char c=fgetc(f);
//	cout << c << endl;
	while (true)  {
		char *ret=fgets(buf, 1000, f);
		if (ret == NULL)
			break;
		Commander.Settings(string(buf));
	}

	fclose(f);

	
	return 0;
}

int SettingsClass::write_all() {



	FILE *f = fopen("/home/igor/copter_set.txt", "w");
	if (f == NULL)
	{
		cout << "Error opening file!\n";
		return -1;
	}

	//HMC
	
	string end = ",1,1,1,1,1,1,1,1,1,1,1\n";
	
	fprintf(f, "0,%s", (Balance.get_set()+end).c_str());
	fprintf(f, "1,%s", (Stabilization.get_z_set() + end).c_str());
	fprintf(f, "2,%s", (Stabilization.get_xy_set() + end).c_str());
//	fprintf(f, "3,%s", (Autopilot.get_set() + end).c_str());
	//fprintf(f, "4,%s", (Mpu.get_set() + end).c_str());
	//fprintf(f, "5,%s", (Hmc.get_set() + end).c_str());
	//fprintf(f, "6,%s", (Commander.get_set() + end).c_str());
	//(f, "9,%s", (Hmc.get_calibr_set() + end).c_str());
	fclose(f);
	return 0;

}


int SettingsClass::write() {

	ofstream f;
	f.open("/home/igor/eeprom.set", fstream::in | fstream::out | fstream::trunc);
	if (f.is_open()) {
		f.write(EEPROM_MEM, EEPROM_SIZE);
		f.close();
		return 0;
	}
	cout << "\n cant write settings \n";
	return -1;
}




void SettingsClass::write(int i, char c) {
	if (EEPROM_SIZE > i) {
		EEPROM_MEM[i] = c;
	}
};

char SettingsClass::read(int i) {
	if (EEPROM_SIZE > i) {
		return EEPROM_MEM[i];
	}
	else
		return 0;

}


SettingsClass Settings;

