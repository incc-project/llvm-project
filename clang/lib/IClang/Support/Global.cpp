#include "iclang/Support/Global.h"

#include "illvm/Support/FileSystem.h"
#include "illvm/Support/Json.h"

namespace iclang {

void Global::saveMetaDataToFile(const std::string &filepath,
                               const std::shared_ptr<MetaData> &md) {
  const auto root = md->serialize();
  const auto content = illvm::JsonWrapper::storeJson(root);
  illvm::FileSystem::saveStr(filepath, content);
}

std::shared_ptr<MetaData>
Global::loadMetaDataFromFile(const std::string &filepath,
                             const IClangMode iClangMode) {
  const std::string content = illvm::FileSystem::readAll(filepath);
  auto root = illvm::JsonWrapper::loadJson(content);

  const auto metaData = createSpMetaData(iClangMode);
  metaData->deserialize(root);

  return metaData;
}

} // namespace iclang
