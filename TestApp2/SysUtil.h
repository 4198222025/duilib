#pragma once

// 操作系统相关
std::vector<std::string> GetSystemDrives();
std::string GetVolumeName(std::string driveName);
std::string GetUserId();

// 驱动相关
void SecInit(std::string strDriveName, std::string strVolumeName, std::string strSidName);

void SecFsdModule(std::string strVolumeName);
void SecRegkeyModule(std::string strDriveName, std::string strSidNam);

void SecDesktopMode();
void SecUseMode(std::string strDriveName, std::string strVolumeName, std::string strSidName);
void SecShutdownMode();

void SecFinish();

// 界面相关
std::string CreateItemXml(std::string strIcon, std::string strName, std::string strOS, std::string strDesc);

// Json相关
std::vector<std::string> PraseJson(std::string strJsonFile);