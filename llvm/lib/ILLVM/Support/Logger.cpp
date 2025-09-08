#include "illvm/Support/Logger.h"

#include <fstream>

#include "illvm/Support/Time.h"

#include "llvm/Support/Error.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"

namespace illvm {

void Logger::initLogPath(const std::string &_logPath) { logPath = _logPath; }

void Logger::info(const std::string &msg) const {
  llvm::errs() << "[ILLVM Info] " << msg << "\n";
  writeLog("Info", msg);
}

void Logger::debug(const std::string &msg) const {
  llvm::errs() << "[ILLVM Debug] " << msg << "\n";
  writeLog("Debug", msg);
}

void Logger::warning(const std::string &msg) const {
  llvm::WithColor::warning(llvm::errs()) << "[ILLVM Warning] " << msg << "\n";
  writeLog("Warning", msg);
}

void Logger::error(const std::string &msg) const {
  llvm::WithColor::error(llvm::errs()) << "[ILLVM Error] " << msg << "\n";
  writeLog("Error", msg);
}

__attribute__((noreturn)) void Logger::fatal(const std::string &msg) const {
  writeLog("Fatal", msg);
  llvm::report_fatal_error(llvm::StringRef("[ILLVM Fatal] " + msg));
}

void Logger::assertTrue(const bool expr, const std::string &msg) const {
  if (expr) {
    return;
  }
  fatal("assert failed: " + msg);
}

void Logger::writeLog(const std::string &level, const std::string &msg) const {
  if (logPath.empty()) {
    return;
  }
  // Append IClang log
  const std::string timeLevelMsg =
      Time::currentDateTime() + " [" + level + "] " + msg;
  std::ofstream logFile(logPath, std::ios::app);
  if (!logFile.is_open()) {
    llvm::report_fatal_error(llvm::StringRef(
        "[ILLVM Fatal] Write IClang log error: can not open " + logPath));
  }
  logFile << timeLevelMsg << std::endl;
  logFile.close();
}

} // namespace illvm
