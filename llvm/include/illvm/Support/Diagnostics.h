#ifndef ILLVM_DIAGNOSTICS_H
#define ILLVM_DIAGNOSTICS_H

#include "llvm/Support/Error.h"
#include "llvm/Support/FormatVariadic.h"

#include "illvm/Support/Logger.h"

namespace illvm {

class ILLVMError : public llvm::ErrorInfo<ILLVMError> {
public:
  static char ID;

  explicit ILLVMError(std::string _msg) : msg(std::move(_msg)) {}

  void log(llvm::raw_ostream &OS) const override { OS << msg; }

  std::error_code convertToErrorCode() const override {
    return llvm::inconvertibleErrorCode();
  }

private:
  std::string msg;
};

#define ILLVM_WARN(MSG)                                                        \
  illvm::Logger::getInstance().warning(                                        \
      llvm::formatv("{0}: {1}: {2}", __FILE__, __PRETTY_FUNCTION__, MSG)       \
          .str());

#define ILLVM_UNREACHABLE(MSG)                                                 \
  illvm::Logger::getInstance().fatal(                                          \
      llvm::formatv("{0}: {1}: unreachable: {2}", __FILE__,                    \
                    __PRETTY_FUNCTION__, MSG)                                  \
          .str());

#define ILLVM_FCHECK(EXPR, MSG)                                                \
  if (__builtin_expect(!(EXPR), 0)) {                                          \
    illvm::Logger::getInstance().fatal(                                        \
        llvm::formatv("{0}: {1}: check failed: `{2}`: {3}", __FILE__,          \
                      __PRETTY_FUNCTION__, #EXPR, MSG)                         \
            .str());                                                           \
  }

#define ILLVM_ECHECK(EXPR, MSG)                                                \
  if (__builtin_expect(!(EXPR), 0)) {                                          \
    return llvm::make_error<illvm::ILLVMError>(                                \
        llvm::formatv("{0}: {1}: check failed: `{2}`: {3}", __FILE__,          \
                      __PRETTY_FUNCTION__, #EXPR, MSG)                         \
            .str());                                                           \
  }

#define ILLVM_ECHECK_TO(EXPR, MSG, ERR)                                        \
  if (__builtin_expect(!(EXPR), 0)) {                                          \
    ERR = llvm::make_error<illvm::ILLVMError>(                                 \
        llvm::formatv("{0}: {1}: check failed: `{2}`: {3}", __FILE__,          \
                      __PRETTY_FUNCTION__, #EXPR, MSG)                         \
            .str());                                                           \
    return;                                                                    \
  }

} // namespace illvm

#endif // ILLVM_DIAGNOSTICS_H
