//===--- Global.h - IClang global data ------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Sharing data between driver and cc1.
//
//===----------------------------------------------------------------------===/

#ifndef ICLANG_GLOBAL_H
#define ICLANG_GLOBAL_H

#include <memory>
#include <string>

#include "iclang/Support/Config.h"
#include "iclang/Support/MetaData.h"

namespace clang {
namespace driver {
class CC1Command;
}
}

namespace iclang {

class Global {
private:
  // Highest permission, IClang will only be enabled after it is set to true.
  bool enabled = true;
  Config config;
  std::shared_ptr<MetaData> metaData;

  Global() {}

public:

  Global(const Global &) = delete;
  Global &operator=(const Global &) = delete;

  static Global &getInstance() {
    static Global instance;
    return instance;
  }

  bool isEnabled() const { return enabled; }

  void setEnabled(const bool _enabled) { enabled = _enabled; }

  const Config &getConfig() const { return config; }

  static std::shared_ptr<MetaData>
  createSpMetaData(const IClangMode iClangMode) {
    std::shared_ptr<MetaData> metaData;
    if (iClangMode == IClangMode::IncMode) {
      metaData = std::make_unique<IncMetaData>();
    } else if (iClangMode == IClangMode::IncTestMode) {
      metaData = std::make_unique<IncTestMetaData>();
    } else if (iClangMode == IClangMode::ShareMasterMode) {
      metaData = std::make_unique<ShareMasterMetaData>();
    } else if (iClangMode == IClangMode::ShareClientMode) {
      metaData = std::make_unique<ShareClientMetaData>();
    } else if (iClangMode == IClangMode::ShareTestMode) {
      metaData = std::make_unique<ShareTestMetaData>();
    } else if (iClangMode == IClangMode::TestMode) {
      metaData = std::make_unique<TestMetaData>();
    } else {
      metaData = std::make_unique<MetaData>();
    }
    return metaData;
  }

  const std::shared_ptr<MetaData> &getMetaData() const { return metaData; }

  std::shared_ptr<IncMetaData> getIncMetaData() const {
    if (config.getIClangMode() == IClangMode::IncMode) {
      return std::static_pointer_cast<IncMetaData>(metaData);
    }
    return nullptr;
  }

  std::shared_ptr<IncTestMetaData> getIncTestMetaData() const {
    if (config.getIClangMode() == IClangMode::IncTestMode) {
      return std::static_pointer_cast<IncTestMetaData>(metaData);
    }
    return nullptr;
  }

  std::shared_ptr<ShareMasterMetaData> getShareMasterMetaData() const {
    if (config.getIClangMode() == IClangMode::ShareMasterMode) {
      return std::static_pointer_cast<ShareMasterMetaData>(metaData);
    }
    return nullptr;
  }

  std::shared_ptr<ShareClientMetaData> getShareClientMetaData() const {
    if (config.getIClangMode() == IClangMode::ShareClientMode) {
      return std::static_pointer_cast<ShareClientMetaData>(metaData);
    }
    return nullptr;
  }

  std::shared_ptr<ShareTestMetaData> getShareTestMetaData() const {
    if (config.getIClangMode() == IClangMode::ShareTestMode) {
      return std::static_pointer_cast<ShareTestMetaData>(metaData);
    }
    return nullptr;
  }

  std::shared_ptr<TestMetaData> getTestMetaData() const {
    if (config.getIClangMode() == IClangMode::TestMode) {
      return std::static_pointer_cast<TestMetaData>(metaData);
    }
    return nullptr;
  }

  void init(const Config &_config, const std::shared_ptr<MetaData> &_metaData) {
    config = _config;
    metaData = _metaData;
  }

  static void saveMetaDataToFile(const std::string &filepath,
                                 const std::shared_ptr<MetaData> &md);

  static std::shared_ptr<MetaData>
  loadMetaDataFromFile(const std::string &filepath, const IClangMode iClangMode);
};

} // namespace iclang

#endif // ICLANG_GLOBAL_H
