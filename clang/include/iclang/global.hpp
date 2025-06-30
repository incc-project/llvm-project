#ifndef ICLANG_GLOBAL_HPP
#define ICLANG_GLOBAL_HPP

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Path.h"

#include "clang/AST/AST.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Mangle.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace iclang {

class String {
public:
  static std::string trimWhitespace(const std::string &str) {
    const auto start =
        std::find_if_not(str.begin(), str.end(), [](const unsigned char ch) {
          return std::isspace(ch);
        });

    const auto end =
        std::find_if_not(str.rbegin(), str.rend(), [](const unsigned char ch) {
          return std::isspace(ch);
        }).base();

    if (start >= end) {
      return ""; // All spaces or empty string
    }

    return std::string(start, end);
  }

  static bool hasPrefix(const std::string &str, const std::string &prefix) {
    if (str.size() < prefix.size()) {
      return false;
    }
    return str.compare(0, prefix.size(), prefix) == 0;
  }

  static std::vector<std::string> splitString(const std::string &str,
                                              const char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
      result.push_back(trimWhitespace(item));
    }

    return result;
  }

  static std::string argVToArgs(const llvm::SmallVector<const char *, 128> &argv) {
    std::string args;
    if (!argv.empty()) {
      args.append(argv[0]);
    }
    for (size_t i = 1; i < argv.size(); i++) {
      args.append(" ");
      args.append(argv[i]);
    }
    return args;
  }

  static std::string vectorToString(const std::vector<std::string>& vec, size_t n) {

#if defined(_WIN32) || defined(_WIN64)
    const std::string newline = "\r\n";
#else
    const std::string newline = "\n";
#endif

    std::string result;
    size_t count = 0;

    for (const auto& item : vec) {
      if (count >= n) {
        break;
      }
      result += item + newline;
      ++count;
    }

    return result;
  }

};

class Time {
public:
  static long long currentTsMs() {
    const auto now = std::chrono::system_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
  }

  static std::string currentDateTime() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    const std::tm local_tm = *std::localtime(&now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
  }
};

class Logger {
private:
  std::string logPath;

  Logger() {}

public:

  static Logger& getInstance() {
    static Logger instance;
    return instance;
  }

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void initLogPath(const std::string &_logPath) {
    logPath = _logPath;
  }

  void info(const std::string &msg) const {
    llvm::errs() << "[IClang Info] " << msg << "\n";
    writeLog("Info", msg);
  }

  void debug(const std::string &msg) const {
    llvm::errs() << "[IClang Debug] " << msg << "\n";
    writeLog("Debug", msg);
  }

  void warning(const std::string &msg) const {
    llvm::errs() << "[IClang Warning] " << msg << "\n";
    writeLog("Warning", msg);
  }

  void error(const std::string &msg) const {
    llvm::errs() << "[IClang Error] " << msg << "\n";
    writeLog("Error", msg);
  }

  void fatal(const std::string &msg) const {
    llvm::errs() << "[IClang Fatal] " << msg << "\n";
    writeLog("Fatal", msg);
    exit(1);
  }

  void writeLog(const std::string &level, const std::string &msg) const {
    if (logPath.empty()) {
      return;
    }
    // Append IClang log
    std::string const timeLevelMsg = Time::currentDateTime() + " [" + level + "] " + msg;
    std::ofstream logFile(logPath, std::ios::app);
    if (!logFile.is_open()) {
      llvm::errs() << "Write IClang log error: can not open " << logPath << "\n";
      logFile.close();
      exit(1);
    }
    logFile << timeLevelMsg << "\n";
    logFile.close();
  }
};

class FileSystem {
public:
  static std::string getCurrentPath() {
    llvm::SmallString<256> res;
    const auto ec = llvm::sys::fs::current_path(res);
    if (ec) {
      Logger::getInstance().fatal("Error getting current path: " + ec.message());
    }
    return res.str().str();
  }

  // Auto remove dot.
  static std::string toAbsPath(const std::string &filepath) {
    llvm::SmallString<256> absolutePath(filepath);
    if (llvm::sys::fs::make_absolute(absolutePath)) {
      Logger::getInstance().fatal("Make absolute error: " + filepath);
    }
    llvm::sys::path::remove_dots(absolutePath);
    return absolutePath.str().str();
  }

  static std::string linkPath(const std::string &path1, const std::string &path2) {
    llvm::SmallString<256> res(path1);

    llvm::sys::path::append(res, path2);

    return res.str().str();
  }

  static bool pathEqual(const std::string& path1, const std::string &path2) {
    return llvm::sys::fs::equivalent(path1, path2);
  }

  static bool checkFileExists(const std::string &filepath) {
    return llvm::sys::fs::exists(filepath);
  }

  static long long getLastModificationTime(const std::string &filepath) {
    llvm::sys::fs::file_status status;
    if (llvm::sys::fs::status(filepath, status)) {
      Logger::getInstance().fatal("Can not read the status of file " + filepath);
    }
    llvm::sys::TimePoint<> time = status.getLastModificationTime();
    auto duration = time.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return milliseconds.count();
  }

  static std::string readAll(const std::string &filepath) {
    std::ifstream inputFile(filepath);
    if (!inputFile.is_open()) {
      Logger::getInstance().fatal("Can not open file " + filepath);
    }

    std::ostringstream buffer;
    buffer << inputFile.rdbuf();

    return buffer.str();
  }

   static bool readFirstNChars(const std::string &filepath, size_t n, std::string &result) {
    std::ifstream file(filepath);
    if (!file) {
      Logger::getInstance().fatal("Can not open file " + filepath);
    }

    result.resize(n);

    file.read(&result[0], n);

    return (size_t)file.gcount() == n;
  }

  static std::vector<std::string> readFirstNLines(const std::string& filepath, size_t n) {
    std::ifstream file(filepath);
    std::vector<std::string> result;
    std::string line;

    if (!file.is_open()) {
      Logger::getInstance().fatal("Can not open file " + filepath);
    }

    size_t lineCount = 0;
    while (std::getline(file, line) && lineCount < n) {
      result.push_back(line);
      ++lineCount;
    }

    file.close();
    return result;
  }

  static std::vector<std::string> readLines(const std::string &filepath) {
    std::ifstream infile(filepath);

    if (!infile) {
      Logger::getInstance().fatal("Can not open file " + filepath);
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(infile, line)) {
      lines.push_back(line);
    }

    infile.close();

    return lines;
  }

  // rm -f file or dir.
  static void rmFile(const std::string &filepath) {
    if (!checkFileExists(filepath)) {
      return;
    }
    bool isDir = llvm::sys::fs::is_directory(filepath);
    if (isDir) {
      auto ec = llvm::sys::fs::remove_directories(filepath);
      if (ec) {
        Logger::getInstance().fatal("Can not remove directory " + filepath);
      }
    } else {
      auto ec = llvm::sys::fs::remove(filepath);
      if (ec) {
        Logger::getInstance().fatal("Can not remove file " + filepath);
      }
    }
  }

  // Auto cover.
  static void mvFile(const std::string &from, const std::string &to) {
    rmFile(to);
    auto ec = llvm::sys::fs::rename(from, to);
    if (ec) {
      Logger::getInstance().fatal("mv " + from + " to " + to + " failed");
    }
  }

  // Auto cover.
  static void cpFile(const std::string &from, const std::string &to) {
    rmFile(to);
    auto ec = llvm::sys::fs::copy_file(from, to);
    if (ec) {
      Logger::getInstance().fatal("cp " + from + " to " + to + " failed");
    }
  }

  // List the first level files (fileFlag) / directories (!fileFlag) under `path`.
  static std::vector<std::string> list(const std::string &path, bool fileFlag) {
    std::vector<std::string> res;

    auto &logger = Logger::getInstance();

    std::error_code ec;

    llvm::sys::fs::directory_iterator it(path, ec);
    llvm::sys::fs::directory_iterator end;

    if (ec) {
      logger.fatal("Failed to open directory: " + path + " - " + ec.message());
    }

    for (; it != end; it.increment(ec)) {
      if (ec) {
        logger.fatal("Error during iteration: " + ec.message());
      }

      std::string entryPath = it->path();
      if (fileFlag) {
        if (!llvm::sys::fs::is_directory(entryPath)) {
          res.push_back(entryPath);
        }
      } else {
        if (llvm::sys::fs::is_directory(entryPath)) {
          res.push_back(entryPath);
        }
      }
    }

    return res;
  }

  static void saveStr(const std::string &filepath, const std::string &str) {
    std::ofstream ofs(filepath);
    if (!ofs.is_open()) {
      Logger::getInstance().fatal("Can not open " + filepath);
    }
    ofs << str;
    ofs.close();
  }

  static void saveSet(const std::string &filepath, std::unordered_set<std::string> &st) {
    std::ostringstream oss;
    for (const auto &elem : st) {
      oss << elem << "\n";
    }
    std::ofstream ofs(filepath);
    if (!ofs.is_open()) {
      Logger::getInstance().fatal("Can not open " + filepath);
    }
    ofs << oss.str();
    ofs.close();
  }
};

class BinFile {
private:
  std::string filePath;

  void* fileData;

  std::streamsize fileSize;

public:
  BinFile(const std::string& path) : filePath(path), fileData(nullptr), fileSize(0) {
    auto &logger = Logger::getInstance();

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      logger.fatal("Failed to open file: " + filePath);
    }

    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    fileData = malloc(fileSize);
    if (!fileData) {
      logger.fatal("Memory allocation failed for file: " + filePath);
    }

    if (!file.read(static_cast<char*>(fileData), fileSize)) {
      free(fileData);
      logger.fatal("Error reading file: " + filePath);
    }

    file.close();
  }

  ~BinFile() {
    if (fileData) {
      free(fileData);
    }
  }

  std::streamsize getFileSize() const {
    return fileSize;
  }

  const std::string& getFilePath() const {
    return filePath;
  }

  const char* readBytes(int offset) const {
    auto &logger = Logger::getInstance();
    if (offset < 0 || offset >= fileSize) {
      logger.fatal("Offset out of range in file: " + filePath);
    }
    return static_cast<char*>(fileData) + offset;
  }
};

} // namespace iclang

#endif //ICLANG_GLOBAL_HPP
