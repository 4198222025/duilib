#pragma once

#include <string>
#include <vector>

#include "SoftwareInfo.h"

struct VendorInfo
{
	std::string name;
	std::vector<SoftwareInfo>  arrSoftware;
};