#ifndef ICLANG_REFERENCE_HPP
#define ICLANG_REFERENCE_HPP

#include "iclang/global.hpp"

namespace iclang {

// Decoupling.
class IdxRef {
private:
  uint64_t value; // Update while writing

public:
  explicit IdxRef(const uint64_t _value) : value(_value) {}

  uint64_t getValue() const { return value; }

  void setValue(const uint64_t _value) { value = _value; }
};

class StrRef {
private:
  const char *value;
  uint64_t offset; // Update while writing.
  uint64_t length;

public:
  StrRef(const char *_value, const uint64_t _offset) : value(_value), offset(_offset) {
    length = strlen(_value);
  }

  const char *getValue() const { return value; }

  uint64_t getOffset() const { return offset; }

  void setOffset(const size_t _offset) { offset = _offset; }

  uint64_t getLength() const { return length; }

  void dump(std::ostream &oss) const {
    oss << value << "(" << offset << ")";
  }
};

} // namespace iclang

#endif //ICLANG_REFERENCE_HPP
