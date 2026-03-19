#import "./chaos_semantic.h"

#include "chaos_parser.h"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

struct Lowering_Context {
  IR_Function *fn;
  std::unordered_map<std::string, IR_Type> value_types;
  static std::unordered_map<std::string, Struct_Data> named_structs;
  std::unordered_map<std::string, std::string> value_struct_types;
  std::unordered_map<std::string, std::string> value_enum_types;
  std::unordered_map<std::string, std::string> value_type_names;
  static std::unordered_map<std::string, IR_Type> named_types;
  static std::unordered_map<std::string, IR_Type> named_function_returns;
  static std::unordered_set<std::string> module_names;

  int label_counter = 0;

  std::string new_label() { return "L" + std::to_string(label_counter++); }
};

std::shared_ptr<Chaos_Type> chaos_type_from_name(const std::string &name) {
  if (name == "int" || name == "i32")
    return Chaos_Type::make_primitive(PRIM_I32);
  if (name == "i8")
    return Chaos_Type::make_primitive(PRIM_I8);
  if (name == "i16")
    return Chaos_Type::make_primitive(PRIM_I16);
  if (name == "i64")
    return Chaos_Type::make_primitive(PRIM_I64);
  if (name == "u8")
    return Chaos_Type::make_primitive(PRIM_U8);
  if (name == "u16")
    return Chaos_Type::make_primitive(PRIM_U16);
  if (name == "u32")
    return Chaos_Type::make_primitive(PRIM_U32);
  if (name == "u64")
    return Chaos_Type::make_primitive(PRIM_U64);
  if (name == "f32" || name == "float")
    return Chaos_Type::make_primitive(PRIM_F32);
  if (name == "f64")
    return Chaos_Type::make_primitive(PRIM_F64);
  if (name == "bool")
    return Chaos_Type::make_primitive(PRIM_BOOL);
  if (name == "void")
    return Chaos_Type::make_void();
  if (name == "string")
    return Chaos_Type::make_string();
  if (name == "ptr")
    return Chaos_Type::make_pointer(Chaos_Type::make_primitive(PRIM_U8));
  if (!name.empty() && name[0] == '*')
    return Chaos_Type::make_pointer(chaos_type_from_name(name.substr(1)));
  if (!name.empty() && name[0] == '[') {
    size_t close = name.find(']');
    size_t n = std::stoull(name.substr(1, close - 1));
    auto elem = chaos_type_from_name(name.substr(close + 1));
    return Chaos_Type::make_array(elem, n);
  }
  auto it = Lowering_Context::named_structs.find(name);
  if (it != Lowering_Context::named_structs.end()) {
    return Chaos_Type::make_struct(name, it->second.fields);
  }
  // probably will happen for enums only
  return Chaos_Type::make_primitive(PRIM_I32);
}

IR_Type lower_type(const Chaos_Type &t) {
  if (t.kind == Chaos_Type::TYPE_PRIMITIVE) {
    switch (t.primitive()) {
    case PRIM_I32:
      return {IR_I32};
    case PRIM_I64:
      return {IR_I64};
    case PRIM_F32:
      return {IR_F32};
    case PRIM_F64:
      return {IR_F64};
    case PRIM_BOOL:
      return {IR_BOOL};
    case PRIM_U8:
      return {IR_U8};
    case PRIM_U16:
      return {IR_U16};
    case PRIM_U32:
      return {IR_U32};
    case PRIM_U64:
      return {IR_U64};
    case PRIM_I8:
      return {IR_I8};
    case PRIM_I16:
      return {IR_I16};

    default:
      break;
    }
  }

  if (t.kind == Chaos_Type::TYPE_POINTER)
    return {IR_PTR};

  if (t.kind == Chaos_Type::TYPE_VOID)
    return {IR_VOID};

  if (t.kind == Chaos_Type::TYPE_ENUM)
    return {IR_I32};

  if (t.kind == Chaos_Type::TYPE_STRUCT)
    return {IR_PTR};

  if (t.kind == Chaos_Type::TYPE_STRING)
    return {IR_STR};

  if (t.kind == Chaos_Type::TYPE_ARRAY)
    return {IR_PTR};

  assert(false && "Unsupported type?");
  return {IR_VOID};
}

IR_Type lower_type_name(const std::string &name) {
  if (name == "int" || name == "i32")
    return {IR_I32};
  if (name == "i8")
    return {IR_I8};
  if (name == "i16")
    return {IR_I16};
  if (name == "i64")
    return {IR_I64};
  if (name == "u8")
    return {IR_U8};
  if (name == "u16")
    return {IR_U16};
  if (name == "u32")
    return {IR_U32};
  if (name == "u64")
    return {IR_U64};
  if (name == "f32" || name == "float")
    return {IR_F32};
  if (name == "f64")
    return {IR_F64};
  if (name == "bool")
    return {IR_BOOL};
  if (name == "void")
    return {IR_VOID};
  if (name == "string")
    return {IR_STR};
  if (name == "ptr")
    return {IR_PTR};
  if (!name.empty() && name[0] == '*')
    return {IR_PTR};
  if (!name.empty() && name[0] == '[')
    return {IR_PTR};

  auto it = Lowering_Context::named_types.find(name);
  if (it != Lowering_Context::named_types.end())
    return it->second;

  std::cout << "Unknown type: " << name << std::endl;
  assert(false && "Unknown type name");
  return {IR_I32};
}

std::string pointee_type_name(const std::string &name) {
  if (!name.empty() && name[0] == '*')
    return name.substr(1);
  return name;
}

std::string elem_type_name(const std::string &name) {
  if (!name.empty() && name[0] == '[') {
    size_t close = name.find(']');
    if (close != std::string::npos)
      return name.substr(close + 1);
  }
  if (!name.empty() && name[0] == '*')
    return name.substr(1);
  return name;
}

IR_Type elem_ir_type(const std::string &arr_type_name) {
  return lower_type_name(elem_type_name(arr_type_name));
}

size_t type_size_bytes(const std::string &name) {
  auto ct = chaos_type_from_name(name);
  return ct->size_bytes();
}

std::string expr_type_name(Chaos_AST *node, Lowering_Context &ctx) {
  if (!node)
    return "i32";
  if (node->kind == AST_INT)
    return "i32";
  if (node->kind == AST_FLOAT)
    return "f32";
  if (node->kind == AST_STRING)
    return "string";
  if (node->kind == AST_IDENT) {
    auto it = ctx.value_type_names.find(std::string(node->ident));
    if (it != ctx.value_type_names.end())
      return it->second;
    return "i32";
  }
  if (node->kind == AST_UNARY && node->unary.op == TOK_STAR) {
    std::string inner = expr_type_name(node->unary.expr, ctx);
    return pointee_type_name(inner);
  }
  if (node->kind == AST_INDEX) {
    std::string arr = expr_type_name(node->index_expr.array, ctx);
    return elem_type_name(arr);
  }
  if (node->kind == AST_MEMBER) {
    std::string struct_name = "";
    if (node->member.object->kind == AST_IDENT) {
      auto it =
          ctx.value_struct_types.find(std::string(node->member.object->ident));
      if (it != ctx.value_struct_types.end())
        struct_name = it->second;
    }
    if (!struct_name.empty()) {
      auto sit = Lowering_Context::named_structs.find(struct_name);
      if (sit != Lowering_Context::named_structs.end()) {
        for (auto &f : sit->second.fields) {
          if (f.name == std::string(node->member.field)) {
            return lower_type(*f.type).kind == IR_I32 ? "i32" : "i32";
          }
        }
      }
    }
    return "i32";
  }
  return "i32";
}

IR_Type get_expr_type(Chaos_AST *node, Lowering_Context &ctx) {
  if (node && node->resolved_type)
    return lower_type(*node->resolved_type);
  std::string tname = expr_type_name(node, ctx);
  return lower_type_name(tname);
}

IR_Type get_expr_type(Chaos_AST *node) {
  if (node && node->resolved_type) {
    return lower_type(*node->resolved_type);
  }
  if (node) {
    if (node->kind == AST_INT)
      return {IR_I32};
    if (node->kind == AST_FLOAT)
      return {IR_F32};
    if (node->kind == AST_STRING)
      return {IR_STR};
  }
  return {IR_I32};
}

size_t struct_field_offset(const Struct_Data &s, const std::string &name) {
  size_t offset = 0;

  for (auto &f : s.fields) {
    if (f.name == name)
      return offset;

    offset += f.type->size_bytes();
  }

  assert(false && "Unknown struct field");
  return 0;
}

size_t struct_type_size_bytes(const Struct_Data &s) {
  size_t n = 0;
  for (const auto &f : s.fields)
    n += f.type->size_bytes();
  return n;
}

const Struct_Data &member_object_struct_data(Chaos_AST *object,
                                             Lowering_Context &ctx) {
  if (object->kind == AST_IDENT) {
    auto it = ctx.value_struct_types.find(std::string(object->ident));
    if (it != ctx.value_struct_types.end()) {
      auto sit = Lowering_Context::named_structs.find(it->second);
      assert(sit != Lowering_Context::named_structs.end() &&
             "Unknown struct type in member access");
      return sit->second;
    }

    auto eit = ctx.value_type_names.find(std::string(object->ident));
    if (eit != ctx.value_type_names.end()) {
      std::string tname = eit->second;
      while (!tname.empty() && tname[0] == '*')
        tname = tname.substr(1);
      auto sit = Lowering_Context::named_structs.find(tname);
      if (sit != Lowering_Context::named_structs.end())
        return sit->second;
    }
  }
  assert(false && "Unsupported member base expression");
  return Lowering_Context::named_structs.begin()->second;
}

std::shared_ptr<Chaos_Type> struct_field_type(const Struct_Data &s,
                                              std::string_view field) {
  for (auto &f : s.fields) {
    if (f.name == field)
      return f.type;
  }

  assert(false && "Unknown struct field");
  return nullptr;
}

IR_Type promote_numeric_type(IR_Type a, IR_Type b) {
  if (a.kind == IR_F64 || b.kind == IR_F64)
    return {IR_F64};
  if (a.kind == IR_F32 || b.kind == IR_F32)
    return {IR_F32};
  if (a.kind == IR_I64 || b.kind == IR_I64)
    return {IR_I64};
  if (a.kind == IR_U64 || b.kind == IR_U64)
    return {IR_U64};
  if (a.kind == IR_U32 || b.kind == IR_U32)
    return {IR_U32};
  if (a.kind == IR_I32 || b.kind == IR_I32)
    return {IR_I32};
  return {IR_I32};
}

IR_Type lower_type_name(std::string_view name) {
  if (name == "int" || name == "i32")
    return {IR_I32};
  if (name == "i64")
    return {IR_I64};
  if (name == "f32" || name == "float")
    return {IR_F32};
  if (name == "f64")
    return {IR_F64};
  if (name == "bool")
    return {IR_BOOL};
  if (name == "void")
    return {IR_VOID};
  if (name == "string")
    return {IR_STR};

  auto it = Lowering_Context::named_types.find(std::string(name));
  if (it != Lowering_Context::named_types.end())
    return it->second;

  std::cout << name << std::endl;
  assert(false && "Unknown type name");
  return {IR_I32};
}

std::string struct_type_name_for_expr(Chaos_AST *expr, Lowering_Context &ctx) {
  if (expr->kind == AST_IDENT) {
    auto it = ctx.value_struct_types.find(std::string(expr->ident));
    if (it != ctx.value_struct_types.end())
      return it->second;

    auto tit = ctx.value_type_names.find(std::string(expr->ident));
    if (tit != ctx.value_type_names.end()) {
      std::string tname = tit->second;
      while (!tname.empty() && tname[0] == '*')
        tname = tname.substr(1);
      if (Lowering_Context::named_structs.count(tname))
        return tname;
    }
  }
  return "";
}

std::string enum_type_name_for_expr(Chaos_AST *expr, Lowering_Context &ctx) {
  if (expr->kind == AST_IDENT) {
    auto it = ctx.value_enum_types.find(std::string(expr->ident));
    if (it != ctx.value_enum_types.end())
      return it->second;
  }
  return "";
}

IR_Value lower_lvalue_addr(Chaos_AST *node, Lowering_Context &ctx);

IR_Value lower_expr(Chaos_AST *node, Lowering_Context &ctx) {
  if (node->kind == AST_INT) {
    IR_Value t = ctx.fn->new_temp({IR_I32});

    IR_Inst inst{};
    inst.op = IR_CONST_INT;
    inst.dst = t;
    inst.int_value = std::stoll(std::string(node->literal));
    inst.type = {IR_I32};
    ctx.fn->code.push_back(inst);

    return t;
  }
  if (node->kind == AST_MEMBER) {
    IR_Value base = lower_expr(node->member.object, ctx);

    const Struct_Data &struct_data =
        member_object_struct_data(node->member.object, ctx);

    size_t offset =
        struct_field_offset(struct_data, std::string(node->member.field));

    IR_Value off = ctx.fn->new_temp({IR_I32});

    IR_Inst c{};
    c.op = IR_CONST_INT;
    c.dst = off;
    c.int_value = offset;
    c.type = {IR_I32};
    ctx.fn->code.push_back(c);

    IR_Value addr = ctx.fn->new_temp({IR_PTR});

    IR_Inst add{};
    add.op = IR_ADD;
    add.dst = addr;
    add.a = base;
    add.b = off;
    add.type = {IR_PTR};
    ctx.fn->code.push_back(add);

    auto member_type = struct_field_type(struct_data, node->member.field);
    IR_Type field_type = lower_type(*member_type);

    IR_Value result = ctx.fn->new_temp(field_type);

    IR_Inst load{};
    load.op = IR_LOAD;
    load.dst = result;
    load.a = addr;
    load.type = field_type;

    ctx.fn->code.push_back(load);

    return result;
  }

  if (node->kind == AST_UNARY) {
    if (node->unary.op == TOK_MINUS) {
      IR_Value val = lower_expr(node->unary.expr, ctx);
      IR_Type vtype = ctx.fn->temp_types[val];
      IR_Value t = ctx.fn->new_temp(vtype);
      IR_Inst inst{};
      inst.op = IR_NEG;
      inst.dst = t;
      inst.a = val;
      inst.type = vtype;
      ctx.fn->code.push_back(inst);
      return t;
    }

    // dereference
    if (node->unary.op == TOK_STAR) {
      IR_Value ptr = lower_expr(node->unary.expr, ctx);
      std::string inner_tname = expr_type_name(node->unary.expr, ctx);
      std::string pointee_tname = pointee_type_name(inner_tname);
      IR_Type result_type = lower_type_name(pointee_tname);

      IR_Value t = ctx.fn->new_temp(result_type);
      IR_Inst load{};
      load.op = IR_LOAD;
      load.dst = t;
      load.a = ptr;
      load.type = result_type;
      ctx.fn->code.push_back(load);
      return t;
    }

    // addresss of
    if (node->unary.op == TOK_AMP) {
      return lower_lvalue_addr(node->unary.expr, ctx);
    }
  }
  if (node->kind == AST_INDEX) {
    IR_Value base = lower_expr(node->index_expr.array, ctx);
    IR_Value idx = lower_expr(node->index_expr.index, ctx);

    std::string arr_tname = expr_type_name(node->index_expr.array, ctx);
    std::string elem_tname = elem_type_name(arr_tname);
    IR_Type elem_type = lower_type_name(elem_tname);
    size_t elem_sz = type_size_bytes(elem_tname);

    IR_Value sz_val = ctx.fn->new_temp({IR_I32});
    IR_Inst sz_inst{};
    sz_inst.op = IR_CONST_INT;
    sz_inst.dst = sz_val;
    sz_inst.int_value = (int64_t)elem_sz;
    sz_inst.type = {IR_I32};
    ctx.fn->code.push_back(sz_inst);

    IR_Value offset = ctx.fn->new_temp({IR_I32});
    IR_Inst mul{};
    mul.op = IR_MUL;
    mul.dst = offset;
    mul.a = idx;
    mul.b = sz_val;
    mul.type = {IR_I32};
    ctx.fn->code.push_back(mul);

    IR_Value addr = ctx.fn->new_temp({IR_PTR});
    IR_Inst add{};
    add.op = IR_ADD;
    add.dst = addr;
    add.a = base;
    add.b = offset;
    add.type = {IR_PTR};
    ctx.fn->code.push_back(add);

    IR_Value result = ctx.fn->new_temp(elem_type);
    IR_Inst load{};
    load.op = IR_LOAD;
    load.dst = result;
    load.a = addr;
    load.type = elem_type;
    ctx.fn->code.push_back(load);
    return result;
  }

  if (node->kind == AST_STRING) {
    IR_Value t = ctx.fn->new_temp({IR_STR});

    IR_Inst inst{};
    inst.op = IR_CONST_STRING;
    inst.dst = t;
    inst.string_value = std::string(node->literal);
    inst.type = {IR_STR};
    ctx.fn->code.push_back(inst);

    return t;
  }
  if (node->kind == AST_FLOAT) {
    IR_Value t = ctx.fn->new_temp({IR_F32});

    IR_Inst inst{};
    inst.op = IR_CONST_FLOAT;
    inst.dst = t;
    inst.float_value = std::stof(std::string(node->literal));
    inst.type = {IR_F32};
    ctx.fn->code.push_back(inst);

    return t;
  }

  if (node->kind == AST_INTRINSIC) {
    const std::string &iname = node->intrinsic.name;

    if (iname == "alloc") {
      IR_Value count = lower_expr(node->intrinsic.args[0], ctx);
      size_t elem_sz = type_size_bytes(node->intrinsic.type_arg);

      IR_Value t = ctx.fn->new_temp({IR_PTR});
      IR_Inst inst{};
      inst.op = IR_INTRINSIC_ALLOC;
      inst.dst = t;
      inst.a = count;
      inst.int_value = (int64_t)elem_sz;
      inst.name = node->intrinsic.type_arg;
      inst.type = {IR_PTR};
      ctx.fn->code.push_back(inst);
      return t;
    }

    if (iname == "free") {
      IR_Value ptr = lower_expr(node->intrinsic.args[0], ctx);
      IR_Value t = ctx.fn->new_temp({IR_VOID});
      IR_Inst inst{};
      inst.op = IR_INTRINSIC_FREE;
      inst.dst = t;
      inst.a = ptr;
      inst.type = {IR_VOID};
      ctx.fn->code.push_back(inst);
      return t;
    }

    if (iname == "memcpy") {
      IR_Value dst = lower_expr(node->intrinsic.args[0], ctx);
      IR_Value src = lower_expr(node->intrinsic.args[1], ctx);
      IR_Value size = lower_expr(node->intrinsic.args[2], ctx);
      IR_Value t = ctx.fn->new_temp({IR_VOID});
      IR_Inst inst{};
      inst.op = IR_INTRINSIC_MEMCPY;
      inst.dst = t;
      inst.a = dst;
      inst.b = src;
      inst.args = {size};
      inst.type = {IR_VOID};
      ctx.fn->code.push_back(inst);
      return t;
    }

    if (iname == "cast") {
      IR_Value val = lower_expr(node->intrinsic.args[0], ctx);
      IR_Type target_type = lower_type_name(node->intrinsic.type_arg);
      IR_Value t = ctx.fn->new_temp(target_type);
      IR_Inst inst{};
      inst.op = IR_INTRINSIC_CAST;
      inst.dst = t;
      inst.a = val;
      inst.name = node->intrinsic.type_arg;
      inst.type = target_type;
      ctx.fn->code.push_back(inst);
      return t;
    }
    std::fprintf(stderr, "Unknown intrinsic: @%s\n", iname.c_str());
    return ctx.fn->new_temp({IR_VOID});
  }

  if (node->kind == AST_CALL) {
    std::vector<IR_Value> arg_values;
    std::string fn_name;

    if (node->call.caller->kind == AST_IDENT) {
      fn_name = std::string(node->call.caller->ident);
      for (Chaos_AST *arg : node->call.args)
        arg_values.push_back(lower_expr(arg, ctx));
    } else if (node->call.caller->kind == AST_MEMBER) {
      Chaos_AST *member = node->call.caller;

      if (member->member.object->kind == AST_IDENT &&
          Lowering_Context::module_names.find(
              std::string(member->member.object->ident)) !=
              Lowering_Context::module_names.end()) {
        fn_name = std::string(member->member.object->ident) + "." +
                  std::string(member->member.field);
        for (Chaos_AST *arg : node->call.args)
          arg_values.push_back(lower_expr(arg, ctx));
      } else {
        std::string owner =
            struct_type_name_for_expr(member->member.object, ctx);
        if (owner.empty())
          owner = enum_type_name_for_expr(member->member.object, ctx);

        if (owner.empty()) {
          if (member->member.object->kind == AST_IDENT) {
            auto eit = ctx.value_type_names.find(
                std::string(member->member.object->ident));
            if (eit != ctx.value_type_names.end()) {
              std::string tname = eit->second;
              while (!tname.empty() && tname[0] == '*')
                tname = tname.substr(1);
              owner = tname;
            }
          }
          if (owner.empty())
            owner = "i32";
        }

        fn_name = owner + "." + std::string(member->member.field);
        arg_values.push_back(lower_expr(member->member.object, ctx));
        for (Chaos_AST *arg : node->call.args)
          arg_values.push_back(lower_expr(arg, ctx));
      }
    } else {
      return ctx.fn->new_temp({IR_I32});
    }

    IR_Inst inst{};
    inst.args = arg_values;

    if (fn_name == "print") {
      std::vector<IR_Type> arg_types;
      for (IR_Value val : arg_values)
        arg_types.push_back(ctx.fn->temp_types[val]);

      IR_Value t = ctx.fn->new_temp({IR_VOID});
      IR_Inst pinst{};
      pinst.op = IR_INTRINSIC_PRINT;
      pinst.args = arg_values;
      pinst.arg_types = arg_types;
      pinst.dst = t;
      pinst.type = {IR_VOID};
      ctx.fn->code.push_back(pinst);
      return t;
    }

    IR_Type call_type = {IR_I32};
    auto fit = Lowering_Context::named_function_returns.find(fn_name);
    if (fit != Lowering_Context::named_function_returns.end()) {
      call_type = fit->second;
    } else if (node->resolved_type) {
      call_type = lower_type(*node->resolved_type);
    }

    IR_Value t = ctx.fn->new_temp(call_type);
    inst.op = IR_CALL;
    inst.dst = t;
    inst.name = fn_name;
    inst.type = call_type;
    ctx.fn->code.push_back(inst);
    return t;
  }

  if (node->kind == AST_IDENT) {
    IR_Type value_type = {IR_I32};
    auto it = ctx.value_types.find(std::string(node->ident));
    if (it != ctx.value_types.end()) {
      value_type = it->second;
    }

    IR_Value t = ctx.fn->new_temp(value_type);

    IR_Inst inst{};
    inst.op = IR_LOAD;
    inst.dst = t;
    inst.name = node->ident;
    inst.type = value_type;

    ctx.fn->code.push_back(inst);

    return t;
  }
  if (node->kind == AST_BINARY) {
    IR_Value left = lower_expr(node->binary.l, ctx);
    IR_Value right = lower_expr(node->binary.r, ctx);

    IR_Type left_type = ctx.fn->temp_types[left];
    IR_Type right_type = ctx.fn->temp_types[right];
    IR_Type result_type = promote_numeric_type(left_type, right_type);

    if (node->binary.op == TOK_LT || node->binary.op == TOK_GT ||
        node->binary.op == TOK_EQEQ || node->binary.op == TOK_NOT) {
      result_type = {IR_BOOL};
    }

    if (left_type.kind == IR_PTR || right_type.kind == IR_PTR)
      result_type = {IR_PTR};

    IR_Value t = ctx.fn->new_temp(result_type);
    IR_Inst inst{};
    inst.dst = t;
    inst.a = left;
    inst.b = right;
    inst.type = result_type;

    switch (node->binary.op) {
    case TOK_PLUS:
      inst.op = IR_ADD;
      break;
    case TOK_MINUS:
      inst.op = IR_SUB;
      break;
    case TOK_STAR:
      inst.op = IR_MUL;
      break;
    case TOK_SLASH:
      inst.op = IR_DIV;
      break;
    case TOK_LT:
      inst.op = IR_CMP_LT;
      break;
    case TOK_GT:
      inst.op = IR_CMP_GT;
      break;
    case TOK_EQEQ:
      inst.op = IR_CMP_EQ;
      break;
    default:
      assert(false && "Unknown binary op");
    }

    ctx.fn->code.push_back(inst);
    return t;
  }

  assert(false && "lower_expr: unhandled AST node kind");
  return 0;
}

IR_Value lower_lvalue_addr(Chaos_AST *node, Lowering_Context &ctx) {
  if (node->kind == AST_IDENT) {
    IR_Value t = ctx.fn->new_temp({IR_PTR});
    IR_Inst inst{};
    inst.op = IR_ADDR;
    inst.dst = t;
    inst.name = std::string(node->ident);
    inst.type = {IR_PTR};
    ctx.fn->code.push_back(inst);
    return t;
  }
  if (node->kind == AST_UNARY && node->unary.op == TOK_STAR) {
    return lower_expr(node->unary.expr, ctx);
  }
  if (node->kind == AST_INDEX) {
    IR_Value base = lower_expr(node->index_expr.array, ctx);
    IR_Value idx = lower_expr(node->index_expr.index, ctx);
    std::string arr_tname = expr_type_name(node->index_expr.array, ctx);
    size_t elem_sz = type_size_bytes(elem_type_name(arr_tname));

    IR_Value sz_val = ctx.fn->new_temp({IR_I32});
    IR_Inst sz_inst{};
    sz_inst.op = IR_CONST_INT;
    sz_inst.dst = sz_val;
    sz_inst.int_value = (int64_t)elem_sz;
    sz_inst.type = {IR_I32};
    ctx.fn->code.push_back(sz_inst);

    IR_Value offset = ctx.fn->new_temp({IR_I32});
    IR_Inst mul{};
    mul.op = IR_MUL;
    mul.dst = offset;
    mul.a = idx;
    mul.b = sz_val;
    mul.type = {IR_I32};
    ctx.fn->code.push_back(mul);

    IR_Value addr = ctx.fn->new_temp({IR_PTR});
    IR_Inst add{};
    add.op = IR_ADD;
    add.dst = addr;
    add.a = base;
    add.b = offset;
    add.type = {IR_PTR};
    ctx.fn->code.push_back(add);
    return addr;
  }
  if (node->kind == AST_MEMBER) {
    IR_Value base = lower_expr(node->member.object, ctx);
    const Struct_Data &s = member_object_struct_data(node->member.object, ctx);
    size_t offset = struct_field_offset(s, std::string(node->member.field));

    IR_Value off = ctx.fn->new_temp({IR_I32});
    IR_Inst c{};
    c.op = IR_CONST_INT;
    c.dst = off;
    c.int_value = (int64_t)offset;
    c.type = {IR_I32};
    ctx.fn->code.push_back(c);

    IR_Value addr = ctx.fn->new_temp({IR_PTR});
    IR_Inst add{};
    add.op = IR_ADD;
    add.dst = addr;
    add.a = base;
    add.b = off;
    add.type = {IR_PTR};
    ctx.fn->code.push_back(add);
    return addr;
  }
  assert(false && "lower_lvalue_addr: not an lvalue");
  return 0;
}

void lower_stmt(Chaos_AST *node, Lowering_Context &ctx) {
  if (node->kind == AST_RETURN) {
    IR_Value val = lower_expr(node->unary.expr, ctx);
    IR_Inst inst{};
    inst.op = IR_RET;
    inst.a = val;
    ctx.fn->code.push_back(inst);
    return;
  }

  if (node->kind == AST_ASIGN) {
    Chaos_AST *target = node->assign.target;

    if (target->kind == AST_MEMBER) {
      IR_Value addr = lower_lvalue_addr(target, ctx);
      IR_Value value = lower_expr(node->assign.value, ctx);
      IR_Inst store{};
      store.op = IR_STORE;
      store.a = addr;
      store.b = value;
      store.type = ctx.fn->temp_types[value];
      ctx.fn->code.push_back(store);
      return;
    }

    if (target->kind == AST_INDEX) {
      IR_Value addr = lower_lvalue_addr(target, ctx);
      IR_Value value = lower_expr(node->assign.value, ctx);
      IR_Inst store{};
      store.op = IR_STORE;
      store.a = addr;
      store.b = value;
      store.type = ctx.fn->temp_types[value];
      ctx.fn->code.push_back(store);
      return;
    }

    if (target->kind == AST_UNARY && target->unary.op == TOK_STAR) {
      IR_Value ptr = lower_expr(target->unary.expr, ctx);
      IR_Value value = lower_expr(node->assign.value, ctx);
      IR_Inst store{};
      store.op = IR_STORE;
      store.a = ptr;
      store.b = value;
      store.type = ctx.fn->temp_types[value];
      ctx.fn->code.push_back(store);
      return;
    }

    IR_Value value = lower_expr(node->assign.value, ctx);
    IR_Inst inst{};
    inst.op = IR_STORE;
    inst.name = std::string(target->ident);
    inst.a = value;
    ctx.fn->code.push_back(inst);
    return;
  }

  if (node->kind == AST_IF) {
    std::string else_label = ctx.new_label();
    std::string end_label = ctx.new_label();

    IR_Value cond = lower_expr(node->if_stmt.cond, ctx);

    IR_Inst jmp{};
    jmp.op = IR_JMP_IF_FALSE;
    jmp.a = cond;
    jmp.name = else_label;
    ctx.fn->code.push_back(jmp);

    lower_stmt(node->if_stmt.then_br, ctx);

    IR_Inst jmp_end{};
    jmp_end.op = IR_JMP;
    jmp_end.name = end_label;
    ctx.fn->code.push_back(jmp_end);

    ctx.fn->code.push_back({IR_LABEL, {}, 0, 0, 0, else_label});

    if (node->if_stmt.else_br)
      lower_stmt(node->if_stmt.else_br, ctx);

    ctx.fn->code.push_back({IR_LABEL, {}, 0, 0, 0, end_label});
    return;
  }

  if (node->kind == AST_WHILE) {
    std::string loop_start = ctx.new_label();
    std::string loop_end = ctx.new_label();

    ctx.fn->code.push_back({IR_LABEL, {}, 0, 0, 0, loop_start});

    IR_Value cond = lower_expr(node->while_stmt.cond, ctx);

    IR_Inst jmp{};
    jmp.op = IR_JMP_IF_FALSE;
    jmp.a = cond;
    jmp.name = loop_end;
    ctx.fn->code.push_back(jmp);

    lower_stmt(node->while_stmt.body, ctx);

    IR_Inst jmp_back{};
    jmp_back.op = IR_JMP;
    jmp_back.name = loop_start;
    ctx.fn->code.push_back(jmp_back);

    ctx.fn->code.push_back({IR_LABEL, {}, 0, 0, 0, loop_end});
    return;
  }

  if (node->kind == AST_VAR_DECL) {
    IR_Local local;
    local.name = std::string(node->var_decl.name);
    local.type = lower_type_name(node->var_decl.type);

    ctx.value_type_names[local.name] = node->var_decl.type;

    const std::string &type_str = node->var_decl.type;

    if (!type_str.empty() && type_str[0] == '[') {
      auto ct = chaos_type_from_name(type_str);
      local.stack_bytes = ct->size_bytes();
    } else {
      auto struct_it = Lowering_Context::named_structs.find(type_str);
      if (struct_it != Lowering_Context::named_structs.end()) {
        ctx.value_struct_types[local.name] = type_str;
        local.stack_bytes = struct_type_size_bytes(struct_it->second);
      } else if (!type_str.empty() && type_str[0] == '*') {
        std::string pointee = type_str.substr(1);
        auto sit = Lowering_Context::named_structs.find(pointee);
        if (sit != Lowering_Context::named_structs.end())
          ctx.value_struct_types[local.name] = pointee;
      } else if (Lowering_Context::named_types.find(type_str) !=
                     Lowering_Context::named_types.end() &&
                 Lowering_Context::named_types[type_str].kind == IR_I32) {
        ctx.value_enum_types[local.name] = type_str;
      }
    }

    ctx.fn->locals.push_back(local);
    ctx.value_types[local.name] = local.type;

    if (node->var_decl.init) {
      IR_Value val = lower_expr(node->var_decl.init, ctx);
      IR_Inst store{};
      store.op = IR_STORE;
      store.name = node->var_decl.name;
      store.a = val;
      ctx.fn->code.push_back(store);
    }
    return;
  }

  if (node->kind == AST_BLOCK) {
    for (Chaos_AST *stmt : node->block.statements)
      lower_stmt(stmt, ctx);
    return;
  }

  if (node->kind == AST_IMPORT || node->kind == AST_MOD_DECL)
    return;

  if (node->kind == AST_CALL || node->kind == AST_BINARY ||
      node->kind == AST_UNARY || node->kind == AST_IDENT ||
      node->kind == AST_INDEX || node->kind == AST_INTRINSIC) {
    lower_expr(node, ctx);
    return;
  }
}

IR_Function lower_function(Chaos_AST *fn_node) {
  IR_Function fn;

  fn.name = fn_node->function.owner.empty()
                ? std::string(fn_node->function.name)
                : std::string(fn_node->function.owner) + "." +
                      std::string(fn_node->function.name);
  fn.return_type = lower_type_name(fn_node->function.return_type);

  for (auto &p : fn_node->function.params) {
    IR_Local local;
    local.name = std::string(p.first);
    local.type = lower_type_name(p.second);
    fn.params.push_back(local);
  }

  Lowering_Context ctx;
  ctx.fn = &fn;

  for (const auto &param : fn.params)
    ctx.value_types[param.name] = param.type;

  for (const auto &param : fn_node->function.params) {
    std::string pname = std::string(param.first);
    std::string ptype = param.second;
    ctx.value_type_names[pname] = ptype;

    if (Lowering_Context::named_structs.count(ptype)) {
      ctx.value_struct_types[pname] = ptype;
    } else if (!ptype.empty() && ptype[0] == '*') {
      std::string pointee = ptype.substr(1);
      if (Lowering_Context::named_structs.count(pointee))
        ctx.value_struct_types[pname] = pointee;
    } else if (Lowering_Context::named_types.count(ptype) &&
               Lowering_Context::named_types[ptype].kind == IR_I32) {
      ctx.value_enum_types[pname] = ptype;
    }
  }

  lower_stmt(fn_node->function.body, ctx);

  return fn;
}

std::unordered_map<std::string, IR_Type> Lowering_Context::named_types;
std::unordered_map<std::string, Struct_Data> Lowering_Context::named_structs;
std::unordered_map<std::string, IR_Type>
    Lowering_Context::named_function_returns;
std::unordered_set<std::string> Lowering_Context::module_names;

IR_Program lower_program(Chaos_AST *program) {
  IR_Program ir;

  Lowering_Context::module_names.clear();

  for (auto *stmt : program->block.statements) {
    if (stmt->kind == AST_MOD_DECL) {
      Lowering_Context::module_names.insert(std::string(stmt->mod_decl.name));
      continue;
    }

    if (stmt->kind == AST_STRUCT) {
      Struct_Data s;
      s.name = std::string(stmt->struct_decl.name);
      for (const auto &field : stmt->struct_decl.fields) {
        Struct_Field f;
        f.name = std::string(field.first);
        f.type = chaos_type_from_name(field.second);
        s.fields.push_back(std::move(f));
      }
      Lowering_Context::named_structs[s.name] = std::move(s);
      Lowering_Context::named_types[std::string(stmt->struct_decl.name)] = {
          IR_PTR};
      continue;
    }

    if (stmt->kind == AST_ENUM) {
      Lowering_Context::named_types[std::string(stmt->enum_decl.name)] = {
          IR_I32};
      continue;
    }

    if (stmt->kind == AST_FUNCTION) {
      std::string fn_name = stmt->function.owner.empty()
                                ? std::string(stmt->function.name)
                                : std::string(stmt->function.owner) + "." +
                                      std::string(stmt->function.name);
      Lowering_Context::named_function_returns[fn_name] =
          lower_type_name(stmt->function.return_type);
    }
  }

  for (auto *stmt : program->block.statements) {
    if (stmt->kind != AST_FUNCTION)
      continue;
    if (stmt->function.is_extern) {
      IR_Function efn;
      efn.is_extern = true;
      efn.name = std::string(stmt->function.name);
      efn.return_type = lower_type_name(stmt->function.return_type);
      for (auto &param : stmt->function.params) {
        IR_Local loc;
        loc.name = std::string(param.first);
        loc.type = lower_type_name(param.second);
        efn.params.push_back(loc);
      }
      ir.extern_decls.push_back(std::move(efn));
    } else {
      ir.functions.push_back(lower_function(stmt));
    }
  }

  return ir;
}
