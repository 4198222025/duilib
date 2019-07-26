#pragma once

string PrasePackageId(string response);

std::vector<UploadFileInfo> PrasePackageFileInfo(string packagefileinfostr);

std::vector<SoftwareInfo> CallQueryPackageService(string url, string category, int page, int pageCount);

