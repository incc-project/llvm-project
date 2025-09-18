#include "illvm/Support/Json.h"

#include "illvm/Support/Diagnostics.h"

namespace illvm {

llvm::json::Object JsonWrapper::loadJson(const std::string &jsonData) {
  auto parseRes = llvm::json::parse(jsonData);
  if (auto err = parseRes.takeError()) {
    ILLVM_UNREACHABLE(llvm::toString(std::move(err)));
  }

  if (auto *root = parseRes->getAsObject()) {
    return *root;
  }

  ILLVM_UNREACHABLE(
      "Failed to parse JSON: Can not convert json data to json object");
}

std::string JsonWrapper::storeJson(const llvm::json::Object &root) {
  auto rootCp = root;
  const auto rootValue = llvm::json::Value(std::move(rootCp));
  return llvm::formatv("{0:2}", rootValue).str();
}

} // namespace illvm