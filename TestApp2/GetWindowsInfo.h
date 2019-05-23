
BOOL GetNtVersionNumbers(DWORD&dwMajorVer, DWORD& dwMinorVer, DWORD& dwBuildNumber);

// ---- get os info ---- //
void getOsInfo();

// ---- get cpu info ---- //

void getCpuInfo();

// ---- get memory info ---- //
void getMemoryInfo();

// ---- get harddisk info ---- //
std::string execCmd(const char *cmd);

void getHardDiskInfo();

// ---- get network info ---- //
void getNetworkInfo();


BOOL GetOperatingSystemName(std::string& strWindowsName, DWORD& dwProcessorArchitecture);