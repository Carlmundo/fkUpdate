#include "pch.h"
#define WIN32_LEAN_AND_MEAN
#include <winInet.h>
#include <string>
#include <windows.h>	
#pragma comment(lib, "wininet.lib")
#pragma comment(lib,"Version.lib")

HMODULE self_module;
HANDLE main_thread;
std::string status;

int compareVersion(std::string version1, std::string version2) {
	int i = 0, j = 0, n1 = version1.size(), n2 = version2.size();
	while (i < n1 || j < n2) {
		int num1 = 0, num2 = 0;
		while (i < n1 && version1[i] != '.')
			num1 = num1 * 10 + (version1[i++] - '0');
		while (j < n2 && version2[j] != '.')
			num2 = num2 * 10 + (version2[j++] - '0');
		if (num1 > num2)
			return 1;
		else if (num1 < num2)
			return -1;

		i++;
		j++;
	}
	return 0;
}
std::string getVersionCurrent() {
	std::string moduleName = "fkUpdate.dll";
	DWORD  zero = 0;
	int verInfoLen = 0;
	BYTE* verInfo = NULL;
	VS_FIXEDFILEINFO* fileInfo = NULL;
	UINT len = 0;

	/* Get the size of FileVersionInfo structure */
	verInfoLen = GetFileVersionInfoSize(moduleName.c_str(), &zero);
	if (verInfoLen == 0) {
		printf("GetFileVersionInfoSize() Failed!");
		return "0";
	}

	/* Get FileVersionInfo structure */
	verInfo = new BYTE[verInfoLen];
	if (!GetFileVersionInfo(moduleName.c_str(), zero, verInfoLen, verInfo)) {
		printf("GetFileVersionInfo Failed!");
		return "0";
	}

	/* Query for File version details. */
	if (!VerQueryValue(verInfo, "\\", (LPVOID*)&fileInfo, &len)) {
		printf("VerQueryValue Failed!");
		return "0";
	}

	if (len > 0)
	{
		char buf[16];
		int len = sprintf_s(buf, "%d.%d.%d.%d",
			HIWORD(fileInfo->dwFileVersionMS),
			LOWORD(fileInfo->dwFileVersionMS),
			HIWORD(fileInfo->dwFileVersionLS),
			LOWORD(fileInfo->dwFileVersionLS)
		);

		return std::string(buf, len);
	}
	else
	{
		return std::string("0");
	}
}
int updateCheck() {
	//Set error status
	int updateError = 0;
	//Attempt to open an internet connection
	HINTERNET hInternet = InternetOpen(0, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInternet != NULL){
		//Update URL
		const char* versionURL = "https://raw.githubusercontent.com/Carlmundo/Worms2-Website/main/version.txtc";
		const char* openWebsite = "explorer \"https://github.com/Carlmundo/W2-Plus/releases/latest\"";

		//Attempt to open URL
		HINTERNET hFile = InternetOpenUrl(hInternet, versionURL, NULL, 0, INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD, NULL);
		//Set var in advance for MessageBox
		int result;
		if (hFile != NULL) {	
			//Constant to hold the read size(1k)
			const int downloadBufferSize = 1024;
			//Store all of the downloaded data
			std::string downloadContents = "";

			char* downloadBuffer = new char[downloadBufferSize];
			DWORD availableSize = 0;
			DWORD bytesRead = 0;
			do {
				//Get size of data
				InternetQueryDataAvailable(hFile, &availableSize, 0, 0);
				//Read only as much data as is available or the downloadBufferSize
				if (availableSize > downloadBufferSize)
					availableSize = downloadBufferSize;
				//Read the current bytes from the online file
				InternetReadFile(hFile, downloadBuffer, availableSize, &bytesRead);
				//Append the buffer to the download contents
				downloadContents.append(downloadBuffer, availableSize);
			} while (bytesRead != 0);

			//Close the handles.
			InternetCloseHandle(hFile);
			InternetCloseHandle(hInternet);
			
			if (downloadContents.size() <= 16) {
				std::string versionCurrent = getVersionCurrent();
				if (versionCurrent != "0") {
					int versionDiff = compareVersion(downloadContents, versionCurrent);
					if (versionDiff == 1) {
						std::string strUpdate = "A new update is available: " + downloadContents + "\n" + "Do you want to download it now?";
						result = MessageBox(NULL, strUpdate.c_str(), "Update Available", MB_YESNO);
						if (result == IDYES)
						{
							WinExec(openWebsite, SW_HIDE);
							return 0;
						}
					}
				}
				else {
					updateError = 1;
				}
			}
			else {
				updateError = 1;
			}
		}
		else {
			updateError = 1;
			InternetCloseHandle(hInternet);
		}
		if (updateError == 1) {
			LPCTSTR strError = "Unable to check for updates. Do you want to check the website yourself?";
			result = MessageBox(NULL, strError, "Error", MB_YESNO);
			if (result == IDYES)
			{
				WinExec(openWebsite, SW_HIDE);
			}
		}
	}
}

BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			self_module = hinstDLL;
			main_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)updateCheck, NULL, 0, NULL);
			if (main_thread == NULL) {
				return false;
			}
		}
		break;

		case DLL_PROCESS_DETACH:
			break;
		}
	return TRUE;
}