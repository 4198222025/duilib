#pragma once

std::vector<std::string> GetSystemDrives();
std::string GetVolumeName(std::string driveName);
std::string GetUserId();

void SecInit(std::string strDriveName, std::string strVolumeName, std::string strSidName);