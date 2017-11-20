#include "stdafx.h"
#include "MYfILTER.h"


MYfILTER::MYfILTER()
{

	 old_val = 0;
}


MYfILTER::~MYfILTER()
{
}



float MYfILTER::update(const float val) {
	float ret = old_val*0.5 + val*0.5;
	old_val = val;
	return ret;
}