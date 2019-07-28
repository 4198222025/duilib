#pragma once

struct YzHttpUtil{

	static string m_strServerName;
	static string m_strServerPort;

	static void InitServerInfo(string serverName, string serverPort);

	static string PrasePackageId(string response);

	static std::vector<UploadFileInfo> PrasePackageFileInfo(string packagefileinfostr);

	static std::vector<SoftwareInfo> CallQueryPackageService(string url, string category, int page, int pageCount);

	static bool DownloadFile(string url, string strLocalFilePath);
};

