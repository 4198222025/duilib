#include "stdafx.h"
#include <vector>
#include <string>

#include "SysUtil.h"

std::vector<std::string> GetSystemDrives(){

	TCHAR  drives[128];				//存储所以驱动器名称
	char* pDrive;				//驱动器指针
	std::vector<std::string> strArray;

	//取得系统的第一个逻辑驱动器
	if (!GetLogicalDriveStrings(sizeof(drives), drives))
	{
		printf("获取驱动器失败\r\n");
		return strArray;
	}
	pDrive = drives; //指向第一个逻辑驱动器
	//将驱动器字符放入列表框中
	while (*pDrive)
	{
		//将驱动器名称加入列表中
		strArray.push_back(pDrive);

		//指向下一个驱动器标识符
		pDrive += strlen(pDrive) + 1;
	}

	for (int i = 0; i < strArray.size(); ++i)
	{
		printf("%ls\r\n", strArray[i].c_str());
	}

	return strArray;
}