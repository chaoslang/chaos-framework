#pragma once

#import "./chaos_backend.h"

#include "chaos_semantic.h"
#include <sstream>
#include <unordered_map>

static std::string mangle_symbol_name_c(const std::string &name) {
  std::string out = name;
  for (char &c : out) {
    if (c == '.')
      c = "_"[0];
  }
  return out;
}

static std::string escape_c_string(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (unsigned char c : s) {
    switch (c) {
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      out += static_cast<char>(c);
      break;
    }
  }
  return out;
}

static std::string lower_type_c(IR_Type type) {
  switch (type.kind) {
  case IR_I32:
    return "int";
    break;
  case IR_I64:
    return "long";
    break;
  case IR_F32:
    return "float";
    break;
  case IR_BOOL:
    return "bool";
    break;
  case IR_VOID:
    return "void";
    break;
  case IR_I8:
    return "int8_t";
    break;
  case IR_PTR:
    return "intptr_t";
    break;
  case IR_U8:
    return "uint8_t";
    break;
  case IR_U64:
    return "size_t";
    break;
  case IR_U32:
    return "uint32_t";
    break;
  case IR_F64:
    return "double";
    break;
  case IR_U16:
    return "uint16_t";
    break;
  case IR_I16:
    return "int16_t";
    break;
  case IR_STR:
    return "ChaosString";
    break;
  default:
    assert(false && "Unknown type name");
    return {"void"};
  };
}

static std::string printf_fmt(IR_Type type) {
  switch (type.kind) {
  case IR_I32:
    return "%d";
  case IR_I64:
    return "%ld";
  case IR_F32:
    return "%f";
  case IR_F64:
    return "%lf";
  case IR_BOOL:
    return "%d";
  case IR_I8:
    return "%d";
  case IR_U8:
    return "%u";
  case IR_U16:
    return "%u";
  case IR_U32:
    return "%u";
  case IR_U64:
    return "%zu";
  case IR_PTR:
    return "%p";
  case IR_STR:
    return "ChaosString";
  default:
    std::cout << "type unsupported: " << lower_type_c(type) << std::endl;
    assert(false && "Unsupported printf type");
    return "";
  }
}

class CBackend {
private:
  std::stringstream output;
  std::unordered_map<int, std::string> temp_names;
  int temp_counter = 0;
  int indent_level = 0;

  std::string indent() { return std::string(indent_level * 2, ' '); }

  std::string get_temp_name(IR_Value v) {
    if (temp_names.find(v) == temp_names.end()) {
      temp_names[v] = "t" + std::to_string(temp_counter++);
    }
    return temp_names[v];
  }

  void emit_inst(const IR_Inst &inst) {
    switch (inst.op) {
    case IR_CONST_INT:
      output << indent() << lower_type_c(inst.type) << " "
             << get_temp_name(inst.dst) << " = " << inst.int_value << ";\n";
      break;

    case IR_CONST_FLOAT:
      output << indent() << lower_type_c(inst.type) << " "
             << get_temp_name(inst.dst) << " = " << inst.float_value << ";\n";
      break;

    case IR_ADD:
      output << indent() << lower_type_c(inst.type) << ' '
             << get_temp_name(inst.dst) << " = " << get_temp_name(inst.a)
             << " + " << get_temp_name(inst.b) << ";\n";
      break;

    case IR_SUB:
      output << indent() << lower_type_c(inst.type) << ' '
             << get_temp_name(inst.dst) << " = " << get_temp_name(inst.a)
             << " - " << get_temp_name(inst.b) << ";\n";
      break;

    case IR_MUL:
      output << indent() << lower_type_c(inst.type) << ' '
             << get_temp_name(inst.dst) << " = " << get_temp_name(inst.a)
             << " * " << get_temp_name(inst.b) << ";\n";
      break;

    case IR_DIV:
      output << indent() << lower_type_c(inst.type) << ' '
             << get_temp_name(inst.dst) << " = " << get_temp_name(inst.a)
             << " / " << get_temp_name(inst.b) << ";\n";
      break;

    case IR_NEG:
      output << indent() << lower_type_c(inst.type) << ' '
             << get_temp_name(inst.dst) << " = -" << get_temp_name(inst.a)
             << ";\n";
      break;

    case IR_NOT:
      output << indent() << "bool " << get_temp_name(inst.dst) << " = !"
             << get_temp_name(inst.a) << ";\n";
      break;

    case IR_CMP_LT:
      output << indent() << "bool " << get_temp_name(inst.dst) << " = "
             << get_temp_name(inst.a) << " < " << get_temp_name(inst.b)
             << ";\n";
      break;

    case IR_CMP_GT:
      output << indent() << "bool " << get_temp_name(inst.dst) << " = "
             << get_temp_name(inst.a) << " > " << get_temp_name(inst.b)
             << ";\n";
      break;

    case IR_CMP_EQ:
      output << indent() << "bool " << get_temp_name(inst.dst) << " = "
             << get_temp_name(inst.a) << " == " << get_temp_name(inst.b)
             << ";\n";
      break;

    case IR_CMP_NEQ:
      output << indent() << "bool " << get_temp_name(inst.dst) << " = "
             << get_temp_name(inst.a) << " != " << get_temp_name(inst.b)
             << ";\n";
      break;

    case IR_CALL: {
      if (inst.type.kind != IR_VOID) {
        output << indent() << lower_type_c(inst.type) << ' '
               << get_temp_name(inst.dst) << " = ";
      } else {
        output << indent();
      }

      output << mangle_symbol_name_c(inst.name) << "(";
      for (size_t i = 0; i < inst.args.size(); i++) {
        output << get_temp_name(inst.args[i]);
        if (i + 1 < inst.args.size())
          output << ", ";
      }
      output << ");\n";
      break;
    }
    case IR_CONST_STRING:
      output << indent() << "ChaosString " << get_temp_name(inst.dst) << " = {"
             << inst.string_value.size() << ", \""
             << escape_c_string(inst.string_value) << "\"};\n";
      break;
    case IR_INTRINSIC_PRINT: {
      for (size_t i = 0; i < inst.args.size(); i++) {
        auto t = inst.arg_types[i];
        auto arg = inst.args[i];
        if (t.kind == IR_STR) {
          output << indent() << "printf(\"%.*s\", (int)" << get_temp_name(arg)
                 << ".len, " << get_temp_name(arg) << ".data);\n";
        } else {
          output << indent() << "printf(\"" << printf_fmt(t) << "\", "
                 << get_temp_name(arg) << ");\n";
        }
        if (i + 1 < inst.args.size()) {
          output << indent() << "printf(\" \" );\n";
        }
      }
      output << indent() << "printf(\"\\n\");\n";
      break;
    }
    case IR_INTRINSIC_ALLOC:
      output << indent() << "intptr_t " << get_temp_name(inst.dst)
             << " = (intptr_t)malloc(" << get_temp_name(inst.a) << " * "
             << inst.int_value << ");\n";
      break;

    case IR_INTRINSIC_FREE:
      output << indent() << "free((void*)(intptr_t)" << get_temp_name(inst.a)
             << ");\n";
      break;

    case IR_INTRINSIC_MEMCPY:
      output << indent() << "memcpy((void*)(intptr_t)" << get_temp_name(inst.a)
             << ", (void*)(intptr_t)" << get_temp_name(inst.b) << ", "
             << get_temp_name(inst.args[0]) << ");\n";
      break;

    case IR_INTRINSIC_CAST:
      output << indent() << lower_type_c(inst.type) << " "
             << get_temp_name(inst.dst) << " = (" << lower_type_c(inst.type)
             << ")" << get_temp_name(inst.a) << ";\n";
      break;

    case IR_INTRINSIC_DLOPEN: {
      bool is_str = !inst.arg_types.empty() && inst.arg_types[0].kind == IR_STR;
      std::string path_expr =
          is_str ? (get_temp_name(inst.a) + ".data")
                 : ("(const char*)(intptr_t)" + get_temp_name(inst.a));
      output << indent() << "intptr_t " << get_temp_name(inst.dst)
             << " = (intptr_t)dlopen(" << path_expr << ", RTLD_LAZY);\n";
      break;
    }

    case IR_INTRINSIC_DLSYM: {
      bool is_str = !inst.arg_types.empty() && inst.arg_types[0].kind == IR_STR;
      std::string name_expr =
          is_str ? (get_temp_name(inst.b) + ".data")
                 : ("(const char*)(intptr_t)" + get_temp_name(inst.b));
      output << indent() << "intptr_t " << get_temp_name(inst.dst)
             << " = (intptr_t)dlsym((void*)(intptr_t)" << get_temp_name(inst.a)
             << ", " << name_expr << ");\n";
      break;
    }

    case IR_INTRINSIC_FFI_CALL: {
      std::string dst = get_temp_name(inst.dst);
      size_t nargs = inst.args.size();
      output << indent() << "intptr_t " << dst << ";\n";
      output << indent() << "{\n";
      output << indent() << "  ffi_cif __cif_" << dst << ";\n";
      if (nargs > 0) {
        output << indent() << "  ffi_type *__atypes_" << dst << "[" << nargs
               << "] = {";
        for (size_t i = 0; i < nargs; i++) {
          output << "&ffi_type_uint64";
          if (i + 1 < nargs)
            output << ", ";
        }
        output << "};\n";
        output << indent() << "  uint64_t __avals_" << dst << "[" << nargs
               << "] = {";
        for (size_t i = 0; i < nargs; i++) {
          size_t ssz = i < inst.arg_struct_sizes.size() ? inst.arg_struct_sizes[i] : 0;
          if (ssz == 1)
            output << "(uint64_t)*(uint8_t *)(intptr_t)" << get_temp_name(inst.args[i]);
          else if (ssz == 2)
            output << "(uint64_t)*(uint16_t *)(intptr_t)" << get_temp_name(inst.args[i]);
          else if (ssz <= 4 && ssz > 0)
            output << "(uint64_t)*(uint32_t *)(intptr_t)" << get_temp_name(inst.args[i]);
          else if (ssz <= 8 && ssz > 0)
            output << "(uint64_t)*(uint64_t *)(intptr_t)" << get_temp_name(inst.args[i]);
          else if (inst.arg_types[i].kind == IR_STR)
            output << "(uint64_t)(uintptr_t)" << get_temp_name(inst.args[i]) << ".data";
          else
            output << "(uint64_t)" << get_temp_name(inst.args[i]);
          if (i + 1 < nargs)
            output << ", ";
        }
        output << "};\n";
        output << indent() << "  void *__aptrs_" << dst << "[" << nargs
               << "] = {";
        for (size_t i = 0; i < nargs; i++) {
          output << "&__avals_" << dst << "[" << i << "]";
          if (i + 1 < nargs)
            output << ", ";
        }
        output << "};\n";
      }
      output << indent() << "  uint64_t __ret_" << dst << " = 0;\n";
      output << indent() << "  ffi_prep_cif(&__cif_" << dst
             << ", FFI_DEFAULT_ABI, " << nargs << ", &ffi_type_uint64, "
             << (nargs > 0 ? ("__atypes_" + dst) : "NULL") << ");\n";
      output << indent() << "  ffi_call(&__cif_" << dst
             << ", FFI_FN((void*)(intptr_t)" << get_temp_name(inst.a)
             << "), &__ret_" << dst << ", "
             << (nargs > 0 ? ("__aptrs_" + dst) : "NULL") << ");\n";
      output << indent() << "  " << dst << " = (intptr_t)__ret_" << dst
             << ";\n";
      output << indent() << "}\n";
      break;
    }

    case IR_RET:
      output << indent() << "return " << get_temp_name(inst.a) << ";\n";
      break;

    case IR_STORE:
      if (!inst.name.empty()) {
        output << indent() << inst.name << " = " << get_temp_name(inst.a)
               << ";\n";
      } else {
        output << indent() << "*(" << lower_type_c(inst.type)
               << " *)((char *)(intptr_t)" << get_temp_name(inst.a)
               << ") = " << get_temp_name(inst.b) << ";\n";
      }
      break;

    case IR_ADDR:
      output << indent() << "intptr_t " << get_temp_name(inst.dst)
             << " = (intptr_t)&" << inst.name << ";\n";
      break;

    case IR_LOAD:
      if (!inst.name.empty()) {
        output << indent() << lower_type_c(inst.type) << ' '
               << get_temp_name(inst.dst) << " = " << inst.name << ";\n";
      } else {
        output << indent() << lower_type_c(inst.type) << ' '
               << get_temp_name(inst.dst) << " = *(" << lower_type_c(inst.type)
               << " *)((char *)(intptr_t)" << get_temp_name(inst.a) << ");\n";
      }
      break;

    default:
      break;
    }
  }

  std::string find_previous_label(const std::vector<IR_Inst> &code, int i) {
    for (int k = i; k >= 0; k--) {
      if (code[k].op == IR_LABEL)
        return code[k].name;
    }
    return "";
  }
  void emit_instructions_with_control_flow(const std::vector<IR_Inst> &code) {
    for (const auto &inst : code) {

      switch (inst.op) {

      case IR_LABEL:
        output << inst.name << ":\n";
        break;

      case IR_JMP:
        output << indent() << "goto " << inst.name << ";\n";
        break;

      case IR_JMP_IF_FALSE:
        output << indent() << "if (!" << get_temp_name(inst.a) << ") "
               << "goto " << inst.name << ";\n";
        break;

      default:
        emit_inst(inst);
        break;
      }
    }
  }

public:
  void codegen(const IR_Program &ir) {
    output << indent() << "#include <stdio.h>\n";
    output << indent() << "#include <stdint.h>\n";
    output << indent() << "#include <stdbool.h>\n";
    output << indent() << "#include <stddef.h>\n";
    output << indent() << "#include <stdlib.h>\n";
    output << indent() << "#include <string.h>\n";
    output << indent() << "#include <dlfcn.h>\n";
    output << indent() << "#include \"ffi.h\"\n";
    output << indent()
           << "typedef struct { size_t len; const char *data; } ChaosString;\n";

    for (const auto &g : ir.globals) {
      output << lower_type_c(g.type) << " " << g.name << " = 0;\n";
    }

    for (const auto &efn : ir.extern_decls) {
      output << lower_type_c(efn.return_type) << " "
             << mangle_symbol_name_c(efn.name) << "(";
      for (size_t i = 0; i < efn.params.size(); i++) {
        output << lower_type_c(efn.params[i].type);
        if (i + 1 < efn.params.size())
          output << ", ";
      }
      output << ");\n";
    }

    for (const auto &fn : ir.functions) {
      output << lower_type_c(fn.return_type) << ' '
             << mangle_symbol_name_c(fn.name) << "(";
      for (size_t i = 0; i < fn.params.size(); i++) {
        output << lower_type_c(fn.params[i].type) << ' ' << fn.params[i].name;
        if (i + 1 < fn.params.size())
          output << ", ";
      }

      output << ") {\n";
      indent_level++;

      for (const auto &local : fn.locals) {
        if (local.type.kind == IR_PTR && local.stack_bytes > 0) {
          output << indent() << "unsigned char " << local.name << "_storage["
                 << local.stack_bytes << "] = {0};\n";
          output << indent() << lower_type_c(local.type) << ' ' << local.name
                 << " = (intptr_t)" << local.name << "_storage;\n";
        } else {
          output << indent() << lower_type_c(local.type) << ' ' << local.name
                 << ";\n";
        }
      }
      emit_instructions_with_control_flow(fn.code);

      indent_level--;
      output << "}\n\n";
    }
  }

  std::string get_output() const { return output.str(); }
};
