#include "illvm/Support/Json.h"

#include "illvm/Support/Logger.h"

namespace illvm {

llvm::json::Object JsonWrapper::loadJson(const std::string &jsonData) {
  const auto &logger = Logger::getInstance();

  auto parseRes = llvm::json::parse(jsonData);
  if (!parseRes) {
    logger.fatal("Failed to parse JSON: " +
                 llvm::toString(parseRes.takeError()));
  }

  if (auto *root = parseRes->getAsObject()) {
    return *root;
  }

  logger.fatal(
      "Failed to parse JSON: Can not convert json data to json object");
}

std::string JsonWrapper::storeJson(const llvm::json::Object &root) {
  auto rootCp = root;
  const auto rootValue = llvm::json::Value(std::move(rootCp));
  return llvm::formatv("{0:2}", rootValue).str();
}

} // namespace illvm