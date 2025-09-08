#include "iclang/Support/MetaData.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Logger.h"

namespace iclang {

std::string IncMetaData::hackMainBuffer(const std::string &originalBuffer,
                                  const std::vector<std::string> &tir) {
  std::istringstream iss(originalBuffer);
  std::string line;
  std::vector<std::string> lines;

  while (getline(iss, line)) {
    lines.push_back(line);
  }

  std::ostringstream oss;
  for (size_t i = 0; i < tir.size(); i++) {
    for (size_t j = 0; j < tir[i].size(); j++) {
      oss << " ";
    }
    oss << std::endl;
  }
  for (size_t i = tir.size(); i < lines.size(); i++) {
    oss << lines[i] << std::endl;
  }

  return oss.str();
}

llvm::json::Object MetaData::serialize() const {
  llvm::json::Object root;

  root["recoverFlag"] = recoverFlag;
  root["recoverReason"] = recoverReason;

  root["currentPath"] = currentPath;
  root["originalCommand"] = originalCommand;
  root["inputPath"] = inputPath;
  root["outputPath"] = outputPath;

  root["totalTimeMs"] = totalTimeMs;
  root["frontTimeMs"] = frontTimeMs;
  root["backTimeMs"] = backTimeMs;

  return root;
}

void MetaData::deserialize(llvm::json::Object &root) {
  recoverFlag = root["recoverFlag"].getAsBoolean().value();
  recoverReason = root["recoverReason"].getAsString().value();

  currentPath = root["currentPath"].getAsString().value().str();
  originalCommand = root["originalCommand"].getAsString().value().str();
  inputPath = root["inputPath"].getAsString().value().str();
  outputPath = root["outputPath"].getAsString().value().str();

  totalTimeMs = root["totalTimeMs"].getAsInteger().value();
  frontTimeMs = root["frontTimeMs"].getAsInteger().value();
  backTimeMs = root["backTimeMs"].getAsInteger().value();
}

llvm::json::Object IncMetaData::serialize() const {
  auto root = MetaData::serialize();

  root["incFlag"] = incFlag;
  root["cannotIncReason"] = cannotIncReason;

  root["funcXTime"] = funcXTime;
  root["funcVTime"] = funcVTime;

  llvm::json::Array arr;
  for (const auto &line : topIncludeRegion) {
    arr.push_back(line);
  }
  root["topIncludeRegion"] = llvm::json::Value(std::move(arr));

  llvm::json::Object sub;
  for (auto &kv : headerTs) {
    sub[kv.first] = kv.second;
  }
  root["headerTs"] = llvm::json::Value(std::move(sub));

  return root;
}

void IncMetaData::deserialize(llvm::json::Object &root) {
  const auto &logger = illvm::Logger::getInstance();

  MetaData::deserialize(root);

  incFlag = root["recoverFlag"].getAsBoolean().value();
  cannotIncReason = root["cannotIncReason"].getAsString().value();

  funcXTime = root["funcXTime"].getAsInteger().value();
  funcVTime = root["funcVTime"].getAsInteger().value();

  auto *arr = root["topIncludeRegion"].getAsArray();
  logger.assertTrue(arr != nullptr,
                    "Failed to parse JSON: Can not convert topIncludeRegion to "
                    "json array");

  topIncludeRegion.clear();
  for (const auto &line : *arr) {
    topIncludeRegion.push_back(line.getAsString().value().str());
  }

  auto *headerTsObj = root["headerTs"].getAsObject();
  logger.assertTrue(
      headerTsObj != nullptr,
      "Failed to parse JSON: Can not convert headerTs to json object");

  headerTs.clear();
  for (const auto &kv : *headerTsObj) {
    const std::string key = kv.first.str();
    headerTs[key] = kv.second.getAsInteger().value();
  }
}

llvm::json::Object ShareTestMetaData::serialize() const {
  auto root = MetaData::serialize();

  root["originalTimeMs"] = originalTimeMs;
  root["masterTimeMs"] = masterTimeMs;
  root["clientTimeMs"] = clientTimeMs;
  root["originalPPLoc"] = originalPPLoc;
  root["funcXedPPLoc"] = funcXedPPLoc;

  return root;
}

void ShareTestMetaData::deserialize(llvm::json::Object &root) {
  MetaData::deserialize(root);

  originalTimeMs = root["originalTimeMs"].getAsInteger().value();
  masterTimeMs = root["masterTimeMs"].getAsInteger().value();
  clientTimeMs = root["clientTimeMs"].getAsInteger().value();
  originalPPLoc = root["originalPPLoc"].getAsInteger().value();
  funcXedPPLoc = root["funcXedPPLoc"].getAsInteger().value();
}

} // namespace iclang