#import "./chaos_lexer.h"

#include <cstdio>
#include <memory>

typedef enum {
  AST_INT,
  AST_FLOAT,
  AST_STRING,

  AST_BINARY,
  AST_UNARY,
  AST_CALL,
  AST_INDEX,

  AST_VAR_DECL,
  AST_BLOCK,
  AST_IF,
  AST_WHILE,
  AST_RETURN,
  AST_FUNCTION,
  AST_STRUCT,
  AST_ENUM,
  AST_IDENT,
  AST_ASIGN,
  AST_MEMBER,
  AST_IMPORT,
  AST_MOD_DECL,

  AST_PROGRAM
} Chaos_AST_Kind;

struct Chaos_Type;

typedef struct Chaos_AST {
  Chaos_AST_Kind kind;

  std::shared_ptr<Chaos_Type> resolved_type;

  std::string_view literal;
  std::string_view ident;

  struct {
    Chaos_Token_Kind op;
    Chaos_AST *l;
    Chaos_AST *r;
  } binary;

  struct {
    Chaos_Token_Kind op;
    Chaos_AST *expr;
  } unary;

  struct {
    std::vector<Chaos_AST *> statements;
  } block;

  struct {
    Chaos_AST *object;
    std::string_view field;
  } member;

  struct {
    Chaos_AST *cond;
    Chaos_AST *then_br;
    Chaos_AST *else_br;
  } if_stmt;

  struct {
    Chaos_AST *cond;
    Chaos_AST *body;
  } while_stmt;

  struct {
    std::string_view name;
    std::vector<std::pair<std::string_view, std::string_view>> params;
    std::string_view owner;
    std::string_view return_type;
    Chaos_AST *body;
  } function;

  struct {
    Chaos_AST *caller;
    std::vector<Chaos_AST *> args;
  } call;

  struct {
    std::string_view name;
    std::string_view type;
    Chaos_AST *init;
  } var_decl;

  struct {
    Chaos_AST *target;
    Chaos_AST *value;
  } assign;

  struct {
    std::string_view name;
    std::vector<std::pair<std::string_view, std::string_view>> fields;
  } struct_decl;

  struct {
    std::string_view name;
    std::vector<std::string_view> items;
  } enum_decl;

  struct {
    std::string_view name;
  } import_decl;

  struct {
    std::string_view name;
  } mod_decl;

  Chaos_AST()
      : kind(AST_PROGRAM), literal(), ident(),
        binary{TOK_INT, nullptr, nullptr}, unary{TOK_INT, nullptr}, block(),
        if_stmt{nullptr, nullptr, nullptr}, while_stmt{nullptr, nullptr},
        function{std::string_view{},
                 {},
                 std::string_view{},
                 std::string_view{},
                 nullptr},
        call{nullptr, {}},
        var_decl{std::string_view{}, std::string_view{}, nullptr},
        assign{nullptr, nullptr}, enum_decl{std::string_view{}, {}},
        struct_decl{std::string_view{}, {}},
        member{nullptr, std::string_view{}}, import_decl{std::string_view{}},
        mod_decl{std::string_view{}} {}
} Chaos_AST;

typedef struct Chaos_Parser {
  Chaos_Tokens *tokens;
  size_t pos;

  std::string_view current_module;

  Chaos_Parser(Chaos_Tokens *tokens = NULL, size_t pos = 0)
      : tokens(tokens), pos(pos), current_module(std::string_view{}) {}

  Chaos_Token *peek() { return &tokens->items[pos]; }
  Chaos_Token *advance() { return &tokens->items[pos++]; }

  bool match(Chaos_Token_Kind kind) {
    if (peek()->kind == kind) {
      advance();
      return true;
    }
    return false;
  }
} Chaos_Parser;

inline Chaos_AST *chaos_make_ast(Chaos_AST_Kind kind) {
  Chaos_AST *node = new Chaos_AST();
  node->kind = kind;
  return node;
}

inline bool chaos_expect(Chaos_Parser *p, Chaos_Token_Kind kind,
                         const char *message) {
  if (p->match(kind))
    return true;
  std::fprintf(stderr, "%s\n", message);
  return false;
}
