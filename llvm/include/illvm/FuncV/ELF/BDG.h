//===--- BDG.h - Binary dependency graph ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// Dependencies:
// * Symbol - section dependencies.
// * Section symbol dependencies.
// * Relocation dependencies.
// * Exception frame dependencies.
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_BDG_H
#define ILLVM_BDG_H

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "illvm/FuncV/ELF/ObjFile.h"
#include "illvm/FuncV/ELF/ReuseNode.h"

namespace illvm {
namespace funcv {
namespace elf {

class BDG {
private:
  using ReuseNodeIdMapType =
      std::unordered_map<std::string, std::shared_ptr<ReuseNode>>;
  using SymbolNameMapType =
      std::unordered_map<std::string, std::shared_ptr<Symbol>>;
  using SymbolSectionMapType =
      std::unordered_map<std::shared_ptr<Symbol>, std::shared_ptr<Section>>;
  using SectionRelaMapType =
      std::unordered_map<std::shared_ptr<Section>,
                         std::shared_ptr<RelocationSection>>;

  // Load from symbol .iclang.reusev.
  uint64_t oldReuseVersion = 0;
  uint64_t newReuseVersion = 1;

  // Reuse node idr -> reuse node.
  ReuseNodeIdMapType reuseNodeIdrMap;

  // Save propagation results.
  ReuseNodeIdMapType funcVReuseNodes;

  void loadOldReuseVersion(ObjFile &oldObjFile);

  static void renameSymbol(ObjFile &objFile, const std::string &symbolName,
                           const std::shared_ptr<Symbol> &symbol,
                           const uint64_t reuseVersion);

  static void renameSection(ObjFile &objFile, const std::string &sectionName,
                            const std::shared_ptr<Section> &section,
                            const uint64_t reuseVersion);

  static void renameAnonymousSymbol(ObjFile &objFile,
                                    const uint64_t reuseVersion);

  // Symbol name -> symbol.
  static SymbolNameMapType symbolNameMapping(ObjFile &objFile);

  // Symbol -> section.
  static SymbolSectionMapType symbolSectionMapping(ObjFile &objFile);

  // Section -> rela section.
  static SectionRelaMapType sectionRelaMapping(ObjFile &objFile);

  void initReuseNodesWithSymbol(const SymbolNameMapType &oldSymbolNameMap,
                                const SymbolNameMapType &newSymbolNameMap);

  void addTDSectionsForReusedNodes(ObjFile &oldObjFile,
                                   ObjFile &newObjFile) const;

  void addRelaSectionsForReusedNodes(ObjFile &oldObjFile,
                                     ObjFile &newObjFile) const;

  void addEHsForReuseNodes(ObjFile &oldObjFile, ObjFile &newObjFile);

  // Example:
  // .text._ZL4testv -> _ZL4testv
  // .data._ZL1x -> _ZL1x
  // .rodata..L__const._ZL4testv.a -> .L__const._ZL4testv.a
  // Invalid: return "".
  static std::string extractSectionSymbolName(const std::string &name);

  void handleSymSecDep(const std::shared_ptr<ReuseNode> &reuseNode);

  llvm::Error handleRelaDep(const std::shared_ptr<ReuseNode> &reuseNode);

  void handleFDEDep(const std::shared_ptr<CIE> &cie,
                    const std::vector<std::shared_ptr<FDE>> &fdes);

  void handleEhDep(ObjFile &oldObjFile);

  llvm::Error buildReuseNodesDependencies(ObjFile &oldObjFile);

public:
  llvm::Error build(ObjFile &oldObjFile, ObjFile &newObjFile);

  void propagation(const std::unordered_set<std::string> &funcXSet);

  const auto &getReuseNodeIdrMap() const { return reuseNodeIdrMap; }

  const auto &getFuncVReuseNodes() const { return funcVReuseNodes; }

  uint64_t getOldReuseVersion() const { return oldReuseVersion; }

  void dump(std::ostream &oss) const;

  std::string toString() const;
};

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif // ILLVM_BDG_H
