/*
* Author: Austin Hall
* Program that sorts and logs all files older than a month old in the downloads folder into a handy .txt
* Will also send the summary as an email hopefully using smtp relay!
*/
#include <windows.h>
#include <ShlObj.h>
#include <string.h>
#include <atlstr.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>

std::vector<std::wstring> flaggedFiles;
PWSTR downloadsPath;
PWSTR logPath;
PWSTR folderPath;
PWSTR folderName;
bool debug = true;

/*
* Creates and formats the log (.txt) written to the current user's desktop. 
*/
int createLog() {
	SYSTEMTIME time;
	GetLocalTime(&time); 
	wchar_t filename[500];

	if (logPath == NULL) {
		SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &logPath);
	}
	swprintf_s(filename, _countof(filename), L"%s\\Deletion Log %d-%d-%d.txt", logPath, time.wMonth, time.wDay, time.wYear);

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
		file << "\nYou can review these files at "; file << logPath; file << " before deletion.\n";
		file << "\nIf any files listed need to be saved, move them out of the folder and to the SharePoint drive before they are lost forever. Thank You!\n";
		file << "\n\n* Feel free to delete this log after you are done reading. *";
		file.close();

		if (debug) {
			printf_s("Log file created successfully!\n");
		}
	}
	else {
		if (debug) {
			printf_s("Error creating file!!!!\n");
		}
		return 1;
	}
	return 0;

}

/*
* This function iterates through the path folder, and if it meets the criteria for deletion, will be moved to the folder specified. 
* bool checkmonth - if true we check the month var, mostly for testing
*/
int findFiles(bool checkMonth) {

	//we need all of this to convert the current system clock to a point where we can get individual yy mm dd
	auto currentTime = std::chrono::system_clock::now();
	auto currentTimeCast = std::chrono::clock_cast<std::chrono::system_clock>(currentTime);
	auto currentTimeFloor = std::chrono::floor<std::chrono::days>(currentTimeCast);
	std::chrono::year_month_day currentYmd{currentTimeFloor};
	if (debug) {
		printf_s("Current Time:");
		std::cout << currentTime;
		std::cout << "\n";
	}

	for (const auto& file : std::filesystem::directory_iterator(downloadsPath)) {
		auto writeTime = file.last_write_time();
		auto clockCast = std::chrono::clock_cast<std::chrono::system_clock>(writeTime);
		auto clockFloor = std::chrono::floor<std::chrono::days>(clockCast);
		std::chrono::year_month_day ymd{clockFloor};

		if (ymd.year() < currentYmd.year()) {
			if (debug) {
				printf_s("Adding (year): ");
				std::cout << file.path().filename();
				std::cout << "\n";
			}

			flaggedFiles.insert(flaggedFiles.end(),file.path().filename().wstring());
			try {
				std::filesystem::copy(file.path(), folderPath);
				std::filesystem::remove_all(file.path());
			}
			catch (std::filesystem::filesystem_error) {
				if (debug) {
					printf_s("error moving file or directory\n");
				}
			}
		} else if (ymd.month() < currentYmd.month() && checkMonth) {
			if (debug) {
				printf_s("Adding (month): ");
				std::cout << file.path().filename();
				std::cout << "\n";
			}

			flaggedFiles.insert(flaggedFiles.end(), file.path().filename().wstring());
		}
	}

	return 0;
}

/**
* creates the folder that the flagged files will be copied to.
*/
int makeFolder() { 

	if (folderPath == NULL) {
		SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &folderPath);
	}
	if (folderName == NULL) {
		folderName = const_cast<PWSTR>(L"Flagged Items");
	}

	std::wstring folder(folderPath);
	folder.append(L"\\");
	folder.append(folderName);
	if (debug) {
		printf("%ws\n%ws\n", folderPath, folder.data());
	}
	PWSTR path = folder.data();
	
	if (!std::filesystem::exists(path)) {
		std::filesystem::create_directories(path);
		if (debug) {
			printf("Deletion Folder Created\n");

		}
		return 0;
	}
	else {
		if (debug) {
			printf("folder already exists or error finding file.\n");
		}
		return 1;
	}
}

/*
* Format Args: FileCleaner.exe [bool debug] [str FOLDERPATH] [str FOLDERNAME]
*/
int main(int numArgs, char* args[]) {

	//we know for sure we want the user's downloads folder
	SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &downloadsPath);

	if (numArgs > 1) {
		if (numArgs == 2) {
			if (strcmp(args[1], "true") == 0) {
				debug = true;
			}
		}
		else if (numArgs == 4) {
			CStringW temp = CStringW(args[2]);
			folderPath = temp.GetBuffer(temp.GetLength());
			temp = CStringW(args[3]);
			folderName = temp.GetBuffer(temp.GetLength());
			if (strcmp(args[1], "true") == 0) {
				debug = true;
			}
		}
	}

	//create folder if not already there.
	makeFolder();

	//flag and move files.
	findFiles(false);

	//create txt log.
	createLog();

	//cleanup
	CoTaskMemFree(logPath);
	CoTaskMemFree(downloadsPath);
	CoTaskMemFree(folderPath);
}