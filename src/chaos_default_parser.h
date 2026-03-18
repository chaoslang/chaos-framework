#import "chaos_parser.h"

inline Chaos_AST *chaos_parse_expression(Chaos_Parser *p);
inline Chaos_AST *chaos_parse_statement(Chaos_Parser *p);

inline Chaos_AST *chaos_parse_primary(Chaos_Parser *p) {
  Chaos_Token *tok = p->peek();

  if (p->match(TOK_INT)) {
    Chaos_AST *node = chaos_make_ast(AST_INT);
    node->literal = tok->text;
    return node;
  }
  if (p->match(TOK_FLOAT)) {
    Chaos_AST *node = chaos_make_ast(AST_FLOAT);
    node->literal = tok->text;
    return node;
  }
  if (p->match(TOK_STRING)) {
    Chaos_AST *node = chaos_make_ast(AST_STRING);
    node->literal = tok->text;
    return node;
  }
  if (p->match(TOK_IDENT)) {
    Chaos_AST *node = chaos_make_ast(AST_IDENT);
    node->ident = tok->text;
    return node;
  }

  if (p->match(TOK_LPAREN)) {
    Chaos_AST *expr = parse_expression(p);
    chaos_expect(p, TOK_RPAREN, "Expected closing ')' ");
    return expr;
  }

  return nullptr;
}

inline Chaos_AST *chaos_parse_postfix(Chaos_Parser *p, Chaos_AST *left) {
  if (!left)
    return nullptr;

  while (true) {
    if (p->peek()->kind == TOK_LPAREN) {
      p->advance();

      Chaos_AST *call_node = chaos_make_ast(AST_CALL);
      call_node->call.caller = left;

      if (p->peek()->kind != TOK_RPAREN) {
        call_node->call.args.push_back(parse_expression(p));
        while (p->match(TOK_COMMA))
          call_node->call.args.push_back(parse_expression(p));
      }

      chaos_expect(p, TOK_RPAREN, "Expected ')' after function arguments");
      left = call_node;
      continue;
    }

    if (p->match(TOK_DOT)) {
      if (p->peek()->kind != TOK_IDENT) {
        std::fprintf(stderr, "Expected field name after '.'\n");
        return left;
      }

      Chaos_AST *node = chaos_make_ast(AST_MEMBER);
      node->member.object = left;
      node->member.field = p->advance()->text;
      left = node;
      continue;
    }

    break;
  }

  return left;
}

inline Chaos_AST *chaos_parse_factor(Chaos_Parser *p) {
  if (p->peek()->kind == TOK_MINUS) {
    Chaos_Token *tok = p->advance();
    Chaos_AST *node = chaos_make_ast(AST_UNARY);
    node->unary.op = tok->kind;
    node->unary.expr = parse_factor(p);
    return node;
  }

  Chaos_AST *primary = parse_primary(p);
  return parse_postfix(p, primary);
}

inline Chaos_AST *chaos_parse_multiplicative(Chaos_Parser *p) {
  Chaos_AST *left = parse_factor(p);

  while (p->peek()->kind == TOK_STAR || p->peek()->kind == TOK_SLASH) {
    Chaos_Token_Kind op = p->advance()->kind;
    Chaos_AST *right = parse_factor(p);

    Chaos_AST *node = chaos_make_ast(AST_BINARY);
    node->binary.l = left;
    node->binary.op = op;
    node->binary.r = right;
    left = node;
  }

  return left;
}

inline Chaos_AST *chaos_parse_additive(Chaos_Parser *p) {
  Chaos_AST *left = parse_multiplicative(p);

  while (p->peek()->kind == TOK_PLUS || p->peek()->kind == TOK_MINUS) {
    Chaos_Token_Kind op = p->advance()->kind;
    Chaos_AST *right = parse_multiplicative(p);

    Chaos_AST *node = chaos_make_ast(AST_BINARY);
    node->binary.l = left;
    node->binary.op = op;
    node->binary.r = right;
    left = node;
  }

  return left;
}

inline Chaos_AST *chaos_parse_comparison(Chaos_Parser *p) {
  Chaos_AST *left = parse_additive(p);

  while (p->peek()->kind == TOK_GT || p->peek()->kind == TOK_LT ||
         p->peek()->kind == TOK_EQEQ || p->peek()->kind == TOK_NOT) {
    Chaos_Token_Kind op = p->advance()->kind;
    Chaos_AST *right = parse_additive(p);

    Chaos_AST *node = chaos_make_ast(AST_BINARY);
    node->binary.l = left;
    node->binary.op = op;
    node->binary.r = right;
    left = node;
  }

  return left;
}

inline Chaos_AST *chaos_parse_assignment(Chaos_Parser *p) {
  Chaos_AST *left = parse_comparison(p);

  if (p->match(TOK_EQUAL)) {
    Chaos_AST *value = parse_assignment(p);
    Chaos_AST *node = chaos_make_ast(AST_ASIGN);
    node->assign.target = left;
    node->assign.value = value;
    return node;
  }

  return left;
}

inline Chaos_AST *chaos_parse_expression(Chaos_Parser *p) {
  return parse_assignment(p);
}

inline Chaos_AST *chaos_parse_var_decl(Chaos_Parser *p) {
  if (!p->match(TOK_VAR))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT) {
    std::fprintf(stderr, "Expected variable name\n");
    return nullptr;
  }

  std::string_view name = p->advance()->text;

  if (!chaos_expect(p, TOK_COLON, "Expected ':' after variable name"))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT) {
    std::fprintf(stderr, "Expected type name\n");
    return nullptr;
  }

  std::string_view type = p->advance()->text;
  Chaos_AST *init = p->match(TOK_EQUAL) ? parse_expression(p) : nullptr;

  chaos_expect(p, TOK_SEMI, "Expected ';' after variable declaration");

  Chaos_AST *node = chaos_make_ast(AST_VAR_DECL);
  node->var_decl.name = name;
  node->var_decl.type = type;
  node->var_decl.init = init;
  return node;
}

inline Chaos_AST *chaos_parse_block(Chaos_Parser *p) {
  Chaos_AST *block = chaos_make_ast(AST_BLOCK);
  while (p->peek()->kind != TOK_RCURLY && p->peek()->kind != TOK_EOF)
    block->block.statements.push_back(parse_statement(p));
  chaos_expect(p, TOK_RCURLY, "Expected '}' after block");
  return block;
}

inline Chaos_AST *chaos_parse_if(Chaos_Parser *p) {
  if (!p->match(TOK_IF))
    return nullptr;

  if (!chaos_expect(p, TOK_LPAREN, "Expected: '('") )
    return nullptr;

  Chaos_AST *cond = parse_expression(p);
  if (!chaos_expect(p, TOK_RPAREN, "Expected ')'") )
    return nullptr;

  Chaos_AST *then_branch = p->match(TOK_LCURLY) ? parse_block(p) : parse_statement(p);
  Chaos_AST *else_branch = nullptr;

  if (p->match(TOK_ELSE))
    else_branch = p->match(TOK_LCURLY) ? parse_block(p) : parse_statement(p);

  Chaos_AST *node = chaos_make_ast(AST_IF);
  node->if_stmt.cond = cond;
  node->if_stmt.then_br = then_branch;
  node->if_stmt.else_br = else_branch;
  return node;
}

inline Chaos_AST *chaos_parse_return(Chaos_Parser *p) {
  if (!p->match(TOK_RETURN))
    return nullptr;

  Chaos_AST *node = chaos_make_ast(AST_RETURN);
  node->unary.expr = parse_expression(p);
  chaos_expect(p, TOK_SEMI, "Expected ';' after return");
  return node;
}

inline Chaos_AST *chaos_parse_while(Chaos_Parser *p) {
  if (!p->match(TOK_WHILE))
    return nullptr;

  if (!chaos_expect(p, TOK_LPAREN, "Expected: '('") )
    return nullptr;

  Chaos_AST *cond = parse_expression(p);
  if (!chaos_expect(p, TOK_RPAREN, "Expected: ')'") )
    return nullptr;

  Chaos_AST *body = p->match(TOK_LCURLY) ? parse_block(p) : parse_statement(p);

  Chaos_AST *node = chaos_make_ast(AST_WHILE);
  node->while_stmt.cond = cond;
  node->while_stmt.body = body;
  return node;
}

inline Chaos_AST *chaos_parse_struct(Chaos_Parser *p) {
  if (!p->match(TOK_STRUCT))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT) {
    std::fprintf(stderr, "Expected struct name\n");
    return nullptr;
  }

  std::string_view name = p->advance()->text;

  if (!chaos_expect(p, TOK_EQUAL, "Expected: '=' after struct name"))
    return nullptr;

  if (!chaos_expect(p, TOK_LCURLY, "Expected: '{' after struct name"))
    return nullptr;

  std::vector<std::pair<std::string_view, std::string_view>> fields;

  while (p->peek()->kind != TOK_RCURLY && p->peek()->kind != TOK_EOF) {
    if (p->peek()->kind != TOK_IDENT) {
      std::fprintf(stderr, "Expected field name\n");
      return nullptr;
    }
    std::string_view field_name = p->advance()->text;

    if (!chaos_expect(p, TOK_COLON, "Expected ':' after field name"))
      return nullptr;

    if (p->peek()->kind != TOK_IDENT) {
      std::fprintf(stderr, "Expected field type\n");
      return nullptr;
    }

    std::string_view field_type = p->advance()->text;
    fields.push_back({field_name, field_type});

    if (!p->match(TOK_COMMA))
      break;
  }

  chaos_expect(p, TOK_RCURLY, "Expected '}' after struct");

  Chaos_AST *node = chaos_make_ast(AST_STRUCT);
  node->struct_decl.name = name;
  node->struct_decl.fields = std::move(fields);
  return node;
}

inline Chaos_AST *chaos_parse_function(Chaos_Parser *p) {
  if (!p->match(TOK_FN))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT) {
    std::fprintf(stderr, "Expected function name\n");
    return nullptr;
  }

  Chaos_Token *name_tok = p->advance();
  std::string_view owner_name = p->current_module;
  std::string_view fn_name = name_tok->text;

  if (p->match(TOK_DOT)) {
    owner_name = name_tok->text;
    if (p->peek()->kind != TOK_IDENT) {
      std::fprintf(stderr, "Expected method name after '.'\n");
      return nullptr;
    }
    fn_name = p->advance()->text;
  }

  if (!chaos_expect(p, TOK_LPAREN, "Expected: '('") )
    return nullptr;

  std::vector<std::pair<std::string_view, std::string_view>> params;
  while (p->peek()->kind != TOK_RPAREN) {
    if (p->peek()->kind != TOK_IDENT) {
      std::fprintf(stderr, "Expected: parameter name\n");
      return nullptr;
    }

    std::string_view param_name = p->advance()->text;

    if (!chaos_expect(p, TOK_COLON, "Expected `:` after argument name"))
      return nullptr;

    if (p->peek()->kind != TOK_IDENT) {
      std::fprintf(stderr, "Expected type name after ':'\n");
      return nullptr;
    }

    std::string_view param_type = p->advance()->text;
    params.push_back({param_name, param_type});

    if (!p->match(TOK_COMMA))
      break;
  }

  if (!chaos_expect(p, TOK_RPAREN, "Expected to close function"))
    return nullptr;

  if (!chaos_expect(p, TOK_COLON,
                    "Expected `:` after function to indicate return type"))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT) {
    std::fprintf(stderr, "Expected return type from function\n");
    return nullptr;
  }

  std::string_view return_type = p->advance()->text;

  if (!chaos_expect(p, TOK_LCURLY, "Expected '{' to begin function body"))
    return nullptr;

  Chaos_AST *body = parse_block(p);

  Chaos_AST *node = chaos_make_ast(AST_FUNCTION);
  node->function.owner = owner_name;
  node->function.name = fn_name;
  node->function.params = std::move(params);
  node->function.return_type = return_type;
  node->function.body = body;
  return node;
}

inline Chaos_AST *chaos_parse_import(Chaos_Parser *p) {
  if (!p->match(TOK_IMPORT))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT && p->peek()->kind != TOK_STRING) {
    std::fprintf(stderr, "Expected module name or path string after import\n");
    return nullptr;
  }

  Chaos_AST *node = chaos_make_ast(AST_IMPORT);
  node->import_decl.name = p->advance()->text;

  chaos_expect(p, TOK_SEMI, "Expected ';' after import");
  return node;
}

inline Chaos_AST *chaos_parse_mod_decl(Chaos_Parser *p) {
  if (!p->match(TOK_MOD))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT) {
    std::fprintf(stderr, "Expected module name after mod\n");
    return nullptr;
  }

  Chaos_AST *node = chaos_make_ast(AST_MOD_DECL);
  node->mod_decl.name = p->advance()->text;
  p->current_module = node->mod_decl.name;

  chaos_expect(p, TOK_SEMI, "Expected ';' after mod");
  return node;
}

inline Chaos_AST *chaos_parse_enum(Chaos_Parser *p) {
  if (!p->match(TOK_ENUM))
    return nullptr;

  if (p->peek()->kind != TOK_IDENT) {
    std::fprintf(stderr, "Expected enum name\n");
    return nullptr;
  }

  std::string_view name = p->advance()->text;

  if (!chaos_expect(p, TOK_EQUAL, "Expected '=' after enum name"))
    return nullptr;

  if (!chaos_expect(p, TOK_LCURLY, "Expected '{' after enum name"))
    return nullptr;

  std::vector<std::string_view> values;

  while (p->peek()->kind != TOK_RCURLY && p->peek()->kind != TOK_EOF) {
    if (p->peek()->kind != TOK_IDENT) {
      std::fprintf(stderr, "Expected enum value\n");
      return nullptr;
    }

    values.push_back(p->advance()->text);

    if (!p->match(TOK_COMMA))
      break;
  }

  chaos_expect(p, TOK_RCURLY, "Expected '}' after enum");

  Chaos_AST *node = chaos_make_ast(AST_ENUM);
  node->enum_decl.name = name;
  node->enum_decl.items = std::move(values);
  return node;
}

inline Chaos_AST *chaos_parse_statement(Chaos_Parser *p) {
  switch (p->peek()->kind) {
  case TOK_IF:
    return parse_if(p);
  case TOK_WHILE:
    return parse_while(p);
  case TOK_FN:
    return parse_function(p);
  case TOK_RETURN:
    return parse_return(p);
  case TOK_VAR:
    return parse_var_decl(p);
  case TOK_STRUCT:
    return parse_struct(p);
  case TOK_ENUM:
    return parse_enum(p);
  case TOK_IMPORT:
    return parse_import(p);
  case TOK_MOD:
    return parse_mod_decl(p);
  default:
    break;
  }

  Chaos_AST *expr = parse_expression(p);
  if (!chaos_expect(p, TOK_SEMI, "Expected ';' after expression"))
    return nullptr;

  return expr;
}

inline Chaos_AST *chaos_parse_program(Chaos_Parser *p) {
  Chaos_AST *program = chaos_make_ast(AST_PROGRAM);
  while (p->peek()->kind != TOK_EOF)
    program->block.statements.push_back(parse_statement(p));
  return program;
}
