/*
* Author: Austin Hall
* Source File that logs all files older than a month old in the downloads folder into a handy .txt
* Will also send the summary as an email!
*/
#include <windows.h>
#include <ShlObj.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string.h>
#include <iostream>

std::vector<std::wstring> flaggedFiles;
PWSTR desktopPath = nullptr;


/*
* Creates and formats the log (.txt) written to the current user's desktop. CHANGE THE SECOND FILENAME TO THE ACUTAL FOLDER'S PLACE!!!!
*/
int createLog() {
	SYSTEMTIME time;
	GetLocalTime(&time); 
	wchar_t filename[500];

	SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &desktopPath);
	swprintf(filename, _countof(filename), L"%s\\Deletion Log %d-%d-%d.txt", desktopPath, time.wMonth, time.wDay, time.wYear);

	std::wofstream file(filename); //create filstream to filename
	
	if (file.is_open()) {
		file << "File Deletion Summary "; file << time.wMonth; file << "/"; file << time.wDay; file << "/"; file << time.wYear; file << ":\n\n";
		file << "The following files have not been used in the past month, and have been flagged for deletion in TWO weeks:\n";
		file << "_________________________________________________________________________________\n\n";
		if (flaggedFiles.size() == 0) {
			file << "	No files met the deletion criteria!\n";
		}
		else {
			for (const std::wstring& files : flaggedFiles) {
				file << "	";
				file << files; 
				file << "\n";
			}
		}
		file << "__________________________________________________________________________________\n";
		file << "\nYou can review these files at "; file << filename; file << " before deletion.\n";
		file << "\nIf any files listed need to be saved, move them out of the folder and to the SharePoint drive before they are lost forever. Thank You!\n";
		file << "\n\n* Feel free to delete this log after you are done reading. *";
		file.close();
		printf("File created successfully!\n");
	}
	else {
		printf("Error creating file!!!!\n");
		return 0;
	}
	return 1;

}

int findFiles(PWSTR path) {

	//we need all of this to convert the current system clock to a point where we can get individual yy mm dd
	auto currentTime = std::chrono::system_clock::now();
	auto currentTimeCast = std::chrono::clock_cast<std::chrono::system_clock>(currentTime);
	auto currentTimeFloor = std::chrono::floor<std::chrono::days>(currentTimeCast);
	std::chrono::year_month_day currentYmd{ currentTimeFloor };


	printf("Current Time:");
	
	std::cout << currentTime;
	std::cout << "\n";


	for (const auto& file : std::filesystem::directory_iterator(path)) {
		auto writeTime = file.last_write_time();
		auto clockCast = std::chrono::clock_cast<std::chrono::system_clock>(writeTime);
		auto clockFloor = std::chrono::floor<std::chrono::days>(clockCast);
		std::chrono::year_month_day ymd{clockFloor};
		if (ymd.year() < currentYmd.year()) {
			
			printf("Adding file to vector (year): ");

			std::cout << file.path().filename();
			std::cout << "\n";
			flaggedFiles.insert(flaggedFiles.end(),file.path().filename().wstring());
		}
		else if (ymd.month() < currentYmd.month()) { //same year different month
			printf("Adding file to vector (month): ");

			std::cout << file.path().filename();
			std::cout << "\n";
			flaggedFiles.insert(flaggedFiles.end(), file.path().filename().wstring());
		}

		
		//if (file.last_write_time() < 0) {
		//	printf("%s\n", file.path().filename().string().c_str());
		//}
		
	}

	return 0;
}

int main() {
	PWSTR path;
	SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &path);
	printf("%ws\n",path);
	findFiles(path);
	createLog();
	CoTaskMemFree(desktopPath);
}