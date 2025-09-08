//===--- Sections.h - ELF sections ---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
//
// * Section
// * Ordinary
// * StrTab
// * SymTab
// * SymTabShNdx
// * RelaTab
// * Group
// * EhFrame
//
//===----------------------------------------------------------------------===/

#ifndef ILLVM_SECTIONS_H
#define ILLVM_SECTIONS_H

#include "illvm/FuncV/ELF/Section.h"
#include "illvm/FuncV/ELF/EHFrameSection.h"
#include "illvm/FuncV/ELF/GroupSection.h"
#include "illvm/FuncV/ELF/RelocationSection.h"
#include "illvm/FuncV/ELF/StringTableSection.h"
#include "illvm/FuncV/ELF/SymbolTableSection.h"
#include "illvm/FuncV/ELF/DebugAbbrevSection.h"
#include "illvm/FuncV/ELF/DebugAddrSection.h"
#include "illvm/FuncV/ELF/DebugArangeSection.h"
#include "illvm/FuncV/ELF/DebugInfoSection.h"
#include "illvm/FuncV/ELF/DebugLineSection.h"
#include "illvm/FuncV/ELF/DebugLineStrSection.h"
#include "illvm/FuncV/ELF/DebugLoclistsSection.h"
#include "illvm/FuncV/ELF/DebugRnglistSection.h"
#include "illvm/FuncV/ELF/DebugStrOffsetsSection.h"
#include "illvm/FuncV/ELF/DebugStrSection.h"

#endif // ILLVM_SECTIONS_H
