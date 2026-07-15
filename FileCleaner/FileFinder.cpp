/**
 * Author: Austin Hall
 * Program that sorts and logs all
 * files older than a month old in
 * the downloads folder, and
 * produces a handy .txt log.
 * User/Administrator can specify
 * where they want the flagged
 * files folder and txt log. Will
 * also send the summary as an
 * email hopefully using smtp
 * relay! Made for Lavoro Group.
 */

#include <ShlObj.h>
#include <atlstr.h>
#include <lmcons.h>
#include <string.h>
#include <windows.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

std::vector<std::wstring> flaggedFiles;
std::vector<std::wstring> directories;
PWSTR logPath;
std::wstring folderPath;
std::wstring folderName;
bool debug = false;

/*
 * @brief Less naive trim that removes all whitespace before or after line.
 * @param ln: address of the line to trim.
 */
int simpleTrim(std::wstring& ln) {
  int start = ln.find_first_not_of(L" ");
  int end = ln.find_last_not_of(L" ");

  ln.erase(0, start);
  ln.erase(end - start + 1, ln.length());
  if (debug) std::wcout << "trim: [" << ln << "]\n";

  return 0;
}
/*
 * @brief Finds all instances of %user in a line, replaces it with the current
 * user's username.
 * @param ln: address of the line to change.
 */
int replaceUserName(std::wstring& ln) {
  int i = ln.find(L"%user");
  if (i == -1) {
    return 0;
  }
  wchar_t username[UNLEN + 1];
  DWORD len = UNLEN + 1;
  GetUserName(username, &len);
  std::wstring user = username;
  while (i != -1) {
    ln.replace(i, user.length(), user.data());
    if (debug) std::wcout << "User found! Changed to " << ln.data() << "\n";
    i = ln.find(L"%user");
    if (debug) std::wcout << "Next index: " << i << "\n";
  }
  return 0;
}

/*
 * @brief breaks up settings list for directories to clear into a vector.
 * @param ln: the line to break up
 */
int getDirectories(std::wstring& ln) {
  std::wstringstream stream(ln);
  std::wstring temp;
  while (std::getline(stream, temp, L',')) {
    if (debug) std::wcout << "Added " << temp << " to directories to check\n";
    simpleTrim(temp);
    directories.push_back(temp);
  }

  // directories.insert(directories.end(), ln.data());
  return 0;
}

/*
 * @brief Reads Settings.txt from same directory as exe and pulls settings from
 * there.
 */
int readSettings() {
  std::wifstream file("Settings.txt");
  std::wstring ln;
  std::wstring start;

  while (std::getline(file, ln)) {
    start = ln.data();
    int offset = ln.find(L":") + 1;
    ln = ln.substr(offset, ln.length());
    simpleTrim(ln);
    replaceUserName(ln);

    if (start.starts_with(L"Debug")) {
      if (ln.starts_with(L"T") || ln.starts_with(L"t")) {
        std::wcout << "Debug enabled!\n";
        debug = ln.data();
      }
    } else if (start.starts_with(L"Folder Name")) {
      folderName = ln.data();

    } else if (start.starts_with(L"Folder Destination")) {
      if (ln._Equal(L"Default")) {
        PWSTR temp;
        SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &temp);
        ln = temp;
        ln.append(L"\\");
        ln.append(folderName);
        folderPath = ln.data();
      }

    } else if (start.starts_with(L"Directories to Check:")) {
      getDirectories(ln);
    } else {
      if (debug) std::wcout << "Skipping: " << start.data() << "\n";
    }
  }
  return 0;
}
/*
 * @brief Creates and formats the log (.txt) written to the current user's
 * desktop.
 */
int createLog() {
  SYSTEMTIME time;
  GetLocalTime(&time);
  wchar_t filename[500];

  if (logPath == NULL) {
    SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &logPath);
    CoTaskMemFree(logPath);
  }
  swprintf_s(filename, _countof(filename), L"%s\\Deletion Log %d-%d-%d.txt",
             logPath, time.wMonth, time.wDay, time.wYear);

  std::wofstream file(filename);  // create filestream to filename

  if (file.is_open()) {
    file << "File Deletion Summary ";
    file << time.wMonth;
    file << "/";
    file << time.wDay;
    file << "/";
    file << time.wYear;
    file << ":\n\n";
    file << "The following files have not been used in the past month, and "
            "have been flagged for deletion in TWO weeks:\n";
    file << "__________________________________________________________________"
            "_______________\n\n";
    if (flaggedFiles.size() == 0) {
      file << "	No files met the deletion criteria!\n";
    } else {
      for (const std::wstring& files : flaggedFiles) {
        file << "	";
        file << files;
        file << "\n";
      }
    }
    file << "__________________________________________________________________"
            "________________\n";
    file << "\nYou can review these files at ";
    file << logPath;
    file << " before deletion.\n";
    file << "\nIf any files listed need to be saved, move them out of the "
            "folder and to the SharePoint drive before they are lost forever. "
            "Thank You!\n";
    file << "\n\n* Feel free to delete this log after you are done reading. *";
    file.close();

    if (debug) {
      std::wcout << L"<FileFinder> Log file created successfully!\n";
    }
  } else {
    if (debug) {
      std::wcout << L"<FileFinder> Error creating file!!!!\n";
    }
    return 1;
  }
  return 0;
}

/*
 * @brief This function iterates through the path folder, and if it meets the
 * criteria for deletion, will be moved to the folder specified.
 * @param checkmonth - if true we check the month var, mostly for testing
 */
int findFiles(bool checkMonth) {
  using namespace std::chrono;

  // we need all of this to convert the current system clock to a point where we
  // can get individual yy mm dd
  auto currentTime = system_clock::now();
  auto currentTimeCast = clock_cast<system_clock>(currentTime);
  auto currentTimeFloor = floor<days>(currentTimeCast);
  year_month_day currentYmd{currentTimeFloor};
  auto currentDay = sys_days(currentYmd);

  if (debug) std::wcout << L"<FileFinder> Current Time:" << currentTime << "\n";
  for (std::wstring path : directories) {
    std::wstring name = L"Items in:" + path + L" ---------->";
    flaggedFiles.push_back(name);
    if (debug) std::wcout << "Starting:" << path << "\n";
    try {
      for (const auto& file :
           std::filesystem::directory_iterator(path.data())) {
        auto writeTime = file.last_write_time();
        auto clockCast = clock_cast<system_clock>(writeTime);
        auto clockFloor = floor<days>(clockCast);
        year_month_day ymd{clockFloor};
        auto days = sys_days{ymd};

        if (ymd.year() < currentYmd.year() ||
            (checkMonth && (ymd.month() < currentYmd.month()))) {
          if (checkMonth) {
            int daysPast = (currentDay - days).count();
            if (debug) {
              std::wcout << L"<FileFinder> Days since: " << daysPast << "\n";
            }

            if (daysPast < 30) {
              if (debug) std::wcout << L"<FileFinder> Continuing...\n";
              continue;
            }
          }
          try {
            if (debug) {
              std::wcout << L"<FileFinder> Adding: " << file.path().filename()
                         << "\n";
            }
            std::filesystem::copy(
                file.path(),
                folderPath.data());  // copy the file into the folder.

          } catch (std::filesystem::filesystem_error e) {
            if (debug) std::wcout << e.what() << "\n";
            continue;
          }

          // only log delete if we can copy the file into the folder.
          flaggedFiles.push_back(file.path().filename().wstring());
          try {
            std::filesystem::remove_all(file.path());
          } catch (std::filesystem::filesystem_error e) {
            std::wcout << e.what() << "\n";
          }
        }
      }
    } catch (std::filesystem::filesystem_error e) {
      std::wcout << e.what();
    }
  }
  return 0;
}

/**
 * @brief Creates the folder that the flagged files will be copied to.
 */
int makeFolder() {
  if (debug) std::wcout << L"<FileFinder> Full Path: " << folderPath << L"\n";

  if (!std::filesystem::exists(folderPath)) {
    std::filesystem::create_directories(folderPath);
    if (debug) {
      std::wcout << L"<FileFinder> Deletion Folder Created\n";
    }
    return 0;
  } else {
    if (debug) {
      std::wcout
          << L"<FileFinder> Folder already exists or error finding file.\n";
    }

    return 1;
  }
}

/*
 * @brief Format Args: FileCleaner.exe [bool debug] [str FOLDERPATH] [str
 * FOLDERNAME]
 */
int main() {
  readSettings();
  // create folder if not already there.
  makeFolder();

  // flag and move files.
  findFiles(true);

  // create txt log.
  createLog();
}
