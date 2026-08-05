/**
 * Author: Austin Hall
 * Setup script for the installer.
 * Will save a hash of all files to
 * registries or something to detect
 * intrusion.
 */
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#define _WIN32_DCOM

#include <Windows.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include <filesystem>
#include <iostream>
#include <openssl/applink.c>
#include <string>

const std::wstring author = L"FlareWare";

const std::wstring FFName = L"FileFinder";
const std::wstring FFDesc =
    L"Process that automatically sifts through old files and adds them to the "
    L"Flagged Files Folder. Sends a log when it's complete.";

const std::wstring FCName = L"FolderCleaner";
const std::wstring FCDesc =
    L"Process that automatically clears the FileCleaner Flagged Files folder.";

/**
 * Function that utilizes the OpenSSL libCrypto library to hash a file given a
 * path.
 * @param filePath the path to the file to hash
 */
int hashFile(const char* filePath) {
  std::wcout << "hashing file: " << filePath << "\n";
  int ret = 0;
  size_t size;
  unsigned char buffer[8192];
  unsigned char outdigest[EVP_MAX_MD_SIZE];
  unsigned int len;

  FILE* file = fopen(filePath, "rb");

  EVP_MD_CTX* ctx = EVP_MD_CTX_new();  // creates digest context - keeps track
                                       // of what the EVP is doing
  EVP_MD* sha256 = EVP_MD_fetch(
      NULL, "SHA256", NULL);  // provides the algorithm for sha256, fips
                              // is a NIST standard for crypto validation!

  EVP_DigestInit_ex(ctx, sha256,
                    NULL);  // Initialize the digest that we created with the
                            // sha256 algroithm we fetched, we use this function
                            // instead of just EVP_DigestInit since the OG is
                            // depricated!
  if (file == NULL) {
    std::wcout << "file is null\n";
    ret = 1;
    goto err;
  }
  if (ctx == NULL) {
    std::wcout << "ctx is null\n";
    ret = 1;
    goto err;
  }
  if (sha256 == NULL) {
    std::wcout << "sha is null\n";
    ret = 1;
    goto err;
  }

  // while the buffer chunks still have data in them
  while ((size = fread(buffer, 1, sizeof(file), file)) > 0) {
    if (!EVP_DigestUpdate(ctx, buffer, sizeof(buffer))) {
      std::wcout << "Error writing from file!\n";
      goto err;
    }
  }

  // calculate digest
  EVP_DigestFinal_ex(ctx, outdigest, &len);

  // print out
  BIO_dump_fp(stdout, outdigest, len);

err:
  /* Clean up all the resources we allocated */
  EVP_MD_free(sha256);
  EVP_MD_CTX_free(ctx);
  if (ret != 0) ERR_print_errors_fp(stderr);
  return ret;
}

int runSchTsk(int monthly, std::wstring process, std::wstring description,
              std::wstring author, std::wstring time, int offset, int weekday,
              int interval) {
  std::wstring line =
      L"cscript ScheduleTask.vbs /recur:" + std::to_wstring(monthly) +
      L" /process:" + process + L" /name:" + author + L" /time:" + time +
      L" /offset:\"" + std::to_wstring(offset) + L"\" /interval:" +
      std::to_wstring(interval) + L" /disc:\"" + description + L"\"";
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  CreateProcess(NULL, line.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

  WaitForSingleObject(pi.hProcess, INFINITE);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return 1;
}

int main() {
  // hashFile("Settings.txt");
  // hashFile(
  //     "C:\\Users\\Flare\\Projects\\FileCleanerSolu\\x64\\Debug\\FileCleaner."
  //     "exe");

  runSchTsk(3, FFName, FFDesc, author, L"8:00", 1, 1, 1);
  runSchTsk(4, FCName, FCDesc, author, L"8:00", 1, 1, 1);
}
