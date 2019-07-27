#pragma once

#include "SoftwareInfo.h"

struct VendorInfo;
struct UploadFileInfo;

// ����ϵͳ���
std::vector<std::string> GetSystemDrives();
std::string GetVolumeName(std::string driveName);
std::string GetUserId();

void CreateDir(const char *dir);
bool DirIsValid(string dir);


bool SecLoad(std::string strSysFile);

// �������
void SecInit(std::string strDriveName, std::string strVolumeName, std::string strSidName);

void SecFsdModule(std::string strVolumeName);
void SecRegkeyModule(std::string strDriveName, std::string strSidNam);

void SecDesktopMode();
void SecUseMode(std::string strDriveName, std::string strVolumeName, std::string strSidName);
void SecShutdownMode();

void SecFinish();

//**************string******************//  
// ASCII��Unicode��ת  
wstring AsciiToUnicode(const string& str);
string  UnicodeToAscii(const wstring& wstr);
// UTF8��Unicode��ת  
wstring Utf8ToUnicode(const string& str);
string  UnicodeToUtf8(const wstring& wstr);
// ASCII��UTF8��ת  
string  AsciiToUtf8(const string& str);
string  Utf8ToAscii(const string& str);


// �������
std::string CreateItemXml(std::string strIcon, std::string strName, std::string strOS, std::string strDesc);

std::string CreateInstalledItemXml(VendorInfo& vendor);

std::string CreateIconListItemXml(std::string strIcon, std::string strName, std::string strOS, std::string strDesc);

// Json���
std::vector<SoftwareInfo> PraseJson(std::string strJsonFile);


