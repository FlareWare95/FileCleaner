/**
 * Author: Austin Hall
 * Program that places the items
 * in the deletion folder into the
 * recycle bin, or wherever
 * specified. Made for Lavoro
 * Group.
 */
#include <ShlObj.h>
#include <lmcons.h>
#include <windows.h>

#include <filesystem>
#include <fstream>
#include <iostream>

std::wstring folderPath;
std::wstring fileDestination;
std::wstring folderName;
bool debug;
bool delFolder;
bool delTemp;

/*
 * @brief Less naive trim that removes all whitespace before or after line.
 * @param ln: address of the line to trim.
 */
void simpleTrim(std::wstring& ln) {
  int start = ln.find_first_not_of(L" ");
  int end = ln.find_last_not_of(L" ");

  ln.erase(0, start);
  ln.erase(end - start + 1, ln.length());
  if (debug) std::wcout << "trim: [" << ln << "]\n";
}
/*
 * @brief Finds all instances of %user in a line, replaces it with the current
 * user's username.
 * @param ln: address of the line to change.
 */
void replaceUserName(std::wstring& ln) {
  int i = ln.find(L"%user");
  if (i == -1) {
    return;
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
}

/*
 * @brief Reads Settings.txt from same directory as exe and pulls settings from
 * there.
 */
void readSettings() {
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

    } else if (start.starts_with(L"Delete Destination")) {
      fileDestination = ln.data();

    } else if (start.starts_with(L"Delete Folder")) {
      if (ln.starts_with(L"T") || ln.starts_with(L"t")) {
        delFolder = true;
      }

    } else if (start.starts_with(L"Delete Temp Files")) {
      if (ln.starts_with(L"T") || ln.starts_with(L"t")) {
        delTemp = true;
      }
    } else {
      if (debug)
        std::wcout << "Error reading settings file: " << ln.data() << "\n";
    }
  }
}

/*
 * @brief moves all files left in the user's flagged files folder
 * (in folderPath) into wherever fileDestination is set to
 */
int clearFolder(PWSTR path, bool cpy) {
  using namespace std::filesystem;
  try {
    for (const auto& file : directory_iterator(path)) {
      if (cpy) {
        try {
          if (debug) std::wcout << "Moving " << file << "\n";
          copy(file.path(), fileDestination.data());
        } catch (filesystem_error e) {
          std::wcout << L"<FolderCleaner> Error copying into destination: "
                     << e.what() << "\n";
          continue;
        }
      }

      try {
        remove_all(file.path());
      } catch (filesystem_error e) {
        if (debug) {
          std::wcout << L"<FolderCleaner> Error deleting tmp file: " << e.what()
                     << "\n";
        }
        continue;
      }
    }
  } catch (std::filesystem::filesystem_error e) {
    if (debug) {
      std::wcout << e.what() << "\n";
    }
  }

  if (delFolder) {
    remove_all(folderPath);
  }
  return 0;
}

void main() {
  readSettings();
  clearFolder(folderPath.data(), true);
  if (delTemp) {
    std::wstring usrPath = L"C:\\Users\\%user\\AppData\\Local\\Temp";
    std::wstring winPath = L"C:\\Windows\\Temp";
    replaceUserName(usrPath);

    clearFolder(usrPath.data(), false);
    clearFolder(winPath.data(), false);
  }
}
