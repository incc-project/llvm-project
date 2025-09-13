#ifndef ILLVM_FORMVALUE_H
#define ILLVM_FORMVALUE_H

#include "DebugLineStrSection.h"

#include <sstream>

#include "illvm/FuncV/ELF/DebugAbbrevSection.h"
#include "illvm/FuncV/ELF/DebugAddrSection.h"
#include "illvm/FuncV/ELF/DebugBase.h"
#include "illvm/FuncV/ELF/DebugLineStrSection.h"
#include "illvm/FuncV/ELF/DebugStrOffsetsSection.h"
#include "illvm/Support/Logger.h"

namespace illvm {
namespace funcv {
namespace elf {

struct FormValueBaseSections {
public:
  std::shared_ptr<DebugAbbrevSection> debugAbbrevSection = nullptr;
  std::shared_ptr<DebugStrSection> debugStrSection = nullptr;
  std::shared_ptr<DebugStrOffsetsSection> debugStrOffsetsSection = nullptr;
  std::shared_ptr<DebugAddrSection> debugAddrSection = nullptr;
  std::shared_ptr<DebugLineStrSection> debugLineStrSection = nullptr;
};

enum class FormType : uint16_t {
  DW_FORM_addr,
  DW_FORM_addrx,
  DW_FORM_addrx_n, // unsupport
  DW_FORM_block, // unsupport
  DW_FORM_block_n,
  DW_FORM_sdata,
  DW_FORM_udata,
  DW_FORM_data_n,
  DW_FORM_data_nx,
  DW_FORM_string,
  DW_FORM_strp,
  DW_FORM_strp_sup, // unsupport
  DW_FORM_strx, // unsupport
  DW_FORM_strx_n,
  DW_FORM_line_strp,
  DW_FORM_ref_addr, // unsupport
  DW_FORM_ref_udata, // unsupport
  DW_FORM_ref_sup_n, // unsupport
  DW_FORM_ref_sig8, // unsupport
  DW_FORM_ref_n,
  DW_FORM_flag, // unsupport
  DW_FORM_flag_present,
  DW_FORM_indirect, // unsupport
  DW_FORM_sec_offset,
  DW_FORM_exprloc,
  DW_FORM_implicit_const,
  DW_FORM_loclistx,
  DW_FORM_rnglistx,
};

// Ref 7.5.5
class FormValue {
public:
  FormType type;

  FormValue(const FormType _type) : type(_type) {}

  virtual ~FormValue() = default;

  virtual int read(const char *buf) = 0;

  virtual int write(char *buf) = 0;

  virtual int size() = 0;

  std::string toString() const {
    std::ostringstream oss;
    dump(oss);
    return oss.str();
  }

  virtual void parseRef(FormValueBaseSections &baseSections);

  virtual void fini() {}

  virtual void dump(std::ostream &oss) const = 0;

  virtual std::string getStringValue() {
    Logger::getInstance().fatal("getStringValue Unreachable:" +
                                std::to_string(static_cast<int>(type)));
  }

  virtual uint64_t getUIntegerValue() {
    Logger::getInstance().fatal("getUIntegerValue Unreachable:" +
                                    std::to_string(static_cast<int>(type)));
  }

  virtual std::vector<uint8_t> getUIArrayValue() {
    Logger::getInstance().fatal("getUIArrayValue Unreachable:" +
                                    std::to_string(static_cast<int>(type)));
  }

  // TODO virtual copy
};

class AddrFormValue final : public FormValue {
public:
  using AddrType = uint64_t;

  AddrType address = 0;

  AddrFormValue() : FormValue(FormType::DW_FORM_addr) {}

  int read(const char *buf) override {
    memcpy(&address, buf, sizeof(AddrType));
    return sizeof(AddrType);
  }

  int write(char *buf) override {
    memcpy(buf, &address, sizeof(AddrType));
    return sizeof(AddrType);
  }

  int size() override { return sizeof(AddrType); }

  void dump(std::ostream &oss) const override { oss << address; }

  uint64_t getUIntegerValue() override {
    return address;
  }
};

class AddrXFormValue final : public FormValue {
public:
  unsigned len = 0;
  uint64_t idx = 0;
  std::shared_ptr<DebugAddrRef> addrRef;

  AddrXFormValue() : FormValue(FormType::DW_FORM_addrx) {}

  int read(const char *buf) override {
    idx = DebugConvert::decodeULEB128(reinterpret_cast<const uint8_t *>(buf), &len);
    return len;
  }

  int write(char *buf) override {
    return DebugConvert::encodeULEB128(idx, reinterpret_cast<uint8_t *>(buf));
  }

  int size() override { return len; }

  void parseRef(FormValueBaseSections &baseSections) override {
    const auto sec = baseSections.debugAddrSection;
    assert(sec != nullptr);
    addrRef = sec->getDebugAddrRef(idx);
  }

  void fini() override {
    idx = addrRef->idx->getValue();
  }

  void dump(std::ostream &oss) const override {
    oss << 0;
  }

  uint64_t getUIntegerValue() override {
    return 0;
  }
};

// TODO: Check binary equivalence of block, if failed, ban block!
class BlockNFormValue final : public FormValue {
public:
  int n;
  uint64_t len = 0;
  std::vector<uint8_t> data;

  BlockNFormValue(const int _n) : FormValue(FormType::DW_FORM_block_n), n(_n) {}

  int read(const char *buf) override {
    memcpy(&len, buf, n);
    const auto temp = new uint8_t[len];
    memcpy(temp, buf + n, len);
    data.reserve(len);
    for (size_t i = 0; i < len; i += 1) {
      data.push_back(temp[i]);
    }
    delete [] temp;
    return n + len;
  }

  int write(char *buf) override {
    memcpy(buf, &len, n);
    memcpy(buf + n, data.data(), len);
    return n + len;
  }

  int size() override { return n + len; }

  void dump(std::ostream &oss) const override {
    oss << len;
    for (size_t i = 0; i < len; i += 1) {
      oss << " " << data[i];
    }
  }

  std::string getStringValue() override {
    Logger::getInstance().fatal("BlockNFormValue::getStringValue unreachable");
  }

  uint64_t getUIntegerValue() override {
    Logger::getInstance().fatal("BlockNFormValue::UIntegerValue unreachable");
  }
};

class SDataFormValue final : public FormValue {
public:
  unsigned len = 0;
  int64_t data = 0;

  SDataFormValue() : FormValue(FormType::DW_FORM_sdata) {}

  int read(const char *buf) override {
    data = DebugConvert::decodeSLEB128(reinterpret_cast<const uint8_t *>(buf), &len);
    return len;
  }

  int write(char *buf) override {
    return DebugConvert::encodeSLEB128(data, reinterpret_cast<uint8_t *>(buf));
  }

  int size() override { return len; }

  void dump(std::ostream &oss) const override {
    oss << data;
  }
};

class UDataFormValue final : public FormValue {
public:
  unsigned len = 0;
  uint64_t data = 0;

  UDataFormValue() : FormValue(FormType::DW_FORM_udata) {}

  int read(const char *buf) override {
    data = DebugConvert::decodeULEB128(reinterpret_cast<const uint8_t *>(buf), &len);
    return len;
  }

  int write(char *buf) override {
    return DebugConvert::encodeULEB128(data, reinterpret_cast<uint8_t *>(buf));
  }

  int size() override { return len; }

  void dump(std::ostream &oss) const override {
    oss << data;
  }

  uint64_t getUIntegerValue() override {
    return data;
  }
};

class DataNFormValue final : public FormValue {
public:
  int n;
  uint64_t data = 0;

  DataNFormValue(const int _n) : FormValue(FormType::DW_FORM_data_n), n(_n) {}

  int read(const char *buf) override {
    memcpy(&data, buf, n);
    return n;
  }

  int write(char *buf) override {
    memcpy(buf, &data, n);
    return n;
  }

  int size() override { return n; }

  void dump(std::ostream &oss) const override { oss << data; }

  uint64_t getUIntegerValue() override {
    return data;
  }
};

class DataNXFormValue final : public FormValue {
public:
  int n;
  std::vector<uint8_t> data;

  DataNXFormValue(const int _n) : FormValue(FormType::DW_FORM_data_nx), n(_n) {
    data.resize(n);
  }

  int read(const char *buf) override {
    memcpy(data.data(), buf, n);
    return n;
  }

  int write(char *buf) override {
    memcpy(buf, data.data(), n);
    return n;
  }

  int size() override { return n; }

  void dump(std::ostream &oss) const override {
    for (int i = 0; i < n; i += 1) {
      if (i != 0) {
        oss << " ";
      }
      oss << data[i];
    }
  }

  std::vector<uint8_t> getUIArrayValue() override {
    return data;
  }
};

class RefNFormValue final : public FormValue {
public:
  int n;
  // Update according to dieOffset
  uint64_t offset = 0;
  // Decouple: update by outer parse reference.
  std::shared_ptr<IdxRef> dieOffset;

  RefNFormValue(const int _n) : FormValue(FormType::DW_FORM_ref_n), n(_n) {}

  int read(const char *buf) override { memcpy(&offset, buf, n); return n; }

  int write(char *buf) override { memcpy(buf, &offset, n); return n; }

  int size() override { return n; }

  void fini() override {
    offset = dieOffset->getValue();
  }

  void dump(std::ostream &oss) const override {
    oss << offset;
  }
};

class FlagPresentFormValue final : public FormValue {
public:
  FlagPresentFormValue() : FormValue(FormType::DW_FORM_flag_present) {}

  int read(const char *buf) override { return 0; }

  int write(char *buf) override { return 0; }

  int size() override { return 0; }

  void dump(std::ostream &oss) const override {
    oss << true;
  }

  uint64_t getUIntegerValue() override {
    return 1;
  }
};

// Single table: do not update offset.
class SecOffsetFormValue final : public FormValue {
public:
  using SecOffsetESType = uint32_t;

  SecOffsetESType offset = 0;

  SecOffsetFormValue() : FormValue(FormType::DW_FORM_sec_offset) {}

  int read(const char *buf) override {
    memcpy(&offset, buf, sizeof(SecOffsetESType));
    return sizeof(SecOffsetESType);
  }

  int write(char *buf) override {
    memcpy(buf, &offset, sizeof(SecOffsetESType));
    return sizeof(SecOffsetESType);
  }

  int size() override { return sizeof(SecOffsetESType); }

  void dump(std::ostream &oss) const override {
    oss << offset;
  }

  uint64_t getUIntegerValue() override {
    return offset;
  }
};

// TODO: 2.5, 2.6
class ExprLocFormValue final : public FormValue {
public:
  unsigned n;
  uint64_t len = 0;
  std::vector<uint8_t> data;

  ExprLocFormValue() : FormValue(FormType::DW_FORM_exprloc), n() {}

  int read(const char *buf) override {
    len = DebugConvert::decodeULEB128(reinterpret_cast<const uint8_t *>(buf), &n);
    const auto temp = new uint8_t[len];
    memcpy(temp, buf + n, len);
    data.reserve(len);
    for (size_t i = 0; i < len; i += 1) {
      data.push_back(temp[i]);
    }
    delete [] temp;
    return n + len;
  }

  int write(char *buf) override {
    DebugConvert::encodeULEB128(len, reinterpret_cast<uint8_t *>(buf));
    memcpy(buf + n, data.data(), len);
    return n + len;
  }

  int size() override { return n + len; }

  void dump(std::ostream &oss) const override {
    oss << len;
    for (size_t i = 0; i < len; i += 1) {
      oss << " " << data[i];
    }
  }
};

class LineStrPFormValue final : public FormValue {
public:
  using OffsetESType = uint32_t;

  OffsetESType offset = 0;
  std::shared_ptr<DebugStrRef> strRef;

  LineStrPFormValue() : FormValue(FormType::DW_FORM_line_strp) {}

  int read(const char *buf) override {
    memcpy(&offset, buf, sizeof(OffsetESType));
    return sizeof(OffsetESType);
  }

  int write(char *buf) override {
    memcpy(buf, &offset, sizeof(OffsetESType));
    return sizeof(OffsetESType);
  }

  int size() override { return sizeof(OffsetESType); }

  void parseRef(FormValueBaseSections &baseSections) override {
    const auto sec = baseSections.debugLineStrSection;
    assert(sec != nullptr);
    strRef = sec->parseOriginalIndex(offset);
  }

  void fini() override {
    offset = strRef->offset->getValue();
  }

  void dump(std::ostream &oss) const override {
    oss << strRef->str;
  }

  std::string getStringValue() override {
    return strRef->str;
  }
};

class ImplicitConstFormValue final : public FormValue {
public:
  ImplicitConstFormValue() : FormValue(FormType::DW_FORM_implicit_const) {}

  // Note: abbrev buffer.
  int read(const char *buf) override {
    return 0;
  }

  // Note: abbrev buffer.
  int write(char *buf) override {
    return 0;
  }

  int size() override { return 0; }

  void dump(std::ostream &oss) const override {}
};

// TODO: DW_FORM_loclistx, DW_FORM_rnglistx

class StringFormValue final : public FormValue {
public:
  std::string str;

  StringFormValue() : FormValue(FormType::DW_FORM_string) {}

  int read(const char *buf) override {
    str = std::string(buf);
    return str.length() + 1;
  }

  int write(char *buf) override {
    memcpy(buf, str.data(), str.length() + 1);
    return str.length() + 1;
  }

  int size() override { return str.length() + 1; }

  void dump(std::ostream &oss) const override {
    oss << str;
  }

  std::string getStringValue() override {
    return str;
  }
};

class StrPFormValue final : public FormValue {
public:
  using OffsetESType = uint32_t;

  OffsetESType offset = 0;
  std::shared_ptr<DebugStrRef> strRef;

  StrPFormValue() : FormValue(FormType::DW_FORM_strp) {}

  int read(const char *buf) override {
    memcpy(&offset, buf, sizeof(OffsetESType));
    return sizeof(OffsetESType);
  }

  int write(char *buf) override {
    memcpy(buf, &offset, sizeof(OffsetESType));
    return sizeof(OffsetESType);
  }

  int size() override { return sizeof(OffsetESType); }

  void parseRef(FormValueBaseSections &baseSections) override {
    const auto sec = baseSections.debugStrSection;
    assert(sec != nullptr);
    strRef = sec->parseOriginalIndex(offset);
  }

  void fini() override {
    offset = strRef->offset->getValue();
  }

  void dump(std::ostream &oss) const override {
    oss << strRef->str;
  }

  std::string getStringValue() override {
    return strRef->str;
  }
};

class StrXNFormValue final : public FormValue {
public:
  int n;
  uint32_t idx = 0;
  std::shared_ptr<DebugStrOffsetRef> strOffsetsRef;

  StrXNFormValue(const int _n) : FormValue(FormType::DW_FORM_strx_n), n(_n) {}

  int read(const char *buf) override { memcpy(&idx, buf, n); return n; }

  int write(char *buf) override { memcpy(buf, &idx, n); return n; }

  int size() override { return n; }

  void parseRef(FormValueBaseSections &baseSections) override {
    const auto sec = baseSections.debugStrOffsetsSection;
    assert(sec != nullptr);
    strOffsetsRef = sec->getStrOffsetRef(idx);
  }

  void fini() override {
    idx = strOffsetsRef->idx->getValue();
  }

  void dump(std::ostream &oss) const override {
    oss << strOffsetsRef->strRef->str;
  }

  std::string getStringValue() override {
    return strOffsetsRef->strRef->str;
  }
};

class FormValueFactory {
public:
  static std::shared_ptr<FormValue> createFormValue(const uint8_t formId);
};

// TODO Note: new do not break old, cp.

} // namespace elf
} // namespace funcv
} // namespace illvm

#endif //ILLVM_FORMVALUE_H
