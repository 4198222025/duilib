#pragma once

#include <string>

struct UploadFileInfo
{
	std::string uuid;
	std::string dir;
	std::string name;
	std::string crc;
	long size;
};