/**
* Author: Austin Hall
* Source File that logs all files older than a month old in the downloads folder
*/
#include <windows.h>
#include <ShlObj.h>
#include <fstream>
#include <vector>
#include <string.h>

std::vector<std::wstring> flaggedFiles;
PWSTR desktopPath = nullptr;

int createLog() {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filename[500];

	SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &desktopPath);
	swprintf(filename, _countof(filename), L"%s\\Deletion Log %d-%d-%d.txt", desktopPath, time.wMonth, time.wDay, time.wYear);

	std::wofstream file(filename); //create filstream to filename
	
	if (file.is_open()) {
		file << filename;
		file << "\n\n";
		file << "We will be deleting the following files off your machine in TWO WEEKS:\n";
		for (const std::wstring& files : flaggedFiles) {
			file << files;
			file << "\n";
		}
		file << "You can review these files at [FOLDER PATH] before deletion.";
		file.close();
		printf("File created successfully!\n");
	}
	else {
		printf("Error creating file!!!!\n");
		return 0;
	}
	return 1;

}

int main() {
	createLog();
	CoTaskMemFree(desktopPath);
}