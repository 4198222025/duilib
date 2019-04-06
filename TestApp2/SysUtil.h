#pragma once

#include "SoftwareInfo.h"

// 操作系统相关
std::vector<std::string> GetSystemDrives();
std::string GetVolumeName(std::string driveName);
std::string GetUserId();

bool SecLoad(std::string strSysFile);

// 驱动相关
void SecInit(std::string strDriveName, std::string strVolumeName, std::string strSidName);

void SecFsdModule(std::string strVolumeName);
void SecRegkeyModule(std::string strDriveName, std::string strSidNam);

void SecDesktopMode();
void SecUseMode(std::string strDriveName, std::string strVolumeName, std::string strSidName);
void SecShutdownMode();

void SecFinish();

//**************string******************//  
// ASCII与Unicode互转  
wstring AsciiToUnicode(const string& str);
string  UnicodeToAscii(const wstring& wstr);
// UTF8与Unicode互转  
wstring Utf8ToUnicode(const string& str);
string  UnicodeToUtf8(const wstring& wstr);
// ASCII与UTF8互转  
string  AsciiToUtf8(const string& str);
string  Utf8ToAscii(const string& str);


// 界面相关
std::string CreateItemXml(std::string strIcon, std::string strName, std::string strOS, std::string strDesc);

// Json相关
std::vector<SoftwareInfo> PraseJson(std::string strJsonFile);

