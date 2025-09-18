#include "illvm/FuncV/ELF/DebugBase.h"

#include <iomanip>
#include <sstream>

#include "illvm/Support/Diagnostics.h"

namespace illvm {
namespace funcv {
namespace elf {

std::string DebugTypeToString::getUnitType(const uint8_t unitType) {
  switch (unitType) {
  case 0x01:
    return "DW_UT_compile";
  case 0x02:
    return "DW_UT_type";
  case 0x03:
    return "DW_UT_partial";
  case 0x04:
    return "DW_UT_skeleton";
  case 0x05:
    return "DW_UT_split_compile";
  case 0x06:
    return "DW_UT_split_type";
  case 0x80:
    return "DW_UT_lo_user";
  case 0xff:
    return "DW_UT_hi_user";
  default:
    return "DW_UT_<unknown>";
  }
}

std::string DebugTypeToString::getTagName(const uint64_t tag) {
  switch (tag) {
  case 0x01:
    return "DW_TAG_array_type";
  case 0x02:
    return "DW_TAG_class_type";
  case 0x03:
    return "DW_TAG_entry_point";
  case 0x04:
    return "DW_TAG_enumeration_type";
  case 0x05:
    return "DW_TAG_formal_parameter";
  case 0x08:
    return "DW_TAG_imported_declaration";
  case 0x0a:
    return "DW_TAG_label";
  case 0x0b:
    return "DW_TAG_lexical_block";
  case 0x0d:
    return "DW_TAG_member";
  case 0x0f:
    return "DW_TAG_pointer_type";
  case 0x10:
    return "DW_TAG_reference_type";
  case 0x11:
    return "DW_TAG_compile_unit";
  case 0x12:
    return "DW_TAG_string_type";
  case 0x13:
    return "DW_TAG_structure_type";
  case 0x15:
    return "DW_TAG_subroutine_type";
  case 0x16:
    return "DW_TAG_typedef";
  case 0x17:
    return "DW_TAG_union_type";
  case 0x18:
    return "DW_TAG_unspecified_parameters";
  case 0x19:
    return "DW_TAG_variant";
  case 0x1a:
    return "DW_TAG_common_block";
  case 0x1b:
    return "DW_TAG_common_inclusion";
  case 0x1c:
    return "DW_TAG_inheritance";
  case 0x1d:
    return "DW_TAG_inlined_subroutine";
  case 0x1e:
    return "DW_TAG_module";
  case 0x1f:
    return "DW_TAG_ptr_to_member_type";
  case 0x20:
    return "DW_TAG_set_type";
  case 0x21:
    return "DW_TAG_subrange_type";
  case 0x22:
    return "DW_TAG_with_stmt";
  case 0x23:
    return "DW_TAG_access_declaration";
  case 0x24:
    return "DW_TAG_base_type";
  case 0x25:
    return "DW_TAG_catch_block";
  case 0x26:
    return "DW_TAG_const_type";
  case 0x27:
    return "DW_TAG_constant";
  case 0x28:
    return "DW_TAG_enumerator";
  case 0x29:
    return "DW_TAG_file_type";
  case 0x2a:
    return "DW_TAG_friend";
  case 0x2b:
    return "DW_TAG_namelist";
  case 0x2c:
    return "DW_TAG_namelist_item";
  case 0x2d:
    return "DW_TAG_packed_type";
  case 0x2e:
    return "DW_TAG_subprogram";
  case 0x2f:
    return "DW_TAG_template_type_parameter";
  case 0x30:
    return "DW_TAG_template_value_parameter";
  case 0x31:
    return "DW_TAG_thrown_type";
  case 0x32:
    return "DW_TAG_try_block";
  case 0x33:
    return "DW_TAG_variant_part";
  case 0x34:
    return "DW_TAG_variable";
  case 0x35:
    return "DW_TAG_volatile_type";
  case 0x36:
    return "DW_TAG_dwarf_procedure";
  case 0x37:
    return "DW_TAG_restrict_type";
  case 0x38:
    return "DW_TAG_interface_type";
  case 0x39:
    return "DW_TAG_namespace";
  case 0x3a:
    return "DW_TAG_imported_module";
  case 0x3b:
    return "DW_TAG_unspecified_type";
  case 0x3c:
    return "DW_TAG_partial_unit";
  case 0x3d:
    return "DW_TAG_imported_unit";
  case 0x3f:
    return "DW_TAG_condition";
  case 0x40:
    return "DW_TAG_shared_type";
  case 0x41:
    return "DW_TAG_type_unit";
  case 0x42:
    return "DW_TAG_rvalue_reference_type";
  case 0x43:
    return "DW_TAG_template_alias";
  case 0x44:
    return "DW_TAG_coarray_type";
  case 0x45:
    return "DW_TAG_generic_subrange";
  case 0x46:
    return "DW_TAG_dynamic_type";
  case 0x47:
    return "DW_TAG_atomic_type";
  case 0x48:
    return "DW_TAG_call_site";
  case 0x49:
    return "DW_TAG_call_site_parameter";
  case 0x4a:
    return "DW_TAG_skeleton_unit";
  case 0x4b:
    return "DW_TAG_immutable_type";
  case 0x4080:
    return "DW_TAG_lo_user";
  case 0xffff:
    return "DW_TAG_hi_user";
  default:
    return "DW_TAG_<unknown>";
  }
}

std::string DebugTypeToString::getAttrName(const uint64_t attr) {
  switch (attr) {
  case 0x01:
    return "DW_AT_sibling";
  case 0x02:
    return "DW_AT_location";
  case 0x03:
    return "DW_AT_name";
  case 0x09:
    return "DW_AT_ordering";
  case 0x0b:
    return "DW_AT_byte_size";
  case 0x0d:
    return "DW_AT_bit_size";
  case 0x10:
    return "DW_AT_stmt_list";
  case 0x11:
    return "DW_AT_low_pc";
  case 0x12:
    return "DW_AT_high_pc";
  case 0x13:
    return "DW_AT_language";
  case 0x15:
    return "DW_AT_discr";
  case 0x16:
    return "DW_AT_discr_value";
  case 0x17:
    return "DW_AT_visibility";
  case 0x18:
    return "DW_AT_import";
  case 0x19:
    return "DW_AT_string_length";
  case 0x1a:
    return "DW_AT_common_reference";
  case 0x1b:
    return "DW_AT_comp_dir";
  case 0x1c:
    return "DW_AT_const_value";
  case 0x1d:
    return "DW_AT_containing_type";
  case 0x1e:
    return "DW_AT_default_value";
  case 0x20:
    return "DW_AT_inline";
  case 0x21:
    return "DW_AT_is_optional";
  case 0x22:
    return "DW_AT_lower_bound";
  case 0x25:
    return "DW_AT_producer";
  case 0x27:
    return "DW_AT_prototyped";
  case 0x2a:
    return "DW_AT_return_addr";
  case 0x2c:
    return "DW_AT_start_scope";
  case 0x2e:
    return "DW_AT_bit_stride";
  case 0x2f:
    return "DW_AT_upper_bound";
  case 0x31:
    return "DW_AT_abstract_origin";
  case 0x32:
    return "DW_AT_accessibility";
  case 0x33:
    return "DW_AT_address_class";
  case 0x34:
    return "DW_AT_artificial";
  case 0x35:
    return "DW_AT_base_types";
  case 0x36:
    return "DW_AT_calling_convention";
  case 0x37:
    return "DW_AT_count";
  case 0x38:
    return "DW_AT_data_member_location";
  case 0x39:
    return "DW_AT_decl_column";
  case 0x3a:
    return "DW_AT_decl_file";
  case 0x3b:
    return "DW_AT_decl_line";
  case 0x3c:
    return "DW_AT_declaration";
  case 0x3d:
    return "DW_AT_discr_list";
  case 0x3e:
    return "DW_AT_encoding";
  case 0x3f:
    return "DW_AT_external";
  case 0x40:
    return "DW_AT_frame_base";
  case 0x41:
    return "DW_AT_friend";
  case 0x42:
    return "DW_AT_identifier_case";
  case 0x44:
    return "DW_AT_namelist_item";
  case 0x45:
    return "DW_AT_priority";
  case 0x46:
    return "DW_AT_segment";
  case 0x47:
    return "DW_AT_specification";
  case 0x48:
    return "DW_AT_static_link";
  case 0x49:
    return "DW_AT_type";
  case 0x4a:
    return "DW_AT_use_location";
  case 0x4b:
    return "DW_AT_variable_parameter";
  case 0x4c:
    return "DW_AT_virtuality";
  case 0x4d:
    return "DW_AT_vtable_elem_location";
  case 0x4e:
    return "DW_AT_allocated";
  case 0x4f:
    return "DW_AT_associated";
  case 0x50:
    return "DW_AT_data_location";
  case 0x51:
    return "DW_AT_byte_stride";
  case 0x52:
    return "DW_AT_entry_pc";
  case 0x53:
    return "DW_AT_use_UTF8";
  case 0x54:
    return "DW_AT_extension";
  case 0x55:
    return "DW_AT_ranges";
  case 0x56:
    return "DW_AT_trampoline";
  case 0x57:
    return "DW_AT_call_column";
  case 0x58:
    return "DW_AT_call_file";
  case 0x59:
    return "DW_AT_call_line";
  case 0x5a:
    return "DW_AT_description";
  case 0x5b:
    return "DW_AT_binary_scale";
  case 0x5c:
    return "DW_AT_decimal_scale";
  case 0x5d:
    return "DW_AT_small";
  case 0x5e:
    return "DW_AT_decimal_sign";
  case 0x5f:
    return "DW_AT_digit_count";
  case 0x60:
    return "DW_AT_picture_string";
  case 0x61:
    return "DW_AT_mutable";
  case 0x62:
    return "DW_AT_threads_scaled";
  case 0x63:
    return "DW_AT_explicit";
  case 0x64:
    return "DW_AT_object_pointer";
  case 0x65:
    return "DW_AT_endianity";
  case 0x66:
    return "DW_AT_elemental";
  case 0x67:
    return "DW_AT_pure";
  case 0x68:
    return "DW_AT_recursive";
  case 0x69:
    return "DW_AT_signature";
  case 0x6a:
    return "DW_AT_main_subprogram";
  case 0x6b:
    return "DW_AT_data_bit_offset";
  case 0x6c:
    return "DW_AT_const_expr";
  case 0x6d:
    return "DW_AT_enum_class";
  case 0x6e:
    return "DW_AT_linkage_name";
  case 0x6f:
    return "DW_AT_string_length_bit_size";
  case 0x70:
    return "DW_AT_string_length_byte_size";
  case 0x71:
    return "DW_AT_rank";
  case 0x72:
    return "DW_AT_str_offsets_base";
  case 0x73:
    return "DW_AT_addr_base";
  case 0x74:
    return "DW_AT_rnglists_base";
  case 0x76:
    return "DW_AT_dwo_name";
  case 0x77:
    return "DW_AT_reference";
  case 0x78:
    return "DW_AT_rvalue_reference";
  case 0x79:
    return "DW_AT_macros";
  case 0x7a:
    return "DW_AT_call_all_calls";
  case 0x7b:
    return "DW_AT_call_all_source_calls";
  case 0x7c:
    return "DW_AT_call_all_tail_calls";
  case 0x7d:
    return "DW_AT_call_return_pc";
  case 0x7e:
    return "DW_AT_call_value";
  case 0x7f:
    return "DW_AT_call_origin";
  case 0x80:
    return "DW_AT_call_parameter";
  case 0x81:
    return "DW_AT_call_pc";
  case 0x82:
    return "DW_AT_call_tail_call";
  case 0x83:
    return "DW_AT_call_target";
  case 0x84:
    return "DW_AT_call_target_clobbered";
  case 0x85:
    return "DW_AT_call_data_location";
  case 0x86:
    return "DW_AT_call_data_value";
  case 0x87:
    return "DW_AT_noreturn";
  case 0x88:
    return "DW_AT_alignment";
  case 0x89:
    return "DW_AT_export_symbols";
  case 0x8a:
    return "DW_AT_deleted";
  case 0x8b:
    return "DW_AT_defaulted";
  case 0x8c:
    return "DW_AT_loclists_base";
  case 0x2000:
    return "DW_AT_lo_user";
  case 0x3fff:
    return "DW_AT_hi_user";
  default:
    return "DW_AT_<unknown>";
  }
}

std::string DebugTypeToString::getFormName(const uint64_t form) {
  switch (form) {
  case 0x01:
    return "DW_FORM_addr";
  case 0x03:
    return "DW_FORM_block2";
  case 0x04:
    return "DW_FORM_block4";
  case 0x05:
    return "DW_FORM_data2";
  case 0x06:
    return "DW_FORM_data4";
  case 0x07:
    return "DW_FORM_data8";
  case 0x08:
    return "DW_FORM_string";
  case 0x09:
    return "DW_FORM_block";
  case 0x0a:
    return "DW_FORM_block1";
  case 0x0b:
    return "DW_FORM_data1";
  case 0x0c:
    return "DW_FORM_flag";
  case 0x0d:
    return "DW_FORM_sdata";
  case 0x0e:
    return "DW_FORM_strp";
  case 0x0f:
    return "DW_FORM_udata";
  case 0x10:
    return "DW_FORM_ref_addr";
  case 0x11:
    return "DW_FORM_ref1";
  case 0x12:
    return "DW_FORM_ref2";
  case 0x13:
    return "DW_FORM_ref4";
  case 0x14:
    return "DW_FORM_ref8";
  case 0x15:
    return "DW_FORM_ref_udata";
  case 0x16:
    return "DW_FORM_indirect";
  case 0x17:
    return "DW_FORM_sec_offset";
  case 0x18:
    return "DW_FORM_exprloc";
  case 0x19:
    return "DW_FORM_flag_present";
  case 0x1a:
    return "DW_FORM_strx";
  case 0x1b:
    return "DW_FORM_addrx";
  case 0x1c:
    return "DW_FORM_ref_sup4";
  case 0x1d:
    return "DW_FORM_strp_sup";
  case 0x1e:
    return "DW_FORM_data16";
  case 0x1f:
    return "DW_FORM_line_strp";
  case 0x20:
    return "DW_FORM_ref_sig8";
  case 0x21:
    return "DW_FORM_implicit_const";
  case 0x22:
    return "DW_FORM_loclistx";
  case 0x23:
    return "DW_FORM_rnglistx";
  case 0x24:
    return "DW_FORM_ref_sup8";
  case 0x25:
    return "DW_FORM_strx1";
  case 0x26:
    return "DW_FORM_strx2";
  case 0x27:
    return "DW_FORM_strx3";
  case 0x28:
    return "DW_FORM_strx4";
  case 0x29:
    return "DW_FORM_addrx1";
  case 0x2a:
    return "DW_FORM_addrx2";
  case 0x2b:
    return "DW_FORM_addrx3";
  case 0x2c:
    return "DW_FORM_addrx4";
  default:
    return "DW_FORM_<unknown>";
  }
}

std::string DebugTypeToString::getLLEName(const uint8_t kind) {
  switch (kind) {
  case 0x00:
    return "DW_LLE_end_of_list";
  case 0x01:
    return "DW_LLE_base_addressx";
  case 0x02:
    return "DW_LLE_startx_endx";
  case 0x03:
    return "DW_LLE_startx_length";
  case 0x04:
    return "DW_LLE_offset_pair";
  case 0x05:
    return "DW_LLE_default_location";
  case 0x06:
    return "DW_LLE_base_address";
  case 0x07:
    return "DW_LLE_start_end";
  case 0x08:
    return "DW_LLE_start_length";
  default:
    return "DW_UT_<unknown>";
  }
}

std::string DebugTypeToString::opcodeName(const uint8_t opcode) {
  switch (opcode) {
  case 1:
    return "copy";
  case 2:
    return "advance_pc";
  case 3:
    return "advance_line";
  case 4:
    return "set_file";
  case 5:
    return "set_column";
  case 6:
    return "negate_stmt";
  case 7:
    return "set_basic_block";
  case 8:
    return "const_add_pc";
  case 9:
    return "fixed_advance_pc";
  case 10:
    return "set_prologue_end";
  case 11:
    return "set_epilogue_begin";
  case 12:
    return "set_isa";
  default:
    return std::to_string(opcode);
  }
}

std::string DebugConvert::intToHex(const uint64_t val, const int width) {
  std::ostringstream oss;
  oss << std::hex << std::setw(width) << std::setfill('0') << val;
  return oss.str();
}

uint64_t DebugConvert::decodeULEB128(const uint8_t *p, unsigned &len) {
  const char *err = nullptr;

  const uint64_t val = llvm::decodeULEB128(p, &len, nullptr, &err);
  ILLVM_FCHECK(err == nullptr, err);
  return val;
}

int64_t DebugConvert::decodeSLEB128(const uint8_t *p, unsigned &len) {
  const char *err = nullptr;

  const int64_t val = llvm::decodeSLEB128(p, &len, nullptr, &err);
  ILLVM_FCHECK(err == nullptr, err);
  return val;
}

unsigned DebugConvert::encodeULEB128(const uint64_t val, uint8_t *p) {
  return llvm::encodeULEB128(val, p);
}

unsigned DebugConvert::encodeSLEB128(const int64_t val, uint8_t *p) {
  return llvm::encodeSLEB128(val, p);
}

} // namespace elf
} // namespace funcv
} // namespace illvm
