#include "illvm/Support/FileSystem.h"

#include <chrono>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "illvm/Support/Logger.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace illvm {

std::string FileSystem::getCurrentPath() {
  llvm::SmallString<256> res;
  const auto ec = llvm::sys::fs::current_path(res);
  if (ec) {
    Logger::getInstance().fatal("Error getting current path: " + ec.message());
  }
  return res.str().str();
}

std::string FileSystem::toAbsPath(const std::string &filepath) {
  llvm::SmallString<256> absolutePath(filepath);
  const auto ec = llvm::sys::fs::make_absolute(absolutePath);
  if (ec) {
    Logger::getInstance().fatal("Make absolute error: " + filepath + ": " +
                                ec.message());
  }
  llvm::sys::path::remove_dots(absolutePath);
  return absolutePath.str().str();
}

std::string FileSystem::linkPath(const std::string &path1,
                                 const std::string &path2) {
  llvm::SmallString<256> res(path1);

  llvm::sys::path::append(res, path2);

  return res.str().str();
}

std::string FileSystem::parentPath(const std::string &path) {
  return llvm::sys::path::parent_path(path).str();
}

bool FileSystem::checkFileExists(const std::string &filepath) {
  return llvm::sys::fs::exists(filepath);
}

long long FileSystem::getLastModificationTime(const std::string &filepath) {
  llvm::sys::fs::file_status status;
  const auto ec = llvm::sys::fs::status(filepath, status);
  if (ec) {
    Logger::getInstance().fatal("Can not read the status of file " + filepath +
                                ": " + ec.message());
  }
  const llvm::sys::TimePoint<> time = status.getLastModificationTime();
  const auto duration = time.time_since_epoch();
  const auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  return milliseconds.count();
}

std::string FileSystem::readAll(const std::string &filepath) {
  std::ifstream inputFile(filepath);
  if (!inputFile.is_open()) {
    Logger::getInstance().fatal("Can not open file " + filepath);
  }

  std::ostringstream buffer;
  buffer << inputFile.rdbuf();

  inputFile.close();

  return buffer.str();
}

std::vector<std::string> FileSystem::readLines(const std::string &filepath) {
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

// Note: check res.size() == n yourself.
std::vector<std::string>
FileSystem::readFirstNLines(const std::string &filepath, const size_t n) {
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

bool FileSystem::mkdir(const std::string &dirpath) {
  const std::error_code ec = llvm::sys::fs::create_directory(dirpath, false);
  if (ec) {
    Logger::getInstance().error("Can not create directory " + dirpath + ": " +
                                ec.message());
    return false;
  }
  return true;
}

void FileSystem::rmFile(const std::string &filepath) {
  if (!checkFileExists(filepath)) {
    return;
  }
  if (!llvm::sys::fs::is_regular_file(filepath)) {
    Logger::getInstance().fatal("Only support rm regular file: " + filepath);
  }
  const auto ec = llvm::sys::fs::remove(filepath);
  if (ec) {
    Logger::getInstance().fatal("Can not remove file " + filepath + ": " +
                                ec.message());
  }
}

void FileSystem::rmEmptyDir(const std::string &filepath) {
  if (!checkFileExists(filepath)) {
    return;
  }
  if (!llvm::sys::fs::is_directory(filepath)) {
    Logger::getInstance().fatal("Only support rm directory: " + filepath);
  }
  const auto ec = llvm::sys::fs::remove_directories(filepath);
  if (ec) {
    Logger::getInstance().fatal("Can not remove directory " + filepath + ": " +
                                ec.message());
  }
}

void FileSystem::rmDirDFS(const std::string &curDirPath) {
  namespace fs = llvm::sys::fs;
  namespace path = llvm::sys::path;

  std::error_code ec;

  for (fs::directory_iterator it(curDirPath, ec), end; it != end && !ec;
       it.increment(ec)) {
    const auto &entry = *it;
    const std::string entryPath = entry.path();
    const std::string entryName = path::filename(entryPath).str();

    if (fs::is_regular_file(entryPath)) {
      rmFile(entryPath);
    } else if (fs::is_directory(entryPath)) {
      const std::string newDirPath = linkPath(curDirPath, entryName);
      rmDirDFS(newDirPath);
    }
  }

  rmEmptyDir(curDirPath);
}

void FileSystem::rmDir(const std::string &filepath) {
  if (!checkFileExists(filepath)) {
    return;
  }
  if (!llvm::sys::fs::is_directory(filepath)) {
    Logger::getInstance().fatal("Only support rm directory: " + filepath);
  }
  rmDirDFS(filepath);
}

void FileSystem::mvFile(const std::string &from, const std::string &to) {
  rmFile(to);
  const auto ec = llvm::sys::fs::rename(from, to);
  if (ec) {
    Logger::getInstance().fatal("mv " + from + " to " + to +
                                " failed: " + ec.message());
  }
}

void FileSystem::mvDirDFS(const std::string &baseFromDirPath,
                          const std::string &baseToDirPath,
                          const std::string &relDirPath) {
  namespace fs = llvm::sys::fs;
  namespace path = llvm::sys::path;

  const auto &logger = Logger::getInstance();

  const std::string fromDirPath = linkPath(baseFromDirPath, relDirPath);
  const std::string toDirPath = linkPath(baseToDirPath, relDirPath);

  std::error_code ec = fs::create_directories(toDirPath);
  if (ec) {
    logger.fatal("Failed to create target directory " + toDirPath + ": " +
                 ec.message());
  }

  for (fs::directory_iterator it(fromDirPath, ec), end; it != end && !ec;
       it.increment(ec)) {
    const auto &entry = *it;
    const std::string fromEntryPath = entry.path();
    const std::string fromEntryName = path::filename(fromEntryPath).str();

    if (fs::is_regular_file(fromEntryPath)) {
      const std::string toEntryPath = linkPath(toDirPath, fromEntryName);
      mvFile(fromEntryPath, toEntryPath);
    } else if (fs::is_directory(fromEntryPath)) {
      const std::string newRelDirPath = linkPath(relDirPath, fromEntryName);
      mvDirDFS(baseFromDirPath, baseToDirPath, newRelDirPath);
    }
  }

  rmEmptyDir(fromDirPath);
}

void FileSystem::mvDir(const std::string &from, const std::string &to) {
  if (!checkFileExists(from)) {
    Logger::getInstance().fatal("mv " + from + " to " + to +
                                " failed: " + "from does not exist");
  }
  rmDir(to);
  mvDirDFS(from, to);
}

void FileSystem::cpFile(const std::string &from, const std::string &to) {
  rmFile(to);
  const auto ec = llvm::sys::fs::copy_file(from, to);
  if (ec) {
    Logger::getInstance().fatal("cp " + from + " to " + to +
                                " failed: " + ec.message());
  }
}

void FileSystem::saveStr(const std::string &filepath, const std::string &str) {
  std::ofstream ofs(filepath);
  if (!ofs.is_open()) {
    Logger::getInstance().fatal("Can not open " + filepath);
  }
  ofs << str;
  ofs.close();
}

void FileSystem::saveVector(const std::string &filepath,
                            const std::vector<std::string> &vec) {
  std::ostringstream oss;
  for (const auto &elem : vec) {
    oss << elem << std::endl;
  }
  std::ofstream ofs(filepath);
  if (!ofs.is_open()) {
    Logger::getInstance().fatal("Can not open " + filepath);
  }
  ofs << oss.str();
  ofs.close();
}

void FileSystem::saveSet(const std::string &filepath,
                         const std::unordered_set<std::string> &st,
                         const bool ordered) {
  std::ostringstream oss;
  if (ordered) {
    const std::set<std::string> orderedSet(st.begin(), st.end());
    for (const auto &elem : orderedSet) {
      oss << elem << std::endl;
      ;
    }
  } else {
    for (const auto &elem : st) {
      oss << elem << std::endl;
      ;
    }
  }
  std::ofstream ofs(filepath);
  if (!ofs.is_open()) {
    Logger::getInstance().fatal("Can not open " + filepath);
  }
  ofs << oss.str();
  ofs.close();
}

} // namespace illvm
