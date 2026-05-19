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

PWSTR downloadsPath;
PWSTR desktopPath = nullptr;
PWSTR folderPath;
PCWSTR folderName = L"Flagged Items";


/*
* Creates and formats the log (.txt) written to the current user's desktop. CHANGE THE SECOND FILENAME TO THE ACUTAL FOLDER'S PLACE!!!!
*/
int createLog() {
	SYSTEMTIME time;
	GetLocalTime(&time); 
	wchar_t filename[500];

	SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &desktopPath);
	swprintf(filename, _countof(filename), L"%s\\Deletion Log %d-%d-%d.txt", desktopPath, time.wMonth, time.wDay, time.wYear);

	std::wofstream file(filename); //create filestream to filename
	
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
		file << "\nYou can review these files at "; file << folderPath; file << " before deletion.\n";
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

int findFiles(PWSTR path, std::wstring folder, bool checkMonth) {

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
			
			printf("Adding (year): ");

			std::cout << file.path().filename();
			std::cout << "\n";
			flaggedFiles.insert(flaggedFiles.end(),file.path().filename().wstring());
			try {
				std::filesystem::copy(file.path(), folder);
				std::filesystem::remove_all(file.path());
				
			}
			catch (std::filesystem::filesystem_error) {
				printf("error moving file / directory: %ws, %ws\n", file.path());
			}
			

		} else if (ymd.month() < currentYmd.month() && checkMonth) {
			printf("Adding (month): ");

			std::cout << file.path().filename();
			std::cout << "\n";
			flaggedFiles.insert(flaggedFiles.end(), file.path().filename().wstring());
		}
		
	}

	return 0;
}

int makeFolder(PWSTR path) { 
	
	if (!std::filesystem::exists(path)) {
		std::filesystem::create_directories(path);
		printf("Deletion Folder Created\n");
		return 0;
	}
	else {
		printf("folder already exists or error finding file.\n");
	}

}

int main() {
	//Get paths of known directories (downloads, desktop, etc) here
	SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &downloadsPath);
	SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &folderPath);
	
	//create folder if not already there.
	printf("%ws\n", folderPath);
	std::wstring folder(folderPath);
	folder.append(L"\\");
	folder.append(folderName);
	makeFolder(folder.data());

	////flag and move files.
	findFiles(downloadsPath, folder, false);

	////create txt log.
	//createLog();

	//cleanup
	CoTaskMemFree(desktopPath);
}