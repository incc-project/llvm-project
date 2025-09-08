#include "iclang/Support/Config.h"

#include "illvm/Support/FileSystem.h"
#include "illvm/Support/Json.h"
#include "illvm/Support/Logger.h"

namespace iclang {

void Config::loadConfig(const std::string &configJsonPath) {
  const auto &logger = illvm::Logger::getInstance();

  const auto configJsonData = illvm::FileSystem::readAll(configJsonPath);

  auto root = illvm::JsonWrapper::loadJson(configJsonData);

  if (root.find("iClangMode") == root.end()) {
    logger.fatal("Load config error: missing iClangMode");
  }
  std::string iClangModeStr;
  iClangModeStr = root["iClangMode"].getAsString().value();
  if (iClangModeStr == "Inc") {
    iClangMode = IClangMode::IncMode;
  } else if (iClangModeStr == "IncTest") {
    iClangMode = IClangMode::IncTestMode;
  } else if (iClangModeStr == "ShareMaster") {
    iClangMode = IClangMode::ShareMasterMode;
  } else if (iClangModeStr == "ShareClient") {
    iClangMode = IClangMode::ShareClientMode;
  } else if (iClangModeStr == "ShareTest") {
    iClangMode = IClangMode::ShareTestMode;
  } else if (iClangModeStr == "Test") {
    iClangMode = IClangMode::TestMode;
  } else if (iClangModeStr == "Profile") {
    iClangMode = IClangMode::ProfileMode;
  } else if (iClangModeStr == "Clang") {
    iClangMode = IClangMode::ClangMode;
  } else {
    logger.fatal("Unknown mode: " + iClangModeStr);
  }

  if (root.find("iClangPaths") != root.end()) {
    llvm::json::Array *arr = root["iClangPaths"].getAsArray();
    if (arr == nullptr) {
      logger.fatal("Failed to parse JSON: Can not convert iClangPaths to "
                   "json array");
    }
    iClangPaths.clear();
    for (const auto &elem : *arr) {
      iClangPaths.insert(elem.getAsString().value().str());
    }
  }
}

} // namespace iclang