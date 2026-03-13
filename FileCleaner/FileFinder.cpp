/**
* Author: Austin Hall
* Source File that logs all files older than a month old in the downloads folder into a handy .txt
* Will also send the summary as an email!
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
		file << "File Deletion Summary "; file << time.wMonth; file << "/"; file << time.wDay; file << "/"; file << time.wYear; file << ":\n\n";
		file << "We will be deleting the following files off your machine in TWO WEEKS:\n";
		file << "__________________________________________________________________________________\n\n";
		if (flaggedFiles.size() == 0) {
			file << "	No files met the deletion criteria!\n";
		}
		else {
			for (const std::wstring& files : flaggedFiles) {
				file << files; 
				file << "\n";
			}
		}
		file << "__________________________________________________________________________________\n";
		file << "\nYou can review these files at "; file << filename; file << " before deletion.\n";
		file << "\nIf any files listed need to be saved, move them out of the folder and to the SharePoint drive. Thank You!\n";
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