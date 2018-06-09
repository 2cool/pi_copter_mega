#include "stdafx.h"
#include "Telemetry.h"


Telemetry::Telemetry()
{
}


Telemetry::~Telemetry()
{
}


uint16_t data[5];
void Telemetry::parser(byte buf[], int n) {


	memcpy((byte*)data, &buf[n], 10);

	m_current[0] = m_current[0] = 0.01953125*(float)(1005 - data[0]);
	m_current[1] = m_current[1] = 0.01953125*(float)(1010 - data[1]);
	m_current[2] = m_current[2] = 0.01953125*(float)(1006 - data[2]);
	m_current[3] = m_current[3] = 0.01953125*(float)(1006 - data[3]);
	voltage = 1.725*(float)(data[4]);

}

	Telemetry tel;