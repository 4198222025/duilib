#pragma once

// ����ϵͳ���
std::vector<std::string> GetSystemDrives();
std::string GetVolumeName(std::string driveName);
std::string GetUserId();

// �������
void SecInit(std::string strDriveName, std::string strVolumeName, std::string strSidName);

void SecFsdModule(std::string strVolumeName);
void SecRegkeyModule(std::string strDriveName, std::string strSidNam);

void SecDesktopMode();
void SecUseMode(std::string strDriveName, std::string strVolumeName, std::string strSidName);
void SecShutdownMode();

void SecFinish();

// �������
std::string CreateItemXml(std::string strIcon, std::string strName, std::string strOS, std::string strDesc);

// Json���
std::vector<std::string> PraseJson(std::string strJsonFile);