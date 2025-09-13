#include "illvm/FuncV/ELF/FormValue.h"

#include "illvm/Support/Logger.h"

namespace illvm {
namespace funcv {
namespace elf {

std::shared_ptr<FormValue>
FormValueFactory::createFormValue(const uint8_t formId) {
  const auto &logger = Logger::getInstance();

  std::shared_ptr<FormValue> res;

  switch (formId) {
  case 1:  // DW_FORM_addr
    res = std::make_shared<AddrFormValue>();
    break;
  case 3:  // DW_FORM_block2
    res = std::make_shared<BlockNFormValue>(2);
    break;
  case 4:  // DW_FORM_block4
    res = std::make_shared<BlockNFormValue>(4);
    break;
  case 5:  // DW_FORM_data2
    res = std::make_shared<DataNFormValue>(2);
    break;
  case 6:  // DW_FORM_data4
    res = std::make_shared<DataNFormValue>(4);
    break;
  case 7:  // DW_FORM_data8
    res = std::make_shared<DataNFormValue>(8);
    break;
  case 8:  // DW_FORM_string
    res = std::make_shared<StringFormValue>();
    break;
  case 9:  // DW_FORM_block
    logger.fatal("Unsupport DW_FORM_block");
  case 10: // DW_FORM_block1
    res = std::make_shared<BlockNFormValue>(1);
    break;
  case 11: // DW_FORM_data1
    res = std::make_shared<DataNFormValue>(1);
    break;
  case 12: // DW_FORM_flag
    logger.fatal("Unsupport DW_FORM_flag");
  case 13: // DW_FORM_sdata
    res = std::make_shared<SDataFormValue>();
    break;
  case 14: // DW_FORM_strp
    res = std::make_shared<StrPFormValue>();
    break;
  case 15: // DW_FORM_udata
    res = std::make_shared<UDataFormValue>();
    break;
  case 16: // DW_FORM_ref_addr
    logger.fatal("Unsupport DW_FORM_ref_addr");
  case 17: // DW_FORM_ref1
    res = std::make_shared<RefNFormValue>(1);
    break;
  case 18: // DW_FORM_ref2
    res = std::make_shared<RefNFormValue>(2);
    break;
  case 19: // DW_FORM_ref4
    res = std::make_shared<RefNFormValue>(4);
    break;
  case 20: // DW_FORM_ref8
    res = std::make_shared<RefNFormValue>(8);
    break;
  case 21: // DW_FORM_ref_udata
    logger.fatal("Unsupport DW_FORM_ref_udata");
  case 22: // DW_FORM_indirect
    logger.fatal("Unsupport DW_FORM_indirect");
  case 23: // DW_FORM_sec_offset
    res = std::make_shared<SecOffsetFormValue>();
    break;
  case 24: // DW_FORM_exprloc
    res = std::make_shared<ExprLocFormValue>();
    break;
  case 25: // DW_FORM_flag_present
    res = std::make_shared<FlagPresentFormValue>();
    break;
  case 26: // DW_FORM_strx
    logger.fatal("Unsupport DW_FORM_strx");
  case 27: // DW_FORM_addrx
    res = std::make_shared<AddrXFormValue>();
    break;
  case 28: // DW_FORM_ref_sup4
    logger.fatal("Unsupport DW_FORM_ref_sup4");
  case 29: // DW_FORM_strp_sup
    logger.fatal("Unsupport DW_FORM_strp_sup");
  case 30: // DW_FORM_data16
    res = std::make_shared<DataNXFormValue>(16);
    break;
  case 31: // DW_FORM_line_strp
    res = std::make_shared<LineStrPFormValue>();
    break;
  case 32: // DW_FORM_ref_sig8
    logger.fatal("Unsupport DW_FORM_ref_sig8");
  case 33: // DW_FORM_implicit_const
    res = std::make_shared<ImplicitConstFormValue>();
    break;
  case 34: // DW_FORM_loclistx
    // TODO: Support
    logger.fatal("Unsupport DW_FORM_loclistx");
  case 35: // DW_FORM_rnglistx
    // TODO: Support
    logger.fatal("Unsupport DW_FORM_rnglistx");
  case 36: // DW_FORM_ref_sup8
    logger.fatal("Unsupport DW_FORM_ref_sup8");
  case 37: // DW_FORM_strx1
    res = std::make_shared<StrXNFormValue>(1);
    break;
  case 38: // DW_FORM_strx2
    res = std::make_shared<StrXNFormValue>(2);
    break;
  case 39: // DW_FORM_strx3
    res = std::make_shared<StrXNFormValue>(3);
    break;
  case 40: // DW_FORM_strx4
    res = std::make_shared<StrXNFormValue>(4);
    break;
  case 41: // DW_FORM_addrx1
    logger.fatal("Unsupport DW_FORM_addrx1");
  case 42: // DW_FORM_addrx2
    logger.fatal("Unsupport DW_FORM_addrx2");
  case 43: // DW_FORM_addrx3
    logger.fatal("Unsupport DW_FORM_addrx3");
  case 44: // DW_FORM_addrx4
    logger.fatal("Unsupport DW_FORM_addrx4");
  default:
    logger.fatal("Unknown form id: " + std::to_string(formId));
    break;
  }

  return res;
}

} // namespace elf
} // namespace funcv
} // namespace illvm